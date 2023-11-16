#ifndef EMP_ONE_HOT_GARBLED_GATE_EVA_H__
#define EMP_ONE_HOT_GARBLED_GATE_EVA_H__
#include "emp-tool/utils/utils.h"
#include "emp-tool/utils/mitccrh.h"
#include "emp-tool/execution/circuit_execution.h"
#include <iostream>
namespace emp {

// return list of ciphertexts size 2^n
inline block *one_hot_eval(std::size_t n, const block *A, std::size_t a, const block *table, MITCCRH<8> *mitccrh) {

    assert(getLSB(delta) == 1);

	bool pa;

    block *seed_buffer = new block[1 << n]();

    PRG prg;

    auto missing = 0;

    pa = (a >> (n - 1)) & 1;
    seed_buffer[!pa] = A[n - 1];

    missing |= pa;

    block prg_buffer[2];

    block even;
    block odd;
    block even_rec;
    block odd_rec;
    block key;
    block leaf_a;

    // seed population
    for (std::size_t i = 1; i < n; ++i) {
        pa = (a >> (n - i - 1)) & 1;

        even_rec = makeBlock(0, 0);
        odd_rec = makeBlock(0, 0);

        even = table[2*(i - 1)];
        odd = table[2*(i - 1) + 1];

        // seeds
        for (int j = (1 << i) - 1; j >= 0; --j) {
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
        prg_buffer[0] = A[n - i - 1];
        prg.reseed(&prg_buffer[0]);
        prg.random_block(prg_buffer, 1);
        key = prg_buffer[0];

        if (pa == 1) {
            seed_buffer[missing ^ 1] = key ^ even ^ even_rec;
        }
        else {
            seed_buffer[missing ^ 1] = key ^ odd ^ odd_rec;
        }

        if (i == n - 1) {
            leaf_a = table[2*(n - 1)];
            for (std::size_t j = 0; j < (1 << n); j++) {
                if (j != a) {
                    leaf_a ^= seed_buffer[j];
                }
            }
            seed_buffer[a] = leaf_a;
        }
    }

    return seed_buffer;

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
    block *one_hot_garbled_gate(std::size_t n, const block *A, size_t a) override {
        std::size_t table_size = 2*(n - 1) + 1;
		block *table = new block[table_size];
        io->recv_block(table, table_size);
        block *res = one_hot_garble(n, A, a, table, &mitccrh);
		return res;
	}
	uint64_t num_and() override {
		return mitccrh.gid/2;
	}
};
}
#endif// EMP_ONE_HOT_GARBLED_GATE_EVA_H__2
