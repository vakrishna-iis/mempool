// Copyright 2021 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Marco Bertuletti, ETH Zurich

/* GAUSS JORDAN INVERSION */

int mempool_GJinv_q32s(int32_t *pSrc, int32_t *pDst, uint32_t n);

int mempool_GJinv_q32s(int32_t * pSrc, int32_t * pDst, uint32_t n) {

    int32_t *pSrcT1, *pSrcT2;                    /* Temporary input data matrix pointer */
    int32_t *pDstT1, *pDstT2;                    /* Temporary output data matrix pointer */
    int32_t *pPivotRowIn;                        /* Temporary input and output data matrix pointer */
    int32_t *pPRT_in, *pPivotRowDst, *pPRT_pDst; /* Temporary input and output data matrix pointer */

    int32_t in = 0;
    int32_t Xchg1, Xchg2, Xchg3, Xchg4;
    int32_t in1, in2, in3, in4;
    int32_t out1, out2, out3, out4;

    uint32_t i, rowCnt, j, loopCnt, k, l;        /* loop counters */
    uint32_t flag;
    uint32_t m = n; /* M is the number of rows. However, the matirces must be square. */

    pDstT1 = pDst;  /* Working pointer for destination matrix */
    rowCnt = m;     /* Loop over the number of rows */
    flag = 0U;

    /* CREATE THE IDENTITY MATRIX */

    while (rowCnt > 0U) {
        j = m - rowCnt;
        while (j > 0U) {
            *pDstT1++ = 0;
            j--;
        }
        *pDstT1++ = 1;
        j = rowCnt - 1U;
        while (j > 0U) {
            *pDstT1++ = 0;
            j--;
        }
        rowCnt--;
    }

    /* Loop over the number of columns of the input matrix. */
    loopCnt = n;
    /* Index modifier to navigate through the columns */
    l = 0U;

    while (loopCnt > 0U) {

        pSrcT1 = pSrc + (l * n);
        pDstT1 = pDst + (l * n);
        k = 1U;
        in = *pSrcT1;

        /* CHECK IF PIVOT ELEMENT IS ZERO */

        if (in == 0) {
            /* Loop over the rows present below */
            for (i = (l + 1U); i < m; i++) {
                pSrcT2 = pSrc + (n * i);
                pDstT2 = pDstT1 + (n * k);

                /* EXCHANGE */

                if (*pSrcT2 != 0) {
                    /* Loop over colums to the right of the pivot */
                    j = 0;
                    while (j < (n - l) - (n - l) % 4) {
                        Xchg1 = *(pSrcT2);
                        Xchg2 = *(pSrcT2 + 1);
                        Xchg3 = *(pSrcT2 + 2);
                        Xchg4 = *(pSrcT2 + 3);
                        out1 = *(pSrcT1);
                        out2 = *(pSrcT1 + 1);
                        out3 = *(pSrcT1 + 2);
                        out4 = *(pSrcT1 + 3);
                        *pSrcT2++ = out1;
                        *pSrcT2++ = out2;
                        *pSrcT2++ = out3;
                        *pSrcT2++ = out4;
                        *pSrcT1++ = Xchg1;
                        *pSrcT1++ = Xchg2;
                        *pSrcT1++ = Xchg3;
                        *pSrcT1++ = Xchg4;
                        j += 4;
                    }
                    while (j < n - l) {
                      Xchg1 = *pSrcT2;
                      *pSrcT2++ = *pSrcT1;
                      *pSrcT1++ = Xchg1;
                      j++;
                    }
                    /* Loop over colums */
                    j = 0;
                    while (j < n - n % 4) {
                        Xchg1 = *(pDstT2);
                        Xchg2 = *(pDstT2 + 1);
                        Xchg3 = *(pDstT2 + 2);
                        Xchg4 = *(pDstT2 + 3);
                        out1 = *(pDstT1);
                        out2 = *(pDstT1 + 1);
                        out3 = *(pDstT1 + 2);
                        out4 = *(pDstT1 + 3);
                        *pDstT2++ = out1;
                        *pDstT2++ = out2;
                        *pDstT2++ = out3;
                        *pDstT2++ = out4;
                        *pDstT1++ = Xchg1;
                        *pDstT1++ = Xchg2;
                        *pDstT1++ = Xchg3;
                        *pDstT1++ = Xchg4;
                        j += 4;
                    }
                    while (j < n) {
                        Xchg1 = *pDstT2;
                        *pDstT2++ = *pDstT1;
                        *pDstT1++ = Xchg1;
                        j++;
                    }
                    flag = 1U;
                    break;
                }
                k++;
            }
        }
        /* Return when the matrix is singular */
        if ((flag == 0U) && (in == 0)) {
            return 1;
        }

        /* DIVIDE BY THE PIVOT */

        /* Points to the pivot row of input and destination matrices */
        pPivotRowIn = pSrc + (l * n);
        pPivotRowDst = pDst + (l * n);
        /* Temporary pointers to the pivot row pointers */
        pSrcT1 = pPivotRowIn;
        pSrcT2 = pPivotRowDst;
        /* Pivot element of the row */
        in = *pPivotRowIn;

        /* Loop over number of columns to the right of the pilot element */
        j = 0;
        while (j < 4 * ((n - l) >> 2U)) {
            in1 = *pSrcT1;
            in2 = *(pSrcT1 + 1);
            in3 = *(pSrcT1 + 2);
            in4 = *(pSrcT1 + 3);
            out1 = FIX_DIV(in1, in);
            out2 = FIX_DIV(in2, in);
            out3 = FIX_DIV(in3, in);
            out4 = FIX_DIV(in4, in);
            *pSrcT1++ = out1;
            *pSrcT1++ = out2;
            *pSrcT1++ = out3;
            *pSrcT1++ = out4;
            j += 4;
        }
        while (j < n - l) {
            in1 = *pSrcT1;
            *pSrcT1++ = FIX_DIV(in1, in);
            j++;
        }
        /* Loop over number of columns of the destination matrix */
        j = 0;
        while (j < 4 * (n >> 2U)) {
            in1 = *pSrcT2;
            in2 = *(pSrcT2 + 1);
            in3 = *(pSrcT2 + 2);
            in4 = *(pSrcT2 + 3);
            out1 = FIX_DIV(in1, in);
            out2 = FIX_DIV(in2, in);
            out3 = FIX_DIV(in3, in);
            out4 = FIX_DIV(in4, in);
            *pSrcT2++ = out1;
            *pSrcT2++ = out2;
            *pSrcT2++ = out3;
            *pSrcT2++ = out4;
            j += 4;
        }
        while (j < n) {
            in1 = *pSrcT2;
            *pSrcT2++ = FIX_DIV(in1, in);
            j++;
        }

        /* REPLACE ROWS */

        pSrcT1 = pSrc;
        pSrcT2 = pDst;
        i = 0U; /* pivot index */
        k = m; /* row index */
        while (k > 0U) {
            /* Only the columns to the right of the pivot are to be processed */
            if (i == l) {
                pSrcT1 += n - l;
                pSrcT2 += n;
            } else {
                /* Element of the reference row */
                in = *pSrcT1;
                /* Reference row pointers */
                pPRT_in = pPivotRowIn;
                pPRT_pDst = pPivotRowDst;
                j = 0;
                while (j < 2 * ((n - l) >> 1U)) {
                    in1 = *pSrcT1;
                    in2 = *(pSrcT1 + 1);
                    // in3 = *(pSrcT1 + 2);
                    // in4 = *(pSrcT1 + 3);
                    out1 = *pPRT_in++;
                    out2 = *pPRT_in++;
                    // out3 = *pPRT_in++;
                    // out4 = *pPRT_in++;
                    *pSrcT1++ = in1 - FIX_MUL(in, out1);
                    *pSrcT1++ = in2 - FIX_MUL(in, out2);
                    // *pSrcT1++ = in3 - FIX_MUL(in, out3);
                    // *pSrcT1++ = in4 - FIX_MUL(in, out4);
                    j += 2;
                }
                while (j < n - l) {
                    in1 = *pSrcT1;
                    out1 = *pPRT_in++;
                    *pSrcT1++ = in1 - FIX_MUL(in, out1);
                    j++;
                }
                /* Loop over the number of columns to
                   replace the elements in the destination matrix */
                j = 0;
                while (j < 4 * (n >> 2U)) {
                    in1 = *pSrcT2;
                    in2 = *(pSrcT2 + 1);
                    in3 = *(pSrcT2 + 2);
                    in4 = *(pSrcT2 + 3);
                    out1 = *pPRT_pDst++;
                    out2 = *pPRT_pDst++;
                    out3 = *pPRT_pDst++;
                    out4 = *pPRT_pDst++;
                    *pSrcT2++ = in1 - FIX_MUL(in, out1);
                    *pSrcT2++ = in2 - FIX_MUL(in, out2);
                    *pSrcT2++ = in3 - FIX_MUL(in, out3);
                    *pSrcT2++ = in4 - FIX_MUL(in, out4);
                    j += 4;
                }
                while (j < n) {
                    in1 = *pSrcT2;
                    out1 = *pPRT_pDst;
                    *pSrcT2++ = in1 - FIX_MUL(in, out1);
                    j++;
                }
            }
            /* Increment temporary input pointer */
            pSrcT1 = pSrcT1 + l;
            /* Decrement loop counter */
            k--;
            /* Increment pivot index */
            i++;
        }

        pSrc++; /* Increment the input pointer */
        loopCnt--; /* Decrement the loop counter */
        l++; /* Increment the index modifier */
    }

//    if ((flag != 1U) && (in == 0)) {
//        for (i = 0; i < m * n; i++) {
//            if (pSrc[i] != 0)
//                break;
//        }
//        if (i == m * n)
//            return 1;
//    }

    return 0;
}
 
