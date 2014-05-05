
#include "pattern_file.hpp"
#include "pattern_file_ng.hpp"

int main(int argc, char const* argv[])
{
    if (argc < 3) {
        std::cout << "USAGE: " << argv[0]
                  << " <outfile> <infile1> <infile2> .... <infilen>";
    }
    pattern_file_ng pf_ng;
    std::string out_file = argv[1];

    // read and add
    for (size_t i = 2; i < argc; i++) {
        pattern_file pf(argv[i]);
        pf.reset();
        size_t len = pf.pattern_len;
        for (size_t j = 0; j < pf.pattern_cnt; j++) {
            const char* pat = (const char*)pf.get_next_pattern();
            pf_ng.add_pattern(std::string(pat, len));
        }
    }

    // output
    std::ofstream ofs(out_file);
    pf_ng.serialize(ofs);

    return 0;
}
