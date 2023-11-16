#include "emp-tool/emp-tool.h"
#include <iostream>
using namespace std;
using namespace emp;

// void printt(block a) {
// 	//uint64_t i0 = _mm_extract_epi64(a, 0);
// 	//uint64_t i1 = _mm_extract_epi64(a, 1);
// 	//printf("%X %X\n", i0, i1);
// 	unsigned char *c = (unsigned char*)(&a);
// 	for(int i = 0; i < 16; ++i) printf("%x ", c[i]);
// 	printf("\n");
// }

int main(void) {

	std::size_t max_n = 9;

	// sender
	block data[max_n], delta, table[2*(max_n - 1) + 1];
	block *w0, *w1;
	MITCCRH<8> mi_gen;
	PRG prg;
	prg.random_block(&delta, 1);
	delta = delta | makeBlock(0x0, 0x1);
	mi_gen.setS(delta);

	// receiver
	block data1[max_n];
	MITCCRH<8> mi_eva;
	mi_eva.setS(delta);

	cout << "Correctness ... ";
	for(int n = 3; n < max_n; ++n) {
		for(int a = 0; a < (1 << n); ++a) {
			for(int i = 0; i < 8; ++i) {
				prg.random_block(data, n);
				
				w0 = one_hot_garble(n, data, a, delta, table, &mi_gen);

				for (int j = 0; j < n; ++j) {
					if (((a >> j) & 1) == 1) {
						data1[j] = data[j] ^ delta;
					}
					else {
						data1[j] = data[j];
					}
				}

				w1 = one_hot_eval(n, data1, a, table, &mi_gen);

				if(cmpBlock(w0, w1, (1 << n)) == false) {cout << "wrong" << endl; abort();}

				delete[] w0;
				delete[] w1;
			}
		}
	}
	cout << "check\n";

	for(int n = 3; n < max_n; ++n) {
		cout << "Efficiency (n=" << n << "): ";
		auto start = clock_start();
		int a = 0;
		for(int i = 0; i < 1024*1024*2; ++i) {
			prg.random_block(data, n);
			w0 = one_hot_garble(n, data, a, delta, table, &mi_gen);
			for (int j = 0; j < n; ++j) {
				if (((a >> j) & 1) == 1) {
					data1[j] = data[j] ^ delta;
				}
				else {
					data1[j] = data[j];
				}
			}
			w1 = one_hot_eval(n, data1, a, table, &mi_gen);
			delete[] w0;
			delete[] w1;
		}
		cout << 1024*1024*128/(time_from(start))*1e6 << " gates/second" << endl;
	}

	return 0;
}
