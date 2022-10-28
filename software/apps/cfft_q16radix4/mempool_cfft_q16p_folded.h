// Copyright 2022 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Marco Bertuletti, ETH Zurich

#ifdef FOLDED_TWIDDLES
static void mempool_cfft_q16p_folded(uint16_t fftLen, int16_t *pCoef_src,
                                     int16_t *pCoef_dst, uint16_t *pBitRevTable,
                                     int16_t *pSrc16, int16_t *pDst16,
                                     uint16_t bitReverseLen, uint8_t ifftFlag,
                                     uint8_t bitReverseFlag, uint32_t nPE);
#else
static void mempool_cfft_q16p_folded(uint16_t fftLen, int16_t *pCoef_src,
                                     uint16_t *pBitRevTable, int16_t *pSrc16,
                                     int16_t *pDst16, uint16_t bitReverseLen,
                                     uint8_t ifftFlag, uint8_t bitReverseFlag,
                                     uint32_t nPE);
#endif

#ifdef FOLDED_TWIDDLES
static void mempool_cfft_q16p_folded(uint16_t fftLen, int16_t *pCoef_src,
                                     int16_t *pCoef_dst, uint16_t *pBitRevTable,
                                     int16_t *pSrc16, int16_t *pDst16,
                                     uint16_t bitReverseLen, uint8_t ifftFlag,
                                     uint8_t bitReverseFlag, uint32_t nPE)

#else
static void mempool_cfft_q16p_folded(uint16_t fftLen, int16_t *pCoef,
                                     uint16_t *pBitRevTable, int16_t *pSrc16,
                                     int16_t *pDst16, uint16_t bitReverseLen,
                                     uint8_t ifftFlag, uint8_t bitReverseFlag,
                                     uint32_t nPE)
#endif
{
  if (ifftFlag == 0) {
#ifdef FOLDED_TWIDDLES
    mempool_butterfly_q16p_folded(pSrc16, pDst16, fftLen, pCoef_src, pCoef_dst,
                                  nPE);
#else
    mempool_butterfly_q16p_folded(pSrc16, pDst16, fftLen, pCoef, nPE);
#endif
  }
  if (bitReverseFlag) {
#ifndef BITREVERSETABLE
    mempool_bitrev_q16p_xpulpimg((uint16_t *)pDst16, (uint16_t *)pSrc16, fftLen,
                                 nPE);
#else
    mempool_bitrev_q16p_xpulpimg((uint16_t *)pDst16, bitReverseLen,
                                 pBitRevTable, nPE);
#endif
  }
}