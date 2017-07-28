/*
  Defines basic operations for the MATSEQAIJMKL matrix class.
  This class is derived from the MATSEQAIJ class and retains the
  compressed row storage (aka Yale sparse matrix format) but uses 
  sparse BLAS operations from the Intel Math Kernel Library (MKL) 
  wherever possible.
*/

#include <../src/mat/impls/aij/seq/aij.h>
#include <../src/mat/impls/aij/seq/aijmkl/aijmkl.h>

/* MKL include files. */
#include <mkl_spblas.h>  /* Sparse BLAS */

typedef struct {
  PetscBool no_SpMV2;  /* If PETSC_TRUE, then don't use the MKL SpMV2 inspector-executor routines. */
  PetscBool sparse_optimized; /* If PETSC_TRUE, then mkl_sparse_optimize() has been called. */
#ifdef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
  sparse_matrix_t csrA; /* "Handle" used by SpMV2 inspector-executor routines. */
  struct matrix_descr descr;
#endif
} Mat_SeqAIJMKL;

extern PetscErrorCode MatAssemblyEnd_SeqAIJ(Mat,MatAssemblyType);

#undef __FUNCT__
#define __FUNCT__ "MatConvert_SeqAIJMKL_SeqAIJ"
PETSC_INTERN PetscErrorCode MatConvert_SeqAIJMKL_SeqAIJ(Mat A,MatType type,MatReuse reuse,Mat *newmat)
{
  /* This routine is only called to convert a MATAIJMKL to its base PETSc type, */
  /* so we will ignore 'MatType type'. */
  PetscErrorCode ierr;
  Mat            B       = *newmat;
  Mat_SeqAIJMKL  *aijmkl=(Mat_SeqAIJMKL*)A->spptr;

  PetscFunctionBegin;
  if (reuse == MAT_INITIAL_MATRIX) {
    ierr = MatDuplicate(A,MAT_COPY_VALUES,&B);CHKERRQ(ierr);
    aijmkl = (Mat_SeqAIJMKL*)B->spptr;
  }

  /* Reset the original function pointers. */
  B->ops->duplicate        = MatDuplicate_SeqAIJ;
  B->ops->assemblyend      = MatAssemblyEnd_SeqAIJ;
  B->ops->destroy          = MatDestroy_SeqAIJ;
  B->ops->mult             = MatMult_SeqAIJ;
  B->ops->multtranspose    = MatMultTranspose_SeqAIJ;
  B->ops->multadd          = MatMultAdd_SeqAIJ;
  B->ops->multtransposeadd = MatMultTransposeAdd_SeqAIJ;

  ierr = PetscObjectComposeFunction((PetscObject)B,"MatConvert_seqaijmkl_seqaij_C",NULL);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)B,"MatMatMult_seqdense_seqaijmkl_C",NULL);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)B,"MatMatMultSymbolic_seqdense_seqaijmkl_C",NULL);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)B,"MatMatMultNumeric_seqdense_seqaijmkl_C",NULL);CHKERRQ(ierr);

  /* Free everything in the Mat_SeqAIJMKL data structure. Currently, this 
   * simply involves destroying the MKL sparse matrix handle and then freeing 
   * the spptr pointer. */
#ifdef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
  if (aijmkl->sparse_optimized) {
    sparse_status_t stat;
    stat = mkl_sparse_destroy(aijmkl->csrA);
    if (stat != SPARSE_STATUS_SUCCESS) {
      PetscFunctionReturn(PETSC_ERR_LIB);
    }
  }
#endif /* PETSC_HAVE_MKL_SPARSE_OPTIMIZE */
  ierr = PetscFree(B->spptr);CHKERRQ(ierr);

  /* Change the type of B to MATSEQAIJ. */
  ierr = PetscObjectChangeTypeName((PetscObject)B, MATSEQAIJ);CHKERRQ(ierr);

  *newmat = B;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatDestroy_SeqAIJMKL"
PetscErrorCode MatDestroy_SeqAIJMKL(Mat A)
{
  PetscErrorCode ierr;
  Mat_SeqAIJMKL *aijmkl = (Mat_SeqAIJMKL*) A->spptr;

  PetscFunctionBegin;

  /* If MatHeaderMerge() was used, then this SeqAIJMKL matrix will not have an 
   * spptr pointer. */
  if (aijmkl) {
    /* Clean up everything in the Mat_SeqAIJMKL data structure, then free A->spptr. */
#ifdef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
    if (aijmkl->sparse_optimized) {
      sparse_status_t stat = SPARSE_STATUS_SUCCESS;
      stat = mkl_sparse_destroy(aijmkl->csrA);
      if (stat != SPARSE_STATUS_SUCCESS) {
        PetscFunctionReturn(PETSC_ERR_LIB);
      }
    }
#endif /* PETSC_HAVE_MKL_SPARSE_OPTIMIZE */
    ierr = PetscFree(A->spptr);CHKERRQ(ierr);
  }

  /* Change the type of A back to SEQAIJ and use MatDestroy_SeqAIJ()
   * to destroy everything that remains. */
  ierr = PetscObjectChangeTypeName((PetscObject)A, MATSEQAIJ);CHKERRQ(ierr);
  /* Note that I don't call MatSetType().  I believe this is because that
   * is only to be called when *building* a matrix.  I could be wrong, but
   * that is how things work for the SuperLU matrix class. */
  ierr = MatDestroy_SeqAIJ(A);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatDuplicate_SeqAIJMKL"
PetscErrorCode MatDuplicate_SeqAIJMKL(Mat A, MatDuplicateOption op, Mat *M)
{
  PetscErrorCode ierr;
  Mat_SeqAIJMKL *aijmkl;
  Mat_SeqAIJMKL *aijmkl_dest;

  PetscFunctionBegin;
  ierr = MatDuplicate_SeqAIJ(A,op,M);CHKERRQ(ierr);
  aijmkl      = (Mat_SeqAIJMKL*) A->spptr;
  aijmkl_dest = (Mat_SeqAIJMKL*) (*M)->spptr;
  ierr = PetscMemcpy(aijmkl_dest,aijmkl,sizeof(Mat_SeqAIJMKL));CHKERRQ(ierr);
  aijmkl_dest->sparse_optimized = PETSC_FALSE;
#ifdef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
  aijmkl_dest->csrA = NULL;
  if (!aijmkl->no_SpMV2) {
    sparse_status_t stat;
    stat = mkl_sparse_copy(aijmkl->csrA,aijmkl->descr,&aijmkl_dest->csrA);
    if (stat != SPARSE_STATUS_SUCCESS) {
      SETERRQ(PETSC_COMM_SELF,PETSC_ERR_LIB,"Intel MKL error: unable to complete mkl_sparse_copy");
      PetscFunctionReturn(PETSC_ERR_LIB);
    }
    stat = mkl_sparse_optimize(aijmkl_dest->csrA);
    if (stat != SPARSE_STATUS_SUCCESS) {
      SETERRQ(PETSC_COMM_SELF,PETSC_ERR_LIB,"Intel MKL error: unable to complete mkl_sparse_optimize");
      PetscFunctionReturn(PETSC_ERR_LIB);
    }
    aijmkl_dest->sparse_optimized = PETSC_TRUE;
  }
#endif /* PETSC_HAVE_MKL_SPARSE_OPTIMIZE */
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatAssemblyEnd_SeqAIJMKL"
PetscErrorCode MatAssemblyEnd_SeqAIJMKL(Mat A, MatAssemblyType mode)
{
  PetscErrorCode  ierr;
  Mat_SeqAIJ      *a = (Mat_SeqAIJ*)A->data;
  Mat_SeqAIJMKL   *aijmkl;

  MatScalar       *aa;
  PetscInt        m,n;
  PetscInt        *aj,*ai;

  PetscFunctionBegin;
  if (mode == MAT_FLUSH_ASSEMBLY) PetscFunctionReturn(0);

  /* Since a MATSEQAIJMKL matrix is really just a MATSEQAIJ with some
   * extra information and some different methods, call the AssemblyEnd 
   * routine for a MATSEQAIJ.
   * I'm not sure if this is the best way to do this, but it avoids
   * a lot of code duplication.
   * I also note that currently MATSEQAIJMKL doesn't know anything about
   * the Mat_CompressedRow data structure that SeqAIJ now uses when there
   * are many zero rows.  If the SeqAIJ assembly end routine decides to use
   * this, this may break things.  (Don't know... haven't looked at it. 
   * Do I need to disable this somehow?) */
  a->inode.use = PETSC_FALSE;  /* Must disable: otherwise the MKL routines won't get used. */
  ierr         = MatAssemblyEnd_SeqAIJ(A, mode);CHKERRQ(ierr);

  aijmkl = (Mat_SeqAIJMKL*) A->spptr;
#ifdef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
  if (!aijmkl->no_SpMV2) {
    sparse_status_t stat;
    if (aijmkl->sparse_optimized) {
      /* Matrix has been previously assembled and optimized. Must destroy old
       * matrix handle before running the optimization step again. */
      sparse_status_t stat;
      stat = mkl_sparse_destroy(aijmkl->csrA);
      if (stat != SPARSE_STATUS_SUCCESS) {
        PetscFunctionReturn(PETSC_ERR_LIB);
      }
    }
    /* Now perform the SpMV2 setup and matrix optimization. */
    aijmkl->descr.type        = SPARSE_MATRIX_TYPE_GENERAL;
    aijmkl->descr.mode        = SPARSE_FILL_MODE_LOWER;
    aijmkl->descr.diag        = SPARSE_DIAG_NON_UNIT;
    m = A->rmap->n;
    n = A->cmap->n;
    aj   = a->j;  /* aj[k] gives column index for element aa[k]. */
    aa   = a->a;  /* Nonzero elements stored row-by-row. */
    ai   = a->i;  /* ai[k] is the position in aa and aj where row k starts. */
    stat = mkl_sparse_x_create_csr (&aijmkl->csrA,SPARSE_INDEX_BASE_ZERO,m,n,ai,ai+1,aj,aa);
    stat = mkl_sparse_set_mv_hint(aijmkl->csrA,SPARSE_OPERATION_NON_TRANSPOSE,aijmkl->descr,1000);
    stat = mkl_sparse_set_memory_hint(aijmkl->csrA,SPARSE_MEMORY_AGGRESSIVE);
    stat = mkl_sparse_optimize(aijmkl->csrA);
    if (stat != SPARSE_STATUS_SUCCESS) {
      SETERRQ(PETSC_COMM_SELF,PETSC_ERR_LIB,"Intel MKL error: unable to create matrix handle/complete mkl_sparse_optimize");
      PetscFunctionReturn(PETSC_ERR_LIB);
    }
    aijmkl->sparse_optimized = PETSC_TRUE;
  }
#endif

  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatMult_SeqAIJMKL"
PetscErrorCode MatMult_SeqAIJMKL(Mat A,Vec xx,Vec yy)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  const PetscScalar *x;
  PetscScalar       *y;
  const MatScalar   *aa;
  PetscErrorCode    ierr;
  PetscInt          m=A->rmap->n;
  const PetscInt    *aj,*ai;

  /* Variables not in MatMult_SeqAIJ. */
  char transa = 'n';  /* Used to indicate to MKL that we are not computing the transpose product. */

  PetscFunctionBegin;
  ierr = VecGetArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecGetArray(yy,&y);CHKERRQ(ierr);
  aj   = a->j;  /* aj[k] gives column index for element aa[k]. */
  aa   = a->a;  /* Nonzero elements stored row-by-row. */
  ai   = a->i;  /* ai[k] is the position in aa and aj where row k starts. */

  /* Call MKL sparse BLAS routine to do the MatMult. */
  mkl_cspblas_xcsrgemv(&transa,&m,aa,ai,aj,x,y);

  ierr = PetscLogFlops(2.0*a->nz - a->nonzerorowcnt);CHKERRQ(ierr);
  ierr = VecRestoreArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecRestoreArray(yy,&y);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#ifdef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
#undef __FUNCT__
#define __FUNCT__ "MatMult_SeqAIJMKL_SpMV2"
PetscErrorCode MatMult_SeqAIJMKL_SpMV2(Mat A,Vec xx,Vec yy)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  Mat_SeqAIJMKL     *aijmkl=(Mat_SeqAIJMKL*)A->spptr;
  const PetscScalar *x;
  PetscScalar       *y;
  PetscErrorCode    ierr;
  sparse_status_t stat = SPARSE_STATUS_SUCCESS;

  PetscFunctionBegin;

  ierr = VecGetArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecGetArray(yy,&y);CHKERRQ(ierr);

  /* Call MKL SpMV2 executor routine to do the MatMult. */
  stat = mkl_sparse_x_mv(SPARSE_OPERATION_NON_TRANSPOSE,1.0,aijmkl->csrA,aijmkl->descr,x,0.0,y);
  
  ierr = PetscLogFlops(2.0*a->nz - a->nonzerorowcnt);CHKERRQ(ierr);
  ierr = VecRestoreArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecRestoreArray(yy,&y);CHKERRQ(ierr);
  if (stat != SPARSE_STATUS_SUCCESS) {
    PetscFunctionReturn(PETSC_ERR_LIB);
  }
  PetscFunctionReturn(0);
}
#endif /* PETSC_HAVE_MKL_SPARSE_OPTIMIZE */

#undef __FUNCT__
#define __FUNCT__ "MatMultTranspose_SeqAIJMKL"
PetscErrorCode MatMultTranspose_SeqAIJMKL(Mat A,Vec xx,Vec yy)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  const PetscScalar *x;
  PetscScalar       *y;
  const MatScalar   *aa;
  PetscErrorCode    ierr;
  PetscInt          m=A->rmap->n;
  const PetscInt    *aj,*ai;

  /* Variables not in MatMultTranspose_SeqAIJ. */
  char transa = 't';  /* Used to indicate to MKL that we are computing the transpose product. */

  PetscFunctionBegin;
  ierr = VecGetArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecGetArray(yy,&y);CHKERRQ(ierr);
  aj   = a->j;  /* aj[k] gives column index for element aa[k]. */
  aa   = a->a;  /* Nonzero elements stored row-by-row. */
  ai   = a->i;  /* ai[k] is the position in aa and aj where row k starts. */

  /* Call MKL sparse BLAS routine to do the MatMult. */
  mkl_cspblas_xcsrgemv(&transa,&m,aa,ai,aj,x,y);

  ierr = PetscLogFlops(2.0*a->nz - a->nonzerorowcnt);CHKERRQ(ierr);
  ierr = VecRestoreArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecRestoreArray(yy,&y);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#ifdef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
#undef __FUNCT__
#define __FUNCT__ "MatMultTranspose_SeqAIJMKL_SpMV2"
PetscErrorCode MatMultTranspose_SeqAIJMKL_SpMV2(Mat A,Vec xx,Vec yy)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  Mat_SeqAIJMKL     *aijmkl=(Mat_SeqAIJMKL*)A->spptr;
  const PetscScalar *x;
  PetscScalar       *y;
  PetscErrorCode    ierr;
  sparse_status_t   stat;

  PetscFunctionBegin;

  ierr = VecGetArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecGetArray(yy,&y);CHKERRQ(ierr);

  /* Call MKL SpMV2 executor routine to do the MatMultTranspose. */
  stat = mkl_sparse_x_mv(SPARSE_OPERATION_TRANSPOSE,1.0,aijmkl->csrA,aijmkl->descr,x,0.0,y);
  
  ierr = PetscLogFlops(2.0*a->nz - a->nonzerorowcnt);CHKERRQ(ierr);
  ierr = VecRestoreArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecRestoreArray(yy,&y);CHKERRQ(ierr);
  if (stat != SPARSE_STATUS_SUCCESS) {
    PetscFunctionReturn(PETSC_ERR_LIB);
  }
  PetscFunctionReturn(0);
}
#endif /* PETSC_HAVE_MKL_SPARSE_OPTIMIZE */

#undef __FUNCT__
#define __FUNCT__ "MatMultAdd_SeqAIJMKL"
PetscErrorCode MatMultAdd_SeqAIJMKL(Mat A,Vec xx,Vec yy,Vec zz)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  const PetscScalar *x;
  PetscScalar       *y,*z;
  const MatScalar   *aa;
  PetscErrorCode    ierr;
  PetscInt          m=A->rmap->n;
  const PetscInt    *aj,*ai;
  PetscInt          i;

  /* Variables not in MatMultAdd_SeqAIJ. */
  char transa = 'n';  /* Used to indicate to MKL that we are not computing the transpose product. */
  PetscScalar       alpha = 1.0;
  PetscScalar       beta = 1.0;
  char              matdescra[6];

  PetscFunctionBegin;
  matdescra[0] = 'g';  /* Indicates to MKL that we using a general CSR matrix. */
  matdescra[3] = 'c';  /* Indicates to MKL that we use C-style (0-based) indexing. */

  ierr = VecGetArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecGetArrayPair(yy,zz,&y,&z);CHKERRQ(ierr);
  aj   = a->j;  /* aj[k] gives column index for element aa[k]. */
  aa   = a->a;  /* Nonzero elements stored row-by-row. */
  ai   = a->i;  /* ai[k] is the position in aa and aj where row k starts. */

  /* Call MKL sparse BLAS routine to do the MatMult. */
  if (zz == yy) {
    /* If zz and yy are the same vector, we can use MKL's mkl_xcsrmv(), which calculates y = alpha*A*x + beta*y. */
    mkl_xcsrmv(&transa,&m,&m,&alpha,matdescra,aa,aj,ai,ai+1,x,&beta,y);
  } else {
    /* zz and yy are different vectors, so we call mkl_cspblas_xcsrgemv(), which calculates y = A*x, and then 
     * we add the contents of vector yy to the result; MKL sparse BLAS does not have a MatMultAdd equivalent. */
    mkl_cspblas_xcsrgemv(&transa,&m,aa,ai,aj,x,z);
    for (i=0; i<m; i++) {
      z[i] += y[i];
    }
  }

  ierr = PetscLogFlops(2.0*a->nz);CHKERRQ(ierr);
  ierr = VecRestoreArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecRestoreArrayPair(yy,zz,&y,&z);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#ifdef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
#undef __FUNCT__
#define __FUNCT__ "MatMultAdd_SeqAIJMKL_SpMV2"
PetscErrorCode MatMultAdd_SeqAIJMKL_SpMV2(Mat A,Vec xx,Vec yy,Vec zz)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  Mat_SeqAIJMKL     *aijmkl=(Mat_SeqAIJMKL*)A->spptr;
  const PetscScalar *x;
  PetscScalar       *y,*z;
  PetscErrorCode    ierr;
  PetscInt          m=A->rmap->n;
  PetscInt          i;

  /* Variables not in MatMultAdd_SeqAIJ. */
  sparse_status_t stat = SPARSE_STATUS_SUCCESS;

  PetscFunctionBegin;


  ierr = VecGetArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecGetArrayPair(yy,zz,&y,&z);CHKERRQ(ierr);

  /* Call MKL sparse BLAS routine to do the MatMult. */
  if (zz == yy) {
    /* If zz and yy are the same vector, we can use mkl_sparse_x_mv, which calculates y = alpha*A*x + beta*y, 
     * with alpha and beta both set to 1.0. */
    stat = mkl_sparse_x_mv(SPARSE_OPERATION_NON_TRANSPOSE,1.0,aijmkl->csrA,aijmkl->descr,x,1.0,y);
  } else {
    /* zz and yy are different vectors, so we call mkl_sparse_x_mv with alpha=1.0 and beta=0.0, and then 
     * we add the contents of vector yy to the result; MKL sparse BLAS does not have a MatMultAdd equivalent. */
    stat = mkl_sparse_x_mv(SPARSE_OPERATION_NON_TRANSPOSE,1.0,aijmkl->csrA,aijmkl->descr,x,0.0,y);
    for (i=0; i<m; i++) {
      z[i] += y[i];
    }
  }

  ierr = PetscLogFlops(2.0*a->nz);CHKERRQ(ierr);
  ierr = VecRestoreArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecRestoreArrayPair(yy,zz,&y,&z);CHKERRQ(ierr);
  if (stat != SPARSE_STATUS_SUCCESS) {
    PetscFunctionReturn(PETSC_ERR_LIB);
  }
  PetscFunctionReturn(0);
}
#endif /* PETSC_HAVE_MKL_SPARSE_OPTIMIZE */

#undef __FUNCT__
#define __FUNCT__ "MatMultTransposeAdd_SeqAIJMKL"
PetscErrorCode MatMultTransposeAdd_SeqAIJMKL(Mat A,Vec xx,Vec yy,Vec zz)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  const PetscScalar *x;
  PetscScalar       *y,*z;
  const MatScalar   *aa;
  PetscErrorCode    ierr;
  PetscInt          m=A->rmap->n;
  const PetscInt    *aj,*ai;
  PetscInt          i;

  /* Variables not in MatMultTransposeAdd_SeqAIJ. */
  char transa = 't';  /* Used to indicate to MKL that we are computing the transpose product. */
  PetscScalar       alpha = 1.0;
  PetscScalar       beta = 1.0;
  char              matdescra[6];

  PetscFunctionBegin;
  matdescra[0] = 'g';  /* Indicates to MKL that we using a general CSR matrix. */
  matdescra[3] = 'c';  /* Indicates to MKL that we use C-style (0-based) indexing. */

  ierr = VecGetArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecGetArrayPair(yy,zz,&y,&z);CHKERRQ(ierr);
  aj   = a->j;  /* aj[k] gives column index for element aa[k]. */
  aa   = a->a;  /* Nonzero elements stored row-by-row. */
  ai   = a->i;  /* ai[k] is the position in aa and aj where row k starts. */

  /* Call MKL sparse BLAS routine to do the MatMult. */
  if (zz == yy) {
    /* If zz and yy are the same vector, we can use MKL's mkl_xcsrmv(), which calculates y = alpha*A*x + beta*y. */
    mkl_xcsrmv(&transa,&m,&m,&alpha,matdescra,aa,aj,ai,ai+1,x,&beta,y);
  } else {
    /* zz and yy are different vectors, so we call mkl_cspblas_xcsrgemv(), which calculates y = A*x, and then 
     * we add the contents of vector yy to the result; MKL sparse BLAS does not have a MatMultAdd equivalent. */
    mkl_cspblas_xcsrgemv(&transa,&m,aa,ai,aj,x,z);
    for (i=0; i<m; i++) {
      z[i] += y[i];
    }
  }

  ierr = PetscLogFlops(2.0*a->nz);CHKERRQ(ierr);
  ierr = VecRestoreArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecRestoreArrayPair(yy,zz,&y,&z);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#ifdef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
#undef __FUNCT__
#define __FUNCT__ "MatMultTransposeAdd_SeqAIJMKL_SpMV2"
PetscErrorCode MatMultTransposeAdd_SeqAIJMKL_SpMV2(Mat A,Vec xx,Vec yy,Vec zz)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  Mat_SeqAIJMKL     *aijmkl=(Mat_SeqAIJMKL*)A->spptr;
  const PetscScalar *x;
  PetscScalar       *y,*z;
  PetscErrorCode    ierr;
  PetscInt          m=A->rmap->n;
  PetscInt          i;

  /* Variables not in MatMultTransposeAdd_SeqAIJ. */
  sparse_status_t stat = SPARSE_STATUS_SUCCESS;

  PetscFunctionBegin;

  ierr = VecGetArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecGetArrayPair(yy,zz,&y,&z);CHKERRQ(ierr);

  /* Call MKL sparse BLAS routine to do the MatMult. */
  if (zz == yy) {
    /* If zz and yy are the same vector, we can use mkl_sparse_x_mv, which calculates y = alpha*A*x + beta*y, 
     * with alpha and beta both set to 1.0. */
    stat = mkl_sparse_x_mv(SPARSE_OPERATION_TRANSPOSE,1.0,aijmkl->csrA,aijmkl->descr,x,1.0,y);
  } else {
    /* zz and yy are different vectors, so we call mkl_sparse_x_mv with alpha=1.0 and beta=0.0, and then 
     * we add the contents of vector yy to the result; MKL sparse BLAS does not have a MatMultAdd equivalent. */
    stat = mkl_sparse_x_mv(SPARSE_OPERATION_TRANSPOSE,1.0,aijmkl->csrA,aijmkl->descr,x,0.0,y);
    for (i=0; i<m; i++) {
      z[i] += y[i];
    }
  }

  ierr = PetscLogFlops(2.0*a->nz);CHKERRQ(ierr);
  ierr = VecRestoreArrayRead(xx,&x);CHKERRQ(ierr);
  ierr = VecRestoreArrayPair(yy,zz,&y,&z);CHKERRQ(ierr);
  if (stat != SPARSE_STATUS_SUCCESS) {
    PetscFunctionReturn(PETSC_ERR_LIB);
  }
  PetscFunctionReturn(0);
}
#endif /* PETSC_HAVE_MKL_SPARSE_OPTIMIZE */


/* MatConvert_SeqAIJ_SeqAIJMKL converts a SeqAIJ matrix into a
 * SeqAIJMKL matrix.  This routine is called by the MatCreate_SeqMKLAIJ()
 * routine, but can also be used to convert an assembled SeqAIJ matrix
 * into a SeqAIJMKL one. */
#undef __FUNCT__
#define __FUNCT__ "MatConvert_SeqAIJ_SeqAIJMKL"
PETSC_INTERN PetscErrorCode MatConvert_SeqAIJ_SeqAIJMKL(Mat A,MatType type,MatReuse reuse,Mat *newmat)
{
  PetscErrorCode ierr;
  Mat            B = *newmat;
  Mat_SeqAIJMKL  *aijmkl;
  PetscBool      set;
  PetscBool      sametype;

  PetscFunctionBegin;
  if (reuse == MAT_INITIAL_MATRIX) {
    ierr = MatDuplicate(A,MAT_COPY_VALUES,&B);CHKERRQ(ierr);
  }

  ierr = PetscObjectTypeCompare((PetscObject)A,type,&sametype);CHKERRQ(ierr);
  if (sametype) PetscFunctionReturn(0);

  ierr     = PetscNewLog(B,&aijmkl);CHKERRQ(ierr);
  B->spptr = (void*) aijmkl;

  /* Set function pointers for methods that we inherit from AIJ but override. 
   * We also parse some command line options below, since those determine some of the methods we point to.
   * Note: Currently the transposed operations are not being set because I encounter memory corruption 
   * when these are enabled.  Need to look at this with Valgrind or similar. --RTM */
  B->ops->duplicate        = MatDuplicate_SeqAIJMKL;
  B->ops->assemblyend      = MatAssemblyEnd_SeqAIJMKL;
  B->ops->destroy          = MatDestroy_SeqAIJMKL;

  aijmkl->sparse_optimized = PETSC_FALSE;
#ifdef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
  aijmkl->no_SpMV2 = PETSC_FALSE;  /* Default to using the SpMV2 routines if our MKL supports them. */
#elif
  aijmkl->no_SpMV2 = PETSC_TRUE;
#endif

  /* Parse command line options. */
  ierr = PetscOptionsBegin(PetscObjectComm((PetscObject)A),((PetscObject)A)->prefix,"AIJMKL Options","Mat");CHKERRQ(ierr);
  ierr = PetscOptionsBool("-mat_aijmkl_no_spmv2","NoSPMV2","None",(PetscBool)aijmkl->no_SpMV2,(PetscBool*)&aijmkl->no_SpMV2,&set);CHKERRQ(ierr);
  ierr = PetscOptionsEnd();CHKERRQ(ierr);
#ifndef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
  if(!aijmkl->no_SpMV2) {
    ierr = PetscInfo(B,"User requested use of MKL SpMV2 routines, but MKL version does not support mkl_sparse_optimize();  defaulting to non-SpMV2 routines.\n");
    aijmkl->no_SpMV2 = PETSC_TRUE;
  }
#endif

  if(!aijmkl->no_SpMV2) {
#ifdef PETSC_HAVE_MKL_SPARSE_OPTIMIZE
    B->ops->mult             = MatMult_SeqAIJMKL_SpMV2;
    /* B->ops->multtranspose    = MatMultTranspose_SeqAIJMKL_SpMV2; */
    B->ops->multadd          = MatMultAdd_SeqAIJMKL_SpMV2;
    /* B->ops->multtransposeadd = MatMultTransposeAdd_SeqAIJMKL_SpMV2; */
#endif
  } else {
    B->ops->mult             = MatMult_SeqAIJMKL;
    /* B->ops->multtranspose    = MatMultTranspose_SeqAIJMKL; */
    B->ops->multadd          = MatMultAdd_SeqAIJMKL;
    /* B->ops->multtransposeadd = MatMultTransposeAdd_SeqAIJMKL; */
  }

  ierr = PetscObjectComposeFunction((PetscObject)B,"MatConvert_seqaijmkl_seqaij_C",MatConvert_SeqAIJMKL_SeqAIJ);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)B,"MatMatMult_seqdense_seqaijmkl_C",MatMatMult_SeqDense_SeqAIJ);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)B,"MatMatMultSymbolic_seqdense_seqaijmkl_C",MatMatMultSymbolic_SeqDense_SeqAIJ);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunction((PetscObject)B,"MatMatMultNumeric_seqdense_seqaijmkl_C",MatMatMultNumeric_SeqDense_SeqAIJ);CHKERRQ(ierr);

  ierr    = PetscObjectChangeTypeName((PetscObject)B,MATSEQAIJMKL);CHKERRQ(ierr);
  *newmat = B;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatCreateSeqAIJMKL"
/*@C
   MatCreateSeqAIJMKL - Creates a sparse matrix of type SEQAIJMKL.
   This type inherits from AIJ and is largely identical, but uses sparse BLAS 
   routines from Intel MKL whenever possible.
   Collective on MPI_Comm

   Input Parameters:
+  comm - MPI communicator, set to PETSC_COMM_SELF
.  m - number of rows
.  n - number of columns
.  nz - number of nonzeros per row (same for all rows)
-  nnz - array containing the number of nonzeros in the various rows
         (possibly different for each row) or NULL

   Output Parameter:
.  A - the matrix

   Notes:
   If nnz is given then nz is ignored

   Level: intermediate

.keywords: matrix, cray, sparse, parallel

.seealso: MatCreate(), MatCreateMPIAIJMKL(), MatSetValues()
@*/
PetscErrorCode  MatCreateSeqAIJMKL(MPI_Comm comm,PetscInt m,PetscInt n,PetscInt nz,const PetscInt nnz[],Mat *A)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = MatCreate(comm,A);CHKERRQ(ierr);
  ierr = MatSetSizes(*A,m,n,m,n);CHKERRQ(ierr);
  ierr = MatSetType(*A,MATSEQAIJMKL);CHKERRQ(ierr);
  ierr = MatSeqAIJSetPreallocation_SeqAIJ(*A,nz,nnz);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatCreate_SeqAIJMKL"
PETSC_EXTERN PetscErrorCode MatCreate_SeqAIJMKL(Mat A)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = MatSetType(A,MATSEQAIJ);CHKERRQ(ierr);
  ierr = MatConvert_SeqAIJ_SeqAIJMKL(A,MATSEQAIJMKL,MAT_INPLACE_MATRIX,&A);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
