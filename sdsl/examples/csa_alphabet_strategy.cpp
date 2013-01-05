/* This example shows how the representation of the alphabet dependent
 * part of a CST can be altered by using policy classes. 
 *
 * Author: Simon Gog
 */

#include <sdsl/suffixarrays.hpp>
#include <iostream>
#include <string>

using namespace sdsl;
using namespace std;

template<class Csa>
void csa_info(Csa &csa, const char* file, bool json){
	cout << "file: " << file << endl;
	construct(csa, file, 1);
	cout << "csa of type " << util::demangle(typeid(csa).name()) << endl;
	cout << "size in bytes : " << util::get_size_in_bytes(csa) << endl;
	if ( json ){
		cout << "json output: " << endl;
		util::write_structure<JSON_FORMAT>( csa, cout );
		cout << endl;
	}
}

int main(int argc, char* argv[]){
	if ( argc < 2 ){
		cout << "Usage: " << argv[0] << " file [json]" << endl;
		return 1;
	}
	bool json = false;
	if ( argc > 2 ){
		json = true;
	}
	csa_sada<enc_vector<>, 32, 32, sa_order_sa_sampling<>, int_vector<>, byte_alphabet_strategy> csa1;
	csa_sada<enc_vector<>, 32, 32, sa_order_sa_sampling<>, int_vector<>, succinct_byte_alphabet_strategy<> > csa2;
	csa_wt<wt_huff<>, 32, 32, sa_order_sa_sampling<>, int_vector<>, byte_alphabet_strategy> csa3;
	csa_wt<wt_huff<>, 32, 32, sa_order_sa_sampling<>, int_vector<>, succinct_byte_alphabet_strategy<> > csa4;
	csa_info(csa1, argv[1], json);	
	csa_info(csa2, argv[1], json);	
	csa_info(csa3, argv[1], json);	
	csa_info(csa4, argv[1], json);	
}
