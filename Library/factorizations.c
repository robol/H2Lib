/* ------------------------------------------------------------
 This is the file "factorizations.c" of the H2Lib package.
 All rights reserved, Steffen Boerm 2010
 ------------------------------------------------------------ */

#include "factorizations.h"

#include "settings.h"
#include "basic.h"
#include "lapack_types.h"

#include <math.h>
#include <stdio.h>

/* ------------------------------------------------------------
 Diagonal matrices
 ------------------------------------------------------------ */

void
diagsolve_amatrix_avector(bool atrans, pcamatrix a, pavector x)
{
  pcfield    aa = a->a;
  LAPACK_INT lda = a->ld;
  pfield     xv = x->v;
  LAPACK_INT n = UINT_MIN(a->rows, a->cols);
  uint       i;

  if (atrans) {
    for (i = 0; i < n; i++)
      xv[i] /= CONJ(aa[i + i * lda]);
  }
  else {
    for (i = 0; i < n; i++)
      xv[i] /= aa[i + i * lda];
  }
}

#ifdef USE_BLAS
IMPORT_PREFIX void
dscal_(const LAPACK_INT *n,
       const double *alpha, double *x, const LAPACK_INT *incx);

void
diagsolve_amatrix(bool atrans, pcamatrix a, bool xtrans, pamatrix x)
{
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      ldx = x->ld;
  double   *aa = a->a;
  double   *xa = x->a;
  double    alpha;
  LAPACK_INT      i;
  LAPACK_INT x_rows = x->rows, x_cols = x->cols;

  if (xtrans) {
    for (i = 0; i < n; i++) {
      alpha = (atrans ? 1.0 / CONJ(aa[i + i * lda]) : 1.0 / aa[i + i * lda]);
      dscal_(&x_rows, &alpha, xa + i * ldx, &l_one);
    }
  }
  else {
    for (i = 0; i < n; i++) {
      alpha = (atrans ? 1.0 / CONJ(aa[i + i * lda]) : 1.0 / aa[i + i * lda]);
      dscal_(&x_cols, &alpha, xa + i, &ldx);
    }
  }
}
#else
void
diagsolve_amatrix(bool atrans, pcamatrix a, bool xtrans, pamatrix x)
{
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      ldx = x->ld;
  double   *aa = a->a;
  double   *xa = x->a;
  double    alpha;
  LAPACK_INT      i, j;

  if (xtrans) {
    for (i = 0; i < n; i++) {
      alpha = (atrans ? 1.0 / CONJ(aa[i + i * lda]) : 1.0 / aa[i + i * lda]);
      for (j = 0; j < x->rows; j++)
	xa[j + i * ldx] *= alpha;
    }
  }
  else {
    for (i = 0; i < n; i++) {
      alpha = (atrans ? 1.0 / CONJ(aa[i + i * lda]) : 1.0 / aa[i + i * lda]);
      for (j = 0; j < x->cols; j++)
	xa[i + j * ldx] *= alpha;
    }
  }
}
#endif

#ifdef USE_BLAS
IMPORT_PREFIX void
dscal_(const LAPACK_INT *n,
       const double *alpha, double *x, const LAPACK_INT *incx);

void
diageval_amatrix(bool atrans, pcamatrix a, bool xtrans, pamatrix x)
{
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      ldx = x->ld;
  double   *aa = a->a;
  double   *xa = x->a;
  double    alpha;
  int       i;
  LAPACK_INT x_cols = x->cols, x_rows = x->rows;

  if (xtrans) {
    for (i = 0; i < n; i++) {
      alpha = (atrans ? CONJ(aa[i + i * lda]) : aa[i + i * lda]);
      dscal_(&x_rows, &alpha, xa + i * ldx, &l_one);
    }
  }
  else {
    for (i = 0; i < n; i++) {
      alpha = (atrans ? CONJ(aa[i + i * lda]) : aa[i + i * lda]);
      dscal_(&x_cols, &alpha, xa + i, &ldx);
    }
  }
}
#else
void
diageval_amatrix(bool atrans, pcamatrix a, bool xtrans, pamatrix x)
{
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      ldx = x->ld;
  double   *aa = a->a;
  double   *xa = x->a;
  double    alpha;
  LAPACK_INT      i, j;

  if (xtrans) {
    for (i = 0; i < n; i++) {
      alpha = (atrans ? CONJ(aa[i + i * lda]) : aa[i + i * lda]);
      for (j = 0; j < x->rows; j++)
	xa[j + i * ldx] *= alpha;
    }
  }
  else {
    for (i = 0; i < n; i++) {
      alpha = (atrans ? CONJ(aa[i + i * lda]) : aa[i + i * lda]);
      for (j = 0; j < x->cols; j++)
	xa[i + j * ldx] *= alpha;
    }
  }
}
#endif

/* ------------------------------------------------------------
 Triangular matrices
 ------------------------------------------------------------ */

#ifdef USE_BLAS
IMPORT_PREFIX void
dtrtrs_(const char *uplo,
	const char *trans,
	const char *diag,
	const LAPACK_INT *n,
	const LAPACK_INT *nrhs,
	const double *a,
	const LAPACK_INT *lda, double *b, const LAPACK_INT *ldb, LAPACK_INT *info);

static void
lowersolve_amatrix_avector(bool aunit, bool atrans, pcamatrix a, pavector x)
{
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      info, a_ld = a->ld, x_dim = x->dim;

  assert(x->dim >= a->rows);
  assert(x->dim >= a->cols);

  if (atrans) {
    dtrtrs_("Lower", "Transposed",
	    (aunit ? "Unit triangular" : "Not unit-triangular"),
	    &n, &l_one, a->a, &a_ld, x->v, &x_dim, &info);
    assert(info == 0);
  }
  else {
    dtrtrs_("Lower", "Not transposed",
	    (aunit ? "Unit triangular" : "Not unit-triangular"),
	    &n, &l_one, a->a, &a_ld, x->v, &x_dim, &info);
    assert(info == 0);
  }
}

static void
uppersolve_amatrix_avector(bool aunit, bool atrans, pcamatrix a, pavector x)
{
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      info;
  LAPACK_INT      a_ld = a->ld, x_dim = x->dim;

  assert(x->dim >= a->rows);
  assert(x->dim >= a->cols);

  if (atrans) {
    dtrtrs_("Upper", "Transposed",
	    (aunit ? "Unit triangular" : "Non-unit triangular"),
	    &n, &l_one, a->a, &a_ld, x->v, &x_dim, &info);
    assert(info == 0);
  }
  else {
    dtrtrs_("Upper", "Not transposed",
	    (aunit ? "Unit triangular" : "Non-unit triangular"),
	    &n, &l_one, a->a, &a_ld, x->v, &x_dim, &info);
    assert(info == 0);
  }
}
#else
static void
lowersolve_amatrix_avector(bool aunit, bool atrans, pcamatrix a, pavector x)
{
  pcfield   aa = a->a;
  LAPACK_INT      lda = a->ld;
  pfield    xv = x->v;
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  field     newval;
  LAPACK_INT      i, j;

  assert(x->dim >= a->rows);
  assert(x->dim >= a->cols);

  if (atrans) {
    for (j = n; j-- > 0;) {
      newval = (aunit ? xv[j] : (xv[j] /= CONJ(aa[j + j * lda])));
      for (i = 0; i < j; i++)
	xv[i] -= CONJ(aa[j + i * lda]) * newval;
    }
  }
  else {
    for (j = 0; j < n; j++) {
      newval = (aunit ? xv[j] : (xv[j] /= aa[j + j * lda]));
      for (i = j + 1; i < n; i++)
	xv[i] -= aa[i + j * lda] * newval;
    }
  }
}

static void
uppersolve_amatrix_avector(bool aunit, bool atrans, pcamatrix a, pavector x)
{
  pcfield   aa = a->a;
  LAPACK_INT      lda = a->ld;
  pfield    xv = x->v;
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  field     newval;
  LAPACK_INT      i, j;

  assert(x->dim >= a->rows);
  assert(x->dim >= a->cols);

  if (atrans) {
    for (j = 0; j < n; j++) {
      newval = (aunit ? xv[j] : (xv[j] /= CONJ(aa[j + j * lda])));
      for (i = j + 1; i < n; i++)
	xv[i] -= CONJ(aa[j + i * lda]) * newval;
    }
  }
  else {
    for (j = n; j-- > 0;) {
      newval = (aunit ? xv[j] : (xv[j] /= aa[j + j * lda]));
      for (i = 0; i < j; i++)
	xv[i] -= aa[i + j * lda] * newval;
    }
  }
}
#endif

void
triangularsolve_amatrix_avector(bool alower, bool aunit, bool atrans,
				pcamatrix a, pavector x)
{
  if (alower)
    lowersolve_amatrix_avector(aunit, atrans, a, x);
  else
    uppersolve_amatrix_avector(aunit, atrans, a, x);
}

#ifdef USE_BLAS
IMPORT_PREFIX void
dtrsm_(const char *side,
       const char *uplo,
       const char *transa,
       const char *diag,
       const LAPACK_INT *m,
       const LAPACK_INT *n,
       const double *alpha,
       const double *a, const LAPACK_INT *lda, double *b, const LAPACK_INT *ldb);

static void
lowersolve_amatrix(bool aunit, bool atrans, pcamatrix a,
		   bool xtrans, pamatrix x)
{
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  double          *aa = a->a;
  LAPACK_INT      lda = a->ld;
  double          *xa = x->a;
  LAPACK_INT      ldx = x->ld;
  LAPACK_INT      x_rows = x->rows, x_cols = x->cols;

  if (atrans) {
    if (xtrans) {
      assert(x->cols >= n);

      dtrsm_("Right", "Lower", "Not transposed",
	     (aunit ? "Unit diagonal" : "Non-unit diagonal"),
	     &x_rows, &n, &f_one, aa, &lda, xa, &ldx);
    }
    else {
      assert(x->rows >= n);

      dtrsm_("Left", "Lower", "Transposed",
	     (aunit ? "Unit diagonal" : "Non-unit diagonal"),
	     &n, &x_cols, &f_one, aa, &lda, xa, &ldx);
    }
  }
  else {
    if (xtrans) {
      assert(x->cols >= n);

      dtrsm_("Right", "Lower", "Transposed",
	     (aunit ? "Unit diagonal" : "Non-unit diagonal"),
	     &x_rows, &n, &f_one, aa, &lda, xa, &ldx);
    }
    else {
      assert(x->rows >= n);

      dtrsm_("Left", "Lower", "Not transposed",
	     (aunit ? "Unit diagonal" : "Non-unit diagonal"),
	     &n, &x_cols, &f_one, aa, &lda, xa, &ldx);
    }
  }
}

static void
uppersolve_amatrix(bool aunit, bool atrans, pcamatrix a,
		   bool xtrans, pamatrix x)
{
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  double          *aa = a->a;
  LAPACK_INT      lda = a->ld;
  double          *xa = x->a;
  LAPACK_INT      ldx = x->ld;
  LAPACK_INT      x_rows = x->rows, x_cols = x->cols;

  if (atrans) {
    if (xtrans) {
      assert(x->cols >= n);

      dtrsm_("Right", "Upper", "Not transposed",
	     (aunit ? "Unit diagonal" : "Non-unit diagonal"),
	     &x_rows, &n, &f_one, aa, &lda, xa, &ldx);
    }
    else {
      assert(x->rows >= n);

      dtrsm_("Left", "Upper", "Transposed",
	     (aunit ? "Unit diagonal" : "Non-unit diagonal"),
	     &n, &x_cols, &f_one, aa, &lda, xa, &ldx);
    }
  }
  else {
    if (xtrans) {
      assert(x->cols >= n);

      dtrsm_("Right", "Upper", "Transposed",
	     (aunit ? "Unit diagonal" : "Non-unit diagonal"),
	     &x_rows, &n, &f_one, aa, &lda, xa, &ldx);
    }
    else {
      assert(x->rows >= n);

      dtrsm_("Left", "Upper", "Not transposed",
	     (aunit ? "Unit diagonal" : "Non-unit diagonal"),
	     &n, &x_cols, &f_one, aa, &lda, xa, &ldx);
    }
  }
}
#else
static void
lowersolve_amatrix(bool aunit, bool atrans, pcamatrix a,
		   bool xtrans, pamatrix x)
{
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      ldx = x->ld;
  pfield    aa = a->a;
  pfield    xa = x->a;
  LAPACK_INT      i, j, k;
  field     alpha;

  if (atrans) {
    if (xtrans) {
      assert(x->cols >= n);

      for (k = n; k-- > 0;) {
	if (!aunit) {
	  alpha = 1.0 / aa[k + k * lda];
	  for (i = 0; i < x->rows; i++)
	    xa[i + k * ldx] *= alpha;
	}
	for (i = 0; i < x->rows; i++)
	  for (j = 0; j < k; j++)
	    xa[i + j * ldx] -= xa[i + k * ldx] * aa[k + j * lda];
      }
    }
    else {
      assert(x->rows >= n);

      for (k = n; k-- > 0;) {
	if (!aunit) {
	  alpha = 1.0 / CONJ(aa[k + k * lda]);
	  for (j = 0; j < x->cols; j++)
	    xa[k + j * ldx] *= alpha;
	}
	for (i = 0; i < k; i++)
	  for (j = 0; j < x->cols; j++)
	    xa[i + j * ldx] -= CONJ(aa[k + i * lda]) * xa[k + j * ldx];
      }
    }
  }
  else {
    if (xtrans) {
      assert(x->cols >= n);

      for (k = 0; k < n; k++) {
	if (!aunit) {
	  alpha = 1.0 / CONJ(aa[k + k * lda]);
	  for (i = 0; i < x->rows; i++)
	    xa[i + k * ldx] *= alpha;
	}
	for (i = 0; i < x->rows; i++)
	  for (j = k + 1; j < n; j++)
	    xa[i + j * ldx] -= xa[i + k * ldx] * CONJ(aa[j + k * lda]);
      }
    }
    else {
      assert(x->rows >= n);

      for (k = 0; k < n; k++) {
	if (!aunit) {
	  alpha = 1.0 / aa[k + k * lda];
	  for (j = 0; j < x->cols; j++)
	    xa[k + j * ldx] *= alpha;
	}
	for (i = k + 1; i < n; i++)
	  for (j = 0; j < x->cols; j++)
	    xa[i + j * ldx] -= aa[i + k * lda] * xa[k + j * ldx];
      }
    }
  }
}

static void
uppersolve_amatrix(bool aunit, bool atrans, pcamatrix a,
		   bool xtrans, pamatrix x)
{
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      ldx = x->ld;
  pfield    aa = a->a;
  pfield    xa = x->a;
  LAPACK_INT      i, j, k;
  field     alpha;

  if (atrans) {
    if (xtrans) {
      assert(x->cols >= n);

      for (k = 0; k < n; k++) {
	if (!aunit) {
	  alpha = 1.0 / aa[k + k * lda];
	  for (i = 0; i < x->rows; i++)
	    xa[i + k * ldx] *= alpha;
	}
	for (i = 0; i < x->rows; i++)
	  for (j = k + 1; j < n; j++)
	    xa[i + j * ldx] -= xa[i + k * ldx] * aa[k + j * lda];
      }
    }
    else {
      assert(x->rows >= n);

      for (k = 0; k < n; k++) {
	if (!aunit) {
	  alpha = 1.0 / CONJ(aa[k + k * lda]);
	  for (j = 0; j < x->cols; j++)
	    xa[k + j * ldx] *= alpha;
	}
	for (i = k + 1; i < n; i++)
	  for (j = 0; j < x->cols; j++)
	    xa[i + j * ldx] -= CONJ(aa[k + i * lda]) * xa[k + j * ldx];
      }
    }
  }
  else {
    if (xtrans) {
      assert(x->cols >= n);

      for (k = n; k-- > 0;) {
	if (!aunit) {
	  alpha = 1.0 / CONJ(aa[k + k * lda]);
	  for (i = 0; i < x->rows; i++)
	    xa[i + k * ldx] *= alpha;
	}
	for (i = 0; i < x->rows; i++)
	  for (j = 0; j < k; j++)
	    xa[i + j * ldx] -= xa[i + k * ldx] * CONJ(aa[j + k * lda]);
      }
    }
    else {
      assert(x->rows >= n);

      for (k = n; k-- > 0;) {
	if (!aunit) {
	  alpha = 1.0 / aa[k + k * lda];
	  for (j = 0; j < x->cols; j++)
	    xa[k + j * ldx] *= alpha;
	}
	for (i = 0; i < k; i++)
	  for (j = 0; j < x->cols; j++)
	    xa[i + j * ldx] -= aa[i + k * lda] * xa[k + j * ldx];
      }
    }
  }
}
#endif

void
triangularsolve_amatrix(bool alower, bool aunit, bool atrans, pcamatrix a,
			bool xtrans, pamatrix x)
{
  if (alower)
    lowersolve_amatrix(aunit, atrans, a, xtrans, x);
  else
    uppersolve_amatrix(aunit, atrans, a, xtrans, x);
}

#ifdef USE_BLAS
IMPORT_PREFIX void
dtrmv_(const char *uplo,
       const char *trans,
       const char *diag,
       const LAPACK_INT *n,
       const double *a, const LAPACK_INT *lda, double *x, const LAPACK_INT *incx);

IMPORT_PREFIX void
dgemv_(const char *trans,
       const LAPACK_INT *m,
       const LAPACK_INT *n,
       const double *alpha,
       const double *a,
       const LAPACK_INT *lda,
       const double *x,
       const LAPACK_INT *incx,
       const double *beta, double *y, const LAPACK_INT *incy);

static void
lowereval_amatrix_avector(bool aunit, bool atrans, pcamatrix a, pavector x)
{
  double   *aa = a->a;
  double   *xv = x->v;
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      n1, i;

  assert(x->dim >= a->rows);
  assert(x->dim >= a->cols);

  if (n == 0)			/* Quick exit */
    return;

  if (atrans) {
    /* Left upper part, upper triangular */
    dtrmv_("Lower", "Transposed",
	   (aunit ? "Unit triangular" : "Non-unit triangular"),
	   &n, aa, &lda, xv, &l_one);

    /* Right part */
    if (n < a->rows) {
      n1 = a->rows - n;
      dgemv_("Transposed", &n1, &n, &f_one,
	     aa + n, &lda, xv + n, &l_one, &f_one, xv, &l_one);
    }

    /* Lower part */
    if (n < a->cols)
      for (i = n; i < a->cols; i++)
	xv[i] = 0.0;
  }
  else {
    /* Lower part */
    if (n < a->rows) {
      for (i = n; i < a->rows; i++)
	xv[i] = 0.0;

      n1 = a->rows - n;
      dgemv_("Not Transposed", &n1, &n, &f_one,
	     aa + n, &lda, xv, &l_one, &f_one, xv + n, &l_one);
    }

    /* Top part, lower triangular */
    dtrmv_("Lower", "Not transposed",
	   (aunit ? "Unit triangular" : "Non-unit triangular"),
	   &n, aa, &lda, xv, &l_one);
  }
}

static void
uppereval_amatrix_avector(bool aunit, bool atrans, pcamatrix a, pavector x)
{
  double   *aa = a->a;
  double   *xv = x->v;
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      n1, i;

  assert(x->dim >= a->rows);
  assert(x->dim >= a->cols);

  if (n == 0)			/* Quick exit */
    return;

  if (atrans) {
    /* Lower part */
    if (n < a->cols) {
      for (i = n; i < a->cols; i++)
	xv[i] = 0.0;

      n1 = a->cols - n;
      dgemv_("Transposed", &n, &n1, &f_one,
	     aa + n * lda, &lda, xv, &l_one, &f_one, xv + n, &l_one);
    }

    /* Top part, lower triangular */
    dtrmv_("Upper", "Transposed",
	   (aunit ? "Unit triangular" : "Non-unit triangular"),
	   &n, aa, &lda, xv, &l_one);
  }
  else {
    /* Left upper part, upper triangular */
    dtrmv_("Upper", "Not transposed",
	   (aunit ? "Unit triangular" : "Non-unit triangular"),
	   &n, aa, &lda, xv, &l_one);

    /* Right part */
    if (n < a->cols) {
      n1 = a->cols - n;
      dgemv_("Not transposed", &n, &n1, &f_one,
	     aa + n * lda, &lda, xv + n, &l_one, &f_one, xv, &l_one);
    }

    /* Lower part */
    if (n < a->rows)
      for (i = n; i < a->rows; i++)
	xv[i] = 0.0;
  }
}
#else
static void
lowereval_amatrix_avector(bool aunit, bool atrans, pcamatrix a, pavector x)
{
  pcfield   aa = a->a;
  LAPACK_INT      lda = a->ld;
  pfield    xv = x->v;
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  field     newval;
  LAPACK_INT      i, j;

  assert(x->dim >= a->rows);
  assert(x->dim >= a->cols);

  if (atrans) {
    /* Left upper part, upper triangular */
    for (i = 0; i < n; i++) {
      newval = (aunit ? xv[i] : CONJ(aa[i + i * lda]) * xv[i]);
      for (j = i + 1; j < n; j++)
	newval += CONJ(aa[j + i * lda]) * xv[j];
      xv[i] = newval;
    }

    /* Right part */
    if (n < a->rows)
      for (i = 0; i < n; i++) {
	newval = xv[i];
	for (j = n; j < a->rows; j++)
	  newval += CONJ(aa[j + i * lda]) * xv[j];
	xv[i] = newval;
      }

    /* Lower part */
    if (n < a->cols)
      for (i = n; i < a->cols; i++)
	xv[i] = 0.0;
  }
  else {
    /* Lower part */
    if (n < a->rows)
      for (i = n; i < a->rows; i++) {
	newval = 0.0;
	for (j = 0; j < a->cols; j++)
	  newval += aa[i + j * lda] * xv[j];
	xv[i] = newval;
      }

    /* Top part, lower triangular */
    for (i = n; i-- > 0;) {
      newval = (aunit ? xv[i] : aa[i + i * lda] * xv[i]);
      for (j = 0; j < i; j++)
	newval += aa[i + j * lda] * xv[j];
      xv[i] = newval;
    }
  }
}

static void
uppereval_amatrix_avector(bool aunit, bool atrans, pcamatrix a, pavector x)
{
  pcfield   aa = a->a;
  LAPACK_INT      lda = a->ld;
  pfield    xv = x->v;
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  field     newval;
  LAPACK_INT      i, j;

  assert(x->dim >= a->rows);
  assert(x->dim >= a->cols);

  if (atrans) {
    /* Lower part */
    if (n < a->cols)
      for (i = n; i < a->cols; i++) {
	newval = 0.0;
	for (j = 0; j < n; j++)
	  newval += CONJ(aa[j + i * lda]) * xv[j];
	xv[i] = newval;
      }

    /* Top part, lower triangular */
    for (i = n; i-- > 0;) {
      newval = (aunit ? xv[i] : CONJ(aa[i + i * lda]) * xv[i]);
      for (j = 0; j < i; j++)
	newval += CONJ(aa[j + i * lda]) * xv[j];
      xv[i] = newval;
    }
  }
  else {
    /* Left upper part, upper triangular */
    for (i = 0; i < n; i++) {
      newval = (aunit ? xv[i] : xv[i] * aa[i + i * lda]);
      for (j = i + 1; j < n; j++)
	newval += aa[i + j * lda] * xv[j];
      xv[i] = newval;
    }

    /* Right part */
    if (n < a->cols)
      for (i = 0; i < n; i++) {
	newval = xv[i];
	for (j = n; j < a->cols; j++)
	  newval += aa[i + j * lda] * xv[j];
	xv[i] = newval;
      }

    /* Lower part */
    if (n < a->rows)
      for (i = n; i < a->rows; i++)
	xv[i] = 0.0;
  }
}
#endif

void
triangulareval_amatrix_avector(bool alower, bool aunit, bool atrans,
			       pcamatrix a, pavector x)
{
  if (alower)
    lowereval_amatrix_avector(aunit, atrans, a, x);
  else
    uppereval_amatrix_avector(aunit, atrans, a, x);
}

#ifdef USE_BLAS
IMPORT_PREFIX void
dtrmm_(const char *side,
       const char *uplo,
       const char *trans,
       const char *diag,
       const LAPACK_INT *m,
       const LAPACK_INT *n,
       const double *alpha,
       const double *a, const LAPACK_INT *lda, double *b, const LAPACK_INT *ldb);

IMPORT_PREFIX void
dgemm_(const char *transa,
       const char *transb,
       const LAPACK_INT *m,
       const LAPACK_INT *n,
       const LAPACK_INT *k,
       const double *alpha,
       const double *a,
       const LAPACK_INT *lda,
       const double *b,
       const LAPACK_INT *ldb,
       const double *beta, double *c, const LAPACK_INT *ldc);

static void
lowereval_amatrix(bool aunit, bool atrans, pcamatrix a,
		  bool xtrans, pamatrix x)
{
  double          *aa = a->a;
  LAPACK_INT      lda = a->ld;
  double          *xa = x->a;
  LAPACK_INT      ldx = x->ld;
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      n1, i, j;
  LAPACK_INT      x_rows = x->rows, x_cols = x->cols;

  if (n == 0)			/* Quick exit */
    return;

  if (xtrans) {
    assert(x->cols >= a->rows);
    assert(x->cols >= a->cols);

    if (atrans) {
      /* Left upper part, upper triangular */
      dtrmm_("Right", "Lower", "Not transposed",
	     (aunit ? "Unit triangular" : "Non-unit triangular"),
	     &x_rows, &n, &f_one, aa, &lda, xa, &ldx);

      /* Right part */
      if (n < a->rows) {
	n1 = a->rows - n;
	dgemm_("Not transposed", "Not transposed", &x_rows, &n, &n1, &f_one,
	       xa + n * ldx, &ldx, aa + n, &lda, &f_one, xa, &ldx);
      }

      /* Lower part */
      if (n < a->cols)
	for (j = 0; j < x->rows; j++)
	  for (i = n; i < a->cols; i++)
	    xa[j + i * ldx] = 0.0;
    }
    else {
      /* Lower part */
      if (n < a->rows) {
	for (j = 0; j < x->rows; j++)
	  for (i = n; i < a->rows; i++)
	    xa[j + i * ldx] = 0.0;

	n1 = a->rows - n;
	dgemm_("Not transposed", "Transposed", &x_rows, &n1, &n, &f_one,
	       xa, &ldx, aa + n, &lda, &f_one, xa + n * ldx, &ldx);
      }

      /* Top part, lower triangular */
      dtrmm_("Right", "Lower", "Transposed",
	     (aunit ? "Unit triangular" : "Non-unit triangular"),
	     &x_rows, &n, &f_one, aa, &lda, xa, &ldx);
    }
  }
  else {
    assert(x->rows >= a->rows);
    assert(x->rows >= a->cols);

    if (atrans) {
      /* Left upper part, upper triangular */
      dtrmm_("Left", "Lower", "Transposed",
	     (aunit ? "Unit triangular" : "Non-unit triangular"),
	     &n, &x_cols, &f_one, aa, &lda, xa, &ldx);

      /* Right part */
      if (n < a->rows) {
	n1 = a->rows - n;
	dgemm_("Transposed", "Not Transposed", &n, &x_cols, &n1, &f_one,
	       aa + n, &lda, xa + n, &ldx, &f_one, xa, &ldx);
      }

      /* Lower part */
      if (n < a->cols)
	for (j = 0; j < x->cols; j++)
	  for (i = n; i < a->cols; i++)
	    xa[i + j * ldx] = 0.0;
    }
    else {
      /* Lower part */
      if (n < a->rows) {
	for (j = 0; j < x->cols; j++)
	  for (i = n; i < a->rows; i++)
	    xa[i + j * ldx] = 0.0;

	n1 = a->rows - n;
	dgemm_("Not Transposed", "Not Transposed", &n1, &x_cols, &n, &f_one,
	       aa + n, &lda, xa, &ldx, &f_one, xa + n, &ldx);
      }

      /* Top part, lower triangular */
      dtrmm_("Left", "Lower", "Not transposed",
	     (aunit ? "Unit triangular" : "Non-unit triangular"),
	     &n, &x_cols, &f_one, aa, &lda, xa, &ldx);
    }
  }
}

static void
uppereval_amatrix(bool aunit, bool atrans, pcamatrix a,
		  bool xtrans, pamatrix x)
{
  double          *aa = a->a;
  LAPACK_INT      lda = a->ld;
  double          *xa = x->a;
  LAPACK_INT      ldx = x->ld;
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  LAPACK_INT      n1, i, j;
  LAPACK_INT      x_rows = x->rows, x_cols = x->cols;

  if (n == 0)			/* Quick exit */
    return;

  if (xtrans) {
    assert(x->cols >= a->rows);
    assert(x->cols >= a->cols);

    if (atrans) {
      /* Lower part */
      if (n < a->cols) {
	for (j = 0; j < x->rows; j++)
	  for (i = n; i < a->cols; i++)
	    xa[j + i * ldx] = 0.0;

	n1 = a->cols - n;
	dgemm_("Not Transposed", "Not transposed", &x_rows, &n1, &n, &f_one,
	       xa, &ldx, aa + n * lda, &lda, &f_one, xa + n * ldx, &ldx);
      }

      /* Top part, lower triangular */
      dtrmm_("Right", "Upper", "Not transposed",
	     (aunit ? "Unit triangular" : "Non-unit triangular"),
	     &x_rows, &n, &f_one, aa, &lda, xa, &ldx);
    }
    else {
      /* Left upper part, upper triangular */
      dtrmm_("Right", "Upper", "Transposed",
	     (aunit ? "Unit triangular" : "Non-unit triangular"),
	     &x_rows, &n, &f_one, aa, &lda, xa, &ldx);

      /* Right part */
      if (n < a->cols) {
	n1 = a->cols - n;
	dgemm_("Not transposed", "Transposed", &x_rows, &n, &n1, &f_one,
	       xa + n * ldx, &ldx, aa + n * lda, &lda, &f_one, xa, &ldx);
      }

      /* Lower part */
      if (n < a->rows)
	for (j = 0; j < x->rows; j++)
	  for (i = n; i < a->rows; i++)
	    xa[j + i * ldx] = 0.0;
    }
  }
  else {
    assert(x->rows >= a->rows);
    assert(x->rows >= a->cols);

    if (atrans) {
      /* Lower part */
      if (n < a->cols) {
	for (j = 0; j < x->cols; j++)
	  for (i = n; i < a->cols; i++)
	    xa[i + j * ldx] = 0.0;

	n1 = a->cols - n;
	dgemm_("Transposed", "Not Transposed", &n1, &x_cols, &n, &f_one,
	       aa + n * lda, &lda, xa, &ldx, &f_one, xa + n, &ldx);
      }

      /* Top part, lower triangular */
      dtrmm_("Left", "Upper", "Transposed",
	     (aunit ? "Unit triangular" : "Non-unit triangular"),
	     &n, &x_cols, &f_one, aa, &lda, xa, &ldx);
    }
    else {
      /* Left upper part, upper triangular */
      dtrmm_("Left", "Upper", "Not transposed",
	     (aunit ? "Unit triangular" : "Non-unit triangular"),
	     &n, &x_cols, &f_one, aa, &lda, xa, &ldx);

      /* Right part */
      if (n < a->cols) {
	n1 = a->cols - n;
	dgemm_("Not transposed", "Not Transposed", &n, &x_cols, &n1, &f_one,
	       aa + n * lda, &lda, xa + n, &ldx, &f_one, xa, &ldx);
      }

      /* Lower part */
      if (n < a->rows)
	for (j = 0; j < x->cols; j++)
	  for (i = n; i < a->rows; i++)
	    xa[i + j * ldx] = 0.0;
    }
  }
}
#else
static void
lowereval_amatrix(bool aunit, bool atrans, pcamatrix a, bool xtrans,
		  pamatrix x)
{
  pcfield   aa = a->a;
  LAPACK_INT      lda = a->ld;
  pfield    xa = x->a;
  LAPACK_INT      ldx = x->ld;
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  field     newval;
  LAPACK_INT      i, j, k;

  if (xtrans) {
    assert(x->cols >= a->rows);
    assert(x->cols >= a->cols);

    if (atrans) {
      for (k = 0; k < x->rows; k++) {
	/* Left upper part, upper triangular */
	for (i = 0; i < n; i++) {
	  newval =
	    (aunit ? xa[k + i * ldx] : aa[i + i * lda] * xa[k + i * ldx]);
	  for (j = i + 1; j < n; j++)
	    newval += aa[j + i * lda] * xa[k + j * ldx];
	  xa[k + i * ldx] = newval;
	}

	/* Right part */
	if (n < a->rows)
	  for (i = 0; i < n; i++) {
	    newval = xa[k + i * ldx];
	    for (j = n; j < a->rows; j++)
	      newval += aa[j + i * lda] * xa[k + j * ldx];
	    xa[k + i * ldx] = newval;
	  }

	/* Lower part */
	if (n < a->cols)
	  for (i = n; i < a->cols; i++)
	    xa[k + i * ldx] = 0.0;
      }
    }
    else {
      for (k = 0; k < x->rows; k++) {
	/* Lower part */
	if (n < a->rows)
	  for (i = n; i < a->rows; i++) {
	    newval = 0.0;
	    for (j = 0; j < a->cols; j++)
	      newval += CONJ(aa[i + j * lda]) * xa[k + j * ldx];
	    xa[k + i * ldx] = newval;
	  }

	/* Top part, lower triangular */
	for (i = n; i-- > 0;) {
	  newval =
	    (aunit ? xa[k + i * ldx] : CONJ(aa[i + i * lda]) *
	     xa[k + i * ldx]);
	  for (j = 0; j < i; j++)
	    newval += CONJ(aa[i + j * lda]) * xa[k + j * ldx];
	  xa[k + i * ldx] = newval;
	}
      }
    }
  }
  else {
    assert(x->rows >= a->rows);
    assert(x->rows >= a->cols);

    if (atrans) {
      for (k = 0; k < x->cols; k++) {
	/* Left upper part, upper triangular */
	for (i = 0; i < n; i++) {
	  newval =
	    (aunit ? xa[i + k * ldx] : CONJ(aa[i + i * lda]) *
	     xa[i + k * ldx]);
	  for (j = i + 1; j < n; j++)
	    newval += CONJ(aa[j + i * lda]) * xa[j + k * ldx];
	  xa[i + k * ldx] = newval;
	}

	/* Right part */
	if (n < a->rows)
	  for (i = 0; i < n; i++) {
	    newval = xa[i + k * ldx];
	    for (j = n; j < a->rows; j++)
	      newval += CONJ(aa[j + i * lda]) * xa[j + k * ldx];
	    xa[i + k * ldx] = newval;
	  }

	/* Lower part */
	if (n < a->cols)
	  for (i = n; i < a->cols; i++)
	    xa[i + k * ldx] = 0.0;
      }
    }
    else {
      for (k = 0; k < x->cols; k++) {
	/* Lower part */
	if (n < a->rows)
	  for (i = n; i < a->rows; i++) {
	    newval = 0.0;
	    for (j = 0; j < a->cols; j++)
	      newval += aa[i + j * lda] * xa[j + k * ldx];
	    xa[i + k * ldx] = newval;
	  }

	/* Top part, lower triangular */
	for (i = n; i-- > 0;) {
	  newval =
	    (aunit ? xa[i + k * ldx] : aa[i + i * lda] * xa[i + k * ldx]);
	  for (j = 0; j < i; j++)
	    newval += aa[i + j * lda] * xa[j + k * ldx];
	  xa[i + k * ldx] = newval;
	}
      }
    }
  }
}

static void
uppereval_amatrix(bool aunit, bool atrans, pcamatrix a, bool xtrans,
		  pamatrix x)
{
  pcfield   aa = a->a;
  LAPACK_INT      lda = a->ld;
  pfield    xa = x->a;
  LAPACK_INT      ldx = x->ld;
  LAPACK_INT      n = UINT_MIN(a->rows, a->cols);
  field     newval;
  LAPACK_INT      i, j, k;

  if (xtrans) {
    assert(x->cols >= a->rows);
    assert(x->cols >= a->cols);

    if (atrans) {
      for (k = 0; k < x->rows; k++) {
	/* Lower part */
	if (n < a->cols)
	  for (i = n; i < a->cols; i++) {
	    newval = 0.0;
	    for (j = 0; j < a->rows; j++)
	      newval += aa[j + i * lda] * xa[k + j * ldx];
	    xa[k + i * ldx] = newval;
	  }

	/* Top part, lower triangular */
	for (i = n; i-- > 0;) {
	  newval =
	    (aunit ? xa[k + i * ldx] : aa[i + i * lda] * xa[k + i * ldx]);
	  for (j = 0; j < i; j++)
	    newval += aa[j + i * lda] * xa[k + j * ldx];
	  xa[k + i * ldx] = newval;
	}
      }
    }
    else {
      for (k = 0; k < x->rows; k++) {
	/* Left upper part, upper triangular */
	for (i = 0; i < n; i++) {
	  newval =
	    (aunit ? xa[k + i * ldx] : CONJ(aa[i + i * lda]) *
	     xa[k + i * ldx]);
	  for (j = i + 1; j < n; j++)
	    newval += CONJ(aa[i + j * lda]) * xa[k + j * ldx];
	  xa[k + i * ldx] = newval;
	}

	/* Right part */
	if (n < a->cols)
	  for (i = 0; i < n; i++) {
	    newval = xa[k + i * ldx];
	    for (j = n; j < a->cols; j++)
	      newval += CONJ(aa[i + j * lda]) * xa[k + j * ldx];
	    xa[k + i * ldx] = newval;
	  }

	/* Lower part */
	if (n < a->rows)
	  for (i = n; i < a->rows; i++)
	    xa[k + i * ldx] = 0.0;
      }
    }
  }
  else {
    assert(x->rows >= a->rows);
    assert(x->rows >= a->cols);

    if (atrans) {
      for (k = 0; k < x->cols; k++) {
	/* Lower part */
	if (n < a->cols)
	  for (i = n; i < a->cols; i++) {
	    newval = 0.0;
	    for (j = 0; j < a->rows; j++)
	      newval += CONJ(aa[j + i * lda]) * xa[j + k * ldx];
	    xa[i + k * ldx] = newval;
	  }

	/* Top part, lower triangular */
	for (i = n; i-- > 0;) {
	  newval =
	    (aunit ? xa[i + k * ldx] : CONJ(aa[i + i * lda]) *
	     xa[i + k * ldx]);
	  for (j = 0; j < i; j++)
	    newval += CONJ(aa[j + i * lda]) * xa[j + k * ldx];
	  xa[i + k * ldx] = newval;
	}
      }
    }
    else {
      for (k = 0; k < x->cols; k++) {
	/* Left upper part, upper triangular */
	for (i = 0; i < n; i++) {
	  newval =
	    (aunit ? xa[i + k * ldx] : aa[i + i * lda] * xa[i + k * ldx]);
	  for (j = i + 1; j < n; j++)
	    newval += aa[i + j * lda] * xa[j + k * ldx];
	  xa[i + k * ldx] = newval;
	}

	/* Right part */
	if (n < a->cols)
	  for (i = 0; i < n; i++) {
	    newval = xa[i + k * ldx];
	    for (j = n; j < a->cols; j++)
	      newval += aa[i + j * lda] * xa[j + k * ldx];
	    xa[i + k * ldx] = newval;
	  }

	/* Lower part */
	if (n < a->rows)
	  for (i = n; i < a->rows; i++)
	    xa[i + k * ldx] = 0.0;
      }
    }
  }
}
#endif

void
triangulareval_amatrix(bool alower, bool aunit, bool atrans, pcamatrix a,
		       bool xtrans, pamatrix x)
{
  if (alower)
    lowereval_amatrix(aunit, atrans, a, xtrans, x);
  else
    uppereval_amatrix(aunit, atrans, a, xtrans, x);
}

#ifdef USE_BLAS
IMPORT_PREFIX void
dger_(const LAPACK_INT *m,
      const LAPACK_INT *n,
      const double *alpha,
      const double *x,
      const LAPACK_INT *incx,
      const double *y, const LAPACK_INT *incy, double *a, const LAPACK_INT *lda);
#endif

void
triangularaddmul_amatrix(field alpha, bool alower, bool atrans,
			 pcamatrix a, bool blower, bool btrans, pcamatrix b,
			 pamatrix c)
{
  pcfield   aa = a->a;
  LAPACK_INT      lda = a->ld;
  pcfield   ba = b->a;
  LAPACK_INT      ldb = b->ld;
  pfield    ca = c->a;
  LAPACK_INT      ldc = c->ld;
  LAPACK_INT      aoff, adim, ainc, boff, bdim, binc;
  LAPACK_INT      j;
#ifndef USE_BLAS
  LAPACK_INT      i, k;
#endif

  if (atrans) {
    assert(c->rows == a->cols);

    ainc = lda;
    lda = 1;

    if (btrans) {
      assert(a->rows == b->cols);
      assert(c->cols == b->rows);

      binc = 1;

      for (j = 0; j < a->rows; j++) {
	if (alower) {		/* A^* upper triangular */
	  aoff = 0;
	  adim = UINT_MIN(j + 1, a->cols);
	}
	else {			/* A^* lower triangular */
	  aoff = j;
	  adim = a->cols - UINT_MIN(j, a->cols);
	}

	if (blower) {		/* B^* upper triangular */
	  boff = j;
	  bdim = b->rows - UINT_MIN(j, b->rows);
	}
	else {			/* B^* lower triangular */
	  boff = 0;
	  bdim = UINT_MIN(j + 1, b->rows);
	}

#ifdef USE_BLAS
	dger_(&adim, &bdim, &alpha,
	      aa + aoff * ainc + j * lda, &ainc,
	      ba + boff * binc + j * ldb, &binc,
	      ca + aoff + boff * ldc, &ldc);
#else
	for (k = 0; k < bdim; k++)
	  for (i = 0; i < adim; i++)
	    ca[(aoff + i) + (boff + k) * ldc] +=
	      alpha * CONJ(aa[(aoff + i) * ainc + j * lda]) *
	      CONJ(ba[(boff + k) * binc + j * ldb]);
#endif
      }
    }
    else {
      assert(a->rows == b->rows);
      assert(c->cols == b->cols);

      binc = ldb;
      ldb = 1;

      for (j = 0; j < a->rows; j++) {
	if (alower) {		/* A^* upper triangular */
	  aoff = 0;
	  adim = UINT_MIN(j + 1, a->cols);
	}
	else {			/* A^* lower triangular */
	  aoff = j;
	  adim = a->cols - UINT_MIN(j, a->cols);
	}

	if (blower) {		/* B lower triangular */
	  boff = 0;
	  bdim = UINT_MIN(j + 1, b->cols);
	}
	else {			/* B upper triangular */
	  boff = j;
	  bdim = b->cols - UINT_MIN(j, b->cols);
	}

#ifdef USE_BLAS
	dger_(&adim, &bdim, &alpha,
	      aa + aoff * ainc + j * lda, &ainc,
	      ba + boff * binc + j * ldb, &binc,
	      ca + aoff + boff * ldc, &ldc);
#else
	for (k = 0; k < bdim; k++)
	  for (i = 0; i < adim; i++)
	    ca[(aoff + i) + (boff + k) * ldc] += alpha
	      * CONJ(aa[(aoff + i) * ainc + j * lda])
	      * ba[(boff + k) * binc + j * ldb];
#endif
      }
    }
  }
  else {
    assert(c->rows == a->rows);

    ainc = 1;

    if (btrans) {
      assert(a->cols == b->cols);
      assert(c->cols == b->rows);

      binc = 1;

      for (j = 0; j < a->cols; j++) {
	if (alower) {		/* A lower triangular */
	  aoff = j;
	  adim = a->rows - UINT_MIN(j, a->rows);
	}
	else {			/* A upper triangular */
	  aoff = 0;
	  adim = UINT_MIN(j + 1, a->rows);
	}

	if (blower) {		/* B^* upper triangular */
	  boff = j;
	  bdim = b->rows - UINT_MIN(j, b->rows);
	}
	else {			/* B^* lower triangular */
	  boff = 0;
	  bdim = UINT_MIN(j + 1, b->rows);
	}

#ifdef USE_BLAS
	dger_(&adim, &bdim, &alpha,
	      aa + aoff * ainc + j * lda, &ainc,
	      ba + boff * binc + j * ldb, &binc,
	      ca + aoff + boff * ldc, &ldc);
#else
	for (k = 0; k < bdim; k++)
	  for (i = 0; i < adim; i++)
	    ca[(aoff + i) + (boff + k) * ldc] += alpha
	      * aa[(aoff + i) * ainc + j * lda]
	      * CONJ(ba[(boff + k) * binc + j * ldb]);
#endif
      }
    }
    else {
      assert(a->cols == b->rows);
      assert(c->cols == b->cols);

      binc = ldb;
      ldb = 1;

      for (j = 0; j < a->cols; j++) {
	if (alower) {		/* A lower triangular */
	  aoff = j;
	  adim = a->rows - UINT_MIN(j, a->rows);
	}
	else {			/* A upper triangular */
	  aoff = 0;
	  adim = UINT_MIN(j + 1, a->rows);
	}

	if (blower) {		/* B lower triangular */
	  boff = 0;
	  bdim = UINT_MIN(j + 1, b->cols);
	}
	else {			/* B upper triangular */
	  boff = j;
	  bdim = b->cols - UINT_MIN(j, b->cols);
	}

#ifdef USE_BLAS
	dger_(&adim, &bdim, &alpha,
	      aa + aoff * ainc + j * lda, &ainc,
	      ba + boff * binc + j * ldb, &binc,
	      ca + aoff + boff * ldc, &ldc);
#else

	for (k = 0; k < bdim; k++)
	  for (i = 0; i < adim; i++)
	    ca[(aoff + i) + (boff + k) * ldc] += alpha
	      * aa[(aoff + i) * ainc + j * lda]
	      * ba[(boff + k) * binc + j * ldb];
#endif
      }
    }
  }
}

void
copy_lower_amatrix(pcamatrix a, bool aunit, pamatrix b)
{
  pfield    aa = a->a;
  LAPACK_INT      lda = a->ld;
  pfield    ba = b->a;
  LAPACK_INT      ldb = b->ld;
  LAPACK_INT      rows;
  LAPACK_INT      cols;
  LAPACK_INT      i, j;

  rows = UINT_MIN(a->rows, b->rows);
  cols = UINT_MIN(a->cols, b->cols);

  for (j = 0; j < cols; j++) {
    if (aunit) {
      for (i = 0; i < rows && i < j; i++)
	ba[i + j * ldb] = 0.0;

      if (i == j) {
	ba[i + j * ldb] = 1.0;
	i++;
      }
    }
    else
      for (i = 0; i < rows && i < j; i++)
	ba[i + j * ldb] = 0.0;

    for (; i < rows; i++)
      ba[i + j * ldb] = aa[i + j * lda];
  }
  for (; j < b->cols; j++)
    for (i = 0; i < rows; i++)
      ba[i + j * ldb] = 0.0;
}

void
copy_upper_amatrix(pcamatrix a, bool aunit, pamatrix b)
{
  pfield    aa = a->a;
  LAPACK_INT      lda = a->ld;
  pfield    ba = b->a;
  LAPACK_INT      ldb = b->ld;
  LAPACK_INT      rows;
  LAPACK_INT      cols;
  LAPACK_INT      i, j;

  rows = UINT_MIN(a->rows, b->rows);
  cols = UINT_MIN(a->cols, b->cols);

  for (j = 0; j < cols; j++) {
    if (aunit) {
      for (i = 0; i < rows && i < j; i++)
	ba[i + j * ldb] = aa[i + j * lda];

      if (i == j && i < rows) {
	ba[i + j * ldb] = 1.0;
	i++;
      }
    }
    else {
      for (i = 0; i < rows && i <= j; i++)
	ba[i + j * ldb] = aa[i + j * lda];
    }

    for (; i < b->rows; i++)
      ba[i + j * ldb] = 0.0;
  }
}

/* ------------------------------------------------------------
 LR decomposition
 ------------------------------------------------------------ */

#ifdef USE_BLAS
IMPORT_PREFIX void
dscal_(const LAPACK_INT *n,
       const double *alpha, double *x, const LAPACK_INT *incx);

IMPORT_PREFIX void
dger_(const LAPACK_INT *m,
      const LAPACK_INT *n,
      const double *alpha,
      const double *x,
      const LAPACK_INT *incx,
      const double *y, const LAPACK_INT *incy, double *a, const LAPACK_INT *lda);

uint
lrdecomp_amatrix(pamatrix a)
{
  double   *aa = a->a;
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      n = a->rows;
  double    alpha;
  LAPACK_INT      i, n1;

  assert(n == a->cols);

  for (i = 0; i < n - 1; i++) {
    if (aa[i + i * lda] == 0.0)
      return i + 1;

    alpha = 1.0 / aa[i + i * lda];

    n1 = n - i - 1;
    dscal_(&n1, &alpha, aa + (i + 1) + i * lda, &l_one);
    dger_(&n1, &n1,
	  &f_minusone,
	  aa + (i + 1) + i * lda, &l_one,
	  aa + i + (i + 1) * lda, &lda, aa + (i + 1) + (i + 1) * lda, &lda);
  }

  if (aa[i + i * lda] == 0.0)
    return i + 1;

  return 0;
}
#else
uint
lrdecomp_amatrix(pamatrix a)
{
  pfield    aa = a->a;
  uint      lda = a->ld;
  uint      n = a->rows;
  field     alpha;
  uint      i, j, k;

  assert(n == a->cols);

  for (i = 0; i < n - 1; i++) {
    if (aa[i + i * lda] == 0.0)
      return i + 1;

    alpha = 1.0 / aa[i + i * lda];

    for (j = i + 1; j < n; j++) {
      aa[j + i * lda] *= alpha;
      for (k = i + 1; k < n; k++)
	aa[j + k * lda] -= aa[j + i * lda] * aa[i + k * lda];
    }
  }

  if (aa[i + i * lda] == 0.0)
    return i + 1;

  return 0;
}
#endif

void
lrsolve_amatrix_avector(pcamatrix a, pavector x)
{
  triangularsolve_amatrix_avector(true, true, false, a, x);
  triangularsolve_amatrix_avector(false, false, false, a, x);
}

/* ------------------------------------------------------------
 Cholesky decomposition
 ------------------------------------------------------------ */

#ifdef USE_BLAS
IMPORT_PREFIX void
dpotrf_(const char *uplo,
	const LAPACK_INT *n, double *a, const LAPACK_INT *lda, LAPACK_INT *info);

uint
choldecomp_amatrix(pamatrix a)
{
  double   *aa = a->a;
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      n = a->rows;

  LAPACK_INT      info;

  assert(n == a->cols);

  dpotrf_("Lower Part", &n, aa, &lda, &info);

  return info;
}
#else
uint
choldecomp_amatrix(pamatrix a)
{
  pfield    aa = a->a;
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      n = a->rows;
  real      diag, alpha;
  LAPACK_INT      i, j, k;

  assert(n == a->cols);

  for (i = 0; i < n - 1; i++) {
    diag = REAL(aa[i + i * lda]);

    if (ABS(aa[i + i * lda] - diag) > 1e-12 || diag <= 0.0)
      return i + 1;

    aa[i + i * lda] = REAL_SQRT(diag);
    alpha = 1.0 / aa[i + i * lda];
    for (j = i + 1; j < n; j++)
      aa[j + i * lda] *= alpha;

    for (j = i + 1; j < n; j++)
      for (k = i + 1; k <= j; k++)
	aa[j + k * lda] -= aa[j + i * lda] * CONJ(aa[k + i * lda]);
  }

  diag = REAL(aa[i + i * lda]);
  if (ABS(aa[i + i * lda] - diag) > 1e-12 || diag <= 0.0)
    return i + 1;

  aa[i + i * lda] = REAL_SQRT(diag);

  return 0;
}
#endif

void
cholsolve_amatrix_avector(pcamatrix a, pavector x)
{
  triangularsolve_amatrix_avector(true, false, false, a, x);
  triangularsolve_amatrix_avector(true, false, true, a, x);
}

/* ------------------------------------------------------------
 LDL^T decomposition
 ------------------------------------------------------------ */

#ifdef USE_BLAS
IMPORT_PREFIX void
dscal_(const LAPACK_INT *n,
       const double *alpha, double *x, const LAPACK_INT *incx);

IMPORT_PREFIX void
dsyr_(const char *uplo,
      const LAPACK_INT *n,
      const double *alpha,
      const double *x, const LAPACK_INT *incx, double *a, const LAPACK_INT *lda);

uint
ldltdecomp_amatrix(pamatrix a)
{
  double   *aa = a->a;
  LAPACK_INT      lda = a->ld;
  LAPACK_INT      n = a->rows;
  double    diag, alpha;
  LAPACK_INT      i, n1;

  assert(n == a->cols);

  for (i = 0; i < n - 1; i++) {
    diag = REAL(aa[i + i * lda]);

    if (ABS(aa[i + i * lda] - diag) > 1e-12 || diag == 0.0)
      return i + 1;

    alpha = 1.0 / diag;
    n1 = n - i - 1;
    dscal_(&n1, &alpha, aa + (i + 1) + i * lda, &l_one);

    alpha = -diag;
    dsyr_("Lower part", &n1,
	  &alpha,
	  aa + (i + 1) + i * lda, &l_one, aa + (i + 1) + (i + 1) * lda, &lda);
  }

  diag = REAL(aa[i + i * lda]);
  if (ABS(aa[i + i * lda] - diag) > 1e-12 || diag == 0.0)
    return i + 1;

  return 0;
}
#else
uint
ldltdecomp_amatrix(pamatrix a)
{
  pfield    aa = a->a;
  uint      lda = a->ld;
  uint      n = a->rows;
  real      diag, alpha;
  uint      i, j, k;

  assert(n == a->cols);

  for (i = 0; i < n - 1; i++) {
    diag = REAL(aa[i + i * lda]);

    if (ABS(aa[i + i * lda] - diag) > 1e-12 || diag == 0.0)
      return i + 1;

    alpha = 1.0 / diag;
    for (j = i + 1; j < n; j++)
      aa[j + i * lda] *= alpha;

    for (j = i + 1; j < n; j++)
      for (k = i + 1; k <= j; k++)
	aa[j + k * lda] -= diag * aa[j + i * lda] * CONJ(aa[k + i * lda]);
  }

  diag = REAL(aa[i + i * lda]);
  if (ABS(aa[i + i * lda] - diag) > 1e-12 || diag == 0.0)
    return i + 1;

  return 0;
}
#endif

void
ldltsolve_amatrix_avector(pcamatrix a, pavector x)
{
  triangularsolve_amatrix_avector(true, true, false, a, x);
  diagsolve_amatrix_avector(false, a, x);
  triangularsolve_amatrix_avector(true, true, true, a, x);
}

/* ------------------------------------------------------------
 Orthogonal decompositions
 ------------------------------------------------------------ */

#ifdef USE_BLAS
IMPORT_PREFIX void
dgeqrf_(LAPACK_INT *m,
	LAPACK_INT *n,
	double *a,
	LAPACK_INT *lda,
	double *tau, double *work, LAPACK_INT *lwork, LAPACK_INT *info);

void
qrdecomp_amatrix(pamatrix a, pavector tau)
{
  LAPACK_INT      rows = a->rows;
  LAPACK_INT      cols = a->cols;
  LAPACK_INT      refl = UINT_MIN(rows, cols);
  LAPACK_INT      a_ld = a->ld;
  double          *work;
  LAPACK_INT      lwork, info;

  assert(a->ld >= rows);

  /* Quick exit if no reflections used */
  if (refl == 0)
    return;

  lwork = 4 * cols;
  work = allocfield(lwork);

  if (tau->dim < refl)
    resize_avector(tau, refl);

  dgeqrf_(&rows, &cols, a->a, &a_ld, tau->v, work, &lwork, &info);
  assert(info == 0);

  freemem (work);
}
#else
void
qrdecomp_amatrix(pamatrix a, pavector tau)
{
  pfield    aa = a->a;
  uint      lda = a->ld;
  pfield    tauv;
  uint      rows = a->rows;
  uint      cols = a->cols;
  uint      refl = UINT_MIN(rows, cols);
  field     alpha, beta, gamma, diag;
  real      norm2, norm;
  uint      i, j, k;

  /* Provide enough storage for scaling factors */
  if (tau->dim < refl)
    resize_avector(tau, refl);
  tauv = tau->v;

  for (k = 0; k < refl; k++) {
    /* Compute norm of k-th column */
    norm2 = 0.0;
    for (i = k; i < rows; i++)
      norm2 += ABSSQR(aa[i + k * lda]);
    norm = REAL_SQRT(norm2);

    if (norm2 == 0.0)
      tauv[k] = 0.0;
    else {
      /* Determine reflection vector v */
      diag = aa[k + k * lda];
      alpha = -SIGN(diag) * norm;

      /* Compute norm of v */
      beta = 1.0 / (norm2 - CONJ(alpha) * diag);

      /* Rescale to ensure v_1 = 1 */
      beta *= ABSSQR(diag - alpha);
      gamma = 1.0 / (diag - alpha);
      for (i = k + 1; i < rows; i++)
	aa[i + k * lda] *= gamma;
      tauv[k] = beta;

      /* Compute k-th column */
      aa[k + k * lda] = alpha;

      /* Update remaining columns */
      for (j = k + 1; j < cols; j++) {
	gamma = aa[k + j * lda];
	for (i = k + 1; i < rows; i++)
	  gamma += CONJ(aa[i + k * lda]) * aa[i + j * lda];

	gamma *= beta;

	aa[k + j * lda] -= gamma;
	for (i = k + 1; i < rows; i++)
	  aa[i + j * lda] -= gamma * aa[i + k * lda];
      }
    }
  }
}
#endif

/* Remark: if compiled the wrong way, DORMQR is currently not
 * thread-safe.
 * gfortran does the right thing if called with "-frecursive", but this
 * appears not to be the standard in, e.g., OpenSUSE Linux. */
#if defined(USE_BLAS) && (defined(THREADSAFE_LAPACK) || !defined(USE_OPENMP))
IMPORT_PREFIX void
dormqr_(const char *side,
	const char *trans,
	const LAPACK_INT *m,
	const LAPACK_INT *n,
	const LAPACK_INT *k,
	const double *a,
	const LAPACK_INT *lda,
	const double *tau,
	double *c,
	const LAPACK_INT *ldc, double *work, const LAPACK_INT *lwork, LAPACK_INT *info);

void
qreval_amatrix_avector(bool qtrans, pcamatrix a, pcavector tau, pavector x)
{
  LAPACK_INT      rows = a->rows;
  LAPACK_INT      cols = a->cols;
  LAPACK_INT      refl;
  double          * work = NULL; 
  LAPACK_INT      lwork, info;
  LAPACK_INT      a_ld = a->ld, x_dim = x->dim;

  refl = UINT_MIN(rows, cols);

  if (refl < 1)
    return;

  assert(tau->dim >= refl);
  assert(x->dim >= rows);

  lwork = 16 * UINT_MAX(a->rows, a->cols);
  work = malloc (lwork * sizeof (double));

  if (qtrans) {
    dormqr_("Left", "Transposed",
	    &rows, &l_one, &refl,
	    a->a, &a_ld, tau->v, x->v, &x_dim, work, &lwork, &info);
    assert(info == 0);
  }
  else {
    dormqr_("Left", "Not Transposed",
	    &rows, &l_one, &refl,
	    a->a, &a_ld, tau->v, x->v, &x_dim, work, &lwork, &info);
    assert(info == 0);
  }

  free (work);
}

void
qreval_amatrix(bool qtrans, pcamatrix a, pcavector tau, pamatrix x)
{
  LAPACK_INT      rows = a->rows;
  LAPACK_INT      cols = a->cols;
  LAPACK_INT      refl;
  double          *work;
  LAPACK_INT      lwork, info;
  LAPACK_INT      a_ld = a->ld, x_ld = x->ld, x_cols = x->cols;

  refl = UINT_MIN(rows, cols);

  if (refl < 1 || x->cols < 1)
    return;

  assert(tau->dim >= refl);
  assert(x->rows >= rows);

  lwork = 4 * x->cols;
  work = (double *) allocmem((size_t) sizeof(double) * lwork);

  if (qtrans) {
    dormqr_("Left", "Transposed",
	    &rows, &x_cols, &refl,
	    a->a, &a_ld, tau->v, x->a, &x_ld, work, &lwork, &info);
    assert(info == 0);
  }
  else {
    dormqr_("Left", "Not Transposed",
	    &rows, &x_cols, &refl,
	    a->a, &a_ld, tau->v, x->a, &x_ld, work, &lwork, &info);
    assert(info == 0);
  }

  freemem(work);
}
#else
void
qreval_amatrix_avector(bool qtrans, pcamatrix a, pcavector tau, pavector x)
{
  pcfield   aa = a->a;
  uint      lda = a->ld;
  pcfield   tauv = tau->v;
  pfield    xv = x->v;
  uint      rows = a->rows;
  uint      cols = a->cols;
  uint      refl;
  field     beta, gamma;
  uint      i, k;

  refl = UINT_MIN(rows, cols);

  assert(tau->dim >= refl);
  assert(x->dim >= rows);

  if (qtrans) {
    for (k = 0; k < refl; k++) {
      beta = tauv[k];

      if (beta != 0.0) {
	gamma = xv[k];
	for (i = k + 1; i < rows; i++)
	  gamma += CONJ(aa[i + k * lda]) * xv[i];

	gamma *= beta;

	xv[k] -= gamma;
	for (i = k + 1; i < rows; i++)
	  xv[i] -= gamma * aa[i + k * lda];
      }
    }
  }
  else {
    for (k = refl; k-- > 0;) {
      beta = tauv[k];

      if (beta != 0.0) {
	gamma = xv[k];
	for (i = k + 1; i < rows; i++)
	  gamma += CONJ(aa[i + k * lda]) * xv[i];

	gamma *= beta;

	xv[k] -= gamma;
	for (i = k + 1; i < rows; i++)
	  xv[i] -= gamma * aa[i + k * lda];
      }
    }
  }
}

void
qreval_amatrix(bool qtrans, pcamatrix a, pcavector tau, pamatrix x)
{
  pcfield   aa = a->a;
  uint      lda = a->ld;
  pcfield   tauv = tau->v;
  pfield    xa = x->a;
  uint      ldx = x->ld;
  uint      rows = a->rows;
  uint      cols = a->cols;
  uint      refl;
  field     beta, gamma;
  uint      i, j, k;

  refl = UINT_MIN(rows, cols);

  assert(tau->dim >= refl);
  assert(x->rows >= rows);

  if (qtrans) {
    for (k = 0; k < refl; k++) {
      beta = tauv[k];

      if (beta != 0.0)
	for (j = 0; j < x->cols; j++) {
	  gamma = xa[k + j * ldx];
	  for (i = k + 1; i < rows; i++)
	    gamma += CONJ(aa[i + k * lda]) * xa[i + j * ldx];

	  gamma *= beta;

	  xa[k + j * ldx] -= gamma;
	  for (i = k + 1; i < rows; i++)
	    xa[i + j * ldx] -= gamma * aa[i + k * lda];
	}
    }
  }
  else {
    for (k = refl; k-- > 0;) {
      beta = tauv[k];

      if (beta != 0.0)
	for (j = 0; j < x->cols; j++) {
	  gamma = xa[k + j * ldx];
	  for (i = k + 1; i < rows; i++)
	    gamma += CONJ(aa[i + k * lda]) * xa[i + j * ldx];

	  gamma *= beta;

	  xa[k + j * ldx] -= gamma;
	  for (i = k + 1; i < rows; i++)
	    xa[i + j * ldx] -= gamma * aa[i + k * lda];
	}
    }
  }
}
#endif

void
qrsolve_amatrix_avector(pcamatrix a, pcavector tau, pavector x)
{
  qreval_amatrix_avector(true, a, tau, x);
  triangularsolve_amatrix_avector(false, false, false, a, x);
}

void
qrinvert_amatrix(pamatrix a)
{
  pamatrix  acopy;
  pavector  tau, b;
  amatrix   atmp;
  avector   ttmp, btmp;
  uint      n = a->rows;
  uint      i;

  assert(n == a->cols);

  acopy = init_amatrix(&atmp, n, n);
  tau = init_avector(&ttmp, n);

  copy_amatrix(false, a, acopy);

  qrdecomp_amatrix(acopy, tau);

  identity_amatrix(a);

  for (i = 0; i < n; i++) {
    b = init_column_avector(&btmp, a, i);
    qrsolve_amatrix_avector(acopy, tau, b);
    uninit_avector(b);
  }

  uninit_avector(tau);
  uninit_amatrix(acopy);
}

#ifdef USE_BLAS
IMPORT_PREFIX void
dorgqr_(const LAPACK_INT *m,
	const LAPACK_INT *n,
	const LAPACK_INT *k,
	double *a,
	const LAPACK_INT *lda,
	double *tau, double *work, const LAPACK_INT *lwork, LAPACK_INT *info);

void
qrexpand_amatrix(pcamatrix a, pcavector tau, pamatrix q)
{
  double      *work;
  LAPACK_INT  refl, lwork, info;
  LAPACK_INT  q_rows = q->rows, q_cols = q->cols, q_ld = q->ld;

  refl = UINT_MIN3(q->cols, a->rows, a->cols);

  /* Quick exit if no reflections used */
  if (refl == 0) {
    identity_amatrix(q);
    return;
  }

  copy_sub_amatrix(false, a, q);

  lwork = 4 * a->rows;
  work = allocfield(lwork);

  dorgqr_(&q_rows, &q_cols, &refl,
	  q->a, &q_ld, tau->v, work, &lwork, &info);
  assert(info == 0);

  freemem(work);
}
#else
void
qrexpand_amatrix(pcamatrix a, pcavector tau, pamatrix q)
{
  pcfield   aa = a->a;
  uint      lda = a->ld;
  pcfield   tauv = tau->v;
  pfield    qa = q->a;
  uint      ldq = q->ld;
  uint      rows = a->rows;
  uint      cols = a->cols;
  uint      refl;
  field     beta, gamma;
  uint      i, j, k;

  /* Determine number of relevant elementary reflections */
  refl = UINT_MIN3(q->cols, rows, cols);

  assert(tau->dim >= refl);
  assert(q->rows >= rows);
  assert(q->cols <= rows);

  /* Create identity matrix */
  for (j = 0; j < q->cols; j++) {
    for (i = 0; i < rows; i++)
      qa[i + j * ldq] = 0.0;
    qa[j + j * ldq] = 1.0;
  }

  /* Apply reflections in reversed order */
  for (k = refl; k-- > 0;) {
    beta = tauv[k];
    if (beta != 0.0)
      for (j = k; j < q->cols; j++) {
	gamma = qa[k + j * ldq];
	for (i = k + 1; i < rows; i++)
	  gamma += CONJ(aa[i + k * lda]) * qa[i + j * ldq];

	gamma *= beta;

	qa[k + j * ldq] -= gamma;
	for (i = k + 1; i < rows; i++)
	  qa[i + j * ldq] -= gamma * aa[i + k * lda];
      }
  }
}
#endif
