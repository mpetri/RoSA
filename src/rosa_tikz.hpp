#ifndef ROSA_TIKZ
#define ROSA_TIKZ

#include <vector>
#include <utility> // for pair
#include <ostream>
#include <string>
#include <sdsl/int_vector.hpp>
#include <sdsl/tikz.hpp>
#include "rosa_helping_structures.hpp"
#include "rosa_helping_functions.hpp"

using namespace std;
using namespace sdsl;

typedef pair<bit_vector::size_type, bit_vector::size_type> tPII;
typedef bit_vector::size_type size_type;


//! Classify a block given as a node in the suffix tree
/* \param cst 	Compressed suffix tree (cst)
 * \param block A node in the cst that corresponds to a block
 */
template <class tCst>
std::string classify_block(const tCst& cst, typename tCst::node_type block)
{
    unsigned char c1;
    size_type lb = cst.lb(block), rb = cst.rb(block) + 1;
    if (rb - lb == 1) {
        return "singleton";
    }
    size_type rank1 = cst.csa.wavelet_tree.inverse_select(lb, c1);
    size_type rank2 = cst.csa.rank_bwt(rb, c1);
    if (rank2 - rank1 == rb - lb) {
        return "reducible";
    } else {
        return "irreducible";
    }
}

template <class tCsa>
vector<string> extract_sorted_suffixes_for_latex(const tCsa& csa)
{
    vector<string> text(csa.size());
    for (size_type i = 0; i < csa.size(); ++i) {
        string tmp = algorithm::extract(csa, csa[i], csa.size() - 1);
        for (size_t j = 0; j < tmp.size(); ++j)
            text[i] += util::to_latex_string((unsigned char)tmp[j]);
    }
    return text;
}


template <class tCsa, class tBitVector>
void write_tikz_output_bwd(ostream& out, const tCsa& csa, const tBitVector& bf,
                           const tBitVector& bl, const bit_vector& bm,
                           size_type b,
                           const vector<bwd_block_info>& fwd_blocks_in_bwd,
                           const int_vector<>& min_depth)
{
    typedef bit_vector::size_type size_type;
    begin_tikzpicture(out, "font=\\tt, inner sep=0.1mm, remember picture");
    write_y_column(out, csa.size() + 1);
    int_vector<> v(csa.size());
    util::set_to_id(v);
    write_tikz_column_from_container(out, v, "i");
    write_tikz_column_from_container(out, csa.bwt, "bwt");
    write_tikz_column_from_container(out, bl, "bwdbl");
    write_tikz_column_from_container(out, bf, "bwdbf");
    vector<string> text = extract_sorted_suffixes_for_latex(csa);
    write_tikz_column_from_container(out, text, "text");
    end_tikzpicture(out);
    write_tikz_array(out, fwd_blocks_in_bwd, "fwdBlocksInBwd");
    write_tikz_array(out, bl, "bwdBlArray");
    write_tikz_array(out, csa.bwt, "bwdBWTArray", true);
    write_tikz_array(out, bm, "bmArray");
    write_tikz_array(out, min_depth, "minDepthArray");
}

template <class tCst>
void write_tikz_output_fwd(ostream& out, const tCst& cst, const bit_vector& bf,
                           size_type b, const vector<block_node>& v_block,
                           const vector<vector<header_item>>& external_blocks)
{
    typedef typename tCst::node_type node_type;
    typedef bit_vector::size_type size_type;
    begin_tikzpicture(out, "font=\\tt, inner sep=0.1mm, remember picture");

    write_y_column(out, cst.csa.size() + 1);

    int_vector<> v(cst.csa.size());
    util::set_to_id(v);
    write_tikz_column_from_container(out, v, "i");
    write_tikz_column_from_container(out, cst.csa, "sa");
    write_tikz_column_from_container(out, cst.lcp, "lcp");
    write_tikz_column_from_container(out, cst.csa.bwt, "bwt");
    write_tikz_column_from_container(out, bf, "fwdbf");
    vector<string> text = extract_sorted_suffixes_for_latex(cst.csa);
    write_tikz_column_from_container(out, text, "text");
    end_tikzpicture(out);

    size_type block_id = 0;
    rank_support_v<> bf_rank(&bf);
    vector<tPII> edges;
    for (typename tCst::const_iterator it = cst.begin(), end = cst.end();
         it != end; ++it) {
        if (it.visit() == 1) {
            node_type v = *it;
            node_type p = cst.parent(v);
            if (cst.leaves_in_the_subtree(v) <= b) {
                size_type lb = cst.lb(v), rb = cst.rb(v);
                it.skip_subtree();
                size_type block_depth = cst.depth(p) + 1;
                string block_type = classify_block(cst, v);
                out << "\\markintervalleft{" << lb << "}{" << rb << "}{"
                    << block_depth << "}{text}{black,st_interval}";
                out << "\\markintervalright{" << lb << "}{" << rb << "}{" << 2
                    << "}{i}{";
                out << "st_" + block_type << "}" << endl;
                out << "\\intervalanchor{" << lb << "}{" << rb << "}{i}{block"
                    << block_id << "}" << endl;
                if (block_type == "reducible") {
                    size_type dest_block_id = bf_rank(cst.csa.psi(lb) + 1) - 1;
                    edges.push_back(tPII(block_id, dest_block_id));
                } else if (block_type == "singleton") {
                    edges.push_back(tPII(block_id, block_id));
                }
                ++block_id;
            }
        }
    }
    // replace above thing with scan of external index


    for (size_t i = 0; i < edges.size(); ++i) {
        out << "\\intervaledge{" << edges[i].first << "}{" << edges[i].second
            << "}{draw=gray}" << endl;
    }
    for (size_t i = 0; i < v_block.size(); ++i) {
        if (v_block[i].delta_d > 0) {
            out << "\\intervaledgereducible{" << i << "}{"
                << v_block[i].dest_block << "}{" << v_block[i].delta_x << "}"
                << endl;
        }
        out << "\\blockinfo{" << i << "}{" << v_block[i].delta_x << "}{"
            << v_block[i].delta_d << "}{" << v_block[i].dest_block << "}{"
            << v_block[i].bwd_id << "}" << endl;
    }

    for (size_t i = 0; i < external_blocks.size(); ++i) {
        if (external_blocks[i].size() > 0) {
            out << "\\externalblockheader{" << i << "}{";
            write_tikz_array(out, external_blocks[i]);
            out << "}%\n";
        }
    }
}


#endif
