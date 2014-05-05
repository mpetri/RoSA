
#ifndef FACTOR_MATCHER_HPP
#define FACTOR_MATCHER_HPP

#include <algorithm>
#include <iterator>
#include <iostream>
#include <chrono>

#include <ctype.h>

using namespace std::chrono;

#include "sdsl/algorithms_for_suffix_array_construction.hpp"

#include "factor_store.hpp"

struct factor_matcher_helper {

    template <class t_itr>
    static void print_pattern_alignment(const t_itr& start, const t_itr& it,
                                        const std::vector<uint8_t>& P)
    {
        auto dist = std::distance(start, it);
        std::cout << " P = ";
        for (size_t i = 0; i < dist; i++) std::cout << "#";
        for (size_t i = 0; i < P.size(); i++) {
            if (isprint(P[i]))
                std::cout << P[i];
            else
                std::cout << "?";
        }
        std::cout << "'" << std::endl;
    }

    template <class t_itr>
    static bool match_decoded(t_itr it, const std::vector<uint8_t>& P)
    {
        size_t m = P.size();
        auto itr = it;
        // check first sym if it is decoded!
        if (itr.is_decoded() && P[0] != *itr) return false;

        itr.skip_to_decoded_pos();
        size_t cur_offset = std::distance(it, itr);
        while (cur_offset < m) {
            if (P[cur_offset] != *itr) return false;
            itr.skip_to_decoded_pos();
            cur_offset = std::distance(it, itr);
        }
        return true;
    }
    template <class t_itr>
    static bool match_brute(t_itr it, const std::vector<uint8_t>& P)
    {
        size_t m = P.size();
        auto itr = it;
        for (size_t i = 0; i < m; i++) {
            if (P[i] != *itr) return false;
            ++itr;
        }
        return true;
    }
    template <class t_itr>
    static bool match_rightfocus(t_itr it, const std::vector<uint8_t>& P)
    {
        size_t m = P.size();
        auto itr = it;
        auto factor_id_before_skip = itr.cur_factor();
        auto pos_before_skip = itr.cur_offset();
        itr.skip_to_rightmost_contained_factor(m);
        auto factor_id_after_skip = itr.cur_factor();
        auto pos_after_skip = itr.cur_offset();
        auto i = std::distance(it, itr);
        size_t matched = 0;
        while (itr != it) {
            /* decode left as far as we can */
            if (i < m) {
                if (P[i] != *itr) return false;
                matched++;
            }
            --itr;
            i--;
        }
        // process final sym from the left pass
        if (P[i] != *itr)
            return false;
        else
            matched++;
        if (matched == m) return true;

        /* now match the rest to the right if there is any */
        itr = it + (m - 1);
        i = m - 1;
        while (matched != m) {
            if (P[i] != *itr) return false;
            --itr;
            i--;
            matched++;
        }
        return true;
    }
    static sdsl::int_vector<32> calculate_sa(const std::vector<uint8_t>& P)
    {
        size_t m = P.size();
        sdsl::int_vector<32> sa(m, 0);
        divsufsort(P.data(), (int32_t*)sa.data(), m);
        return sa;
    }
    static void print_sa(const sdsl::int_vector<32>& sa,
                         const std::vector<uint8_t>& T)
    {
        for (size_t i = 0; i < sa.size(); i++) {
            std::cout << std::setw(3) << i << "  " << std::setw(3) << sa[i]
                      << "  ";
            for (size_t j = sa[i]; j < T.size(); j++) {
                if (isprint(T[j]))
                    std::cout << T[j];
                else
                    std::cout << "_";
            }
            std::cout << "$\n";
        }
    }
    static size_t search_sa(const sdsl::int_vector<32>& sa,
                            const std::vector<uint8_t>& T,
                            const std::vector<uint8_t>& P, size_t cur_alignment)
    {
        typedef sdsl::int_vector<>::value_type value_type;
        size_t sp = 0;
        size_t ep = sa.size() - 1;
        size_t pos = P.size() - 1;
        size_t matched = 0;

        // SA comparison function
        struct sa_cmp {
            const std::vector<uint8_t>& T;
            size_t offset;
            sa_cmp(const std::vector<uint8_t>& _T, size_t o) : T(_T), offset(o)
            {
            }
            bool operator()(const value_type& a, uint8_t sym)
            {
                if (offset + a >= T.size()) return true;
                if (T[offset + a] < sym) return true;
                return false;
            }
            bool operator()(uint8_t sym, const value_type& a)
            {
                if (offset + a >= T.size()) return false;
                if (sym < T[offset + a]) return true;
                return false;
            }
        };

        size_t prefix_len = 0;
        while (matched < P.size()) {
            auto range = std::equal_range(sa.begin() + sp, sa.begin() + ep + 1,
                                          P[pos], sa_cmp(T, matched));
            if (range.first != sa.begin() + ep + 1
                && std::distance(range.first, range.second) > 0) {
                matched++;
                sp = std::distance(sa.begin(), range.first);
                ep = std::distance(sa.begin(), range.second) - 1;

                // prefix check
                auto prefix
                    = std::find(range.first, range.second, sa.size() - matched);
                if (prefix != range.second) {
                    prefix_len = matched;
                }
            } else {
                break;
            }
            pos--;
        }

        /* calculate shift */
        size_t shift = cur_alignment + 1;
        if (matched == P.size()) {
            /* complete match */
            if ((ep + 1 - sp) > 1) {
                for (size_t i = sp; i <= ep; i++) {
                    size_t pos = T.size() - sa[i] - P.size();
                    if (pos < cur_alignment) {
                        shift = std::min(shift, cur_alignment - pos);
                    }
                }
            } else {
                size_t revpos_of_factor = T.size() - cur_alignment - 1
                                          - (P.size() - 1);
                bool aligned = true;
                for (size_t i = 0; i < P.size(); i++) {
                    if (T[revpos_of_factor + i] != P[P.size() - 1 - i])
                        aligned = false;
                }
                if (aligned) {
                    shift = cur_alignment + 1;
                } else {
                    size_t new_align_pos = T.size() - sa[sp] - P.size();
                    if (cur_alignment > new_align_pos) {
                        shift = cur_alignment - new_align_pos;
                    } else {
                        shift = cur_alignment + 1;
                    }
                }
            }
        } else {
            if (prefix_len) {
                shift = cur_alignment + (P.size() - prefix_len);
            } else {
                // no prefix match. move past complete factor
                shift = cur_alignment + P.size();
            }
        }
        return shift;
    }
};

struct factor_matcher_exhaustive {
    static std::string name() { return "EXH"; }
    template <class t_idx>
    static bool match(factor_store<t_idx>& T, const std::vector<uint8_t>& P)
    {
        auto start = T.begin();
        auto itr = start;
        while (itr.cur_factor() <= T.last_regular_factor()) {
            if (factor_matcher_helper::match_brute(itr, P)) {
                return true;
            }
            ++itr;
        }
        return false;
    }
};

struct factor_matcher_exhaustive_cd {
    static std::string name() { return "EXH-CD"; }
    template <class t_idx>
    static bool match(factor_store<t_idx>& T, const std::vector<uint8_t>& P)
    {
        auto start = T.begin();
        auto itr = start;
        while (itr.cur_factor() <= T.last_regular_factor()) {
            if (factor_matcher_helper::match_decoded(itr, P)) {
                if (factor_matcher_helper::match_brute(itr, P)) {
                    return true;
                }
            }
            ++itr;
        }
        return false;
    }
};

struct factor_matcher_kmp {
    static std::string name() { return "KMP"; }
    static sdsl::int_vector<>
    calculate_kmp_shift_table(const std::vector<uint8_t>& P)
    {
        size_t m = P.size();

        sdsl::int_vector<> kmp_table(m, 0, sdsl::bit_magic::l1BP(m) + 1);
        size_t i = 1, j = 0;
        while (i < m) {
            if (P[i] == P[j]) {
                kmp_table[i++] = ++j;
            } else {
                if (j > 0) {
                    j = kmp_table[j - 1];
                } else {
                    kmp_table[i] = 0;
                    ++i;
                }
            }
        }
        return std::move(kmp_table);
    }
    template <class t_idx>
    static bool match(factor_store<t_idx>& T, const std::vector<uint8_t>& P)
    {
        auto kmp_table = calculate_kmp_shift_table(P);
        size_t m = P.size();

        auto start = T.begin();
        auto itr = start;
        size_t matched = 0;
        while (itr.cur_factor() < T.num_factors()) {
            if (*itr == P[matched]) {
                if (matched == m - 1) return true;
                ++itr;
                ++matched;
            } else {
                if (matched > 0) {
                    matched = kmp_table[matched - 1];
                } else {
                    ++itr;
                }
            }
        }
        return false;
    }
};


struct factor_matcher_bmh {
    static std::string name() { return "BMH"; }
    static sdsl::int_vector<>
    calculate_bmh_shift_table(const std::vector<uint8_t>& P)
    {
        size_t m = P.size();
        sdsl::int_vector<> bmh_table(256, m, sdsl::bit_magic::l1BP(m) + 1);
        for (size_t i = 0; i < m - 1; i++) {
            bmh_table[P[i]] = m - i - 1;
        }
        return std::move(bmh_table);
    }
    template <class t_idx>
    static bool match(factor_store<t_idx>& T, const std::vector<uint8_t>& P)
    {
        auto bmh_table = calculate_bmh_shift_table(P);
        size_t m = P.size();
        auto start = T.begin();
        auto itr = start;
        while (itr.cur_factor() <= T.last_regular_factor()) {
            // T.print_state(true);
            // factor_matcher_helper::print_pattern_alignment(start,itr,P);

            size_t matched = 0;
            auto last = itr + (m - 1);
            auto it = last;
            while (*it == P[m - matched - 1]) {
                if (matched == m - 1) {
                    return true;
                }
                --it;
                ++matched;
            }
            itr += bmh_table[*last];
        }

        // T.decode_all();
        // T.print_state(true);

        return false;
    }
};

struct factor_matcher_bmh_cd {
    static std::string name() { return "BMH-CD"; }
    static sdsl::int_vector<>
    calculate_bmh_shift_table(const std::vector<uint8_t>& P)
    {
        size_t m = P.size();
        sdsl::int_vector<> bmh_table(256, m, sdsl::bit_magic::l1BP(m) + 1);
        for (size_t i = 0; i < m - 1; i++) {
            bmh_table[P[i]] = m - i - 1;
        }
        return std::move(bmh_table);
    }
    template <class t_idx>
    static bool match(factor_store<t_idx>& T, const std::vector<uint8_t>& P)
    {
        auto bmh_table = calculate_bmh_shift_table(P);
        size_t m = P.size();
        auto start = T.begin();
        auto itr = start;
        while (itr.cur_factor() <= T.last_regular_factor()) {
            auto last = itr + (m - 1);
            if (factor_matcher_helper::match_decoded(itr, P)) {
                size_t matched = 0;
                auto it = last;
                while (*it == P[m - matched - 1]) {
                    if (matched == m - 1) {
                        return true;
                    }
                    --it;
                    ++matched;
                }
            }
            itr += bmh_table[*last];
        }
        return false;
    }
};

struct factor_matcher_exhaustive_cdr {
    static std::string name() { return "EXH-CD-R"; }
    template <class t_idx>
    static bool match(factor_store<t_idx>& T, const std::vector<uint8_t>& P)
    {
        auto start = T.begin();
        auto itr = start;
        while (itr.cur_factor() <= T.last_regular_factor()) {
            if (factor_matcher_helper::match_decoded(itr, P)) {
                if (factor_matcher_helper::match_rightfocus(itr, P)) {
                    return true;
                }
            }
            ++itr;
        }
        return false;
    }
};

struct factor_matcher_sa_smart {
    static std::string name() { return "SA"; }
    template <class t_itr>
    static std::tuple<std::vector<uint8_t>, size_t, bool>
    determine_factor(t_itr it, const std::vector<uint8_t>& P)
    {
        std::vector<uint8_t> factor;
        auto itr = it + (P.size() - 1);
        auto factor_pos = 0;
        bool found = false;
        bool finished = false;
        bool matched = true;
        size_t j = P.size() - 1;
        while (itr != it) {
            if (itr.is_decoded()) {
                found = true;
                factor.insert(factor.begin(), *itr);
                factor_pos = std::distance(it, itr);
                if (*itr != P[j]) matched = false;
            } else {
                if (!factor.empty()) {
                    finished = true;
                    break;
                }
            }
            j--;
            --itr;
        }
        if (itr == it && !finished) { // does it extend into the first sym?
            factor.insert(factor.begin(), *it);
            if (*it != P[0]) matched = false;
            factor_pos = 0;
        }
        return std::make_tuple(factor, factor_pos, matched);
    }
    template <class t_itr>
    static t_itr find_new_alignment(t_itr it, sdsl::int_vector<32>& sa,
                                    const std::vector<uint8_t>& PRev,
                                    const std::vector<uint8_t>& P)
    {
        /* (1) find the rightmost factor to align with P */
        auto factor_data = determine_factor(it, P);
        auto factor = std::get<0>(factor_data);
        auto factor_pos = std::get<1>(factor_data);
        auto factor_matched = std::get<2>(factor_data);

        /* (2) calculate the new position */
        auto shift
            = factor_matcher_helper::search_sa(sa, PRev, factor, factor_pos);

        return it + shift;
    }
    template <class t_idx>
    static bool match(factor_store<t_idx>& T, const std::vector<uint8_t>& P)
    {
        auto PRev = P;
        std::reverse(std::begin(PRev), std::end(PRev));
        auto sa = factor_matcher_helper::calculate_sa(PRev);

        auto start = T.begin();
        auto itr = start;
        while (itr.cur_factor() <= T.last_regular_factor()) {
            if (factor_matcher_helper::match_decoded(itr, P)) {
                if (factor_matcher_helper::match_rightfocus(itr, P)) {
                    return true;
                }
            }
            itr = find_new_alignment(itr, sa, PRev, P);
        }

        return false;
    }
};


struct factor_matcher_mbmh_cd {
    static std::string name() { return "MBMH"; }
    static std::vector<uint64_t>
    calculate_mbmh_shift_matrix(const std::vector<uint8_t>& P)
    {
        auto m = P.size();
        std::vector<uint64_t> mbmh_matrix(256 * m, 0);
        std::vector<uint64_t> last(256, m - 1);
        for (int64_t i = m - 2; i >= 0; i--) {
            size_t shift = last[P[i]] - i;
            for (size_t j = last[P[i]]; j > 0; j--) {
                mbmh_matrix[j * 256 + (uint64_t)P[i]] = shift;
                shift--;
                if (shift == 0) break;
            }
            last[P[i]] = i;
        }
        return mbmh_matrix;
    }
    static void print_matrix(const std::vector<uint64_t>& M,
                             const std::vector<uint8_t>& P)
    {
        size_t m = P.size();
        std::vector<uint64_t> exists(256, 0);
        for (const auto& sym : P) exists[(int)sym] = 1;

        std::cout << "=============================================\n";
        for (size_t i = 0; i < exists.size(); i++) {
            if (exists[i] != 0) {
                if (isprint((int)i))
                    std::cout << (char)i << " ";
                else
                    std::cout << "? ";
            }
        }
        std::cout << std::endl;
        for (size_t i = 0; i < m; i++) {
            for (size_t j = 0; j < exists.size(); j++) {
                if (exists[j] != 0) {
                    if (M[i * 256 + j] != 0)
                        std::cout << M[i * 256 + j] << " ";
                    else
                        std::cout << "  ";
                }
            }
            std::cout << std::endl;
        }
    }

    template <class t_itr>
    static t_itr find_new_alignment(t_itr it, const std::vector<uint64_t>& M,
                                    const std::vector<uint8_t>& P)
    {
        auto itr = it;
        size_t max_shift = 1;
        size_t m = P.size();
        itr.skip_to_decoded_pos();
        size_t cur_offset = std::distance(it, itr);
        while (cur_offset < m) {
            size_t shift = M[cur_offset * 256 + (int)*itr];
            if (shift == 0) {
                shift = cur_offset + 1;
            }
            max_shift = std::max(max_shift, shift);
            itr.skip_to_decoded_pos();
            cur_offset = std::distance(it, itr);
        }
        return it + max_shift;
    }

    template <class t_idx>
    static bool match(factor_store<t_idx>& T, const std::vector<uint8_t>& P)
    {
        auto mbmh_matrix = calculate_mbmh_shift_matrix(P);
        size_t m = P.size();
        auto start = T.begin();
        auto itr = start;
        while (itr.cur_factor() <= T.last_regular_factor()) {
            if (factor_matcher_helper::match_decoded(itr, P)) {
                if (factor_matcher_helper::match_rightfocus(itr, P)) {
                    return true;
                }
            }
            itr = find_new_alignment(itr, mbmh_matrix, P);
        }
        return false;
    }
};


#endif
