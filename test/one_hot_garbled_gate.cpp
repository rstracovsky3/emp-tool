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

	std::size_t max_n = 6;

	// sender
	block data[max_n], delta, table[2*(max_n - 1) + 1], w0, w1;
	block *w0p, *w1p;
	MITCCRH<8> mi_gen;
	PRG prg;
	prg.random_block(&delta, 1);
	delta = delta | makeBlock(0x0, 0x1);
	mi_gen.setS(delta);

	// receiver
	block data1[2];
	MITCCRH<8> mi_eva;
	mi_eva.setS(delta);
	block ret;

	cout << "Correctness ... ";
	for(int n = 3; n < max_n; ++n) {
		for(int a = 0; a < (1 << n); ++a) {

			for(int i = 0; i < 1; ++i) {
				printf("========== TEST n=%u, a=%u, rep=%u ==========\n\n", n, a, i);

				prg.random_block(data, n);
				
				w0p = one_hot_garble(n, data, a, delta, table, &mi_gen);
				printf("GRBL: ");
    			printtf(w0p, (1 << n));
				printf("TABL: ");
    			printtf(table, 2*(n-1)+1);


				for (int j = 0; j < n; ++j) {
					if ((a >> j) & 1 == 1) {
						data[j] = data[j] ^ delta;
					}
				}

				w1p = one_hot_eval(n, data, a, table, &mi_gen);
				printf("EVAL: ");
				printtf(w1p, (1 << n));
				

				if(cmpBlock(w0p, w1p, (1 << n)) == false) {cout << "wrong" << endl; abort();}
				

				// w0 = halfgates_garble(data[0], data[0]^delta, data[1], data[1]^delta, delta, table, &mi_gen); // OUTPUTS LABEL for 0
				// w1 = w0 ^ delta;

				// if(ii == 1) data1[0] = data[0] ^ delta; else data1[0] = data[0];
				// if(jj == 1) data1[1] = data[1] ^ delta; else data1[1] = data[1];
				// ret = halfgates_eval(data1[0], data1[1], table, &mi_eva);

				// block ret1 = w0;
				// if(ii == 1 && jj == 1) ret1 = w1;
				// if(cmpBlock(&ret, &ret1, 1) == false) {cout << "wrong" << endl; abort();}
			}
		}
	}
	cout << "check\n";

	cout << "Efficiency: ";
	auto start = clock_start();
	for(int i = 0; i < 1024*1024*2; ++i) {
		prg.random_block(data, 2);
		w0 = halfgates_garble(data[0], data[0]^delta, data[1], data[1]^delta, delta, table, &mi_gen);
		w1 = w0 ^ delta;

		data1[0] = data[0] ^ delta;
		data1[1] = data[1] ^ delta;
		ret = halfgates_eval(data1[0], data1[1], table, &mi_eva);
	}
	cout << 1024*1024*128/(time_from(start))*1e6 << " gates/second" << endl;

	return 0;
}
