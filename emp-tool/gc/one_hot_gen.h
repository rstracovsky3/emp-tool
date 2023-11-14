#ifndef EMP_ONE_HOT_GEN_H__
#define EMP_ONE_HOT_GEN_H__
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
inline void one_hot_garble(std::size_t n, const block *A, block delta, block *table, MITCCRH<8> *mitccrh) {
    bool pa;
    std::vector<Share<mode>> seed_buffer(1 << n);

    PRG prg;

    // base case
    pa = getLSB(A[0]);
    if (pa == 0) {
        seed_buffer[0] = A[0] ^ delta; // should we be hashing here?
        seed_buffer[1] = A[0];  // should we be hashing here?
    }
    else {
        seed_buffer[0] = A[0];  // should we be hashing here?
        seed_buffer[1] = A[0] ^ delta;  // should we be hashing here?
    }

    block prg_buffer[2];
    block mitccrh_buffer[2];

    block even;
    block odd;
    block even_key;
    block odd_key;

    // seed population
    for (std::size_t i = 1; i < n; i++) {
        pa = getLSB(A[i]);

        // seeds
        for (int j = (1 << (i - 1)); j >= 0; j--) {
            prg_buffer[0] = seed_buffer[j];
            prg.reseed(&prg_buffer[0]);
            prg.random_block(prg_buffer, 2);
            seed_buffer[2*j] = prg_buffer[0];
            seed_buffer[2*j + 1] = prg_buffer[1];
            even ^= seed_buffer[j*2];
            odd ^= seed_buffer[j*2 + 1];
        }

        // encryption keys
        if (pa == 0) {
            mitccrh_buffer[0] = x[i] ^ delta;
            mitccrh_buffer[1] = x[i];
        }
        else {
            mitccrh_buffer[0] = x[i];
            mitccrh_buffer[1] = x[i] ^ delta;
        }
        mitccrh->hash<2,1>(mitccrh_buffer);
        even_key = mitccrh_buffer[0];
        odd_key = mitccrh_buffer[1];

        table[2*(i - 1)] = even ^ even_key;
        table[2*(i - 1) + 1] = odd ^ odd_key;

        if (i == n - 1) {
            block leaf = seed_buffer[0];
            for (std::size_t j = 1; j < (1 << n); j++) {
                leaf ^= seed_buffer[i];
            }
            leaf ^= delta;
        }

        table[2*(n - 1)] = leaf;
    }



}

}