//
// Created by Ahmed Abou-Aliaa on 18.02.20.
//
#include "dmatrix.h"
#include "log.h"
#include <gtest/gtest.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wglobal-constructors"

TEST(dmatrix_test, constat_value) { // NOLINT
  DMATRIX *md;
  MATRIX *mf;
  double v;

  mf = MatrixConstVal(14, 7, 2, nullptr);  // NOLINT (magic numbers)
  md = DMatrixConstVal(14, 7, 2, nullptr); // NOLINT (magic numbers)

  EXPECT_NE(md, nullptr);
  EXPECT_NE(mf, nullptr);

  v = DMatrixCompareFMatrix(mf, md);

  EXPECT_LT(v, .0001);

  MatrixFree(&mf);
  DMatrixFree(&md);
}

TEST(dmatrix_test, dmatrix_add) { // NOLINT
  DMATRIX *m1d;
  DMATRIX *m2d;
  DMATRIX *md;
  MATRIX *m1f;
  MATRIX *m2f;
  MATRIX *mf;
  double v;

  m1f = MatrixDRand48(14, 7, nullptr); // NOLINT
  m2f = MatrixDRand48(14, 7, nullptr); // NOLINT
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  m2d = DMatrixCopyFMatrix(m2f, nullptr);

  mf = MatrixAdd(m1f, m2f, nullptr);
  md = DMatrixAdd(m1d, m2d, nullptr);

  EXPECT_NE(md, nullptr);
  v = DMatrixCompareFMatrix(mf, md);
  EXPECT_LT(v, .0001);
}

TEST(dmatrix_test, dmatrix_subtract) { // NOLINT
  DMATRIX *m1d;
  DMATRIX *m2d;
  DMATRIX *md;
  MATRIX *m1f;
  MATRIX *m2f;
  MATRIX *mf;
  double v;

  m1f = MatrixDRand48(14, 7, nullptr); // NOLINT
  m2f = MatrixDRand48(14, 7, nullptr); // NOLINT
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  m2d = DMatrixCopyFMatrix(m2f, nullptr);

  mf = MatrixSubtract(m1f, m2f, nullptr);
  md = DMatrixSubtract(m1d, m2d, nullptr);

  EXPECT_NE(md, nullptr);
  v = DMatrixCompareFMatrix(mf, md);
  EXPECT_LT(v, .0001);
}

TEST(dmatrix_test, dmatrix_scalar_mult) { // NOLINT
  DMATRIX *m1d;
  DMATRIX *m2d;
  DMATRIX *md;
  MATRIX *m1f;
  MATRIX *m2f;
  MATRIX *mf;
  double v;

  m1f = MatrixDRand48(14, 7, nullptr); // NOLINT
  m2f = MatrixDRand48(14, 7, nullptr); // NOLINT
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  m2d = DMatrixCopyFMatrix(m2f, nullptr);

  mf = MatrixScalarMul(m1f, static_cast<float>(2.3), nullptr); // NOLINT
  md = DMatrixScalarMul(m1d, 2.3, nullptr);                    // NOLINT

  EXPECT_NE(md, nullptr);
  v = DMatrixCompareFMatrix(mf, md);
  EXPECT_LT(v, .0001);
}

TEST(dmatrix_test, dmatrix_multiply) { // NOLINT
  DMATRIX *m1d;
  DMATRIX *m2d;
  DMATRIX *md;
  MATRIX *m1f;
  MATRIX *m2f;
  MATRIX *mf;
  double v;
  m1f = MatrixDRand48(7, 14, nullptr); // NOLINT
  m2f = MatrixDRand48(14, 7, nullptr); // NOLINT
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  m2d = DMatrixCopyFMatrix(m2f, nullptr);

  mf = MatrixMultiplyD(m1f, m2f, nullptr);
  md = DMatrixMultiply(m1d, m2d, nullptr);

  EXPECT_NE(md, nullptr);
  v = DMatrixCompareFMatrix(mf, md);
  EXPECT_LT(v, .0001);
}

TEST(dmatrix_test, dmatrix_transpose) { // NOLINT
  DMATRIX *m1d;
  DMATRIX *m2d;
  DMATRIX *md = nullptr;
  MATRIX *m1f;
  MATRIX *m2f;
  MATRIX *mf = nullptr;
  double v;

  m1f = MatrixDRand48(7, 14, nullptr); // NOLINT
  m2f = MatrixDRand48(14, 7, nullptr); // NOLINT
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  m2d = DMatrixCopyFMatrix(m2f, nullptr);

  mf = MatrixTranspose(m1f, mf);
  md = DMatrixTranspose(m1d, md);

  EXPECT_NE(md, nullptr);
  v = DMatrixCompareFMatrix(mf, md);
  EXPECT_LT(v, .0001);

  DMatrixFree(&md);
  MatrixFree(&mf);

  mf = MatrixTranspose(m2f, mf);
  md = DMatrixTranspose(m2d, md);

  EXPECT_NE(md, nullptr);
  v = DMatrixCompareFMatrix(mf, md);
  EXPECT_LT(v, .0001);
}

TEST(dmatrix_test, dmatrix_vector_cross) { // NOLINT
  DMATRIX *m1d;
  DMATRIX *m2d;
  DMATRIX *md;
  MATRIX *m1f;
  MATRIX *m2f;
  MATRIX *mf;
  double v;
  m1f = MatrixDRand48(3, 1, nullptr);
  m2f = MatrixDRand48(3, 1, nullptr);
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  m2d = DMatrixCopyFMatrix(m2f, nullptr);

  mf = VectorCrossProduct(m1f, m2f, nullptr);
  md = DVectorCrossProduct(m1d, m2d, nullptr);

  EXPECT_NE(md, nullptr);
  v = DMatrixCompareFMatrix(mf, md);
  EXPECT_LT(v, .0001);
}

TEST(dmatrix_test, dmatrix_vector_dot) { // NOLINT
  DMATRIX *m1d;
  DMATRIX *m2d;
  MATRIX *m1f;
  MATRIX *m2f;
  double v;
  float vf;
  double vd;
  m1f = MatrixDRand48(3, 1, nullptr);
  m2f = MatrixDRand48(3, 1, nullptr);
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  m2d = DMatrixCopyFMatrix(m2f, nullptr);

  vf = VectorDot(m1f, m2f);
  vd = DVectorDot(m1d, m2d);

  EXPECT_GE(vd, 0);
  v = fabs(static_cast<double>(vf) - vd);
  EXPECT_LT(v, .0001);
}

TEST(dmatrix_test, dmatrix_vector_length) { // NOLINT
  DMATRIX *m1d;
  DMATRIX *m2d;
  MATRIX *m1f;
  MATRIX *m2f;
  double v;
  double vf;
  double vd;
  m1f = MatrixDRand48(3, 1, nullptr);
  m2f = MatrixDRand48(3, 1, nullptr);
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  m2d = DMatrixCopyFMatrix(m2f, nullptr);

  vf = static_cast<double>(VectorLen(m1f));
  vd = DVectorLen(m1d);

  EXPECT_GE(vd, 0);
  v = fabs(vf - vd);
  EXPECT_LT(v, .0001);
}

TEST(dmatrix_test, dmatrix_free_null) { // NOLINT
  DMATRIX *temp;
  temp = nullptr;
  EXPECT_EQ(DMatrixFree(nullptr), ERROR_BADPARM);
  EXPECT_EQ(DMatrixFree(&temp), 0);
}

TEST(dmatrix_test, dmatrix_check_dims) { // NOLINT
  DMATRIX *temp;
  DMATRIX *temp1;
  MATRIX *m1f;
  DMATRIX *m1d;
  temp = nullptr;
  temp1 = nullptr;
  int res = DMatrixCheckDims(temp, temp1, 3, stdout, "");
  EXPECT_EQ(res, 1);
  m1f = MatrixDRand48(3, 1, nullptr);
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  res = DMatrixCheckDims(m1d, temp1, 3, stdout, "");
  EXPECT_EQ(res, 1);
  temp1 = DMatrixCopyFMatrix(m1f, nullptr);
  m1f = MatrixDRand48(4, 1, nullptr);
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  res = DMatrixCheckDims(m1d, temp1, 3, stdout, "");
  EXPECT_EQ(res, 1);
  res = DMatrixCheckDims(m1d, temp1, 1, stdout, "");
  EXPECT_EQ(res, 1);
  temp1 = DMatrixCopyFMatrix(m1f, nullptr);
  m1f = MatrixDRand48(4, 4, nullptr);
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  res = DMatrixCheckDims(m1d, temp1, 3, stdout, "");
  EXPECT_EQ(res, 1);
  res = DMatrixCheckDims(m1d, temp1, 2, stdout, "");
  EXPECT_EQ(res, 1);
  temp1 = DMatrixCopyFMatrix(m1f, nullptr);
  m1f = MatrixDRand48(5, 5, nullptr);
  m1d = DMatrixCopyFMatrix(m1f, nullptr);
  res = DMatrixCheckDims(m1d, temp1, 4, stdout, "");
  EXPECT_EQ(res, 1);

  EXPECT_EQ(DMatrixFree(&temp), 0);
}

auto main(int argc, char *argv[]) -> int { // NOLINT (unused parameters)

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
#pragma GCC diagnostic push
