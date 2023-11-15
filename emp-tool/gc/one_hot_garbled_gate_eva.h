#ifndef EMP_ONE_HOT_GARBLED_GATE_EVA_H__
#define EMP_ONE_HOT_GARBLED_GATE_EVA_H__
#include "emp-tool/utils/utils.h"
#include "emp-tool/utils/mitccrh.h"
#include "emp-tool/execution/circuit_execution.h"
#include <iostream>
namespace emp {

inline block one_hot_eval(std::size_t n, const block *A, std::size_t a, const block *table, MITCCRH<8> *mitccrh) {
	bool pa;
    std::vector<block> seed_buffer(1 << n);

    PRG prg;

    auto missing = 0;

    // base case
    pa = getLSB(A[0]);
    seed_buffer[1 - pa] = A[0]; // should we be hashing here?

    missing += pa;

    block prg_buffer[2];
    block mitccrh_buffer[1];

    block even;
    block odd;
    block even_rec = makeBlock(0, 0); // I don't like
    block odd_rec = makeBlock(0, 0); // I don't like this change sometime?
    block key;
    block one;

    // seed population
    for (std::size_t i = 1; i < n; i++) {
        pa = getLSB(A[i]);

        even = table[2*(i - 1)];
        odd = table[2*(i - 1) + 1];

        // seeds
        for (int j = (1 << (i - 1)); j >= 0; j--) {
            if (j != missing) {
                prg_buffer[0] = seed_buffer[j];
                prg.reseed(&prg_buffer[0]);
                prg.random_block(prg_buffer, 2);
                seed_buffer[2*j] = prg_buffer[0];
                seed_buffer[2*j + 1] = prg_buffer[1];
                even_rec ^= seed_buffer[j*2];
                odd_rec ^= seed_buffer[j*2 + 1];
            }
        }

        missing = (missing << 1) | pa; 

        // encryption key
        mitccrh_buffer[0] = A[i];
        mitccrh->hash<2,1>(mitccrh_buffer);
        key = mitccrh_buffer[0];

        if (pa == 0) {
            seed_buffer[missing ^ 1] = key ^ even ^ even_rec;
        }
        else {
            seed_buffer[missing ^ 1] = key ^ odd ^ odd_rec;
        }

        if (i == n - 1) {
            one = table[2*(n - 1)];
            for (std::size_t j = 0; j < a; j++) {
                one ^= seed_buffer[i];
            }
            for (std::size_t j = a + 1; j < (1 << n); j++) {
                one ^= seed_buffer[i];
            }
        }
    }

    return one; // TODO fix

}


template<typename T>
class OneHotGarbledGateEva:public CircuitExecution {
public:
	T * io;
	block constant[2];
	MITCCRH<8> mitccrh;
	OneHotGarbledGateEva(T * io) :io(io) {
		set_delta();
		block tmp;
		io->recv_block(&tmp, 1);
		mitccrh.setS(tmp);
	}
	void set_delta() {
		io->recv_block(constant, 2);
	}
	block public_label(bool b) override {
		return constant[b];
	}
	block and_gate(const block& a, const block& b) override {
		error("Operation not supported by one hot garbled gates.");
	}
	block xor_gate(const block& a, const block& b) override {
		error("Operation not supported by one hot garbled gates.");
	}
	block not_gate(const block&a) override {
		error("Operation not supported by one hot garbled gates.");
	}
    block one_hot_garbled_gate(std::size_t n, const block *A, size_t a) override {
        std::size_t table_size = 2*(n - 1) + 1;
		block *table = new block[table_size];
        io->recv_block(table, table_size);
        block res = one_hot_garble(n, A, a, table, &mitccrh);
		return res;
	}
	uint64_t num_and() override {
		return mitccrh.gid/2;
	}
};
}
#endif// EMP_ONE_HOT_GARBLED_GATE_EVA_H__2
