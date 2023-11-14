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

inline block one_hot_garble(size_t n, const block *a, block *table, MITCCRH<8> *mitccrh) {
    // TODO
}

}