// Copyright 2022 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Marco Bertuletti,  ETH Zurich

// Define Matrix dimensions:
// C = AB with A=[MxN], B=[NxP], C=[MxP]
#define matrix_M 128
#define matrix_N 64
#define matrix_P 128

/* TEST1 Single-Core matrix-matrix multiplication --> #define SINGLE
   TEST4 Parallel matrix-matrix multiplication --> #define PARALLEL
   TEST5 Two parallel matrix-matrix multiplications
   run over half of the cores, concurrently --> #define PARALLEL_CONCURRENT */

#if !(defined(SINGLE) || defined(PARALLEL) || defined(PARALLEL_CONCURRENT))
#define SINGLE
//#define SINGLE
//#define PARALLEL
//#define PARALLEL_CONCURRENT
#endif

#if !(defined(matrix_M) && defined(matrix_N) && defined(matrix_P))
#define matrix_M (128)
#define matrix_N (64)
#define matrix_P (128)
#endif

#if !(defined(NUM_PARALLEL_CORES))
/* Sets the number of cores that are used to run the parallel kernel */
#define NUM_PARALLEL_CORES (256)
#endif
