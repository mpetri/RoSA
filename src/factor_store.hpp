
#ifndef FACTOR_STORE_HPP
#define FACTOR_STORE_HPP

#include <vector>
#include <cstdint>
#include <iterator>
#include <algorithm>
#include "sdsl/int_vector.hpp"
#include <chrono>

struct matchlz_stats {
    std::string method;
    std::chrono::microseconds match_time;
    size_t len_total;
    size_t len_accessed;
    size_t decoded;
    size_t num_factors;
    size_t factor_ids_decoded;
    size_t factor_density;
    std::vector<uint64_t> factor_lens;
};


struct factor_state {
	uint64_t bwd_id = 0;
	uint64_t len = 0;
	uint64_t cumlen = 0;
	uint64_t num_decoded = 0;
	uint64_t cur_lb;
	std::vector<uint8_t> content;
};

template<class t_idx>
class factor_store;

template<class t_idx>
class factor_iterator : public std::iterator<std::random_access_iterator_tag, 
									 typename factor_store<t_idx>::value_type, 
									 typename factor_store<t_idx>::difference_type>
{
public: /* types */
	typedef factor_store<t_idx> store_type;
	typedef typename factor_store<t_idx>::size_type size_type;
	typedef typename factor_store<t_idx>::difference_type difference_type;
	typedef typename factor_store<t_idx>::value_type value_type;
public: /* default stuff */
    factor_iterator(const factor_iterator& fi) = default;
    factor_iterator(factor_iterator&& fi) = default;
    factor_iterator& operator=(const factor_iterator& fi) = default;
    factor_iterator& operator=(factor_iterator&& fi) = default;
private:
	store_type* m_store_ptr;
    size_type m_cur_pos = 0;
    size_type m_cur_factor_id = 0;
public:
	factor_iterator(store_type& s,size_t pos) 
		: m_store_ptr(&s), m_cur_pos(pos)
	{
		m_cur_factor_id = s.pos_to_factor(m_cur_pos,0);
	}
    factor_iterator& operator++() {
        m_cur_pos++;
        const auto& factor = m_store_ptr->factor(m_cur_factor_id);
        if( m_cur_pos == factor.cumlen + factor.len) {
        	m_cur_factor_id++;
        }
	    return (*this);
    }
    factor_iterator& operator--() {
        m_cur_pos--;
        const auto& factor = m_store_ptr->factor(m_cur_factor_id);
        if( m_cur_pos < factor.cumlen) {
        	m_cur_factor_id--;
        }
	    return (*this);
    }
    bool operator == (const factor_iterator& b) const {
    	return ((*this).m_cur_pos == b.m_cur_pos) && ((*this).m_store_ptr == b.m_store_ptr);
    }
    bool operator != (const factor_iterator& b) const {
    	return !(*this==b);
    }
    factor_iterator& operator+=(difference_type i) {
        m_cur_pos += i;
        m_cur_factor_id = m_store_ptr->pos_to_factor(m_cur_pos,m_cur_factor_id);
        return *this;
    }
    factor_iterator operator+(difference_type i) const {
        factor_iterator it = *this;
        return it += i;
    }
    difference_type operator-(const factor_iterator& b) const {
        factor_iterator it = *this;
        return (difference_type)m_cur_pos - (difference_type)b.m_cur_pos;
    }
    bool is_decoded() const {
    	return m_store_ptr->is_decoded(m_cur_pos,m_cur_factor_id);
    }
    size_type cur_factor() const {
    	return m_cur_factor_id;
    }
    size_type cur_offset() const {
    	return m_cur_pos;
    }
    value_type operator*() const {
        return m_store_ptr->access(m_cur_pos,m_cur_factor_id);
    }
    void skip_to_rightmost_contained_factor(size_type m) {
    	auto new_factor_id = m_cur_factor_id;
    	auto f = m_store_ptr->factor(new_factor_id);
    	size_t skipped_factors = 0;
    	while( f.cumlen + f.len < (m_cur_pos+m-1) ) {
    		new_factor_id++;
    		f = m_store_ptr->factor(new_factor_id);
    		skipped_factors++;
    	}
    	if(skipped_factors && f.cumlen + f.len > (m_cur_pos+m) ) {
    		new_factor_id--;
    		f = m_store_ptr->factor(new_factor_id);
    	}
    	m_cur_factor_id = new_factor_id;
    	m_cur_pos = f.cumlen + f.len - 1; // we start at the end of that factor!
    }

    void skip_to_decoded_pos() {
    	auto next_decoded = m_store_ptr->next_decoded_pos(m_cur_pos,m_cur_factor_id);
    	m_cur_pos = std::get<0>(next_decoded);
    	m_cur_factor_id = std::get<1>(next_decoded);
    }
};

template<class t_idx>
class factor_store {
public:
	typedef sdsl::bit_vector::size_type size_type;
	typedef sdsl::bit_vector::difference_type difference_type;
	typedef uint8_t value_type;
	typedef factor_iterator<t_idx> iterator_type;
	friend class factor_iterator<t_idx>;
private:
	std::vector<factor_state> m_factors;
	const t_idx& m_idx;
private:
	size_type decode_factor_id(size_type i) {
		if(m_factors[i].len == 0) {
			auto factor_data = m_idx.decode_factor_id(m_factors[i].bwd_id);
			m_factors[i].len = std::get<0>(factor_data);
			m_factors[i].content.resize(m_factors[i].len);
			m_factors[i].content.back() = std::get<1>(factor_data);
			m_factors[i].cur_lb = std::get<2>(factor_data);
			m_factors[i].num_decoded = 1;
			if(i == 0) {
				m_factors[i].cumlen = 0;
			} else {
				m_factors[i].cumlen = m_factors[i-1].cumlen + m_factors[i-1].len;
			}
		}
		return m_factors[i].len;
	}

	size_type pos_to_factor(size_type pos,size_type factor_id) {
		while( factor_cumlen(factor_id) <= pos ) factor_id++;
		return factor_id-1;
	}

	bool length_decoded(size_type factor_id) {
		return m_factors[factor_id].len != 0;
	}

	size_type factor_length(size_type factor_id) {
		if(!length_decoded(factor_id)) {
			return decode_factor_id(factor_id);
		}
		return m_factors[factor_id].len;
	}

	size_type factor_cumlen(size_type factor_id) {
		if(!length_decoded(factor_id)) {
			decode_factor_id(factor_id);
		}
		return m_factors[factor_id].cumlen;
	}

	bool is_decoded(size_type pos,size_type factor_id) {
		if(!length_decoded(factor_id)) return false; 

		size_t factor_offset = m_factors[factor_id].cumlen;
		size_t in_factor_offset = pos-factor_offset;
		size_t rev_offset = m_factors[factor_id].len - in_factor_offset;
		return rev_offset <= m_factors[factor_id].num_decoded;
	}

	void decode_syms(size_type pos,size_type factor_id) {
		auto& f = factor(factor_id);
		size_t factor_offset =  f.cumlen;
		size_t in_factor_offset = pos-factor_offset;
		size_t rev_offset =  f.len - in_factor_offset;
		size_t items_to_decode = rev_offset -  f.num_decoded;
		f.cur_lb = m_idx.decode_syms( f.cur_lb,items_to_decode,f.content.rbegin()+ f.num_decoded);
		f.num_decoded += items_to_decode;
	}

	std::pair<size_type,size_type> 
	next_decoded_pos(size_type pos,size_type factor_id) const {
		size_t factor_offset = m_factors[factor_id].cumlen;
		size_t in_factor_offset = pos-factor_offset;
		size_t decoded_offset = m_factors[factor_id].len - m_factors[factor_id].num_decoded;
		if(in_factor_offset < decoded_offset) {
			// skip to decoded in block
			return {factor_offset+decoded_offset,factor_id};
		}
		if(in_factor_offset != m_factors[factor_id].len-1 && in_factor_offset >= decoded_offset ) {
			// already in decoded section and there is more.
			return {pos+1,factor_id};
		}
		// we have to go to the next factor if it is decoded
		if(factor_id+1 < m_factors.size() && m_factors[factor_id+1].len ) {
			factor_offset = m_factors[factor_id+1].cumlen;
			decoded_offset = m_factors[factor_id+1].len - m_factors[factor_id+1].num_decoded;
			return {factor_offset+decoded_offset,factor_id+1};
		}
		// we can't find more decoded things
		return {std::numeric_limits<int64_t>::max(),m_factors.size()-1};
	}

	value_type access(size_type pos,size_type factor_id) {
		if(!is_decoded(pos,factor_id)) {
			decode_syms(pos,factor_id);
		}
		size_t factor_offset = m_factors[factor_id].cumlen;
		return m_factors[factor_id].content[pos-factor_offset];
	}

	factor_state& factor(size_type id) {
		if(!length_decoded(id)) {
			decode_factor_id(id);
		}
		return m_factors[id];
	}
public:
	factor_store() = delete;
	factor_store(const factor_store&) = delete;
	factor_store(factor_store&&) = delete;
	factor_store(const t_idx& i,size_type lz_offset,size_type m) : m_idx(i) 
	{
		/* read the factor ids */
		sdsl::int_vector<> fid_buf;
		m_idx.read_factor_ids(fid_buf,lz_offset,m);
		m_factors.resize(fid_buf.size());
		for(size_type i=0;i<fid_buf.size();i++) { 
			m_factors[i].bwd_id = fid_buf[i];
		}

		/* decode the length till we have at least m syms */
		size_type decoded_len = 0; size_type j=0;
		while(decoded_len < m) {
			decoded_len += decode_factor_id(j++);
		}

		// for(size_type i=0;i<fid_buf.size();i++) { 
		// 	print_factor(i);
		// }
	}

	void print_factor(size_t id) {
		std::cout << "<";
		std::cout << "id = " << id << ",";
		std::cout << "bwd_id = " << m_factors[id].bwd_id << ",";
		std::cout << "len = " << m_factors[id].len << ",";
		std::cout << "cumlen = " << m_factors[id].cumlen << ",";
		std::cout << "num_decoded = " << m_factors[id].num_decoded << ",";
		std::cout << "cur_lb = " << m_factors[id].cur_lb;
		if(m_factors[id].num_decoded) {
			std::cout << ",syms = '";
			for(size_t i=0;i<m_factors[id].len-m_factors[id].num_decoded;i++) {
				std::cout << "_";
			}
			for(size_t i=m_factors[id].len-m_factors[id].num_decoded;i<m_factors[id].len;i++) {
				if( isprint(m_factors[id].content[i])) std::cout << m_factors[id].content[i];
				else std::cout << "?";
			}
			std::cout << "'";
		}
		std::cout << ">\n";
	}

	void print_state(bool only_text = false) {
		if(!only_text) {
			for(size_t i=0;i<m_factors.size();i++) {
				print_factor(i);
			}
		}
		// print text
		std::cout << " T = ";
		for(size_t id=0;id<m_factors.size();id++) {
			for(size_t i=0;i<m_factors[id].len-m_factors[id].num_decoded;i++) {
				std::cout << "_";
			}
			for(size_t i=m_factors[id].len-m_factors[id].num_decoded;i<m_factors[id].len;i++) {
				if( isprint(m_factors[id].content[i])) std::cout << m_factors[id].content[i];
				else std::cout << "?";
			}
		}
		std::cout << std::endl;
	}
	
	iterator_type begin() {
		return iterator_type(*this,0);
	}

	size_type last_regular_factor() {
		return m_idx.fac_dens+1;
	}

	size_type cum_decoded_len() const {
		return std::accumulate(m_factors.begin(),m_factors.end(),0,[](size_t sum,const factor_state& f) { return sum+=f.len; });
	}

	size_type num_decoded() const {
		return std::accumulate(m_factors.begin(),m_factors.end(),0,[](size_t sum,const factor_state& f) { return sum+=f.num_decoded; });
	}

    size_type num_factors() const {
        return m_factors.size();
    }

	size_type calculate_total_len() const {
		size_type sum = 0;
		for(const auto& f : m_factors) {
			if(f.len) {
				sum += f.len;
			} else {
				auto res = m_idx.decode_factor_id(f.bwd_id);
				sum += std::get<0>(res);
			}
		}
		return sum;
	}

    void decode_all() {
        for(size_t i=0;i<m_factors.size();i++) {
            if(m_factors[i].len==0) {
			    decode_factor_id(i);
            }
            for(size_t j=0;j<m_factors[i].len;j++) {
                access(m_factors[i].cumlen+j,i);
            }
        }
    }

	size_type max_factor_len() const {
		size_type max_len = 0;
		for(const auto& f : m_factors) {
			if(f.len) {
				max_len = std::max(max_len,f.len);
			} else {
				auto res = m_idx.decode_factor_id(f.bwd_id);
				size_type len = std::get<0>(res);
				max_len = std::max(max_len,len);
			}
		}
		return max_len;
	}

	size_type min_factor_len() const {
		size_type min_len = std::numeric_limits<size_type>::max();
		for(const auto& f : m_factors) {
			if(f.len) {
				min_len = std::min(min_len,f.len);
			} else {
				auto res = m_idx.decode_factor_id(f.bwd_id);
				size_type len = std::get<0>(res);
				min_len = std::min(min_len,len);
			}
		}
		return min_len;
	}

	void match_stats(matchlz_stats& s) const {
        s.factor_ids_decoded = 0;
        for(size_t i=0;i<m_factors.size();i++) {
            if(m_factors[i].len != 0) {
                s.factor_lens.push_back(m_factors[i].len);
                s.factor_ids_decoded++;
            }
        }
		s.decoded = num_decoded();
		s.factor_density = m_idx.fac_dens;
		s.num_factors = m_factors.size();
		s.len_total = calculate_total_len();
		s.len_accessed = cum_decoded_len();
	}
};

#endif
