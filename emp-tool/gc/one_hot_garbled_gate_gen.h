#ifndef EMP_ONE_HOT_GARBLED_GATE_GEN_H__
#define EMP_ONE_HOT_GARBLED_GATE_GEN_H__
#include "emp-tool/utils/utils.h"
#include "emp-tool/utils/mitccrh.h"
#include "emp-tool/execution/circuit_execution.h"
#include <iostream>
namespace emp {

/*
 * The one-hot garbling scheme,
 * [REF] Implementation of "One Hot Garbling"
 * https://eprint.iacr.org/2022/798.pdf
 */

// table will be of length 2n - 1 that are the ciphertexts G sends to E
inline block *one_hot_garble(std::size_t n, const block *A, std::size_t a, block delta, block *table, MITCCRH<8> *mitccrh) {

    assert(getLSB(delta) == 1);

    bool pa;
    block *seed_buffer = new block[1 << n]();

    PRG prg;

    // base case
    pa = getLSB(A[n - 1]);
    if (pa == 0) {
        seed_buffer[0] = A[n - 1] ^ delta; // should we be hashing here?
        seed_buffer[1] = A[n - 1];  // should we be hashing here?
    }
    else {
        seed_buffer[0] = A[n - 1];  // should we be hashing here?
        seed_buffer[1] = A[n - 1] ^ delta;  // should we be hashing here?
    }

    printf("BUFF 0: ");
    printtf(seed_buffer, (1 << n));

    block prg_buffer[2];
    block mitccrh_buffer[1];

    block even = makeBlock(0, 0); // I don't like this change sometime?
    block odd = makeBlock(0, 0); // I don't like this change sometime?
    block even_key;
    block odd_key;
    block leaf;

    // seed population
    for (std::size_t i = 1; i < n; ++i) {
        pa = getLSB(A[n - i - 1]);

        // seeds
        for (int j = (1 << i) - 1; j >= 0; --j) {
            prg_buffer[0] = seed_buffer[j];
            prg.reseed(&prg_buffer[0]);
            prg.random_block(prg_buffer, 2);
            seed_buffer[2*j] = prg_buffer[0];
            seed_buffer[2*j + 1] = prg_buffer[1];
            even ^= seed_buffer[j*2];
            odd ^= seed_buffer[j*2 + 1];
        }

        printf("BUFF %x: ", i);
        printtf(seed_buffer, (1 << n));

        // encryption keys
        if (pa == 0) {
            prg_buffer[0] = A[n - i - 1] ^ delta;
            prg.reseed(&prg_buffer[0]);
            prg.random_block(prg_buffer, 1);
            even_key = prg_buffer[0];
            prg_buffer[0] = A[n - i - 1];
            prg.reseed(&prg_buffer[0]);
            prg.random_block(prg_buffer, 1);
            odd_key = prg_buffer[0];
        }
        else {
            prg_buffer[0] = A[n - i - 1];
            prg.reseed(&prg_buffer[0]);
            prg.random_block(prg_buffer, 1);
            even_key = prg_buffer[0];
            prg_buffer[0] = A[n - i - 1] ^ delta;
            prg.reseed(&prg_buffer[0]);
            prg.random_block(prg_buffer, 1);
            odd_key = prg_buffer[0];
        }

        printf("Ai ");
        printt(A[n - i - 1]);
        printf("Ai^D ");
        printt(A[n - i - 1] ^ delta);
        printf("ekey ");
        printt(even_key);
        printf("okey ");
        printt(odd_key);

        table[2*(i - 1)] = even ^ even_key;
        table[2*(i - 1) + 1] = odd ^ odd_key;

        if (i == n - 1) {
            leaf = seed_buffer[0];
            for (std::size_t j = 1; j < (1 << n); j++) {
                leaf ^= seed_buffer[i];
            }
            leaf ^= delta;
        }

        table[2*(n - 1)] = leaf;
    }

    seed_buffer[(1 << n) - a - 1] = seed_buffer[(1 << n) - a - 1] ^ delta;
    //seed_buffer[a] = makeBlock(10, 10);

    return seed_buffer;

}

template<typename T>
class OneHotGarbledGateGen:public CircuitExecution {
public:
	block delta;
	T * io;
	block constant[2];
	MITCCRH<8> mitccrh;
	OneHotGarbledGateGen(T * io) :io(io) {
		block tmp[2];
		PRG().random_block(tmp, 2);
		set_delta(tmp[0]);
		io->send_block(tmp+1, 1);
		mitccrh.setS(tmp[1]);
	}
	void set_delta(const block & _delta) {
		delta = set_bit(_delta, 0);
		PRG().random_block(constant, 2);
		io->send_block(constant, 2);
		constant[1] = constant[1] ^ delta;
	}
	block public_label(bool b) override {
		return constant[b];
	}
    block and_gate(const block& a, const block& b) override {
		error("Operation not supported by one hot garbled gates.");
	}
	block xor_gate(const block&a, const block& b) override {
		error("Operation not supported by one hot garbled gates.");
	}
	block not_gate(const block&a) override {
		error("Operation not supported by one hot garbled gates.");
	}
	block *one_hot_garbled_gate(std::size_t n, const block *A, size_t a) override {
        std::size_t table_size = 2*(n - 1) + 1;
		block *table = new block[table_size];
        block *res = one_hot_garble(n, A, a, delta, table, &mitccrh);
		io->send_block(table, table_size);
		return res;
	}
	uint64_t num_and() override {
		return mitccrh.gid/2;
	}
};
}
#endif// EMP_ONE_HOT_GARBLED_GATE_GEN_H__