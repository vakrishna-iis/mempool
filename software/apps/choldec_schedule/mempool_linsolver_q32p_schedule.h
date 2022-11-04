// Copyright 2021 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Marco Bertuletti, ETH Zurich

void mempool_linsolver_q32p_fold(int32_t *pSrcA, int32_t *pSrcB, int32_t *pLL,
                                 int32_t *pLR, int32_t *pIn, const uint32_t n,
                                 const uint32_t n_row, const uint32_t n_col);

void mempool_linsolver_q32p_FLsqrtsum(int32_t *pSrc, int32_t *pL, int32_t *pIn,
                                      uint32_t core_id, const uint32_t j);

void mempool_linsolver_q32p_FRsqrtsum(int32_t *pSrc, int32_t *pL, int32_t *pIn,
                                      uint32_t core_id, const uint32_t n,
                                      const uint32_t j);

void mempool_linsolver_q32p_FLdivisum(int32_t *pSrc, int32_t *pL, int32_t *pIn,
                                      uint32_t core_id, const uint32_t n,
                                      const uint32_t j);

void mempool_linsolver_q32p_FRdivisum(int32_t *pSrc, int32_t *pL, int32_t *pIn,
                                      uint32_t core_id, const uint32_t n,
                                      const uint32_t j);

void mempool_linsolver_q32p_trisolverL(int32_t *pL, int32_t *pIn,
                                       uint32_t core_id, const uint32_t n);

void mempool_linsolver_q32p_trisolverR(int32_t *pL, int32_t *pIn,
                                       uint32_t core_id, const uint32_t n,
                                       const uint32_t nPE);

void mempool_linsolver_q32p_fold(int32_t *pSrcA, int32_t *pSrcB, int32_t *pLL,
                                 int32_t *pLR, int32_t *pIn, const uint32_t n,
                                 const uint32_t n_row, const uint32_t n_col) {

  uint32_t absolute_core_id = mempool_get_core_id();
  uint32_t column_id = absolute_core_id / (n >> 2U);
  uint32_t core_id = absolute_core_id % (n >> 2U);
  uint32_t idx_row, idx_col;
  uint32_t j;

  for (j = 0; j < n; j++) {
    for (idx_col = column_id; idx_col < n_col; idx_col += n_col) {
      for (idx_row = 0; idx_row < n_row; idx_row++) {
        mempool_linsolver_q32p_FLsqrtsum(
            pSrcA + idx_col * n, pLL + idx_col * n + idx_row * (n * N_BANKS),
            pIn + idx_col * n, core_id, j);
        mempool_linsolver_q32p_FRsqrtsum(
            pSrcB + idx_col * n, pLR + idx_col * n + idx_row * (n * N_BANKS),
            pIn + idx_col * n, core_id, n, j);
      }
    }
    mempool_log_partial_barrier(2, absolute_core_id, n_col * (n >> 2U));
    for (idx_col = column_id; idx_col < n_col; idx_col += n_col) {
      for (idx_row = 0; idx_row < n_row; idx_row++) {
        mempool_linsolver_q32p_FLdivisum(
            pSrcA + idx_col * n, pLL + idx_col * n + idx_row * (n * N_BANKS),
            pIn + idx_col * n + idx_row * N_BANKS, core_id, n, j);
        mempool_linsolver_q32p_FRdivisum(
            pSrcB + idx_col * n, pLR + idx_col * n + idx_row * (n * N_BANKS),
            pIn + idx_col * n, core_id, n, j);
      }
    }
    mempool_log_partial_barrier(2, absolute_core_id, n_col * (n >> 2U));
  }

  for (idx_col = column_id; idx_col < n_col; idx_col += n_col) {
    for (idx_row = 0; idx_row < n_row; idx_row++) {
      mempool_linsolver_q32p_trisolverL(pLL + idx_col * n +
                                            idx_row * (n * N_BANKS),
                                        pIn + idx_col * n, core_id, n);
      mempool_linsolver_q32p_trisolverR(
          pLR + idx_col * n + idx_row * (n * N_BANKS), pIn + idx_col * n,
          core_id, n, n_col * (n >> 2U));
    }
  }
  mempool_log_partial_barrier(2, absolute_core_id, n_col * (n >> 2U));
}

void mempool_linsolver_q32p_FLsqrtsum(int32_t *pSrc, int32_t *pL, int32_t *pIn,
                                      uint32_t core_id, const uint32_t j) {
  int32_t sum, in, result;
  int32_t pivot;
  uint32_t k;
  int32_t a0, a1, a2, a3;
  /* Elements on the diagonal are computed with a single core */
  if (core_id == (j >> 2U)) {
    in = pIn[j];
    pivot = pSrc[j * N_BANKS + j];
    sum = 0;
    for (k = 0; k < 4 * (j >> 2U); k++) {
      a0 = pL[j + k * N_BANKS];
      a1 = pL[j + (k + 1) * N_BANKS];
      a2 = pL[j + (k + 2) * N_BANKS];
      a3 = pL[j + (k + 3) * N_BANKS];
      asm volatile("mul  %[a0],%[a0],%[a0];"
                   "mul  %[a1],%[a1],%[a1];"
                   "mul  %[a2],%[a2],%[a2];"
                   "mul  %[a3],%[a3],%[a3];"
                   "addi %[a0],%[a0],%[h];"
                   "addi %[a1],%[a1],%[h];"
                   "addi %[a2],%[a2],%[h];"
                   "addi %[a3],%[a3],%[h];"
                   "srai  %[a0],%[a0],%[s];"
                   "srai  %[a1],%[a1],%[s];"
                   "srai  %[a2],%[a2],%[s];"
                   "srai  %[a3],%[a3],%[s];"
                   "add  %[sum],%[a0],%[sum];"
                   "add  %[sum],%[a1],%[sum];"
                   "add  %[sum],%[a2],%[sum];"
                   "add  %[sum],%[a3],%[sum];"
                   : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2),
                     [a3] "+&r"(a3), [sum] "+&r"(sum)
                   : [s] "I"(FIXED_POINT), [h] "I"(HALF)
                   :);
    }
    switch (j % 4) {
    case 3:
      a0 = pL[j + k * N_BANKS];
      a1 = pL[j + (k + 1) * N_BANKS];
      a2 = pL[j + (k + 2) * N_BANKS];
      asm volatile(
          "mul  %[a0],%[a0],%[a0];"
          "mul  %[a1],%[a1],%[a1];"
          "mul  %[a1],%[a2],%[a2];"
          "addi %[a0],%[a0],%[h];"
          "addi %[a1],%[a1],%[h];"
          "addi %[a2],%[a2],%[h];"
          "srai  %[a0],%[a0],%[s];"
          "srai  %[a1],%[a1],%[s];"
          "srai  %[a2],%[a2],%[s];"
          "add  %[sum],%[a0],%[sum];"
          "add  %[sum],%[a1],%[sum];"
          "add  %[sum],%[a2],%[sum];"
          : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2), [sum] "+&r"(sum)
          : [s] "I"(FIXED_POINT), [h] "I"(HALF)
          :);
      break;
    case 2:
      a0 = pL[j + k * N_BANKS];
      a1 = pL[j + (k + 1) * N_BANKS];
      asm volatile("mul  %[a0],%[a0],%[a0];"
                   "mul  %[a1],%[a1],%[a1];"
                   "addi %[a0],%[a0],%[h];"
                   "addi %[a1],%[a1],%[h];"
                   "srai  %[a0],%[a0],%[s];"
                   "srai  %[a1],%[a1],%[s];"
                   "add  %[sum],%[a0],%[sum];"
                   "add  %[sum],%[a1],%[sum];"
                   : [a0] "+&r"(a0), [a1] "+&r"(a1), [sum] "+&r"(sum)
                   : [s] "I"(FIXED_POINT), [h] "I"(HALF)
                   :);
      break;
    case 1:
      a0 = pL[j + k * N_BANKS];
      asm volatile("mul  %[a0],%[a0],%[a0];"
                   "addi %[a0],%[a0],%[h];"
                   "srai  %[a0],%[a0],%[s];"
                   "add  %[sum],%[a0],%[sum];"
                   : [a0] "+&r"(a0), [sum] "+&r"(sum)
                   : [s] "I"(FIXED_POINT), [h] "I"(HALF)
                   :);
      break;
    case 0:
      break;
    }
    result = mempool_sqrt_q32s(pivot - sum);
    pIn[j] = FIX_DIV(in, result);
    pL[j * N_BANKS + j] = result;
  }
}

void mempool_linsolver_q32p_FRsqrtsum(int32_t *pSrc, int32_t *pL, int32_t *pIn,
                                      uint32_t core_id, const uint32_t n,
                                      const uint32_t j) {
  int32_t sum, in, result;
  int32_t pivot;
  uint32_t k;
  int32_t a0, a1, a2, a3;
  /* Elements on the diagonal are computed with a single core */
  if (core_id == (n >> 2U) - 1 - (j >> 2U)) {
    in = pIn[j];
    pivot = pSrc[j * N_BANKS + j];
    sum = 0;
    for (k = 0; k < 4 * (j >> 2U); k++) {
      a0 = pL[n - 1 - j + k * N_BANKS];
      a1 = pL[n - 1 - j + (k + 1) * N_BANKS];
      a2 = pL[n - 1 - j + (k + 2) * N_BANKS];
      a3 = pL[n - 1 - j + (k + 3) * N_BANKS];
      asm volatile("mul  %[a0],%[a0],%[a0];"
                   "mul  %[a1],%[a1],%[a1];"
                   "mul  %[a2],%[a2],%[a2];"
                   "mul  %[a3],%[a3],%[a3];"
                   "addi %[a0],%[a0],%[h];"
                   "addi %[a1],%[a1],%[h];"
                   "addi %[a2],%[a2],%[h];"
                   "addi %[a3],%[a3],%[h];"
                   "srai  %[a0],%[a0],%[s];"
                   "srai  %[a1],%[a1],%[s];"
                   "srai  %[a2],%[a2],%[s];"
                   "srai  %[a3],%[a3],%[s];"
                   "add  %[sum],%[a0],%[sum];"
                   "add  %[sum],%[a1],%[sum];"
                   "add  %[sum],%[a2],%[sum];"
                   "add  %[sum],%[a3],%[sum];"
                   : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2),
                     [a3] "+&r"(a3), [sum] "+&r"(sum)
                   : [s] "I"(FIXED_POINT), [h] "I"(HALF)
                   :);
    }
    switch (j % 4) {
    case 3:
      a0 = pL[n - 1 - j + k * N_BANKS];
      a1 = pL[n - 1 - j + (k + 1) * N_BANKS];
      a2 = pL[n - 1 - j + (k + 2) * N_BANKS];
      asm volatile(
          "mul  %[a0],%[a0],%[a0];"
          "mul  %[a1],%[a1],%[a1];"
          "mul  %[a1],%[a2],%[a2];"
          "addi %[a0],%[a0],%[h];"
          "addi %[a1],%[a1],%[h];"
          "addi %[a2],%[a2],%[h];"
          "srai  %[a0],%[a0],%[s];"
          "srai  %[a1],%[a1],%[s];"
          "srai  %[a2],%[a2],%[s];"
          "add  %[sum],%[a0],%[sum];"
          "add  %[sum],%[a1],%[sum];"
          "add  %[sum],%[a2],%[sum];"
          : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2), [sum] "+&r"(sum)
          : [s] "I"(FIXED_POINT), [h] "I"(HALF)
          :);
      break;
    case 2:
      a0 = pL[n - 1 - j + k * N_BANKS];
      a1 = pL[n - 1 - j + (k + 1) * N_BANKS];
      asm volatile("mul  %[a0],%[a0],%[a0];"
                   "mul  %[a1],%[a1],%[a1];"
                   "addi %[a0],%[a0],%[h];"
                   "addi %[a1],%[a1],%[h];"
                   "srai  %[a0],%[a0],%[s];"
                   "srai  %[a1],%[a1],%[s];"
                   "add  %[sum],%[a0],%[sum];"
                   "add  %[sum],%[a1],%[sum];"
                   : [a0] "+&r"(a0), [a1] "+&r"(a1), [sum] "+&r"(sum)
                   : [s] "I"(FIXED_POINT), [h] "I"(HALF)
                   :);
      break;
    case 1:
      a0 = pL[n - 1 - j + k * N_BANKS];
      asm volatile("mul  %[a0],%[a0],%[a0];"
                   "addi %[a0],%[a0],%[h];"
                   "srai  %[a0],%[a0],%[s];"
                   "add  %[sum],%[a0],%[sum];"
                   : [a0] "+&r"(a0), [sum] "+&r"(sum)
                   : [s] "I"(FIXED_POINT), [h] "I"(HALF)
                   :);
      break;
    case 0:
      break;
    }
    result = mempool_sqrt_q32s(pivot - sum);
    pIn[j] = FIX_DIV(in, result);
    pL[j * N_BANKS + n - 1 - j] = result;
  }
}

void mempool_linsolver_q32p_FLdivisum(int32_t *pSrc, int32_t *pL, int32_t *pIn,
                                      uint32_t core_id, const uint32_t n,
                                      const uint32_t j) {
  int32_t sum, in, sum_r, result;
  int32_t pivot, diag;
  uint32_t i, k;
  int32_t a0, a1, a2, a3;
  int32_t b0, b1, b2, b3;
  /* Elements on rows are computed in parallel, each core gets 4 rows */
  for (i = j + 1; i < n; i++) {
    if (core_id == (i / 4)) {
      sum = 0;
      pivot = pSrc[i * N_BANKS + j];
      diag = pL[j + j * N_BANKS];
      in = pIn[j];
      sum_r = pIn[i];
      for (k = 0; k < 4 * (j >> 2U); k += 4) {
        a0 = pL[i + k * N_BANKS];
        a1 = pL[i + (k + 1) * N_BANKS];
        a2 = pL[i + (k + 2) * N_BANKS];
        a3 = pL[i + (k + 3) * N_BANKS];
        b0 = pL[j + k * N_BANKS];
        b1 = pL[j + (k + 1) * N_BANKS];
        b2 = pL[j + (k + 2) * N_BANKS];
        b3 = pL[j + (k + 3) * N_BANKS];
        asm volatile("mul  %[a0],%[a0],%[b0];"
                     "mul  %[a1],%[a1],%[b1];"
                     "mul  %[a2],%[a2],%[b2];"
                     "mul  %[a3],%[a3],%[b3];"
                     "addi %[a0],%[a0],%[h];"
                     "addi %[a1],%[a1],%[h];"
                     "addi %[a2],%[a2],%[h];"
                     "addi %[a3],%[a3],%[h];"
                     "srai  %[a0],%[a0],%[s];"
                     "srai  %[a1],%[a1],%[s];"
                     "srai  %[a2],%[a2],%[s];"
                     "srai  %[a3],%[a3],%[s];"
                     "add  %[sum],%[a0],%[sum];"
                     "add  %[sum],%[a1],%[sum];"
                     "add  %[sum],%[a2],%[sum];"
                     "add  %[sum],%[a3],%[sum];"
                     : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2),
                       [a3] "+&r"(a3), [sum] "+&r"(sum)
                     : [b0] "r"(b0), [b1] "r"(b1), [b2] "r"(b2), [b3] "r"(b3),
                       [s] "I"(FIXED_POINT), [h] "I"(HALF)
                     :);
      }
      switch (j % 4) {
      case 3:
        a0 = pL[i + k * N_BANKS];
        a1 = pL[i + (k + 1) * N_BANKS];
        a2 = pL[i + (k + 2) * N_BANKS];
        b0 = pL[j + k * N_BANKS];
        b1 = pL[j + (k + 1) * N_BANKS];
        b2 = pL[j + (k + 2) * N_BANKS];
        asm volatile(
            "mul  %[a0],%[a0],%[b0];"
            "mul  %[a1],%[a1],%[b1];"
            "mul  %[a1],%[a2],%[b2];"
            "addi %[a0],%[a0],%[h];"
            "addi %[a1],%[a1],%[h];"
            "addi %[a2],%[a2],%[h];"
            "srai  %[a0],%[a0],%[s];"
            "srai  %[a1],%[a1],%[s];"
            "srai  %[a2],%[a2],%[s];"
            "add  %[sum],%[a0],%[sum];"
            "add  %[sum],%[a1],%[sum];"
            "add  %[sum],%[a2],%[sum];"
            : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2), [sum] "+&r"(sum)
            : [b0] "r"(b0), [b1] "r"(b1), [b2] "r"(b2), [s] "I"(FIXED_POINT),
              [h] "I"(HALF)
            :);
        break;
      case 2:
        a0 = pL[i + k * N_BANKS];
        a1 = pL[i + (k + 1) * N_BANKS];
        b0 = pL[j + k * N_BANKS];
        b1 = pL[j + (k + 1) * N_BANKS];
        asm volatile(
            "mul  %[a0],%[a0],%[b0];"
            "mul  %[a1],%[a1],%[b1];"
            "addi %[a0],%[a0],%[h];"
            "addi %[a1],%[a1],%[h];"
            "srai  %[a0],%[a0],%[s];"
            "srai  %[a1],%[a1],%[s];"
            "add  %[sum],%[a0],%[sum];"
            "add  %[sum],%[a1],%[sum];"
            : [a0] "+&r"(a0), [a1] "+&r"(a1), [sum] "+&r"(sum)
            : [b0] "r"(b0), [b1] "r"(b1), [s] "I"(FIXED_POINT), [h] "I"(HALF)
            :);
        break;
      case 1:
        a0 = pL[i + k * N_BANKS];
        b0 = pL[j + k * N_BANKS];
        asm volatile("mul  %[a0],%[a0],%[b0];"
                     "addi %[a0],%[a0],%[h];"
                     "srai  %[a0],%[a0],%[s];"
                     "add  %[sum],%[a0],%[sum];"
                     : [a0] "+&r"(a0), [sum] "+&r"(sum)
                     : [b0] "r"(b0), [s] "I"(FIXED_POINT), [h] "I"(HALF)
                     :);
        break;
      case 0:
        break;
      }
      result = FIX_DIV((pivot - sum), diag);
      pIn[i] = sum_r - result * in;
      pL[i + j * N_BANKS] = result;
    }
  }
}

void mempool_linsolver_q32p_FRdivisum(int32_t *pSrc, int32_t *pL, int32_t *pIn,
                                      uint32_t core_id, const uint32_t n,
                                      const uint32_t j) {
  int32_t sum, in, sum_r, result;
  int32_t pivot, diag;
  uint32_t i, k;
  int32_t a0, a1, a2, a3;
  int32_t b0, b1, b2, b3;
  for (i = j + 1; i < n; i++) {
    if (core_id == (n >> 2U) - 1 - (i / 4)) {
      sum = 0;
      pivot = pSrc[i * N_BANKS + j];
      diag = pL[n - 1 - j + j * N_BANKS];
      in = pIn[j];
      sum_r = pIn[i];
      for (k = 0; k < 4 * (j >> 2U); k += 4) {
        a0 = pL[n - 1 - i + k * N_BANKS];
        a1 = pL[n - 1 - i + (k + 1) * N_BANKS];
        a2 = pL[n - 1 - i + (k + 2) * N_BANKS];
        a3 = pL[n - 1 - i + (k + 3) * N_BANKS];
        b0 = pL[n - 1 - j + k * N_BANKS];
        b1 = pL[n - 1 - j + (k + 1) * N_BANKS];
        b2 = pL[n - 1 - j + (k + 2) * N_BANKS];
        b3 = pL[n - 1 - j + (k + 3) * N_BANKS];
        asm volatile("mul  %[a0],%[a0],%[b0];"
                     "mul  %[a1],%[a1],%[b1];"
                     "mul  %[a2],%[a2],%[b2];"
                     "mul  %[a3],%[a3],%[b3];"
                     "addi %[a0],%[a0],%[h];"
                     "addi %[a1],%[a1],%[h];"
                     "addi %[a2],%[a2],%[h];"
                     "addi %[a3],%[a3],%[h];"
                     "srai  %[a0],%[a0],%[s];"
                     "srai  %[a1],%[a1],%[s];"
                     "srai  %[a2],%[a2],%[s];"
                     "srai  %[a3],%[a3],%[s];"
                     "add  %[sum],%[a0],%[sum];"
                     "add  %[sum],%[a1],%[sum];"
                     "add  %[sum],%[a2],%[sum];"
                     "add  %[sum],%[a3],%[sum];"
                     : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2),
                       [a3] "+&r"(a3), [sum] "+&r"(sum)
                     : [b0] "r"(b0), [b1] "r"(b1), [b2] "r"(b2), [b3] "r"(b3),
                       [s] "I"(FIXED_POINT), [h] "I"(HALF)
                     :);
      }
      switch (j % 4) {
      case 3:
        a0 = pL[n - 1 - i + k * N_BANKS];
        a1 = pL[n - 1 - i + (k + 1) * N_BANKS];
        a2 = pL[n - 1 - i + (k + 2) * N_BANKS];
        b0 = pL[n - 1 - j + k * N_BANKS];
        b1 = pL[n - 1 - j + (k + 1) * N_BANKS];
        b2 = pL[n - 1 - j + (k + 2) * N_BANKS];
        asm volatile(
            "mul  %[a0],%[a0],%[b0];"
            "mul  %[a1],%[a1],%[b1];"
            "mul  %[a1],%[a2],%[b2];"
            "addi %[a0],%[a0],%[h];"
            "addi %[a1],%[a1],%[h];"
            "addi %[a2],%[a2],%[h];"
            "srai  %[a0],%[a0],%[s];"
            "srai  %[a1],%[a1],%[s];"
            "srai  %[a2],%[a2],%[s];"
            "add  %[sum],%[a0],%[sum];"
            "add  %[sum],%[a1],%[sum];"
            "add  %[sum],%[a2],%[sum];"
            : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2), [sum] "+&r"(sum)
            : [b0] "r"(b0), [b1] "r"(b1), [b2] "r"(b2), [s] "I"(FIXED_POINT),
              [h] "I"(HALF)
            :);
        break;
      case 2:
        a0 = pL[n - 1 - i + k * N_BANKS];
        a1 = pL[n - 1 - i + (k + 1) * N_BANKS];
        b0 = pL[n - 1 - j + k * N_BANKS];
        b1 = pL[n - 1 - j + (k + 1) * N_BANKS];
        asm volatile(
            "mul  %[a0],%[a0],%[b0];"
            "mul  %[a1],%[a1],%[b1];"
            "addi %[a0],%[a0],%[h];"
            "addi %[a1],%[a1],%[h];"
            "srai  %[a0],%[a0],%[s];"
            "srai  %[a1],%[a1],%[s];"
            "add  %[sum],%[a0],%[sum];"
            "add  %[sum],%[a1],%[sum];"
            : [a0] "+&r"(a0), [a1] "+&r"(a1), [sum] "+&r"(sum)
            : [b0] "r"(b0), [b1] "r"(b1), [s] "I"(FIXED_POINT), [h] "I"(HALF)
            :);
        break;
      case 1:
        a0 = pL[n - 1 - i + k * N_BANKS];
        b0 = pL[n - 1 - j + k * N_BANKS];
        asm volatile("mul  %[a0],%[a0],%[b0];"
                     "addi %[a0],%[a0],%[h];"
                     "srai  %[a0],%[a0],%[s];"
                     "add  %[sum],%[a0],%[sum];"
                     : [a0] "+&r"(a0), [sum] "+&r"(sum)
                     : [b0] "r"(b0), [s] "I"(FIXED_POINT), [h] "I"(HALF)
                     :);
        break;
      case 0:
        break;
      }
      result = FIX_DIV((pivot - sum), diag);
      pIn[i] = sum_r - result * in;
      pL[n - 1 - i + j * N_BANKS] = result;
    }
  }
}

void mempool_linsolver_q32p_trisolverL(int32_t *pL, int32_t *pIn,
                                       uint32_t core_id, const uint32_t n) {
  int32_t sum;
  uint32_t i, k;
  int32_t a0, a1, a2, a3;
  int32_t b0, b1, b2, b3;
  if (core_id == 0) {
    /* LEFT */
    for (i = n - 1; i < n; i--) {
      sum = pIn[i];
      for (k = n - 1; k >= (4 * (i >> 2U) + 4); k -= 4) {
        a0 = pIn[k];
        a1 = pIn[k - 1];
        a2 = pIn[k - 2];
        a3 = pIn[k - 3];
        b0 = pL[k + i * N_BANKS];
        b1 = pL[(k - 1) + i * N_BANKS];
        b2 = pL[(k - 2) + i * N_BANKS];
        b3 = pL[(k - 3) + i * N_BANKS];
        asm volatile("mul  %[a0],%[a0],%[b0];"
                     "mul  %[a1],%[a1],%[b1];"
                     "mul  %[a2],%[a2],%[b2];"
                     "mul  %[a3],%[a3],%[b3];"
                     "addi %[a0],%[a0],%[h];"
                     "addi %[a1],%[a1],%[h];"
                     "addi %[a2],%[a2],%[h];"
                     "addi %[a3],%[a3],%[h];"
                     "srai %[a0],%[a0],%[s];"
                     "srai %[a1],%[a1],%[s];"
                     "srai %[a2],%[a2],%[s];"
                     "srai %[a3],%[a3],%[s];"
                     "sub  %[sum],%[sum],%[a0];"
                     "sub  %[sum],%[sum],%[a1];"
                     "sub  %[sum],%[sum],%[a2];"
                     "sub  %[sum],%[sum],%[a3];"
                     : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2),
                       [a3] "+&r"(a3), [sum] "+&r"(sum)
                     : [b0] "r"(b0), [b1] "r"(b1), [b2] "r"(b2), [b3] "r"(b3),
                       [s] "I"(FIXED_POINT), [h] "I"(HALF)
                     :);
      }
      switch (i % 4) {
      case 1:
        a0 = pIn[k];
        a1 = pIn[k - 1];
        a2 = pIn[k - 2];
        b0 = pL[k + i * N_BANKS];
        b1 = pL[(k - 1) + i * N_BANKS];
        b2 = pL[(k - 2) + i * N_BANKS];
        asm volatile(
            "mul  %[a0],%[a0],%[b0];"
            "mul  %[a1],%[a1],%[b1];"
            "mul  %[a2],%[a2],%[b2];"
            "addi %[a0],%[a0],%[h];"
            "addi %[a1],%[a1],%[h];"
            "addi %[a2],%[a2],%[h];"
            "srai %[a0],%[a0],%[s];"
            "srai %[a1],%[a1],%[s];"
            "srai %[a2],%[a2],%[s];"
            "sub  %[sum],%[sum],%[a0];"
            "sub  %[sum],%[sum],%[a1];"
            "sub  %[sum],%[sum],%[a2];"
            : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2), [sum] "+&r"(sum)
            : [b0] "r"(b0), [b1] "r"(b1), [b2] "r"(b2), [s] "I"(FIXED_POINT),
              [h] "I"(HALF)
            :);
        break;
      case 2:
        a0 = pIn[k];
        a1 = pIn[k - 1];
        b0 = pL[k + i * N_BANKS];
        b1 = pL[(k - 1) + i * N_BANKS];
        asm volatile(
            "mul  %[a0],%[a0],%[b0];"
            "mul  %[a1],%[a1],%[b1];"
            "addi %[a0],%[a0],%[h];"
            "addi %[a1],%[a1],%[h];"
            "srai %[a0],%[a0],%[s];"
            "srai %[a1],%[a1],%[s];"
            "sub  %[sum],%[sum],%[a0];"
            "sub  %[sum],%[sum],%[a1];"
            : [a0] "+&r"(a0), [a1] "+&r"(a1), [sum] "+&r"(sum)
            : [b0] "r"(b0), [b1] "r"(b1), [s] "I"(FIXED_POINT), [h] "I"(HALF)
            :);
        break;
      case 3:
        a0 = pIn[k];
        b0 = pL[k + i * N_BANKS];
        asm volatile("mul  %[a0],%[a0],%[b0];"
                     "addi %[a0],%[a0],%[h];"
                     "srai %[a0],%[a0],%[s];"
                     "sub  %[sum],%[sum],%[a0];"
                     : [a0] "+&r"(a0), [sum] "+&r"(sum)
                     : [b0] "r"(b0), [s] "I"(FIXED_POINT), [h] "I"(HALF)
                     :);
        break;
      case 0:
        break;
      }
      pIn[i] = FIX_DIV(sum, pL[i * N_BANKS + i]);
    }
  }
}

void mempool_linsolver_q32p_trisolverR(int32_t *pL, int32_t *pIn,
                                       uint32_t core_id, const uint32_t n,
                                       const uint32_t nPE) {
  int32_t sum;
  uint32_t i, k;
  int32_t a0, a1, a2, a3;
  int32_t b0, b1, b2, b3;
  if (core_id == nPE - 1) {
    for (i = n - 1; i < n; i++) {
      sum = pIn[i];
      for (k = 0; k < 4 * ((n - i) >> 2U); k += 4) {
        a0 = pIn[n - 1 - k];
        a1 = pIn[n - 1 - k - 1];
        a2 = pIn[n - 1 - k - 2];
        a3 = pIn[n - 1 - k - 3];
        b0 = pL[k + i * N_BANKS];
        b1 = pL[(k + 1) + i * N_BANKS];
        b2 = pL[(k + 2) + i * N_BANKS];
        b3 = pL[(k + 3) + i * N_BANKS];
        asm volatile("mul  %[a0],%[a0],%[b0];"
                     "mul  %[a1],%[a1],%[b1];"
                     "mul  %[a2],%[a2],%[b2];"
                     "mul  %[a3],%[a3],%[b3];"
                     "addi %[a0],%[a0],%[h];"
                     "addi %[a1],%[a1],%[h];"
                     "addi %[a2],%[a2],%[h];"
                     "addi %[a3],%[a3],%[h];"
                     "srai %[a0],%[a0],%[s];"
                     "srai %[a1],%[a1],%[s];"
                     "srai %[a2],%[a2],%[s];"
                     "srai %[a3],%[a3],%[s];"
                     "sub  %[sum],%[sum],%[a0];"
                     "sub  %[sum],%[sum],%[a1];"
                     "sub  %[sum],%[sum],%[a2];"
                     "sub  %[sum],%[sum],%[a3];"
                     : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2),
                       [a3] "+&r"(a3), [sum] "+&r"(sum)
                     : [b0] "r"(b0), [b1] "r"(b1), [b2] "r"(b2), [b3] "r"(b3),
                       [s] "I"(FIXED_POINT), [h] "I"(HALF)
                     :);
      }
      switch ((n - i) % 4) {
      case 3:
        a0 = pIn[n - 1 - k];
        a1 = pIn[n - 1 - k - 1];
        a2 = pIn[n - 1 - k - 2];
        b0 = pL[k + i * N_BANKS];
        b1 = pL[(k + 1) + i * N_BANKS];
        b2 = pL[(k + 2) + i * N_BANKS];
        asm volatile(
            "mul  %[a0],%[a0],%[b0];"
            "mul  %[a1],%[a1],%[b1];"
            "mul  %[a2],%[a2],%[b2];"
            "addi %[a0],%[a0],%[h];"
            "addi %[a1],%[a1],%[h];"
            "addi %[a2],%[a2],%[h];"
            "srai %[a0],%[a0],%[s];"
            "srai %[a1],%[a1],%[s];"
            "srai %[a2],%[a2],%[s];"
            "sub  %[sum],%[sum],%[a0];"
            "sub  %[sum],%[sum],%[a1];"
            "sub  %[sum],%[sum],%[a2];"
            : [a0] "+&r"(a0), [a1] "+&r"(a1), [a2] "+&r"(a2), [sum] "+&r"(sum)
            : [b0] "r"(b0), [b1] "r"(b1), [b2] "r"(b2), [s] "I"(FIXED_POINT),
              [h] "I"(HALF)
            :);
        break;
      case 2:
        a0 = pIn[n - 1 - k];
        a1 = pIn[n - 1 - k - 1];
        b0 = pL[k + i * N_BANKS];
        b1 = pL[(k + 1) + i * N_BANKS];
        asm volatile(
            "mul  %[a0],%[a0],%[b0];"
            "mul  %[a1],%[a1],%[b1];"
            "addi %[a0],%[a0],%[h];"
            "addi %[a1],%[a1],%[h];"
            "srai %[a0],%[a0],%[s];"
            "srai %[a1],%[a1],%[s];"
            "sub  %[sum],%[sum],%[a0];"
            "sub  %[sum],%[sum],%[a1];"
            : [a0] "+&r"(a0), [a1] "+&r"(a1), [sum] "+&r"(sum)
            : [b0] "r"(b0), [b1] "r"(b1), [s] "I"(FIXED_POINT), [h] "I"(HALF)
            :);
        break;
      case 1:
        a0 = pIn[n - 1 - k];
        b0 = pL[k + i * N_BANKS];
        asm volatile("mul  %[a0],%[a0],%[b0];"
                     "addi %[a0],%[a0],%[h];"
                     "srai %[a0],%[a0],%[s];"
                     "sub  %[sum],%[sum],%[a0];"
                     : [a0] "+&r"(a0), [sum] "+&r"(sum)
                     : [b0] "r"(b0), [s] "I"(FIXED_POINT), [h] "I"(HALF)
                     :);
        break;
      case 0:
        break;
      }
      pIn[i] = FIX_DIV(sum, pL[i * N_BANKS + i]);
    }
  }
}
