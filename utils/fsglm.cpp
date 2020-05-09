/*
 *
 * Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
 *
 * Terms and conditions for use, reproduction, distribution and contribution
 * are found in the 'FreeSurfer Software License Agreement' contained
 * in the file 'LICENSE' found in the FreeSurfer distribution, and here:
 *
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
 *
 * Reporting: freesurfer@nmr.mgh.harvard.edu
 *
 */

// fsglm.c - routines to perform GLM analysis.
/*
  y = X*beta + n;                      Forward Model
  beta = inv(X'*X)*X'*y;               Fit beta
  dof = Xrows-Xcols                    Degrees of Freedom
  yhat = X*beta;                       Signal Estimate
  eres = y - yhat;                     Residual Error
  rvar = eres'*eres/dof;               Residual Error Variance
  gamma = C*beta;                      Contrast
  gammacvm = J*rvar*(C*inv(X'*X)*C');  Covariance Matrix of gamma,
  J = rows in C
  F = gamma' * inv(gammacvm) * gamma;  F ratio
  p = FTest(F,dof1,dof2);              p-value for F-test
  Mpmf = C'*inv(C*C')*C;               Partial model fit matrix
  ypmf = X*Mpmf*beta;                  Partial model fit

  While all these operations could be perfomed within one function, they
  have been separated into several functions based on the assumption
  that they will be used in certain ways, mainly to perform massive
  univariate analysis (ie, many different y's). In all cases, we will
  assume that the contrast matrices will be the same for each y. However,
  the X may be:

  1. X fixed for all y - in this case, most of the matrices above only need
  to be computed once and then applied over and over again. This can
  save a lot of time, eg, inv(X'*X) only needs to be computed once.

  2. X different for each y - this can happen if each voxel has some
  voxel-dependent regressors or if X is fixed,
  but there is voxel-dependent
  weighting (which effectively changes X for each y).

  The software has been written in a way that both these situations
  can be run efficiently by not making the same calculations many
  times and by not having to allocate and free matrices many
  times.

  Memory management: pointers to all matrices (intermediate and final)
  are maintained inside the GLMMAT structure.  The caller allocates y,
  X, and the Cs ONCE. The functions will allocate the rest of the
  matrices ONCE on the first pass through. Subsequent passes do not
  require re-allocation. As a result, the caller is not allowed to
  change the sizes of y, X, and the Cs after the first pass. Note:
  there is a function called GLMalloc(). This only allocates the GLMMAT
  structure and assures that all MATRIX pointers are NULL. GLMfree()
  will free all the matrices.

  Workflow 0: Easiest
  1. Allocate GLMMAT: glm = GLMalloc();
  2. Allocate and fill design matrix (glm->X)
  3. Allocate and fill input vector (glm->y)
  4. Set number of contrasts (glm->ncontrasts)
  5. Allocate and fill each contrast matrix (glm->C[n])
  6. Run GLManalyze(glm)
  7. Extract results
  8. GLMfree(&glm);


  Workflow 1: X fixed for all y
  1. Allocate GLMMAT: glm = GLMalloc();
  2. Allocate and fill contrast matrices: glm->C[n] = YourConMatrix
  3. GLMcMatrices(glm) - computes the "intermediate" contrast matrices.
  These are matrices that are not dependent on y and X
  eg, C', C*C', inv(C*C'), C'*inv(C*C'), and Mpmf = C'*inv(C*C')*C.
  4. Allocate and fill design matrix: glm->X = YourDesignMatrix
  5. GLMallocY(glm) - Allocates y based on number of rows in X
  6. GLMxMatrices(glm) - computes "intermediate" matrices
  dependent upon X and C
  but independent of y:
  Eg, X', X'*X, inv(X'*X), C*inv(X'*X), C*inv(X'*X)*C'.
  For each voxel:
  7. Matrix glm->y filled by caller (eg, MRIglmLoadVox())
  8. GLMfit(glm) - computes results dependent upon X and y (but not C):
  X'*y, beta, yhat, eres, rvar.
  9. GLMtest(glm) - computes all results dependent upon X, y, and C:
  gamma, gammacvm, F, p, ypmf.
  10. Save your results
  End voxel loop
  11. GLMfree(&glm);

  Workflow 2: different X for each y
  1. Allocate GLMMAT: glm = GLMalloc();
  2. Allocate and fill contrast matrices: glm->C[n] = YourContrastMatrix
  3. GLMcMatrices(glm) - computes the "intermediate" contrast matrices.
  These are
  matrices that are not dependent on y and X (eg, C', C*C', inv(C*C'),
  C'*inv(C*C'), and Mpmf = C'*inv(C*C')*C.
  4. GLMallocX(glm,nrows,ncols) - Allocates design matrix
  5. GLMallocY(glm) - Allocates y based on number of rows in X
  For each voxel:
  6. Fill design matrix: MatrixCopy(YourVoxelDesignMatrix,glm->X)
  7. GLMxMatrices(glm) - computes "intermediate" matrices
  dependent upon X and C:
  Eg, X', X'*X, inv(X'*X), C*inv(X'*X), C*inv(X'*X)*C'.
  8. Matrix glm->y filled by caller (eg, MRIglmLoadVox())
  9. GLMfit(glm) - computes results dependent upon X and y (but not C):
  X'*y, beta, yhat, eres, rvar.
  10. GLMtest(glm) - computes all results dependent upon X, y, and C:
  gamma, gammacvm, F, p, ypmf.
  11. Save your results
  End voxel loop
  12. GLMfree(&glm);

  Notes:
  1. Any weighting of y and X must be done prior to GLMfit().

*/

#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <sys/stat.h>

#include "diag.h"
#include "fsglm.h"
#include "numerics.h"
#include "randomfields.h"
#include "timer.h"
#include "utils.h"
#undef X

/*------------------------------------------------------------
  GLManalyze() - fill y, X, ncontrasts, and C in the glm
  and run this to fit and test.
  -----------------------------------------------------------*/
int GLManalyze(GLMMAT *glm) {
  GLMcMatrices(glm);
  GLMxMatrices(glm);
  GLMfit(glm);
  GLMtest(glm);
  return (0);
}

/*-------------------------------------------------------
  GLMalloc - allocs a GLMMAT struct and makes sure
  that everything is either NULL or 0. Does not alloc any
  of the matrices (that's done by GLMfit(), GLMtest(), and
  GLMcMatrices(), and GLMxMatrices.
  -------------------------------------------------------*/
GLMMAT *GLMalloc() {
  int     n;
  GLMMAT *glm;

  glm                = (GLMMAT *)calloc(sizeof(GLMMAT), 1);
  glm->y             = nullptr;
  glm->X             = nullptr;
  glm->beta          = nullptr;
  glm->yhat          = nullptr;
  glm->eres          = nullptr;
  glm->rvar          = 0;
  glm->dof           = 0;
  glm->AllowZeroDOF  = 0;
  glm->ill_cond_flag = 0;
  glm->ReScaleX      = 0;

  glm->yffxvar = nullptr;
  glm->ffxdof  = 0;

  glm->Xt   = nullptr;
  glm->XtX  = nullptr;
  glm->iXtX = nullptr;
  glm->Xty  = nullptr;

  glm->DoPCC      = 0;
  glm->ncontrasts = 0;

  for (n = 0; n < GLMMAT_NCONTRASTS_MAX; n++) {
    glm->C[n]         = nullptr;
    glm->Cname[n]     = nullptr;
    glm->Ccond[n]     = -1;
    glm->gamma0[n]    = nullptr;
    glm->UseGamma0[n] = 0;

    glm->Mpmf[n]     = nullptr;
    glm->ypmfflag[n] = 0;
    glm->ypmf[n]     = nullptr;

    glm->gamma[n] = nullptr;
    glm->gCVM[n]  = nullptr;

    glm->F[n]   = 0;
    glm->p[n]   = 0;
    glm->z[n]   = 0;
    glm->pcc[n] = 0;

    glm->Ct[n]      = nullptr;
    glm->CiXtX[n]   = nullptr;
    glm->CiXtXCt[n] = nullptr;

    glm->XCt[n] = nullptr;
    glm->Dt[n]  = nullptr;
    glm->XDt[n] = nullptr;
    glm->RD[n]  = nullptr;

    glm->igCVM[n]   = nullptr;
    glm->gammat[n]  = nullptr;
    glm->gtigCVM[n] = nullptr;
  }
  return (glm);
}

/*--------------------------------------------------------------------
  GLMdof() - computes DOF = #Xrows - #Xcols
  ------------------------------------------------------------------*/
int GLMdof(GLMMAT *glm) {
  glm->dof = glm->X->rows - glm->X->cols;
  if (glm->dof == 0 && glm->AllowZeroDOF)
    glm->dof = 1;
  return (glm->dof);
}

/*---------------------------------------------------------------------
  GLMallocX() - allocate the X matrix. If it has already been alloced
  and the dims are correct, then just returns. If dims are not correct,
  frees and then allocs.
  ------------------------------------------------------------------*/
int GLMallocX(GLMMAT *glm, int nrows, int ncols) {
  if (glm->X != nullptr) {
    if (glm->X->rows == nrows && glm->X->cols == ncols)
      return (0);
    else
      MatrixFree(&glm->X);
  }
  glm->X = MatrixAlloc(nrows, ncols, MATRIX_REAL);
  return (0);
}

/*---------------------------------------------------------------------
  GLMallocY() - allocate the y matrix. If it has already been alloced
  and the dims are correct, then just returns. If dims are not correct,
  frees and then allocs.
  ------------------------------------------------------------------*/
int GLMallocY(GLMMAT *glm) {
  if (glm->y != nullptr) {
    if (glm->y->rows == glm->X->rows && glm->y->cols == 1)
      return (0);
    else
      MatrixFree(&glm->y);
  }
  glm->y = MatrixAlloc(glm->X->rows, 1, MATRIX_REAL);
  return (0);
}

/*---------------------------------------------------------------------
  GLMallocYFFxVar() - allocate the yffxvar matrix. If it has already
  been alloced and the dims are correct, then just returns. If dims
  are not correct, frees and then  allocs.
------------------------------------------------------------------*/
int GLMallocYFFxVar(GLMMAT *glm) {
  if (glm->yffxvar != nullptr) {
    if (glm->yffxvar->rows == glm->X->rows && glm->yffxvar->cols == 1)
      return (0);
    else
      MatrixFree(&glm->yffxvar);
  }
  glm->yffxvar = MatrixAlloc(glm->X->rows, 1, MATRIX_REAL);
  return (0);
}

/*---------------------------------------------------------------------
  GLMfree() - frees all the matrices associcated with the GLM struct,
  and the GLM struct itself.
  ------------------------------------------------------------------*/
int GLMfree(GLMMAT **pglm) {
  int     n;
  GLMMAT *glm;
  glm = *pglm;

  if (glm->y)
    MatrixFree(&glm->y);
  if (glm->X)
    MatrixFree(&glm->X);
  if (glm->beta)
    MatrixFree(&glm->beta);
  if (glm->yhat)
    MatrixFree(&glm->yhat);
  if (glm->eres)
    MatrixFree(&glm->eres);

  if (glm->Xt)
    MatrixFree(&glm->Xt);
  if (glm->XtX)
    MatrixFree(&glm->XtX);
  if (glm->iXtX)
    MatrixFree(&glm->iXtX);
  if (glm->Xty)
    MatrixFree(&glm->Xty);

  if (glm->yffxvar)
    MatrixFree(&glm->yffxvar);

  for (n = 0; n < GLMMAT_NCONTRASTS_MAX; n++) {
    if (glm->C[n])
      MatrixFree(&glm->C[n]);
    if (glm->Cname[n])
      free(&glm->Cname[n]);
    if (glm->Mpmf[n])
      MatrixFree(&glm->Mpmf[n]);
    if (glm->ypmf[n])
      MatrixFree(&glm->ypmf[n]);
    if (glm->Ct[n])
      MatrixFree(&glm->Ct[n]);
    if (glm->CiXtX[n])
      MatrixFree(&glm->CiXtX[n]);
    if (glm->CiXtXCt[n])
      MatrixFree(&glm->CiXtXCt[n]);
    if (glm->gCVM[n])
      MatrixFree(&glm->gCVM[n]);
    if (glm->igCVM[n])
      MatrixFree(&glm->igCVM[n]);
    if (glm->gamma[n])
      MatrixFree(&glm->gamma[n]);
    if (glm->gamma0[n])
      MatrixFree(&glm->gamma0[n]);
    if (glm->gammat[n])
      MatrixFree(&glm->gammat[n]);
    if (glm->gtigCVM[n])
      MatrixFree(&glm->gtigCVM[n]);
    if (glm->XCt[n])
      MatrixFree(&glm->XCt[n]);
    if (glm->Dt[n])
      MatrixFree(&glm->Dt[n]);
    if (glm->XDt[n])
      MatrixFree(&glm->XDt[n]);
    if (glm->RD[n])
      MatrixFree(&glm->RD[n]);
    if (glm->Xcd[n])
      MatrixFree(&glm->Xcd[n]);
    if (glm->Xcdt[n])
      MatrixFree(&glm->Xcdt[n]);
    if (glm->sumXcd[n])
      MatrixFree(&glm->sumXcd[n]);
    if (glm->sumXcd2[n])
      MatrixFree(&glm->sumXcd2[n]);
    if (glm->yhatd[n])
      MatrixFree(&glm->yhatd[n]);
    if (glm->Xcdyhatd[n])
      MatrixFree(&glm->Xcdyhatd[n]);
    if (glm->sumyhatd[n])
      MatrixFree(&glm->sumyhatd[n]);
    if (glm->sumyhatd2[n])
      MatrixFree(&glm->sumyhatd2[n]);
  }
  free(*pglm);
  *pglm = nullptr;
  return (0);
}

/*-----------------------------------------------------------------
  GLMcMatrices() - given all the C's computes all the Ct's.  Also
  computes condition number of each C as well as it's PMF.  This
  includes the allocation of Ct[n] and PMF. It would be possible to do
  this within GLMtest(), but GLMtest() may be run many times whereas
  Ct only needs to be computed once.
  ----------------------------------------------------------------*/
int GLMcMatrices(GLMMAT *glm) {
  int n, err;

  for (n = 0; n < glm->ncontrasts; n++) {
    glm->Ct[n]   = MatrixTranspose(glm->C[n], nullptr);
    glm->Mpmf[n] = GLMpmfMatrix(glm->C[n], &glm->Ccond[n], nullptr);

    if (glm->C[n]->rows == 1 && glm->DoPCC) {
      // These are for the computation of partial correlation coef
      // null space of contrast space
      glm->Dt[n] = MatrixColNullSpace(glm->Ct[n], &err);
      if (glm->Dt[n] == nullptr)
        continue;
      // design matrix projected onto contrast space
      glm->XCt[n] = MatrixMultiplyD(glm->X, glm->Ct[n], nullptr);
      // design matrix projected onto contrast null space (nuisance reg space)
      glm->XDt[n] = MatrixMultiplyD(glm->X, glm->Dt[n], nullptr);
      glm->RD[n]  = MatrixResidualForming(glm->XDt[n], nullptr);
      if (glm->RD[n] == nullptr) {
        printf("RD is not invertable n = %d\n", n);
        MatrixWriteTxt("X.mtx", glm->X);
        MatrixWriteTxt("C.mtx", glm->C[n]);
        MatrixWriteTxt("Dt.mtx", glm->Dt[n]);
        MatrixWriteTxt("XDt.mtx", glm->XDt[n]);
        exit(1);
      }
      // Orthogonalize Xc wrt the nuisance regressors (yhat too, but later)
      glm->Xcd[n]     = MatrixMultiplyD(glm->RD[n], glm->XCt[n], nullptr);
      glm->Xcdt[n]    = MatrixTranspose(glm->Xcd[n], nullptr);
      glm->sumXcd[n]  = MatrixSum(glm->Xcd[n], 1, nullptr);
      glm->sumXcd2[n] = MatrixSumSquare(glm->Xcd[n], 1, nullptr);
    } else
      glm->Dt[n] = nullptr; // make sure
  }
  return (0);
}

/*---------------------------------------------------------------
  GLMxMatrices() - compute all the matrices needed to do the
  estimation and testing, but does not do estimation or testing.
  This could be done inside of GLMfit() and/or GLMtest(). However,
  it may or may not be necessary to compute these matrices more
  than once. Eg, when doing an analysis where the desgin matrix
  is the same at all voxels, then it is not necessary.
  ---------------------------------------------------------------*/
int GLMxMatrices(GLMMAT *glm) {
  int     n, c, r;
  MATRIX *Mtmp, *Xnorm, *Xtnorm, *Xscale, *XtX;
  double  v;
  Xscale = nullptr;

  glm->dof = glm->X->rows - glm->X->cols;
  if (glm->dof == 0 && glm->AllowZeroDOF)
    glm->dof = 1;

  // If necessary, free Xt so that it can be realloced (FrameMask)
  if (glm->Xt && glm->Xt->cols != glm->X->rows)
    MatrixFree(&glm->Xt);

  glm->Xt  = MatrixTranspose(glm->X, glm->Xt);
  glm->XtX = MatrixMultiplyD(glm->Xt, glm->X, glm->XtX);
  if (glm->ReScaleX) {
    Xscale = MatrixAlloc(glm->X->cols, 1, MATRIX_REAL);
    Xnorm  = MatrixNormalizeCol(glm->X, nullptr, Xscale);
    Xtnorm = MatrixTranspose(Xnorm, nullptr);
    XtX    = MatrixMultiplyD(Xtnorm, Xnorm, nullptr);
  } else
    XtX = glm->XtX;

  Mtmp = MatrixInverse(XtX, glm->iXtX);
  if (Mtmp == nullptr) {
    if (Gdiag_no > 0) {
      printf("Matrix is Ill-conditioned\n");
      MatrixPrint(stdout, glm->X);
    }
    glm->ill_cond_flag = 1;
    return (1);
  }
  glm->ill_cond_flag = 0;
  glm->iXtX          = Mtmp;
  if (glm->ReScaleX) {
    for (c = 1; c <= glm->iXtX->rows; c++) {
      for (r = 1; r <= glm->iXtX->rows; r++) {
        v = Xscale->rptr[1][c] * Xscale->rptr[1][r];
        glm->iXtX->rptr[c][r] /= v;
      }
    }
    MatrixFree(&Xnorm);
    MatrixFree(&Xtnorm);
    MatrixFree(&Xscale);
    MatrixFree(&XtX);
  }

  for (n = 0; n < glm->ncontrasts; n++) {
    // gamma = C*beta
    // gCVM  = rvar*J*C*inv(X'*X)*C'
    // F     = gamma' * inv(gCVM) * gamma;
    glm->CiXtX[n] = MatrixMultiplyD(glm->C[n], glm->iXtX, glm->CiXtX[n]);
    glm->CiXtXCt[n] =
        MatrixMultiplyD(glm->CiXtX[n], glm->Ct[n], glm->CiXtXCt[n]);
  }
  return (0);
}

/*---------------------------------------------------------------
  GLMfit() - fit linear parameters (betas). Also computes yhat,
  eres, and rvar. May want to defer rvar at some point (eg, to
  do spatial filtering on the eres). XtX and iXtX must have been
  computed by GLMxMatrices() first.
  ---------------------------------------------------------------*/
int GLMfit(GLMMAT *glm) {
  int f;

  if (glm->ill_cond_flag)
    return (0);

  // Compute X'*y
  glm->Xty = MatrixMultiplyD(glm->Xt, glm->y, glm->Xty);

  // Now do the actual parameter (beta) estmation
  // beta = inv(X'*X)*X'*y
  glm->beta = MatrixMultiplyD(glm->iXtX, glm->Xty, glm->beta);

  // If necessary, free vectors so that they can be realloced (FrameMask)
  if (glm->yhat && glm->yhat->rows != glm->X->rows)
    MatrixFree(&glm->yhat);
  if (glm->eres && glm->eres->rows != glm->X->rows)
    MatrixFree(&glm->eres);

  // Compute yhat, eres, and residual variance
  glm->yhat = MatrixMultiplyD(glm->X, glm->beta, glm->yhat);
  glm->eres = MatrixSubtract(glm->y, glm->yhat, glm->eres);
  glm->rvar = 0;
  for (f = 1; f <= glm->eres->rows; f++)
    glm->rvar += (glm->eres->rptr[f][1] * glm->eres->rptr[f][1]);
  glm->rvar /= glm->dof;

  // What to do when rvar=0? Set to FLT_MIN. Affects resynth test.
  if (glm->rvar < FLT_MIN)
    glm->rvar = FLT_MIN;

  return (0);
}

/*------------------------------------------------------------------------
  GLMtest() - tests all the contrasts for the given GLM. Must have already
  run GLMcMatrices(), GLMxMatrices(), and GLMfit(). See also GLMtestFFX().
  ------------------------------------------------------------------------*/
int GLMtest(GLMMAT *glm) {
  int            n;
  double         dtmp;
  static MATRIX *F = nullptr, *mtmp = nullptr;
  static RFS *   rfs = nullptr;

  if (rfs == nullptr) {
    rfs       = RFspecInit(0, nullptr);
    rfs->name = strcpyalloc("z");
  }

  if (glm->ill_cond_flag) {
    // If it's ill cond, just return F=0
    for (n = 0; n < glm->ncontrasts; n++) {
      glm->F[n]   = 0;
      glm->p[n]   = 1;
      glm->z[n]   = 0;
      glm->pcc[n] = 0;
    }
    return (0);
  }

  for (n = 0; n < glm->ncontrasts; n++) {
    // gamma = C*beta
    // gCVM  = rvar*J*C*inv(X'*X)*C'
    // F     = gamma' * inv(gCVM) * gamma;
    // CiXtX and CiXtXCt are now computed by GLMxMatrices().
    // glm->CiXtX[n]    =
    //  MatrixMultiplyD(glm->C[n],glm->iXtX,glm->CiXtX[n]);
    // glm->CiXtXCt[n]  =
    //  MatrixMultiplyD(glm->CiXtX[n],glm->Ct[n],glm->CiXtXCt[n]);

    if (glm->igCVM[n] == nullptr)
      glm->igCVM[n] =
          MatrixAlloc(glm->C[n]->rows, glm->C[n]->rows, MATRIX_REAL);

    // Error trap for when rvar==0
    if (glm->rvar < 2 * FLT_MIN)
      dtmp = 1e10 * glm->C[n]->rows;
    else
      dtmp = glm->rvar * glm->C[n]->rows;

    glm->gamma[n] = MatrixMultiplyD(glm->C[n], glm->beta, glm->gamma[n]);
    if (glm->UseGamma0[n])
      MatrixSubtract(glm->gamma[n], glm->gamma0[n], glm->gamma[n]);
    glm->gammat[n] = MatrixTranspose(glm->gamma[n], glm->gammat[n]);
    glm->gCVM[n]   = MatrixScalarMul(glm->CiXtXCt[n], dtmp, glm->gCVM[n]);
    mtmp           = MatrixInverse(glm->CiXtXCt[n], glm->igCVM[n]);
    if (mtmp != nullptr && glm->rvar > FLT_MIN) {
      glm->igCVM[n] = MatrixScalarMul(glm->igCVM[n], 1.0 / dtmp, glm->igCVM[n]);
      glm->gtigCVM[n] =
          MatrixMultiplyD(glm->gammat[n], glm->igCVM[n], glm->gtigCVM[n]);
      F = MatrixMultiplyD(glm->gtigCVM[n], glm->gamma[n], F);
      if (F->rptr[1][1] >= 0) {
        glm->F[n] = F->rptr[1][1];
        glm->p[n] = sc_cdf_fdist_Q(glm->F[n], glm->C[n]->rows, glm->dof);
        glm->z[n] = RFp2StatVal(rfs, glm->p[n] / 2.0);
      } else {
        // Neg F can sometimes happen when the design matrix is ill-cond. One
        // example is in kinetic modeling when a voxel comes from the reference
        // region
        glm->F[n] = 0;
        glm->p[n] = 1;
        glm->z[n] = 0;
      }
      if (glm->C[n]->rows == 1 && glm->gamma[n]->rptr[1][1] < 0)
        glm->z[n] *= -1;

      if (glm->Dt[n] != nullptr) {
        // compute partial correlation coefficient (pcc)
        glm->yhatd[n] = MatrixMultiplyD(glm->RD[n], glm->yhat, glm->yhatd[n]);
        glm->Xcdyhatd[n] =
            MatrixMultiplyD(glm->Xcdt[n], glm->yhatd[n], glm->Xcdyhatd[n]);
        glm->sumyhatd[n] = MatrixSum(glm->yhatd[n], 1, glm->sumyhatd[n]);
        glm->sumyhatd2[n] =
            MatrixSumSquare(glm->yhatd[n], 1, glm->sumyhatd2[n]);
        glm->sumyhatd2[n]->rptr[1][1] += (glm->dof * glm->rvar);
        glm->pcc[n] =
            (glm->Xcdyhatd[n]->rptr[1][1] -
             glm->sumXcd[n]->rptr[1][1] * glm->sumyhatd[n]->rptr[1][1]) /
            sqrt((glm->sumXcd2[n]->rptr[1][1] -
                  glm->sumXcd[n]->rptr[1][1] * glm->sumXcd[n]->rptr[1][1]) *
                 (glm->sumyhatd2[n]->rptr[1][1] -
                  glm->sumyhatd[n]->rptr[1][1] * glm->sumyhatd[n]->rptr[1][1]));
      } else
        glm->pcc[n] = 0;
    } else {
      // this usually happens when the var is close to 0. But if this is
      // happening, should probably use a mask.
      glm->F[n]   = 0;
      glm->p[n]   = 1;
      glm->z[n]   = 0;
      glm->pcc[n] = 0;
    }
    if (glm->ypmfflag[n])
      glm->ypmf[n] = MatrixMultiplyD(glm->Mpmf[n], glm->beta, glm->ypmf[n]);
  }
  return (0);
}

/*------------------------------------------------------------------------
  GLMtestFFx() - tests all the contrasts for the given GLM. Must have already
  run GLMcMatrices(), GLMxMatrices(), and GLMfit(). See also GLMtest().
  ------------------------------------------------------------------------*/
int GLMtestFFx(GLMMAT *glm) {
  double         val;
  int            n, r, c;
  static MATRIX *F = nullptr, *mtmp = nullptr;
  MATRIX *Xs = nullptr, *Xst = nullptr, *CiXtXXs = nullptr, *CiXtXXst = nullptr;

  if (glm->ill_cond_flag) {
    // If it's ill cond, just return F=0
    for (n = 0; n < glm->ncontrasts; n++) {
      glm->F[n] = 0;
      glm->p[n] = 1;
    }
    return (0);
  }

  Xs = MatrixAlloc(glm->X->rows, glm->X->cols, MATRIX_REAL);
  for (r = 1; r <= glm->X->rows; r++) {
    val = sqrt(glm->yffxvar->rptr[r][1]);
    for (c = 1; c <= glm->X->cols; c++)
      Xs->rptr[r][c] = glm->X->rptr[r][c] * val;
  }
  Xst = MatrixTranspose(Xs, nullptr);

  for (n = 0; n < glm->ncontrasts; n++) {
    if (glm->igCVM[n] == nullptr)
      glm->igCVM[n] =
          MatrixAlloc(glm->C[n]->rows, glm->C[n]->rows, MATRIX_REAL);

    glm->gamma[n] = MatrixMultiplyD(glm->C[n], glm->beta, glm->gamma[n]);
    if (glm->UseGamma0[n])
      MatrixSubtract(glm->gamma[n], glm->gamma0[n], glm->gamma[n]);
    glm->gammat[n] = MatrixTranspose(glm->gamma[n], glm->gammat[n]);

    CiXtXXs      = MatrixMultiplyD(glm->CiXtX[n], Xst, nullptr);
    CiXtXXst     = MatrixTranspose(CiXtXXs, nullptr);
    glm->gCVM[n] = MatrixMultiplyD(CiXtXXs, CiXtXXst, glm->gCVM[n]);
    mtmp         = MatrixInverse(glm->gCVM[n], glm->igCVM[n]);
    if (mtmp != nullptr) {
      glm->gtigCVM[n] =
          MatrixMultiplyD(glm->gammat[n], glm->igCVM[n], glm->gtigCVM[n]);
      F             = MatrixMultiplyD(glm->gtigCVM[n], glm->gamma[n], F);
      glm->F[n]     = F->rptr[1][1];
      glm->p[n]     = sc_cdf_fdist_Q(glm->F[n], glm->C[n]->rows, glm->ffxdof);
      glm->igCVM[n] = mtmp;
    } else {
      // this usually happens when the var is close to 0. But if this is
      // happening, should probably use a mask.
      glm->F[n] = 0;
      glm->p[n] = 1;
    }
  }

  MatrixFree(&Xs);
  MatrixFree(&Xst);
  MatrixFree(&CiXtXXs);
  MatrixFree(&CiXtXXst);

  return (0);
}

/*-----------------------------------------------------------
  GLMprofile() - this can be used as both a profile and
  a memory leak tester. Design matrix is nrows-by-ncols
  (which forces y to be nrows-by-1). ncon contrasts are
  tested, where each contrast matrix is 2-by-ncols. Returns
  the number of msec used.
  -----------------------------------------------------------*/
int GLMprofile(int nrows, int ncols, int ncon, int niters) {
  int     n, c, msec;
  GLMMAT *glm;
  Timer   then;

  for (n = 0; n < niters; n++) {
    glm             = GLMalloc();
    glm->y          = MatrixDRand48(nrows, 1, nullptr);
    glm->X          = MatrixDRand48(nrows, ncols, nullptr);
    glm->ncontrasts = ncon;
    for (c = 0; c < glm->ncontrasts; c++) {
      glm->C[c]        = MatrixDRand48(2, ncols, nullptr);
      glm->ypmfflag[c] = 1;
    }
    GLMcMatrices(glm);
    GLMxMatrices(glm);
    GLMfit(glm);
    GLMtest(glm);
    GLMfree(&glm);
  }
  msec = then.milliseconds();

  printf("GLMprofile: nrows=%d, ncols=%d, ncon=%d, "
         "niters=%d, msec=%d, avgmsec=%g\n",
         nrows, ncols, ncon, niters, msec, (float)msec / niters);

  return (msec);
}

/*---------------------------------------------------------
  GLMsynth() - synthesizes y, X, and Cs and fits and tests.
  ---------------------------------------------------------*/
GLMMAT *GLMsynth() {
  static char tmpstr[1000];
  int         nrows, ncols, ncon, c;
  GLMMAT *    glm;

  nrows = 100;
  ncols = 10;
  ncon  = 3;

  glm             = GLMalloc();
  glm->y          = MatrixDRand48(nrows, 1, nullptr);
  glm->X          = MatrixDRand48(nrows, ncols, nullptr);
  glm->ncontrasts = ncon;
  for (c = 0; c < ncon; c++) {
    glm->C[c]        = MatrixDRand48(c + 1, ncols, nullptr);
    glm->ypmfflag[c] = 1;
    sprintf(tmpstr, "contrast%02d", c);
    glm->Cname[c] = strcpyalloc(tmpstr);
  }
  GLMcMatrices(glm);
  GLMxMatrices(glm);
  GLMfit(glm);
  GLMtest(glm);

  return (glm);
}

/*----------------------------------------------------------------------
  GLMresynthTest() - tests GLM by synthesizing y, X, fitting, then
  setting y=yhat (ie, resynth), refitting. rvar should be 0, but less
  than 10e-9 suffices. This is repeated niter times.  If any of the
  niters are over tolerance, then 1 is returned immediately. Otherwise
  0 is returned after all iters.  If prvar is non-null then rvar of
  the failure is passed back. If there is no failure, then the max
  rvar is passed.
  ---------------------------------------------------------*/
int GLMresynthTest(int niters, double *prvar) {
  int     nrows, ncols, n;
  GLMMAT *glm;
  double  rvarmax;

  nrows = 100;
  ncols = 10;
  glm   = GLMalloc();

  rvarmax = 0;
  for (n = 0; n < niters; n++) {
    // synthesize a GLM
    glm->y = MatrixDRand48(nrows, 1, glm->y);
    glm->X = MatrixDRand48(nrows, ncols, glm->X);
    // Fit
    GLMxMatrices(glm);
    GLMfit(glm);
    // Copy yhat into y
    glm->y = MatrixCopy(glm->yhat, glm->y);
    // Re-Fit
    GLMfit(glm);
    if (glm->rvar > 10e-9) {
      // rvar should be 0, but 10e-9 should be sufficient
      // Report an error if not.
      printf("GLMresynth failure: rvar = %le\n", glm->rvar);
      if (prvar != nullptr)
        *prvar = glm->rvar;
      GLMfree(&glm);
      return (1);
    }
    if (glm->rvar > rvarmax)
      rvarmax = glm->rvar;
  }
  GLMfree(&glm);
  if (prvar != nullptr)
    *prvar = rvarmax;
  return (0);
}

/*---------------------------------------------------------
  GLMdump() - saves a lot of the stuff from the GLMMAT
  struct into ascii files in the given directory.
  ---------------------------------------------------------*/
int GLMdump(const char *dumpdir, GLMMAT *glm)
{
  char fname[1000], condir[1000];
  FILE *fp;
  int   c;

  mkdir(dumpdir, 0777);

  sprintf(fname, "%s/y.dat", dumpdir);
  MatrixWriteTxt(fname, glm->y);

  sprintf(fname, "%s/X.dat", dumpdir);
  MatrixWriteTxt(fname, glm->X);

  sprintf(fname, "%s/dof.dat", dumpdir);
  fp = fopen(fname, "w");
  fprintf(fp, "%lf\n", glm->dof);
  fclose(fp);

  sprintf(fname, "%s/ill_cond_flag.dat", dumpdir);
  fp = fopen(fname, "w");
  fprintf(fp, "%d\n", glm->ill_cond_flag);
  fclose(fp);
  if (glm->ill_cond_flag)
    return (0);

  sprintf(fname, "%s/beta.dat", dumpdir);
  MatrixWriteTxt(fname, glm->beta);

  sprintf(fname, "%s/yhat.dat", dumpdir);
  MatrixWriteTxt(fname, glm->yhat);

  sprintf(fname, "%s/eres.dat", dumpdir);
  MatrixWriteTxt(fname, glm->eres);

  sprintf(fname, "%s/rvar.dat", dumpdir);
  fp = fopen(fname, "w");
  fprintf(fp, "%lf\n", glm->rvar);
  fclose(fp);

  sprintf(fname, "%s/ncontrasts.dat", dumpdir);
  fp = fopen(fname, "w");
  fprintf(fp, "%d\n", glm->ncontrasts);
  fclose(fp);

  for (c = 0; c < glm->ncontrasts; c++) {
    if (glm->Cname[c] != nullptr)
      sprintf(condir, "%s/%s", dumpdir, glm->Cname[c]);
    else
      sprintf(condir, "%s/contrast%03d", dumpdir, c + 1);
    mkdir(condir, 0777);

    sprintf(fname, "%s/C.dat", condir);
    MatrixWriteTxt(fname, glm->C[c]);

    sprintf(fname, "%s/Ccond.dat", condir);
    fp = fopen(fname, "w");
    fprintf(fp, "%f\n", glm->Ccond[c]);
    fclose(fp);

    sprintf(fname, "%s/Mpmf.dat", condir);
    MatrixWriteTxt(fname, glm->Mpmf[c]);

    sprintf(fname, "%s/gamma.dat", condir);
    MatrixWriteTxt(fname, glm->gamma[c]);
    if (glm->UseGamma0[c]) {
      sprintf(fname, "%s/gamma0.dat", condir);
      MatrixWriteTxt(fname, glm->gamma0[c]);
    }

    sprintf(fname, "%s/F.dat", condir);
    fp = fopen(fname, "w");
    fprintf(fp, "%lf\n", glm->F[c]);
    fclose(fp);

    sprintf(fname, "%s/p.dat", condir);
    fp = fopen(fname, "w");
    fprintf(fp, "%le\n", glm->p[c]);
    fclose(fp);

    if (glm->ypmfflag[c]) {
      sprintf(fname, "%s/ypmf.dat", condir);
      MatrixWriteTxt(fname, glm->ypmf[c]);
    }
  }

  return (0);
}

/*--------------------------------------------------------------------
  GLMpmfMatrix() - compute matrix P used for partial model fit (pmf)
  of a contrast. P = C'*inv(C*C')*C, where C is the contrast
  matrix. Then ypmf = X*P*beta, where ypmf is y projected into the
  contrast space. To do this, C must be well-conditioned. The
  condition number is computed and returned through cond so that the
  calling program can decide whether it is  ill-conditioned.
  ------------------------------------------------------------------*/
MATRIX *GLMpmfMatrix(MATRIX *C, double *cond, MATRIX *P) {
  MATRIX *Ct = nullptr, *CCt = nullptr, *iCCt = nullptr, *CtiCCt = nullptr;

  if (P != nullptr) {
    if (P->rows != C->cols || P->cols != C->cols) {
      printf("ERROR: GLMpmfMatrix: dimension mismatch\n");
      return (nullptr);
    }
  }

  Ct     = MatrixTranspose(C, Ct);
  CCt    = MatrixMultiplyD(C, Ct, CCt);
  *cond  = MatrixConditionNumber(CCt);
  iCCt   = MatrixInverse(CCt, iCCt);
  CtiCCt = MatrixMultiplyD(Ct, iCCt, CtiCCt);
  P      = MatrixMultiplyD(CtiCCt, C, P);

  MatrixFree(&Ct);
  MatrixFree(&CCt);
  MatrixFree(&iCCt);
  MatrixFree(&CtiCCt);

  return (P);
}
