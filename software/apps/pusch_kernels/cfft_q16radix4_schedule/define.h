// Copyright 2022 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Marco Bertuletti, ETH Zurich

/* DEFINES */
#if !(defined(TEST_64) || defined(TEST_256) || defined(TEST_1024) ||           \
      defined(TEST_4096))
#define TEST_4096
#endif

#if defined(TEST_64)
#define N_CSAMPLES (64)
#define LOG2 (6)
#elif defined(TEST_256)
#define N_CSAMPLES (256)
#define LOG2 (8)
#elif defined(TEST_1024)
#define N_CSAMPLES (1024)
#define LOG2 (10)
#elif defined(TEST_4096)
#define N_CSAMPLES (4096)
#define LOG2 (12)
#endif

#define N_RSAMPLES (2 * N_CSAMPLES)
#define N_BANKS (NUM_CORES * 4)

#define ASM       // Use asm_volatile statements
#define BIT_REV 1 // Apply bitreversal permutations
#define XPULP     // Use xpulpimg extensions

/* Single core implementation. Uncomment XPULP to have xpulpimg extensions
   enabled. N_FFTs determines the number of FFTs that the single-core has to run
   in sequence. */

// #define SINGLE
// #define N_FFTs 1

/* Parallel implementation. Uncomment COMPUTE_BITREV to compute the
   permutation addresses on the run. N_FFTs_COL determines the number of FFTs
   run in parallel over different sub-sets of cores in the cluster. N_FFTs_ROW
   determines the number of FFTs run in sequence over a sub-set of cores in the
   cluster. E.g. if N_CSAMPLES is 4096 the FFT is parallelized over 256 cores.
   We therefore fit 4 columns on TeraPool.
   Each sub-set of 256 cores can operate on several FFTs in sequence. */

//#define PARALLEL
//#define COMPUTE_BITREV
//#define N_FFTs_ROW 4
//#define N_FFTs_COL 1

#if !(defined(SINGLE) || defined(PARALLEL))
#define SINGLE
#define N_FFTs 1
#endif

#define N_BANKS_SINGLE (N_BANKS * ((N_CSAMPLES + N_BANKS - 1) / N_BANKS))
#define MAX_COL (N_BANKS / (N_CSAMPLES / 4))

/* FUNCTIONS */
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) < (y)) ? (y) : (x))
