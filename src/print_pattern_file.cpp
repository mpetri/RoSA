#include <ctype.h>
#include "pattern_file.hpp"
#include "pattern_file_ng.hpp"

int main(int argc, char const* argv[])
{
    if (argc != 2) {
        std::cout << "USAGE: " << argv[0] << " <file>";
    }
    pattern_file_ng pf_ng(argv[1]);

    size_t i = 1;
    for (auto& P : pf_ng) {
        std::cout << "(" << i << ") m=" << P.size() << " P='";
        for (const auto& p : P) {
            if (isprint(p))
                std::cout << p;
            else
                std::cout << "?";
        }
        std::cout << "'" << std::endl;
        i++;
    }

    return 0;
}
