// Copyright 2021 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Marco Bertuletti, ETH Zurich

#define FIXED_POINT 16
#define FIX_DIV(a,b) ((int32_t)((a << FIXED_POINT) / b))
#define FIX_MUL(a,b) ((int32_t)((a * b) >> FIXED_POINT))
#define MIN(a,b) (a < b ? a : b)

void Transpose(int32_t *matrix, int32_t *t_matrix, int32_t n, int32_t m);

void MatrixMult(int32_t *matrix_1,  int32_t *matrix_2,  int32_t *matrix_product, int32_t n, int32_t m, int32_t o);

void init_matrix(int32_t  *matrix, uint32_t num_rows, uint32_t num_columns, int32_t a, int32_t b, int32_t c, uint32_t core_id);

void init_matrix_zeros(int32_t  *matrix, uint32_t num_rows, uint32_t num_columns, uint32_t core_id);


void Transpose(int32_t *matrix,  int32_t *t_matrix, int32_t n, int32_t m) {
  int32_t i, j;
  for (i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
          t_matrix[j * n + i] = matrix[i * m + j];
      }
  }
}

void MatrixMult(int32_t *matrix_1,  int32_t *matrix_2,  int32_t *matrix_product, int32_t n, int32_t m, int32_t o) {
  int32_t i, j, k;
  for (i = 0; i < n; i++) {
      for (j = 0; j < o; j++) {
        matrix_product[i * o + j] = 0;
        for (k = 0; k < m; k++) {
          matrix_product[i * o + j] += FIX_MUL(matrix_1[i * m + k], matrix_2[k * o + j]);
      }
    }
  }
}

void init_matrix(int32_t  *matrix, uint32_t num_rows, uint32_t num_columns, int32_t a, int32_t b, int32_t c, uint32_t core_id) {
  if(core_id == 0) {
    for(uint32_t j = 0; j < num_rows; j++) {
      for(uint32_t i = 0; i < num_columns; i++) {
          matrix[j * num_columns + i] = a * (int32_t)i + b * (int32_t)j + c;
      }
    }
  }
}


void init_matrix_zeros (int32_t  *matrix, uint32_t num_rows, uint32_t num_columns, uint32_t core_id) {
  if(core_id == 0) {
    for(uint32_t i = 0; i < num_columns; i++) {
      for(uint32_t j = 0; j < num_rows; j++) {
          matrix[j * num_columns + i] = 0;
      }
    }
  }
}
