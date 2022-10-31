// Copyright 2021 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Marco Bertuletti, ETH Zurich

#define N_BANKS (NUM_CORES * 4)

/* TEST1 Single-Core Colesky decomposition --> #define SINGLE
   TEST2 Single-Core system inversion --> #define SINGLE #define LINSOLVER
   TEST4 Parallel folded Cholesky decomposition --> #define PARALLEL
   TEST5 Parallel folded system inversion --> #define PARALLEL #define LINSOLVER
 */

#if !(defined(SINGLE) || defined(PARALLEL))
#define SINGLE
#define N_DEC 1
//#define SINGLE
//#define PARALLEL
//#define LINSOLVER
//#define N_COL 1024
//#define N_ROW 1
#endif
#if !(defined N)
/* Dimension of the input matrix */
#define N (4)
#endif

int32_t A_matrix[N * N] __attribute__((aligned(N_BANKS), section(".l1")));
int32_t AT_matrix[N * N] __attribute__((aligned(N_BANKS), section(".l1")));
int32_t M_matrix[N * N] __attribute__((aligned(N_BANKS), section(".l1")));

#if defined(SINGLE)
int32_t L_matrix[N * N] __attribute__((aligned(N_BANKS), section(".l1")));
int32_t In[N] __attribute__((aligned(N_BANKS), section(".l1")));

#elif defined(PARALLEL)
int32_t LL_matrix[N_ROW * N * N_BANKS]
    __attribute__((aligned(N_BANKS), section(".l1")));
int32_t LR_matrix[N_ROW * N * N_BANKS]
    __attribute__((aligned(N_BANKS), section(".l1")));
int32_t In_matrix[N * N_BANKS]
    __attribute__((aligned(N_BANKS), section(".l1")));
#elif defined(LINSOLVER)
int32_t LL_matrix[N_ROW * N * N_BANKS]
    __attribute__((aligned(N_BANKS), section(".l1")));
int32_t LR_matrix[N_ROW * N * N_BANKS]
    __attribute__((aligned(N_BANKS), section(".l1")));
int32_t In_matrix[N * N_BANKS]
    __attribute__((aligned(N_BANKS), section(".l1")));
int32_t In[N_BANKS] __attribute__((aligned(N_BANKS), section(".l1")));
#endif
