#ifndef ROSA_PATTERN2_FILE
#define ROSA_PATTERN2_FILE


#include <iostream>
#include <fstream>
#include <vector>

using namespace sdsl;
using namespace std;


class pattern_file_ng {
public:
    typedef bit_vector::size_type size_type;
    std::vector<std::string> m_patterns;
public:
	pattern_file_ng() = default;
    pattern_file_ng(const char* pattern_file_name) {
    	ifstream ifs(pattern_file_name,std::ifstream::binary);
        if(!ifs.is_open()) {
            std::cerr << "pattern file " << pattern_file_name << " can not be opened." << std::endl;
            exit(EXIT_FAILURE);
        }
    	// read number of patterns
    	uint64_t num_patterns;
    	uint64_t len;
    	ifs.read(reinterpret_cast<char*>(&num_patterns),sizeof(num_patterns));
    	while(num_patterns--) {
    		ifs.read(reinterpret_cast<char*>(&len),sizeof(len));
    		std::string str;
    		str.resize(len, ' ');
    		char* begin = &*str.begin();
    		ifs.read(begin, len);
    		m_patterns.push_back(str);
    	}
    }

    void add_pattern(std::string P) {
    	m_patterns.push_back(P);
    }

    size_type serialize(std::ostream& out, structure_tree_node* v = NULL,std::string name = "") const {
    	size_type written_bytes = 0;
    	uint64_t num_patterns = m_patterns.size();
    	out.write(reinterpret_cast<char*>(&num_patterns),sizeof(num_patterns));
    	written_bytes += sizeof(num_patterns);
    	for(size_t i=0;i<m_patterns.size();i++) {
    		auto& pat = m_patterns[i];
    		uint64_t plen = pat.size();
    		out.write(reinterpret_cast<char*>(&plen),sizeof(plen));
    		const char* begin = &*pat.begin();
    		out.write(begin, plen);
    		written_bytes += sizeof(plen)+plen;
    	}
    	return written_bytes;
    }

    size_type size() {
    	return m_patterns.size();
    }

    auto begin() -> decltype(m_patterns.begin()) {
    	return m_patterns.begin();
    }

    auto end() -> decltype(m_patterns.end()) {
    	return m_patterns.end();
    }

};


#endif
