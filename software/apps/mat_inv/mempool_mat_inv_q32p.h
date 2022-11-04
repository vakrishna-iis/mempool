// Copyright 2021 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Marco Bertuletti, ETH Zurich

/* GAUSS JORDAN INVERSION */

int mempool_GJinv_q32p(int32_t *pSrc, int32_t *pDst, uint32_t n,
                       uint32_t *flag);

int mempool_GJinv_q32p(int32_t *pSrc, int32_t *pDst, uint32_t n,
                       uint32_t *flag) {

  int32_t *pSrcT1, *pSrcT2; /* Temporary input data matrix pointer */
  int32_t *pDstT1, *pDstT2; /* Temporary output data matrix pointer */
  int32_t *pPivotRowIn;     /* Temporary input and output data matrix pointer */
  int32_t *pPRT_in, *pPivotRowDst,
      *pPRT_pDst; /* Temporary input and output data matrix pointer */

  int32_t in = 0;
  int32_t Xchg1, Xchg2, Xchg3, Xchg4;
  int32_t in1, in2, in3, in4;
  int32_t out1, out2, out3, out4;

  uint32_t core_id = mempool_get_core_id();
  uint32_t i, j, loopCnt, k, l; /* loop counters */
  uint32_t m =
      n; /* M is the number of rows. However, the matirces must be square. */

  /* CREATE THE IDENTITY MATRIX */

  pDstT1 = pDst;
  for (k = core_id * 4; k < m; k += 4 * NUM_CORES) {
    for (j = 0; j < m; j++) {
      pDstT1[k * m + j] = (uint32_t)(k == j);
      pDstT1[(k + 1) * m + j] = (uint32_t)((k + 1) == j);
      pDstT1[(k + 2) * m + j] = (uint32_t)((k + 2) == j);
      pDstT1[(k + 3) * m + j] = (uint32_t)((k + 3) == j);
    }
  }
  mempool_log_partial_barrier(2, core_id, MIN(NUM_CORES, n / 4));

  /* Loop over the number of columns of the input matrix. */
  loopCnt = n;
  /* Index modifier to navigate through the columns */
  l = 0U;

  while (loopCnt > 0U) {

    pSrcT1 = pSrc + (l * n);
    pDstT1 = pDst + (l * n);
    in = *pSrcT1;

    /* CHECK IF PIVOT ELEMENT IS ZERO */
    if (core_id == 0) {
      if (in == 0U) {
        /* Loop over the rows present below */
        for (k = l + 1U; k < m; k++) {
          pSrcT2 = pSrc + (n * k);
          pDstT2 = pDst + (n * k);
          /* EXCHANGE */
          if (*pSrcT2 != 0) {
            /* Loop over colums to the right of the pivot */
            j = 0;
            while (j < 4 * ((n - l) >> 2U)) {
              Xchg1 = pSrcT2[j];
              Xchg2 = pSrcT2[j + 1];
              Xchg3 = pSrcT2[j + 2];
              Xchg4 = pSrcT2[j + 3];
              out1 = pSrcT1[j];
              out2 = pSrcT1[j + 1];
              out3 = pSrcT1[j + 2];
              out4 = pSrcT1[j + 3];
              pSrcT2[j] = out1;
              pSrcT2[j + 1] = out2;
              pSrcT2[j + 2] = out3;
              pSrcT2[j + 3] = out4;
              pSrcT1[j] = Xchg1;
              pSrcT1[j + 1] = Xchg2;
              pSrcT1[j + 2] = Xchg3;
              pSrcT1[j + 3] = Xchg4;
              j += 4;
            }
            while (j < n - l) {
              Xchg1 = pSrcT2[j];
              pSrcT2[j] = pSrcT1[j];
              pSrcT1[j] = Xchg1;
              j++;
            }
            /* Loop over colums */
            j = 0;
            while (j < 4 * (n >> 2U)) {
              Xchg1 = pDstT2[j];
              Xchg2 = pDstT2[j + 1];
              Xchg3 = pDstT2[j + 2];
              Xchg4 = pDstT2[j + 3];
              out1 = pDstT1[j];
              out2 = pDstT1[j + 1];
              out3 = pDstT1[j + 2];
              out4 = pDstT1[j + 3];
              pDstT2[j] = out1;
              pDstT2[j + 1] = out2;
              pDstT2[j + 2] = out3;
              pDstT2[j + 3] = out4;
              pDstT1[j] = Xchg1;
              pDstT1[j + 1] = Xchg2;
              pDstT1[j + 2] = Xchg3;
              pDstT1[j + 3] = Xchg4;
              j += 4;
            }
            while (j < n) {
              Xchg1 = pDstT2[j];
              pDstT2[j] = pDstT1[j];
              pDstT1[j] = Xchg1;
              j++;
            }
            *flag = 1U;
            break;
          }
        }
      }
      /* Update the status if the matrix is singular */
      if ((*flag == 0U) && (in == 0U)) {
        return 1;
      }
    }
    mempool_log_partial_barrier(2, core_id, MIN(NUM_CORES, n / 4));

    /* DIVIDE BY THE PIVOT */
    /* Points to the pivot row of input and destination matrices */
    pPivotRowIn = pSrc + (l * n);
    pPivotRowDst = pDst + (l * n);
    /* Temporary pointers to the pivot row pointers */
    pSrcT1 = pPivotRowIn;
    pSrcT2 = pPivotRowDst;
    /* Pivot element of the row */
    in = *pPivotRowIn;

    ///* Loop over columns to the right of pivot */
    for (j = core_id * 4; j < 4 * ((n - l) >> 2U); j += NUM_CORES * 4) {
      in1 = pSrcT1[j];
      in2 = pSrcT1[j + 1];
      in3 = pSrcT1[j + 2];
      in4 = pSrcT1[j + 3];
      out1 = FIX_DIV(in1, in);
      out2 = FIX_DIV(in2, in);
      out3 = FIX_DIV(in3, in);
      out4 = FIX_DIV(in4, in);
      pSrcT1[j] = out1;
      pSrcT1[j + 1] = out2;
      pSrcT1[j + 2] = out3;
      pSrcT1[j + 3] = out4;
      // j += NUM_CORES * 4;
    }
    if (core_id == (n >> 2U) - 1) {
      j = 4 * ((n - l) >> 2U);
      while (j < n - l) {
        in1 = pSrcT1[j];
        pSrcT1[j] = FIX_DIV(in1, in);
        j++;
      }
    }
    /* Loop over columns */
    for (j = core_id * 4; j < 4 * (n >> 2U); j += NUM_CORES * 4) {
      in1 = pSrcT2[j];
      in2 = pSrcT2[j + 1];
      in3 = pSrcT2[j + 2];
      in4 = pSrcT2[j + 3];
      out1 = FIX_DIV(in1, in);
      out2 = FIX_DIV(in2, in);
      out3 = FIX_DIV(in3, in);
      out4 = FIX_DIV(in4, in);
      pSrcT2[j] = out1;
      pSrcT2[j + 1] = out2;
      pSrcT2[j + 2] = out3;
      pSrcT2[j + 3] = out4;
    }
    if (core_id == (n >> 2U) - 1) {
      j = 4 * (n >> 2U);
      while (j < n) {
        in1 = pSrcT2[j];
        pSrcT2[j] = FIX_DIV(in1, in);
        j++;
      }
    }
    mempool_log_partial_barrier(2, core_id, MIN(NUM_CORES, n / 4));

    /* REPLACE ROWS */
    pSrcT1 = pSrc;
    pSrcT2 = pDst;
    /* Loop over rows */
    for (k = core_id * 4; k < m; k += NUM_CORES * 4) {
      i = 0U;
      while (i < 4) {
        if ((i + k) != l) {
          pSrcT1 = pSrc + (i + k) * n;
          pSrcT2 = pDst + (i + k) * n;
          /* Element of the reference row */
          in = *pSrcT1;
          pPRT_in = pPivotRowIn;
          pPRT_pDst = pPivotRowDst;
          /* Loop over columns to the right of pivot */
          j = 0;
          while (j < 4 * ((n - l) >> 2U)) {
            in1 = pSrcT1[j];
            in2 = pSrcT1[j + 1];
            in3 = pSrcT1[j + 2];
            in4 = pSrcT1[j + 3];
            out1 = pPRT_in[j];
            out2 = pPRT_in[j + 1];
            out3 = pPRT_in[j + 2];
            out4 = pPRT_in[j + 3];
            pSrcT1[j] = in1 - FIX_MUL(in, out1);
            pSrcT1[j + 1] = in2 - FIX_MUL(in, out2);
            pSrcT1[j + 2] = in3 - FIX_MUL(in, out3);
            pSrcT1[j + 3] = in4 - FIX_MUL(in, out4);
            j += 4;
          }
          while (j < n - l) {
            in1 = pSrcT1[j];
            out1 = pPRT_in[j];
            pSrcT1[j] = in1 - FIX_MUL(in, out1);
            j++;
          }
          /* Loop over columns */
          j = 0;
          while (j < 4 * (n >> 2U)) {
            in1 = pSrcT2[j];
            in2 = pSrcT2[j + 1];
            in3 = pSrcT2[j + 2];
            in4 = pSrcT2[j + 3];
            out1 = pPRT_pDst[j];
            out2 = pPRT_pDst[j + 1];
            out3 = pPRT_pDst[j + 2];
            out4 = pPRT_pDst[j + 3];
            pSrcT2[j] = in1 - FIX_MUL(in, out1);
            pSrcT2[j + 1] = in2 - FIX_MUL(in, out2);
            pSrcT2[j + 2] = in3 - FIX_MUL(in, out3);
            pSrcT2[j + 3] = in4 - FIX_MUL(in, out4);
            j += 4;
          }
          while (j < n) {
            in1 = pSrcT2[j];
            out1 = pPRT_pDst[j];
            pSrcT2[j] = in1 - FIX_MUL(in, out1);
            j++;
          }
        }
        i++;
      }
    }
    mempool_log_partial_barrier(2, core_id, MIN(NUM_CORES, n / 4));

    //        /* REPLACE ROWS */
    //        pSrcT1 = pSrc;
    //        pSrcT2 = pDst;
    //        /* Loop over rows */
    //        for (k = 0; k < m; k++) {
    //            if (k != l) {
    //                pSrcT1 = pSrc + k * n;
    //                pSrcT2 = pDst + k * n;
    //                /* Element of the reference row */
    //                in = *pSrcT1;
    //                pPRT_in = pPivotRowIn;
    //                pPRT_pDst = pPivotRowDst;
    //                /* Loop over columns to the right of pivot */
    //                j = core_id * 4;
    //                // j = core_id * 4 > 4 * (l >> 2U) ? core_id * 4 : 4 * ((n
    //                - l) >> 2U); while (j < 4 * ((n - l) >> 2U)) {
    //                    in1 = pSrcT1[j];
    //                    in2 = pSrcT1[j + 1];
    //                    in3 = pSrcT1[j + 2];
    //                    in4 = pSrcT1[j + 3];
    //                    out1 = pPRT_in[j];
    //                    out2 = pPRT_in[j + 1];
    //                    out3 = pPRT_in[j + 2];
    //                    out4 = pPRT_in[j + 3];
    //                    pSrcT1[j]     = in1 - FIX_MUL(in, out1);
    //                    pSrcT1[j + 1] = in2 - FIX_MUL(in, out2);
    //                    pSrcT1[j + 2] = in3 - FIX_MUL(in, out3);
    //                    pSrcT1[j + 3] = in4 - FIX_MUL(in, out4);
    //                    j += 4 * NUM_CORES;
    //                }
    //                if (core_id == (n >> 2U) - 1) {
    //                    j = 4 * ((n - l) >> 2U);
    //                    while (j < n - l) {
    //                        in1 = pSrcT1[j];
    //                        out1 = pPRT_in[j];
    //                        pSrcT1[j] = in1 - FIX_MUL(in, out1);
    //                        j++;
    //                    }
    //                }
    //                /* Loop over columns */
    //                j = core_id * 4;
    //                while (j < 4 * (n >> 2U)) {
    //                    in1 = pSrcT2[j];
    //                    in2 = pSrcT2[j + 1];
    //                    in3 = pSrcT2[j + 2];
    //                    in4 = pSrcT2[j + 3];
    //                    out1 = pPRT_pDst[j];
    //                    out2 = pPRT_pDst[j + 1];
    //                    out3 = pPRT_pDst[j + 2];
    //                    out4 = pPRT_pDst[j + 3];
    //                    pSrcT2[j]     = in1 - FIX_MUL(in, out1);
    //                    pSrcT2[j + 1] = in2 - FIX_MUL(in, out2);
    //                    pSrcT2[j + 2] = in3 - FIX_MUL(in, out3);
    //                    pSrcT2[j + 3] = in4 - FIX_MUL(in, out4);
    //                    j += 4 * NUM_CORES;
    //                }
    //                if (core_id == (n >> 2U) - 1) {
    //                    j = 4 * (n >> 2U);
    //                    while (j < n) {
    //                        in1 = pSrcT2[j];
    //                        out1 = pPRT_pDst[j];
    //                        pSrcT2[j] = in1 - FIX_MUL(in, out1);
    //                        j++;
    //                    }
    //                }
    //                mempool_log_partial_barrier(2, core_id, MIN(NUM_CORES, n /
    //                4));
    //            }
    //        }
    //        mempool_log_partial_barrier(2, core_id, MIN(NUM_CORES, n / 4));

    pSrc++;    /* Increment the input pointer */
    loopCnt--; /* Decrement the loop counter */
    l++;       /* Increment the index modifier */
  }

  //    if ((flag != 1U) && (x == 0)) {
  //        for (i = 0; i < m * n; i++) {
  //            if (pSrc[i] != 0)
  //                break;
  //        }
  //        if (i == m * n)
  //            return 1;
  //    }
  return 0;
}
