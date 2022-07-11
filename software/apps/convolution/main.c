// Copyright 2021 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Samuel Riedel, ETH Zurich

#include <stdint.h>
#include <string.h>

#include "data.h"
#include "dma.h"
#include "encoding.h"
#include "kernel/convolution.h"
#include "printf.h"
#include "runtime.h"
#include "synchronization.h"

// #include "convolution_riscv.h"
// #include "halide_runtime.h"

#define M (96)
#define N (4 * NUM_CORES)
#define KERNEL_N 3
// #define VERBOSE

dump(time, 0);

volatile int32_t in[M * N] __attribute__((section(".l1_prio")));
volatile int32_t out[M * N] __attribute__((section(".l1_prio")));
volatile int32_t out_l2[M * N] __attribute__((section(".l2")));
// volatile int32_t kernel[KERNEL_N * KERNEL_N] __attribute__((section(".l1")));

int main() {
  uint32_t core_id = mempool_get_core_id();
  uint32_t num_cores = mempool_get_core_count();
  mempool_barrier_init(core_id);

  int32_t kernel[KERNEL_N * KERNEL_N];

  // Initialize img
  if (core_id == 0) {
#ifdef VERBOSE
    printf("Initialize\n");
#endif
    dma_memcpy_blocking((void *)in, (void *)in_l2, M * N / 2 * sizeof(int32_t));
  }

  kernel[0] = 1;
  kernel[1] = 2;
  kernel[2] = 1;

  kernel[3] = 2;
  kernel[4] = 4;
  kernel[5] = 2;

  kernel[6] = 1;
  kernel[7] = 2;
  kernel[8] = 1;

  // Matrices are initialized --> Start calculating
  // Wait at barrier until everyone is ready
  mempool_barrier(num_cores);

  // Double-buffered convolution
  int last_round = 6;
  int first = 0;
  int last = 2 * (int)num_cores;
  int32_t *round_barrier = (int32_t *)(64 * 1024);

  // Initial setup
  if (core_id == 0) {
    *round_barrier = 0;
    wake_up_all();
  }

  const int32_t *in_comp;
  const int32_t *in_dma;
  int32_t *out_comp;
  int32_t *out_dma;
  for (int round = 0; round < last_round; ++round) {
    if (round % 2 == 0) {
      in_comp = (const int32_t *)&in[0];
      out_comp = (int32_t *)&out[0];
      in_dma = (const int32_t *)&in[N * M / 2];
      out_dma = (int32_t *)&out[N * M / 2];
    } else {
      in_dma = (const int32_t *)&in[0];
      out_dma = (int32_t *)&out[0];
      in_comp = (const int32_t *)&in[N * M / 2];
      out_comp = (int32_t *)&out[N * M / 2];
    }
    mempool_start_benchmark();
    // Barrier, launch DMA for next iteration
    mempool_wfi();
    int bar = __atomic_fetch_add(round_barrier, 2, __ATOMIC_RELAXED);
    // Are we the first to reach the next round?
    if (bar == first) {
      dma_wait();
      if (round != last_round - 1) {
        dma_memcpy_nonblocking((void *)in_dma, (void *)in_l2,
                               M * N / 2 * sizeof(int32_t));
      }
      if (round != 0) {
        dma_memcpy_nonblocking((void *)out_l2, (void *)out_dma,
                               M * N / 2 * sizeof(int32_t));
      }
      bar = __atomic_fetch_add(round_barrier, 2, __ATOMIC_RELAXED);
    }
    // Are we the last one?
    if (bar == last) {
      *round_barrier = 0;
      if (round != last_round - 1) {
        wake_up_all();
      }
    }
    mempool_stop_benchmark();
    mempool_start_benchmark();
    conv2d_3x3_crazy_parallel((const int32_t *)in_comp, N, M / 2,
                              (const int32_t *)kernel, (int32_t *)out_comp,
                              core_id, num_cores);
    mempool_stop_benchmark();
  }

  mempool_start_benchmark();

  // Wait at barrier befor checking
  mempool_barrier(num_cores);
  mempool_stop_benchmark();

  // TODO Verify

  return 0;
}
