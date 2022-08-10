// Copyright 2021 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Marco Bertuletti, ETH Zurich

#include "encoding.h"
#include "printf.h"
#include "runtime.h"
#include "synchronization.h"

#include "initialization.h"
#include "mempool_cholesky_q32s.h"

#define N 8
#define SINGLE
#define VERBOSE

int32_t M_matrix[N * N]     __attribute__((aligned(N), section(".l1")));
int32_t L_matrix[N * N]     __attribute__((aligned(N), section(".l1")));
int32_t LT_matrix[N * N]    __attribute__((aligned(N), section(".l1")));

// Driver program
void single_core() {

    uint32_t core_id = mempool_get_core_id();
    uint32_t num_cores = mempool_get_core_count();
    mempool_barrier_init(core_id); // Initialize barrier and synchronize

    init_matrix(M_matrix, N, N, -156, 427, -219, core_id);
    init_matrix_zeros(L_matrix, N, N, core_id);
    init_matrix_zeros(LT_matrix, N, N, core_id);
    mempool_barrier(num_cores);

    if(core_id == 0) {
        mempool_start_benchmark();
        mempool_cholesky_q32s(M_matrix, L_matrix, LT_matrix, N, FIXED_POINT);
        mempool_stop_benchmark();
    }
    mempool_barrier(num_cores);

    #ifdef VERBOSE
    if (core_id == 0) {
        display(L_matrix, N, N);
        display(LT_matrix, N, N);
    }
    #endif
    mempool_barrier(num_cores);
}

int main() {
    #if defined(SINGLE)
    single_core();
    #endif
    return 0;
}