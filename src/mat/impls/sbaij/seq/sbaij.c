/*$Id: sbaij.c,v 1.12 2000/07/28 21:14:06 hzhang Exp balay $*/

/*
    Defines the basic matrix operations for the BAIJ (compressed row)
  matrix storage format.
*/
#include "petscsys.h"                            /*I "petscmat.h" I*/
#include "src/mat/impls/baij/seq/baij.h"
#include "src/vec/vecimpl.h"
#include "src/inline/spops.h"
#include "src/mat/impls/sbaij/seq/sbaij.h"

#define CHUNKSIZE  10

/*
     Checks for missing diagonals
*/
#undef __FUNC__  
#define __FUNC__ "MatMissingDiagonal_SeqSBAIJ"
int MatMissingDiagonal_SeqSBAIJ(Mat A)
{
  Mat_SeqBAIJ *a = (Mat_SeqBAIJ*)A->data; 
  int         *diag,*jj = a->j,i,ierr;

  PetscFunctionBegin;
  ierr = MatMarkDiagonal_SeqBAIJ(A);CHKERRQ(ierr);
  diag = a->diag;
  for (i=0; i<a->mbs; i++) {
    if (jj[diag[i]] != i) {
      SETERRQ1(1,1,"Matrix is missing diagonal number %d",i);
    }
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatMarkDiagonal_SeqSBAIJ"
int MatMarkDiagonal_SeqSBAIJ(Mat A)
{
  Mat_SeqBAIJ *a = (Mat_SeqBAIJ*)A->data; 
  int         i,j,*diag,m = a->mbs;

  PetscFunctionBegin;
  if (a->diag) PetscFunctionReturn(0);

  diag = (int*)PetscMalloc((m+1)*sizeof(int));CHKPTRQ(diag);
  PLogObjectMemory(A,(m+1)*sizeof(int));
  for (i=0; i<m; i++) {
    diag[i] = a->i[i+1];
    for (j=a->i[i]; j<a->i[i+1]; j++) {
      if (a->j[j] == i) {
        diag[i] = j;
        break;
      }
    }
  }
  a->diag = diag;
  PetscFunctionReturn(0);
}


extern int MatToSymmetricIJ_SeqAIJ(int,int*,int*,int,int,int**,int**);

#undef __FUNC__  
#define __FUNC__ "MatGetRowIJ_SeqSBAIJ"
static int MatGetRowIJ_SeqSBAIJ(Mat A,int oshift,PetscTruth symmetric,int *nn,int **ia,int **ja,PetscTruth *done)
{
  Mat_SeqBAIJ *a = (Mat_SeqBAIJ*)A->data;
  int         ierr,n = a->mbs,i;

  PetscFunctionBegin;
  *nn = n;
  if (!ia) PetscFunctionReturn(0);
  if (symmetric) {
    ierr = MatToSymmetricIJ_SeqAIJ(n,a->i,a->j,0,oshift,ia,ja);CHKERRQ(ierr);
  } else if (oshift == 1) {
    /* temporarily add 1 to i and j indices */
    int nz = a->i[n] + 1; 
    for (i=0; i<nz; i++) a->j[i]++;
    for (i=0; i<n+1; i++) a->i[i]++;
    *ia = a->i; *ja = a->j;
  } else {
    *ia = a->i; *ja = a->j;
  }

  PetscFunctionReturn(0); 
}

#undef __FUNC__  
#define __FUNC__ "MatRestoreRowIJ_SeqSBAIJ" 
static int MatRestoreRowIJ_SeqSBAIJ(Mat A,int oshift,PetscTruth symmetric,int *nn,int **ia,int **ja,
                                PetscTruth *done)
{
  Mat_SeqBAIJ *a = (Mat_SeqBAIJ*)A->data;
  int         i,n = a->mbs,ierr;

  PetscFunctionBegin;
  if (!ia) PetscFunctionReturn(0);
  if (symmetric) {
    ierr = PetscFree(*ia);CHKERRQ(ierr);
    ierr = PetscFree(*ja);CHKERRQ(ierr);
  } else if (oshift == 1) {
    int nz = a->i[n]; 
    for (i=0; i<nz; i++) a->j[i]--;
    for (i=0; i<n+1; i++) a->i[i]--;
  }
  PetscFunctionReturn(0); 
}

#undef __FUNC__  
#define __FUNC__ "MatGetBlockSize_SeqSBAIJ"
int MatGetBlockSize_SeqSBAIJ(Mat mat,int *bs)
{
  Mat_SeqSBAIJ *sbaij = (Mat_SeqSBAIJ*)mat->data;

  PetscFunctionBegin;
  *bs = sbaij->bs;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatDestroy_SeqSBAIJ"
int MatDestroy_SeqSBAIJ(Mat A)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  int         ierr;

  PetscFunctionBegin;
  if (A->mapping) {
    ierr = ISLocalToGlobalMappingDestroy(A->mapping);CHKERRQ(ierr);
  }
  if (A->bmapping) {
    ierr = ISLocalToGlobalMappingDestroy(A->bmapping);CHKERRQ(ierr);
  }
  if (A->rmap) {
    ierr = MapDestroy(A->rmap);CHKERRQ(ierr);
  }
  if (A->cmap) {
    ierr = MapDestroy(A->cmap);CHKERRQ(ierr);
  }
#if defined(PETSC_USE_LOG)
  PLogObjectState((PetscObject)A,"Rows=%d, s_NZ=%d",a->m,a->s_nz);
#endif
  ierr = PetscFree(a->a);CHKERRQ(ierr);
  if (!a->singlemalloc) {
    ierr = PetscFree(a->i);CHKERRQ(ierr);
    ierr = PetscFree(a->j);CHKERRQ(ierr); 
  }
  if (a->row) {
    ierr = ISDestroy(a->row);CHKERRQ(ierr);
  }
  if (a->col) {
    ierr = ISDestroy(a->col);CHKERRQ(ierr);
  }
  if (a->diag) {ierr = PetscFree(a->diag);CHKERRQ(ierr);}
  if (a->ilen) {ierr = PetscFree(a->ilen);CHKERRQ(ierr);}
  if (a->imax) {ierr = PetscFree(a->imax);CHKERRQ(ierr);}
  if (a->solve_work) {ierr = PetscFree(a->solve_work);CHKERRQ(ierr);}
  if (a->mult_work) {ierr = PetscFree(a->mult_work);CHKERRQ(ierr);}
  if (a->icol) {ierr = ISDestroy(a->icol);CHKERRQ(ierr);}
  if (a->saved_values) {ierr = PetscFree(a->saved_values);CHKERRQ(ierr);}
  ierr = PetscFree(a);CHKERRQ(ierr);
  PLogObjectDestroy(A);
  PetscHeaderDestroy(A);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatSetOption_SeqSBAIJ"
int MatSetOption_SeqSBAIJ(Mat A,MatOption op)
{
  Mat_SeqBAIJ *a = (Mat_SeqBAIJ*)A->data;

  PetscFunctionBegin;
  if      (op == MAT_ROW_ORIENTED)                 a->roworiented    = PETSC_TRUE;
  else if (op == MAT_COLUMN_ORIENTED)              a->roworiented    = PETSC_FALSE;
  else if (op == MAT_COLUMNS_SORTED)               a->sorted         = PETSC_TRUE;
  else if (op == MAT_COLUMNS_UNSORTED)             a->sorted         = PETSC_FALSE;
  else if (op == MAT_KEEP_ZEROED_ROWS)             a->keepzeroedrows = PETSC_TRUE;
  else if (op == MAT_NO_NEW_NONZERO_LOCATIONS)     a->nonew          = 1;
  else if (op == MAT_NEW_NONZERO_LOCATION_ERR)     a->nonew          = -1;
  else if (op == MAT_NEW_NONZERO_ALLOCATION_ERR)   a->nonew          = -2;
  else if (op == MAT_YES_NEW_NONZERO_LOCATIONS)    a->nonew          = 0;
  else if (op == MAT_ROWS_SORTED || 
           op == MAT_ROWS_UNSORTED ||
           op == MAT_SYMMETRIC ||
           op == MAT_STRUCTURALLY_SYMMETRIC ||
           op == MAT_YES_NEW_DIAGONALS ||
           op == MAT_IGNORE_OFF_PROC_ENTRIES ||
           op == MAT_USE_HASH_TABLE) {
    PLogInfo(A,"MatSetOption_SeqBAIJ:Option ignored\n");
  } else if (op == MAT_NO_NEW_DIAGONALS) {
    SETERRQ(PETSC_ERR_SUP,0,"MAT_NO_NEW_DIAGONALS");
  } else {
    SETERRQ(PETSC_ERR_SUP,0,"unknown option");
  }
  PetscFunctionReturn(0);
}


#undef __FUNC__  
#define __FUNC__ "MatGetSize_SeqSBAIJ"
int MatGetSize_SeqSBAIJ(Mat A,int *m,int *n)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;

  PetscFunctionBegin;
  if (m) *m = a->m; 
  if (n) *n = a->m;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatGetOwnershipRange_SeqSBAIJ"
int MatGetOwnershipRange_SeqSBAIJ(Mat A,int *m,int *n)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;

  PetscFunctionBegin;
  *m = 0; *n = a->m;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatGetRow_SeqSBAIJ"
int MatGetRow_SeqSBAIJ(Mat A,int row,int *ncols,int **cols,Scalar **v)
{
  Mat_SeqSBAIJ  *a = (Mat_SeqSBAIJ*)A->data;
  int          itmp,i,j,k,M,*ai,*aj,bs,bn,bp,*cols_i,bs2;
  MatScalar    *aa,*aa_i;
  Scalar       *v_i;

  PetscFunctionBegin;
  bs  = a->bs;
  ai  = a->i;
  aj  = a->j;
  aa  = a->a;
  bs2 = a->bs2;
  
  if (row < 0 || row >= a->m) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Row out of range");
  
  bn  = row/bs;   /* Block number */
  bp  = row % bs; /* Block position */
  M   = ai[bn+1] - ai[bn];
  *ncols = bs*M;
  
  if (v) {
    *v = 0;
    if (*ncols) {
      *v = (Scalar*)PetscMalloc((*ncols+row)*sizeof(Scalar));CHKPTRQ(*v);
      for (i=0; i<M; i++) { /* for each block in the block row */
        v_i  = *v + i*bs;
        aa_i = aa + bs2*(ai[bn] + i);
        for (j=bp,k=0; j<bs2; j+=bs,k++) {v_i[k] = aa_i[j];}
      }
    }
  }

  if (cols) {
    *cols = 0;
    if (*ncols) {
      *cols = (int*)PetscMalloc((*ncols+row)*sizeof(int));CHKPTRQ(*cols);
      for (i=0; i<M; i++) { /* for each block in the block row */
        cols_i = *cols + i*bs;
        itmp  = bs*aj[ai[bn] + i];
        for (j=0; j<bs; j++) {cols_i[j] = itmp++;}
      }
    }
  }

  /*search column A(0:row-1,row) (=A(row,0:row-1)). Could be expensive! */
  /* this segment is currently removed, so only entries in the upper triangle are obtained */
#ifdef column_search
  v_i    = *v    + M*bs;
  cols_i = *cols + M*bs;
  for (i=0; i<bn; i++){ /* for each block row */
    M = ai[i+1] - ai[i];
    for (j=0; j<M; j++){
      itmp = aj[ai[i] + j];    /* block column value */
      if (itmp == bn){ 
        aa_i   = aa    + bs2*(ai[i] + j) + bs*bp;
        for (k=0; k<bs; k++) {
          *cols_i++ = i*bs+k; 
          *v_i++    = aa_i[k]; 
        }
        *ncols += bs;
        break;
      }
    }
  }
#endif

  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatRestoreRow_SeqSBAIJ"
int MatRestoreRow_SeqSBAIJ(Mat A,int row,int *nz,int **idx,Scalar **v)
{
  int ierr;

  PetscFunctionBegin;
  if (idx) {if (*idx) {ierr = PetscFree(*idx);CHKERRQ(ierr);}}
  if (v)   {if (*v)   {ierr = PetscFree(*v);CHKERRQ(ierr);}}
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatTranspose_SeqSBAIJ"
int MatTranspose_SeqSBAIJ(Mat A,Mat *B)
{ 
  Mat_SeqBAIJ *a=(Mat_SeqBAIJ *)A->data;
  Mat         C;
  int         i,j,k,ierr,*aj=a->j,*ai=a->i,bs=a->bs,mbs=a->mbs,nbs=a->nbs,len,*col;
  int         *rows,*cols,bs2=a->bs2,refct;
  MatScalar   *array = a->a;

  PetscFunctionBegin;
  if (!B && mbs!=nbs) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Square matrix only for in-place");
  col  = (int*)PetscMalloc((1+nbs)*sizeof(int));CHKPTRQ(col);
  ierr = PetscMemzero(col,(1+nbs)*sizeof(int));CHKERRQ(ierr);

  for (i=0; i<ai[mbs]; i++) col[aj[i]] += 1;
  ierr = MatCreateSeqBAIJ(A->comm,bs,a->n,a->m,PETSC_NULL,col,&C);CHKERRQ(ierr);
  ierr = PetscFree(col);CHKERRQ(ierr);
  rows = (int*)PetscMalloc(2*bs*sizeof(int));CHKPTRQ(rows);
  cols = rows + bs;
  for (i=0; i<mbs; i++) {
    cols[0] = i*bs;
    for (k=1; k<bs; k++) cols[k] = cols[k-1] + 1;
    len = ai[i+1] - ai[i];
    for (j=0; j<len; j++) {
      rows[0] = (*aj++)*bs;
      for (k=1; k<bs; k++) rows[k] = rows[k-1] + 1;
      ierr = MatSetValues(C,bs,rows,bs,cols,array,INSERT_VALUES);CHKERRQ(ierr);
      array += bs2;
    }
  }
  ierr = PetscFree(rows);CHKERRQ(ierr);
  
  ierr = MatAssemblyBegin(C,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(C,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  
  if (B) {
    *B = C;
  } else {
    PetscOps *Abops;
    MatOps   Aops;

    /* This isn't really an in-place transpose */
    ierr = PetscFree(a->a);CHKERRQ(ierr);
    if (!a->singlemalloc) {
      ierr = PetscFree(a->i);CHKERRQ(ierr);
      ierr = PetscFree(a->j);CHKERRQ(ierr);
    }
    if (a->diag) {ierr = PetscFree(a->diag);CHKERRQ(ierr);}
    if (a->ilen) {ierr = PetscFree(a->ilen);CHKERRQ(ierr);}
    if (a->imax) {ierr = PetscFree(a->imax);CHKERRQ(ierr);}
    if (a->solve_work) {ierr = PetscFree(a->solve_work);CHKERRQ(ierr);}
    ierr = PetscFree(a);CHKERRQ(ierr);


    ierr = MapDestroy(A->rmap);CHKERRQ(ierr);
    ierr = MapDestroy(A->cmap);CHKERRQ(ierr);

    /*
       This is horrible, horrible code. We need to keep the 
      A pointers for the bops and ops but copy everything 
      else from C.
    */
    Abops    = A->bops;
    Aops     = A->ops;
    refct    = A->refct;
    ierr     = PetscMemcpy(A,C,sizeof(struct _p_Mat));CHKERRQ(ierr);
    A->bops  = Abops;
    A->ops   = Aops;
    A->qlist = 0;
    A->refct = refct;

    PetscHeaderDestroy(C);
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatView_SeqSBAIJ_Binary"
static int MatView_SeqSBAIJ_Binary(Mat A,Viewer viewer)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  int         i,fd,*col_lens,ierr,bs = a->bs,count,*jj,j,k,l,bs2=a->bs2;
  Scalar      *aa;
  FILE        *file;

  PetscFunctionBegin;
  ierr = ViewerBinaryGetDescriptor(viewer,&fd);CHKERRQ(ierr);
  col_lens = (int*)PetscMalloc((4+a->m)*sizeof(int));CHKPTRQ(col_lens);
  col_lens[0] = MAT_COOKIE;

  col_lens[1] = a->m;
  col_lens[2] = a->m;
  col_lens[3] = a->s_nz*bs2;

  /* store lengths of each row and write (including header) to file */
  count = 0;
  for (i=0; i<a->mbs; i++) {
    for (j=0; j<bs; j++) {
      col_lens[4+count++] = bs*(a->i[i+1] - a->i[i]);
    }
  }
  ierr = PetscBinaryWrite(fd,col_lens,4+a->m,PETSC_INT,1);CHKERRQ(ierr);
  ierr = PetscFree(col_lens);CHKERRQ(ierr);

  /* store column indices (zero start index) */
  jj = (int*)PetscMalloc((a->s_nz+1)*bs2*sizeof(int));CHKPTRQ(jj);
  count = 0;
  for (i=0; i<a->mbs; i++) {
    for (j=0; j<bs; j++) {
      for (k=a->i[i]; k<a->i[i+1]; k++) {
        for (l=0; l<bs; l++) {
          jj[count++] = bs*a->j[k] + l;
        }
      }
    }
  }
  ierr = PetscBinaryWrite(fd,jj,bs2*a->s_nz,PETSC_INT,0);CHKERRQ(ierr);
  ierr = PetscFree(jj);CHKERRQ(ierr);

  /* store nonzero values */
  aa = (Scalar*)PetscMalloc((a->s_nz+1)*bs2*sizeof(Scalar));CHKPTRQ(aa);
  count = 0;
  for (i=0; i<a->mbs; i++) {
    for (j=0; j<bs; j++) {
      for (k=a->i[i]; k<a->i[i+1]; k++) {
        for (l=0; l<bs; l++) {
          aa[count++] = a->a[bs2*k + l*bs + j];
        }
      }
    }
  }
  ierr = PetscBinaryWrite(fd,aa,bs2*a->s_nz,PETSC_SCALAR,0);CHKERRQ(ierr);
  ierr = PetscFree(aa);CHKERRQ(ierr);

  ierr = ViewerBinaryGetInfoPointer(viewer,&file);CHKERRQ(ierr);
  if (file) {
    fprintf(file,"-matload_block_size %d\n",a->bs);
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatView_SeqSBAIJ_ASCII"
static int MatView_SeqSBAIJ_ASCII(Mat A,Viewer viewer)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  int         ierr,i,j,format,bs = a->bs,k,l,bs2=a->bs2;
  char        *outputname;

  PetscFunctionBegin;
  ierr = ViewerGetOutputname(viewer,&outputname);CHKERRQ(ierr);
  ierr = ViewerGetFormat(viewer,&format);CHKERRQ(ierr);
  if (format == VIEWER_FORMAT_ASCII_INFO || format == VIEWER_FORMAT_ASCII_INFO_LONG) {
    ierr = ViewerASCIIPrintf(viewer,"  block size is %d\n",bs);CHKERRQ(ierr);
  } else if (format == VIEWER_FORMAT_ASCII_MATLAB) {
    SETERRQ(PETSC_ERR_SUP,0,"Matlab format not supported");
  } else if (format == VIEWER_FORMAT_ASCII_COMMON) {
    ierr = ViewerASCIIUseTabs(viewer,PETSC_NO);CHKERRQ(ierr);
    for (i=0; i<a->mbs; i++) {
      for (j=0; j<bs; j++) {
        ierr = ViewerASCIIPrintf(viewer,"row %d:",i*bs+j);CHKERRQ(ierr);
        for (k=a->i[i]; k<a->i[i+1]; k++) {
          for (l=0; l<bs; l++) {
#if defined(PETSC_USE_COMPLEX)
            if (PetscImaginaryPart(a->a[bs2*k + l*bs + j]) > 0.0 && PetscRealPart(a->a[bs2*k + l*bs + j]) != 0.0) {
              ierr = ViewerASCIIPrintf(viewer," %d %g + %g i",bs*a->j[k]+l,
                      PetscRealPart(a->a[bs2*k + l*bs + j]),PetscImaginaryPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            } else if (PetscImaginaryPart(a->a[bs2*k + l*bs + j]) < 0.0 && PetscRealPart(a->a[bs2*k + l*bs + j]) != 0.0) {
              ierr = ViewerASCIIPrintf(viewer," %d %g - %g i",bs*a->j[k]+l,
                      PetscRealPart(a->a[bs2*k + l*bs + j]),-PetscImaginaryPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            } else if (PetscRealPart(a->a[bs2*k + l*bs + j]) != 0.0) {
              ierr = ViewerASCIIPrintf(viewer," %d %g ",bs*a->j[k]+l,PetscRealPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            }
#else
            if (a->a[bs2*k + l*bs + j] != 0.0) {
              ierr = ViewerASCIIPrintf(viewer," %d %g ",bs*a->j[k]+l,a->a[bs2*k + l*bs + j]);CHKERRQ(ierr);
            }
#endif
          }
        }
        ierr = ViewerASCIIPrintf(viewer,"\n");CHKERRQ(ierr);
      }
    } 
    ierr = ViewerASCIIUseTabs(viewer,PETSC_YES);CHKERRQ(ierr);
  } else {
    ierr = ViewerASCIIUseTabs(viewer,PETSC_NO);CHKERRQ(ierr);
    for (i=0; i<a->mbs; i++) {
      for (j=0; j<bs; j++) {
        ierr = ViewerASCIIPrintf(viewer,"row %d:",i*bs+j);CHKERRQ(ierr);
        for (k=a->i[i]; k<a->i[i+1]; k++) {
          for (l=0; l<bs; l++) {
#if defined(PETSC_USE_COMPLEX)
            if (PetscImaginaryPart(a->a[bs2*k + l*bs + j]) > 0.0) {
              ierr = ViewerASCIIPrintf(viewer," %d %g + %g i",bs*a->j[k]+l,
                PetscRealPart(a->a[bs2*k + l*bs + j]),PetscImaginaryPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            } else if (PetscImaginaryPart(a->a[bs2*k + l*bs + j]) < 0.0) {
              ierr = ViewerASCIIPrintf(viewer," %d %g - %g i",bs*a->j[k]+l,
                PetscRealPart(a->a[bs2*k + l*bs + j]),-PetscImaginaryPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            } else {
              ierr = ViewerASCIIPrintf(viewer," %d %g ",bs*a->j[k]+l,PetscRealPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            }
#else
            ierr = ViewerASCIIPrintf(viewer," %d %g ",bs*a->j[k]+l,a->a[bs2*k + l*bs + j]);CHKERRQ(ierr);
#endif
          }
        }
        ierr = ViewerASCIIPrintf(viewer,"\n");CHKERRQ(ierr);
      }
    } 
    ierr = ViewerASCIIUseTabs(viewer,PETSC_YES);CHKERRQ(ierr);
  }
  ierr = ViewerFlush(viewer);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatView_SeqSBAIJ_Draw_Zoom"
static int MatView_SeqSBAIJ_Draw_Zoom(Draw draw,void *Aa)
{
  Mat          A = (Mat) Aa;
  Mat_SeqSBAIJ  *a=(Mat_SeqSBAIJ*)A->data;
  int          row,ierr,i,j,k,l,mbs=a->mbs,color,bs=a->bs,bs2=a->bs2,rank;
  PetscReal    xl,yl,xr,yr,x_l,x_r,y_l,y_r;
  MatScalar    *aa;
  MPI_Comm     comm;
  Viewer       viewer;

  PetscFunctionBegin; 
  /*
      This is nasty. If this is called from an originally parallel matrix
   then all processes call this,but only the first has the matrix so the
   rest should return immediately.
  */
  ierr = PetscObjectGetComm((PetscObject)draw,&comm);CHKERRQ(ierr);
  ierr = MPI_Comm_rank(comm,&rank);CHKERRQ(ierr);
  if (rank) PetscFunctionReturn(0);

  ierr = PetscObjectQuery((PetscObject)A,"Zoomviewer",(PetscObject*)&viewer);CHKERRQ(ierr); 

  ierr = DrawGetCoordinates(draw,&xl,&yl,&xr,&yr);CHKERRQ(ierr);
  DrawString(draw, .3*(xl+xr), .3*(yl+yr), DRAW_BLACK, "symmetric");

  /* loop over matrix elements drawing boxes */
  color = DRAW_BLUE;
  for (i=0,row=0; i<mbs; i++,row+=bs) {
    for (j=a->i[i]; j<a->i[i+1]; j++) {
      y_l = a->m - row - 1.0; y_r = y_l + 1.0;
      x_l = a->j[j]*bs; x_r = x_l + 1.0;
      aa = a->a + j*bs2;
      for (k=0; k<bs; k++) {
        for (l=0; l<bs; l++) {
          if (PetscRealPart(*aa++) >=  0.) continue;
          ierr = DrawRectangle(draw,x_l+k,y_l-l,x_r+k,y_r-l,color,color,color,color);CHKERRQ(ierr);
        }
      }
    } 
  }
  color = DRAW_CYAN;
  for (i=0,row=0; i<mbs; i++,row+=bs) {
    for (j=a->i[i]; j<a->i[i+1]; j++) {
      y_l = a->m - row - 1.0; y_r = y_l + 1.0;
      x_l = a->j[j]*bs; x_r = x_l + 1.0;
      aa = a->a + j*bs2;
      for (k=0; k<bs; k++) {
        for (l=0; l<bs; l++) {
          if (PetscRealPart(*aa++) != 0.) continue;
          ierr = DrawRectangle(draw,x_l+k,y_l-l,x_r+k,y_r-l,color,color,color,color);CHKERRQ(ierr);
        }
      }
    } 
  }

  color = DRAW_RED;
  for (i=0,row=0; i<mbs; i++,row+=bs) {
    for (j=a->i[i]; j<a->i[i+1]; j++) {
      y_l = a->m - row - 1.0; y_r = y_l + 1.0;
      x_l = a->j[j]*bs; x_r = x_l + 1.0;
      aa = a->a + j*bs2;
      for (k=0; k<bs; k++) {
        for (l=0; l<bs; l++) {
          if (PetscRealPart(*aa++) <= 0.) continue;
          ierr = DrawRectangle(draw,x_l+k,y_l-l,x_r+k,y_r-l,color,color,color,color);CHKERRQ(ierr);
        }
      }
    } 
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatView_SeqSBAIJ_Draw"
static int MatView_SeqSBAIJ_Draw(Mat A,Viewer viewer)
{
  Mat_SeqSBAIJ  *a=(Mat_SeqSBAIJ*)A->data;
  int          ierr;
  PetscReal    xl,yl,xr,yr,w,h;
  Draw         draw;
  PetscTruth   isnull;

  PetscFunctionBegin; 

  ierr = ViewerDrawGetDraw(viewer,0,&draw);CHKERRQ(ierr);
  ierr = DrawIsNull(draw,&isnull);CHKERRQ(ierr); if (isnull) PetscFunctionReturn(0);

  ierr = PetscObjectCompose((PetscObject)A,"Zoomviewer",(PetscObject)viewer);CHKERRQ(ierr);
  xr  = a->m; yr = a->m; h = yr/10.0; w = xr/10.0; 
  xr += w;    yr += h;  xl = -w;     yl = -h;
  ierr = DrawSetCoordinates(draw,xl,yl,xr,yr);CHKERRQ(ierr);
  ierr = DrawZoom(draw,MatView_SeqSBAIJ_Draw_Zoom,A);CHKERRQ(ierr);
  ierr = PetscObjectCompose((PetscObject)A,"Zoomviewer",PETSC_NULL);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatView_SeqSBAIJ"
int MatView_SeqSBAIJ(Mat A,Viewer viewer)
{
  int        ierr;
  PetscTruth issocket,isascii,isbinary,isdraw;

  PetscFunctionBegin;
  ierr = PetscTypeCompare((PetscObject)viewer,SOCKET_VIEWER,&issocket);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject)viewer,ASCII_VIEWER,&isascii);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject)viewer,BINARY_VIEWER,&isbinary);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject)viewer,DRAW_VIEWER,&isdraw);CHKERRQ(ierr);
  if (issocket) {
    SETERRQ(PETSC_ERR_SUP,0,"Socket viewer not supported");
  } else if (isascii){
    ierr = MatView_SeqSBAIJ_ASCII(A,viewer);CHKERRQ(ierr);
  } else if (isbinary) {
    ierr = MatView_SeqSBAIJ_Binary(A,viewer);CHKERRQ(ierr);
  } else if (isdraw) {
    ierr = MatView_SeqSBAIJ_Draw(A,viewer);CHKERRQ(ierr);
  } else {
    SETERRQ1(1,1,"Viewer type %s not supported by SeqBAIJ matrices",((PetscObject)viewer)->type_name);
  }
  PetscFunctionReturn(0);
}


#undef __FUNC__  
#define __FUNC__ "MatGetValues_SeqSBAIJ"
int MatGetValues_SeqSBAIJ(Mat A,int m,int *im,int n,int *in,Scalar *v)
{
  Mat_SeqBAIJ *a = (Mat_SeqBAIJ*)A->data;
  int        *rp,k,low,high,t,row,nrow,i,col,l,*aj = a->j;
  int        *ai = a->i,*ailen = a->ilen;
  int        brow,bcol,ridx,cidx,bs=a->bs,bs2=a->bs2;
  MatScalar  *ap,*aa = a->a,zero = 0.0;

  PetscFunctionBegin;
  for (k=0; k<m; k++) { /* loop over rows */
    row  = im[k]; brow = row/bs;  
    if (row < 0) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Negative row");
    if (row >= a->m) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Row too large");
    rp   = aj + ai[brow] ; ap = aa + bs2*ai[brow] ;
    nrow = ailen[brow]; 
    for (l=0; l<n; l++) { /* loop over columns */
      if (in[l] < 0) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Negative column");
      if (in[l] >= a->n) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Column too large");
      col  = in[l] ; 
      bcol = col/bs;
      cidx = col%bs; 
      ridx = row%bs;
      high = nrow; 
      low  = 0; /* assume unsorted */
      while (high-low > 5) {
        t = (low+high)/2;
        if (rp[t] > bcol) high = t;
        else             low  = t;
      }
      for (i=low; i<high; i++) {
        if (rp[i] > bcol) break;
        if (rp[i] == bcol) {
          *v++ = ap[bs2*i+bs*cidx+ridx];
          goto finished;
        }
      } 
      *v++ = zero;
      finished:;
    }
  }
  PetscFunctionReturn(0);
} 


#undef __FUNC__  
#define __FUNC__ "MatSetValuesBlocked_SeqSBAIJ"
int MatSetValuesBlocked_SeqSBAIJ(Mat A,int m,int *im,int n,int *in,Scalar *v,InsertMode is)
{
  Mat_SeqBAIJ *a = (Mat_SeqBAIJ*)A->data;
  int         *rp,k,low,high,t,ii,jj,row,nrow,i,col,l,rmax,N,sorted=a->sorted;
  int         *imax=a->imax,*ai=a->i,*ailen=a->ilen,roworiented=a->roworiented; 
  int         *aj=a->j,nonew=a->nonew,bs2=a->bs2,bs=a->bs,stepval,ierr;
  Scalar      *value = v;
  MatScalar   *ap,*aa=a->a,*bap;

  PetscFunctionBegin;
  if (roworiented) { 
    stepval = (n-1)*bs;
  } else {
    stepval = (m-1)*bs;
  }
  for (k=0; k<m; k++) { /* loop over added rows */
    row  = im[k]; 
    if (row < 0) continue;
#if defined(PETSC_USE_BOPT_g)  
    if (row >= a->mbs) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Row too large");
#endif
    rp   = aj + ai[row]; 
    ap   = aa + bs2*ai[row];
    rmax = imax[row]; 
    nrow = ailen[row]; 
    low  = 0;
    for (l=0; l<n; l++) { /* loop over added columns */
      if (in[l] < 0) continue;
#if defined(PETSC_USE_BOPT_g)  
      if (in[l] >= a->nbs) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Column too large");
#endif
      col = in[l]; 
      if (roworiented) { 
        value = v + k*(stepval+bs)*bs + l*bs;
      } else {
        value = v + l*(stepval+bs)*bs + k*bs;
      }
      if (!sorted) low = 0; high = nrow;
      while (high-low > 7) {
        t = (low+high)/2;
        if (rp[t] > col) high = t;
        else             low  = t;
      }
      for (i=low; i<high; i++) {
        if (rp[i] > col) break;
        if (rp[i] == col) {
          bap  = ap +  bs2*i;
          if (roworiented) { 
            if (is == ADD_VALUES) {
              for (ii=0; ii<bs; ii++,value+=stepval) {
                for (jj=ii; jj<bs2; jj+=bs) {
                  bap[jj] += *value++; 
                }
              }
            } else {
              for (ii=0; ii<bs; ii++,value+=stepval) {
                for (jj=ii; jj<bs2; jj+=bs) {
                  bap[jj] = *value++; 
                }
              }
            }
          } else {
            if (is == ADD_VALUES) {
              for (ii=0; ii<bs; ii++,value+=stepval) {
                for (jj=0; jj<bs; jj++) {
                  *bap++ += *value++; 
                }
              }
            } else {
              for (ii=0; ii<bs; ii++,value+=stepval) {
                for (jj=0; jj<bs; jj++) {
                  *bap++  = *value++; 
                }
              }
            }
          }
          goto noinsert2;
        }
      } 
      if (nonew == 1) goto noinsert2;
      else if (nonew == -1) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Inserting a new nonzero in the matrix");
      if (nrow >= rmax) {
        /* there is no extra room in row, therefore enlarge */
        int       new_nz = ai[a->mbs] + CHUNKSIZE,len,*new_i,*new_j;
        MatScalar *new_a;

        if (nonew == -2) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Inserting a new nonzero in the matrix");

        /* malloc new storage space */
        len     = new_nz*(sizeof(int)+bs2*sizeof(MatScalar))+(a->mbs+1)*sizeof(int);
        new_a   = (MatScalar*)PetscMalloc(len);CHKPTRQ(new_a);
        new_j   = (int*)(new_a + bs2*new_nz);
        new_i   = new_j + new_nz;

        /* copy over old data into new slots */
        for (ii=0; ii<row+1; ii++) {new_i[ii] = ai[ii];}
        for (ii=row+1; ii<a->mbs+1; ii++) {new_i[ii] = ai[ii]+CHUNKSIZE;}
        ierr = PetscMemcpy(new_j,aj,(ai[row]+nrow)*sizeof(int));CHKERRQ(ierr);
        len  = (new_nz - CHUNKSIZE - ai[row] - nrow);
        ierr = PetscMemcpy(new_j+ai[row]+nrow+CHUNKSIZE,aj+ai[row]+nrow,len*sizeof(int));CHKERRQ(ierr);
        ierr = PetscMemcpy(new_a,aa,(ai[row]+nrow)*bs2*sizeof(MatScalar));CHKERRQ(ierr);
        ierr = PetscMemzero(new_a+bs2*(ai[row]+nrow),bs2*CHUNKSIZE*sizeof(MatScalar));CHKERRQ(ierr);
        ierr = PetscMemcpy(new_a+bs2*(ai[row]+nrow+CHUNKSIZE),aa+bs2*(ai[row]+nrow),bs2*len*sizeof(MatScalar));CHKERRQ(ierr);
        /* free up old matrix storage */
        ierr = PetscFree(a->a);CHKERRQ(ierr);
        if (!a->singlemalloc) {
          ierr = PetscFree(a->i);CHKERRQ(ierr);
          ierr = PetscFree(a->j);CHKERRQ(ierr);
        }
        aa = a->a = new_a; ai = a->i = new_i; aj = a->j = new_j; 
        a->singlemalloc = PETSC_TRUE;

        rp   = aj + ai[row]; ap = aa + bs2*ai[row];
        rmax = imax[row] = imax[row] + CHUNKSIZE;
        PLogObjectMemory(A,CHUNKSIZE*(sizeof(int) + bs2*sizeof(MatScalar)));
        a->maxnz += bs2*CHUNKSIZE;
        a->reallocs++;
        a->nz++;
      }
      N = nrow++ - 1; 
      /* shift up all the later entries in this row */
      for (ii=N; ii>=i; ii--) {
        rp[ii+1] = rp[ii];
        ierr = PetscMemcpy(ap+bs2*(ii+1),ap+bs2*(ii),bs2*sizeof(MatScalar));CHKERRQ(ierr);
      }
      if (N >= i) {
        ierr = PetscMemzero(ap+bs2*i,bs2*sizeof(MatScalar));CHKERRQ(ierr);
      }
      rp[i] = col; 
      bap   = ap +  bs2*i;
      if (roworiented) { 
        for (ii=0; ii<bs; ii++,value+=stepval) {
          for (jj=ii; jj<bs2; jj+=bs) {
            bap[jj] = *value++; 
          }
        }
      } else {
        for (ii=0; ii<bs; ii++,value+=stepval) {
          for (jj=0; jj<bs; jj++) {
            *bap++  = *value++; 
          }
        }
      }
      noinsert2:;
      low = i;
    }
    ailen[row] = nrow;
  }
  PetscFunctionReturn(0);
} 

#undef __FUNC__  
#define __FUNC__ "MatAssemblyEnd_SeqSBAIJ"
int MatAssemblyEnd_SeqSBAIJ(Mat A,MatAssemblyType mode)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  int        fshift = 0,i,j,*ai = a->i,*aj = a->j,*imax = a->imax;
  int        m = a->m,*ip,N,*ailen = a->ilen;
  int        mbs = a->mbs,bs2 = a->bs2,rmax = 0,ierr;
  MatScalar  *aa = a->a,*ap;

  PetscFunctionBegin;
  if (mode == MAT_FLUSH_ASSEMBLY) PetscFunctionReturn(0);

  if (m) rmax = ailen[0];
  for (i=1; i<mbs; i++) {
    /* move each row back by the amount of empty slots (fshift) before it*/
    fshift += imax[i-1] - ailen[i-1];
    rmax   = PetscMax(rmax,ailen[i]);
    if (fshift) {
      ip = aj + ai[i]; ap = aa + bs2*ai[i];
      N = ailen[i];
      for (j=0; j<N; j++) {
        ip[j-fshift] = ip[j];
        ierr = PetscMemcpy(ap+(j-fshift)*bs2,ap+j*bs2,bs2*sizeof(MatScalar));CHKERRQ(ierr);
      }
    } 
    ai[i] = ai[i-1] + ailen[i-1];
  }
  if (mbs) {
    fshift += imax[mbs-1] - ailen[mbs-1];
    ai[mbs] = ai[mbs-1] + ailen[mbs-1];
  }
  /* reset ilen and imax for each row */
  for (i=0; i<mbs; i++) {
    ailen[i] = imax[i] = ai[i+1] - ai[i];
  }
  a->s_nz = ai[mbs]; 

  /* diagonals may have moved, so kill the diagonal pointers */
  if (fshift && a->diag) {
    ierr = PetscFree(a->diag);CHKERRQ(ierr);
    PLogObjectMemory(A,-(m+1)*sizeof(int));
    a->diag = 0;
  } 
  PLogInfo(A,"MatAssemblyEnd_SeqSBAIJ:Matrix size: %d X %d, block size %d; storage space: %d unneeded, %d used\n",
           m,a->m,a->bs,fshift*bs2,a->s_nz*bs2);
  PLogInfo(A,"MatAssemblyEnd_SeqSBAIJ:Number of mallocs during MatSetValues is %d\n",
           a->reallocs);
  PLogInfo(A,"MatAssemblyEnd_SeqSBAIJ:Most nonzeros blocks in any row is %d\n",rmax);
  a->reallocs          = 0;
  A->info.nz_unneeded  = (PetscReal)fshift*bs2;

  PetscFunctionReturn(0);
}

/* 
   This function returns an array of flags which indicate the locations of contiguous
   blocks that should be zeroed. for eg: if bs = 3  and is = [0,1,2,3,5,6,7,8,9]
   then the resulting sizes = [3,1,1,3,1] correspondig to sets [(0,1,2),(3),(5),(6,7,8),(9)]
   Assume: sizes should be long enough to hold all the values.
*/
#undef __FUNC__  
#define __FUNC__ "MatZeroRows_SeqSBAIJ_Check_Blocks"
static int MatZeroRows_SeqSBAIJ_Check_Blocks(int idx[],int n,int bs,int sizes[], int *bs_max)
{
  int        i,j,k,row;
  PetscTruth flg;

  PetscFunctionBegin;
  for (i=0,j=0; i<n; j++) {
    row = idx[i];
    if (row%bs!=0) { /* Not the begining of a block */
      sizes[j] = 1;
      i++; 
    } else if (i+bs > n) { /* Beginning of a block, but complete block doesn't exist (at idx end) */
      sizes[j] = 1;         /* Also makes sure atleast 'bs' values exist for next else */
      i++; 
    } else { /* Begining of the block, so check if the complete block exists */
      flg = PETSC_TRUE;
      for (k=1; k<bs; k++) {
        if (row+k != idx[i+k]) { /* break in the block */
          flg = PETSC_FALSE;
          break;
        }
      }
      if (flg == PETSC_TRUE) { /* No break in the bs */
        sizes[j] = bs;
        i+= bs;
      } else {
        sizes[j] = 1;
        i++;
      }
    }
  }
  *bs_max = j;
  PetscFunctionReturn(0);
}
  
#undef __FUNC__  
#define __FUNC__ "MatZeroRows_SeqSBAIJ"
int MatZeroRows_SeqSBAIJ(Mat A,IS is,Scalar *diag)
{
  Mat_SeqSBAIJ *sbaij=(Mat_SeqSBAIJ*)A->data;
  int         ierr,i,j,k,count,is_n,*is_idx,*rows;
  int         bs=sbaij->bs,bs2=sbaij->bs2,*sizes,row,bs_max;
  Scalar      zero = 0.0;
  MatScalar   *aa;

  PetscFunctionBegin;
  /* Make a copy of the IS and  sort it */
  ierr = ISGetSize(is,&is_n);CHKERRQ(ierr);
  ierr = ISGetIndices(is,&is_idx);CHKERRQ(ierr);

  /* allocate memory for rows,sizes */
  rows  = (int*)PetscMalloc((3*is_n+1)*sizeof(int));CHKPTRQ(rows);
  sizes = rows + is_n;

  /* initialize copy IS values to rows, and sort them */
  for (i=0; i<is_n; i++) { rows[i] = is_idx[i]; }
  ierr = PetscSortInt(is_n,rows);CHKERRQ(ierr);
  if (sbaij->keepzeroedrows) { /* do not change nonzero structure */
    for (i=0; i<is_n; i++) { sizes[i] = 1; } /* sizes: size of blocks, = 1 or bs */
    bs_max = is_n;            /* bs_max: num. of contiguous block row in the row */
  } else {
    ierr = MatZeroRows_SeqSBAIJ_Check_Blocks(rows,is_n,bs,sizes,&bs_max);CHKERRQ(ierr);
  }
  ierr = ISRestoreIndices(is,&is_idx);CHKERRQ(ierr);

  for (i=0,j=0; i<bs_max; j+=sizes[i],i++) {
    row   = rows[j];                  /* row to be zeroed */
    if (row < 0 || row > sbaij->m) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,0,"row %d out of range",row);
    count = (sbaij->i[row/bs +1] - sbaij->i[row/bs])*bs; /* num. of elements in the row */
    aa    = sbaij->a + sbaij->i[row/bs]*bs2 + (row%bs);
    if (sizes[i] == bs && !sbaij->keepzeroedrows) {
      if (diag) {
        if (sbaij->ilen[row/bs] > 0) {
          sbaij->ilen[row/bs] = 1;
          sbaij->j[sbaij->i[row/bs]] = row/bs;
          ierr = PetscMemzero(aa,count*bs*sizeof(MatScalar));CHKERRQ(ierr);
        } 
        /* Now insert all the diagoanl values for this bs */
        for (k=0; k<bs; k++) {
          ierr = (*A->ops->setvalues)(A,1,rows+j+k,1,rows+j+k,diag,INSERT_VALUES);CHKERRQ(ierr);
        } 
      } else { /* (!diag) */
        sbaij->ilen[row/bs] = 0;
      } /* end (!diag) */
    } else { /* (sizes[i] != bs), broken block */
#if defined (PETSC_USE_DEBUG)
      if (sizes[i] != 1) SETERRQ(1,0,"Internal Error. Value should be 1");
#endif
      for (k=0; k<count; k++) { 
        aa[0] = zero; 
        aa+=bs;
      }
      if (diag) {
        ierr = (*A->ops->setvalues)(A,1,rows+j,1,rows+j,diag,INSERT_VALUES);CHKERRQ(ierr);
      }
    }
  }

  ierr = PetscFree(rows);CHKERRQ(ierr);
  ierr = MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/* Only add/insert a(i,j) with i<=j (blocks). 
   Any a(i,j) with i>j input by user is ingored. 
*/

#undef __FUNC__  
#define __FUNC__ "MatSetValues_SeqSBAIJ"
int MatSetValues_SeqSBAIJ(Mat A,int m,int *im,int n,int *in,Scalar *v,InsertMode is)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  int         *rp,k,low,high,t,ii,row,nrow,i,col,l,rmax,N,sorted=a->sorted;
  int         *imax=a->imax,*ai=a->i,*ailen=a->ilen,roworiented=a->roworiented;
  int         *aj=a->j,nonew=a->nonew,bs=a->bs,brow,bcol;
  int         ridx,cidx,bs2=a->bs2,ierr;
  MatScalar   *ap,value,*aa=a->a,*bap;

  PetscFunctionBegin;

  for (k=0; k<m; k++) { /* loop over added rows */
    row  = im[k];       /* row number */ 
    brow = row/bs;      /* block row number */ 
    if (row < 0) continue;
#if defined(PETSC_USE_BOPT_g)  
    if (row >= a->m) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,0,"Row too large: row %d max %d",row,a->m);
#endif
    rp   = aj + ai[brow]; /*ptr to beginning of column value of the row block*/
    ap   = aa + bs2*ai[brow]; /*ptr to beginning of element value of the row block*/
    rmax = imax[brow];  /* maximum space allocated for this row */
    nrow = ailen[brow]; /* actual length of this row */
    low  = 0;

    for (l=0; l<n; l++) { /* loop over added columns */
      if (in[l] < 0) continue;
#if defined(PETSC_USE_BOPT_g)  
      if (in[l] >= a->m) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,0,"Column too large: col %d max %d",in[l],a->m);
#endif
      col = in[l]; 
      bcol = col/bs;              /* block col number */

      if (brow <= bcol){
        ridx = row % bs; cidx = col % bs; /*row and col index inside the block */
        /* if ((brow==bcol && ridx<=cidx) || (brow<bcol)){ */     
          /* element value a(k,l) */
          if (roworiented) {
            value = v[l + k*n];             
          } else {
            value = v[k + l*m];
          }

          /* move pointer bap to a(k,l) quickly and add/insert value */
          if (!sorted) low = 0; high = nrow;
          while (high-low > 7) {
            t = (low+high)/2;
            if (rp[t] > bcol) high = t;
            else              low  = t;
          }
          for (i=low; i<high; i++) {
            /* printf("The loop of i=low.., rp[%d]=%d\n",i,rp[i]); */
            if (rp[i] > bcol) break;
            if (rp[i] == bcol) {
              bap  = ap +  bs2*i + bs*cidx + ridx;
              if (is == ADD_VALUES) *bap += value;  
              else                  *bap  = value; 
              goto noinsert1;
            }
          }      
      
      if (nonew == 1) goto noinsert1;
      else if (nonew == -1) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Inserting a new nonzero in the matrix");
      if (nrow >= rmax) {
        /* there is no extra room in row, therefore enlarge */
        int       new_nz = ai[a->mbs] + CHUNKSIZE,len,*new_i,*new_j;
        MatScalar *new_a;

        if (nonew == -2) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Inserting a new nonzero in the matrix");

        /* Malloc new storage space */
        len     = new_nz*(sizeof(int)+bs2*sizeof(MatScalar))+(a->mbs+1)*sizeof(int);
        new_a   = (MatScalar*)PetscMalloc(len);CHKPTRQ(new_a);
        new_j   = (int*)(new_a + bs2*new_nz);
        new_i   = new_j + new_nz;

        /* copy over old data into new slots */
        for (ii=0; ii<brow+1; ii++) {new_i[ii] = ai[ii];}
        for (ii=brow+1; ii<a->mbs+1; ii++) {new_i[ii] = ai[ii]+CHUNKSIZE;}
        ierr = PetscMemcpy(new_j,aj,(ai[brow]+nrow)*sizeof(int));CHKERRQ(ierr);
        len  = (new_nz - CHUNKSIZE - ai[brow] - nrow);
        ierr = PetscMemcpy(new_j+ai[brow]+nrow+CHUNKSIZE,aj+ai[brow]+nrow,len*sizeof(int));CHKERRQ(ierr);
        ierr = PetscMemcpy(new_a,aa,(ai[brow]+nrow)*bs2*sizeof(MatScalar));CHKERRQ(ierr);
        ierr = PetscMemzero(new_a+bs2*(ai[brow]+nrow),bs2*CHUNKSIZE*sizeof(MatScalar));CHKERRQ(ierr);
        ierr = PetscMemcpy(new_a+bs2*(ai[brow]+nrow+CHUNKSIZE),aa+bs2*(ai[brow]+nrow),bs2*len*sizeof(MatScalar));CHKERRQ(ierr);
        /* free up old matrix storage */
        ierr = PetscFree(a->a);CHKERRQ(ierr);
        if (!a->singlemalloc) {
          ierr = PetscFree(a->i);CHKERRQ(ierr);
          ierr = PetscFree(a->j);CHKERRQ(ierr);
        }
        aa = a->a = new_a; ai = a->i = new_i; aj = a->j = new_j; 
        a->singlemalloc = PETSC_TRUE;

        rp   = aj + ai[brow]; ap = aa + bs2*ai[brow];
        rmax = imax[brow] = imax[brow] + CHUNKSIZE;
        PLogObjectMemory(A,CHUNKSIZE*(sizeof(int) + bs2*sizeof(MatScalar)));
        a->s_maxnz += bs2*CHUNKSIZE;
        a->reallocs++;
        a->s_nz++;
      }

      N = nrow++ - 1;     
      /* shift up all the later entries in this row */
      for (ii=N; ii>=i; ii--) {
        rp[ii+1] = rp[ii];
        ierr     = PetscMemcpy(ap+bs2*(ii+1),ap+bs2*(ii),bs2*sizeof(MatScalar));CHKERRQ(ierr);
      }
      if (N>=i) {
        ierr = PetscMemzero(ap+bs2*i,bs2*sizeof(MatScalar));CHKERRQ(ierr);
      }
      rp[i]                      = bcol; 
      ap[bs2*i + bs*cidx + ridx] = value; 
      noinsert1:;
      low = i;      
      /* } */
    } /* end of if .. if..  */
    }                     /* end of loop over added columns */
    ailen[brow] = nrow; 
  }                       /* end of loop over added rows */

  PetscFunctionReturn(0);
} 

extern int MatCholeskyFactorSymbolic_SeqSBAIJ(Mat,IS,PetscReal,Mat*);
extern int MatCholeskyFactor_SeqSBAIJ(Mat,IS,PetscReal);
extern int MatIncreaseOverlap_SeqSBAIJ(Mat,int,IS*,int);
extern int MatGetSubMatrix_SeqSBAIJ(Mat,IS,IS,int,MatReuse,Mat*);
extern int MatGetSubMatrices_SeqSBAIJ(Mat,int,IS*,IS*,MatReuse,Mat**);
extern int MatMultTranspose_SeqSBAIJ(Mat,Vec,Vec);
extern int MatMultTransposeAdd_SeqSBAIJ(Mat,Vec,Vec,Vec);
extern int MatScale_SeqSBAIJ(Scalar*,Mat);
extern int MatNorm_SeqSBAIJ(Mat,NormType,PetscReal *);
extern int MatEqual_SeqSBAIJ(Mat,Mat,PetscTruth*);
extern int MatGetDiagonal_SeqSBAIJ(Mat,Vec);
extern int MatDiagonalScale_SeqSBAIJ(Mat,Vec,Vec);
extern int MatGetInfo_SeqSBAIJ(Mat,MatInfoType,MatInfo *);
extern int MatZeroEntries_SeqSBAIJ(Mat);

extern int MatSolve_SeqSBAIJ_N(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_1(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_2(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_3(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_4(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_5(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_6(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_7(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_7(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_6(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_5(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_4(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_3(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_2(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_1(Mat,Vec,Vec);

extern int MatCholeskyFactorNumeric_SeqSBAIJ_N(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_1(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_2(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_3(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_4(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_5(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_6(Mat,Mat*);

extern int MatMult_SeqSBAIJ_1(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_2(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_3(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_4(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_5(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_6(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_7(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_N(Mat,Vec,Vec);

extern int MatMultAdd_SeqSBAIJ_1(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_2(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_3(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_4(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_5(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_6(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_7(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_N(Mat,Vec,Vec,Vec);

#ifdef MatIncompleteCholeskyFactor
/* This function is modified from MatILUFactor_SeqSBAIJ. 
   Needs further work! Don't forget to add the function to the matrix table. */
#undef __FUNC__  
#define __FUNC__ "MatIncompleteCholeskyFactor_SeqSBAIJ"
int MatIncompleteCholeskyFactor_SeqSBAIJ(Mat inA,IS row,IS col,MatILUInfo *info)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)inA->data;
  Mat         outA;
  int         ierr;
  PetscTruth  row_identity,col_identity;

  PetscFunctionBegin;
  if (info && info->levels != 0) SETERRQ(PETSC_ERR_SUP,0,"Only levels = 0 supported for in-place ILU");
  ierr = ISIdentity(row,&row_identity);CHKERRQ(ierr);
  ierr = ISIdentity(col,&col_identity);CHKERRQ(ierr);
  if (!row_identity || !col_identity) {
    SETERRQ(1,1,"Row and column permutations must be identity for in-place ILU");
  }

  outA          = inA; 
  inA->factor   = FACTOR_LU;

  if (!a->diag) {
    ierr = MatMarkDiagonal_SeqBAIJ(inA);CHKERRQ(ierr);
  }
  /*
      Blocksize 2, 3, 4, 5, 6 and 7 have a special faster factorization/solver 
      for ILU(0) factorization with natural ordering
  */
  switch (a->bs) {
  case 1:
    inA->ops->solvetranspose   = MatSolveTranspose_SeqBAIJ_2_NaturalOrdering;
    PLogInfo(inA,"MatILUFactor_SeqBAIJ:Using special in-place natural ordering solvetrans BS=1\n");
  case 2:
    inA->ops->lufactornumeric = MatLUFactorNumeric_SeqBAIJ_2_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqBAIJ_2_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolveTranspose_SeqBAIJ_2_NaturalOrdering;
    PLogInfo(inA,"MatILUFactor_SeqBAIJ:Using special in-place natural ordering factor and solve BS=2\n");
    break;
  case 3:
    inA->ops->lufactornumeric = MatLUFactorNumeric_SeqBAIJ_3_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqBAIJ_3_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolveTranspose_SeqBAIJ_3_NaturalOrdering;
    PLogInfo(inA,"MatILUFactor_SeqBAIJ:Using special in-place natural ordering factor and solve BS=3\n");
    break; 
  case 4:
    inA->ops->lufactornumeric = MatLUFactorNumeric_SeqBAIJ_4_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqBAIJ_4_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolveTranspose_SeqBAIJ_4_NaturalOrdering;
    PLogInfo(inA,"MatILUFactor_SeqBAIJ:Using special in-place natural ordering factor and solve BS=4\n"); 
    break;
  case 5:
    inA->ops->lufactornumeric = MatLUFactorNumeric_SeqBAIJ_5_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqBAIJ_5_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolveTranspose_SeqBAIJ_5_NaturalOrdering;
    PLogInfo(inA,"MatILUFactor_SeqBAIJ:Using special in-place natural ordering factor and solve BS=5\n"); 
    break;
  case 6: 
    inA->ops->lufactornumeric = MatLUFactorNumeric_SeqBAIJ_6_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqBAIJ_6_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolveTranspose_SeqBAIJ_6_NaturalOrdering;
    PLogInfo(inA,"MatILUFactor_SeqBAIJ:Using special in-place natural ordering factor and solve BS=6\n");
    break; 
  case 7:
    inA->ops->lufactornumeric = MatLUFactorNumeric_SeqBAIJ_7_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolveTranspose_SeqBAIJ_7_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqBAIJ_7_NaturalOrdering;
    PLogInfo(inA,"MatILUFactor_SeqBAIJ:Using special in-place natural ordering factor and solve BS=7\n");
    break; 
  default:
    a->row        = row;
    a->col        = col;
    ierr          = PetscObjectReference((PetscObject)row);CHKERRQ(ierr);
    ierr          = PetscObjectReference((PetscObject)col);CHKERRQ(ierr);

    /* Create the invert permutation so that it can be used in MatLUFactorNumeric() */
    ierr = ISInvertPermutation(col,PETSC_DECIDE, &(a->icol));CHKERRQ(ierr);
    PLogObjectParent(inA,a->icol);

    if (!a->solve_work) {
      a->solve_work = (Scalar*)PetscMalloc((a->m+a->bs)*sizeof(Scalar));CHKPTRQ(a->solve_work);
      PLogObjectMemory(inA,(a->m+a->bs)*sizeof(Scalar));
    }
  }

  ierr = MatLUFactorNumeric(inA,&outA);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}
#endif

#undef __FUNC__  
#define __FUNC__ "MatPrintHelp_SeqSBAIJ"
int MatPrintHelp_SeqSBAIJ(Mat A)
{
  static PetscTruth called = PETSC_FALSE; 
  MPI_Comm          comm = A->comm;
  int               ierr;

  PetscFunctionBegin;
  if (called) {PetscFunctionReturn(0);} else called = PETSC_TRUE;
  ierr = (*PetscHelpPrintf)(comm," Options for MATSEQSBAIJ and MATMPISBAIJ matrix formats (the defaults):\n");CHKERRQ(ierr);
  ierr = (*PetscHelpPrintf)(comm,"  -mat_block_size <block_size>\n");CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

EXTERN_C_BEGIN
#undef __FUNC__  
#define __FUNC__ "MatSeqSBAIJSetColumnIndices_SeqSBAIJ"
int MatSeqSBAIJSetColumnIndices_SeqSBAIJ(Mat mat,int *indices)
{
  Mat_SeqBAIJ *baij = (Mat_SeqBAIJ *)mat->data;
  int         i,nz,n;

  PetscFunctionBegin;
  nz = baij->maxnz;
  n  = baij->n;
  for (i=0; i<nz; i++) {
    baij->j[i] = indices[i];
  }
  baij->nz = nz;
  for (i=0; i<n; i++) {
    baij->ilen[i] = baij->imax[i];
  }

  PetscFunctionReturn(0);
}
EXTERN_C_END

#undef __FUNC__  
#define __FUNC__ "MatSeqSBAIJSetColumnIndices"
/*@
    MatSeqBAIJSetColumnIndices - Set the column indices for all the rows
       in the matrix.

  Input Parameters:
+  mat - the SeqBAIJ matrix
-  indices - the column indices

  Level: advanced

  Notes:
    This can be called if you have precomputed the nonzero structure of the 
  matrix and want to provide it to the matrix object to improve the performance
  of the MatSetValues() operation.

    You MUST have set the correct numbers of nonzeros per row in the call to 
  MatCreateSeqBAIJ().

    MUST be called before any calls to MatSetValues()

.seealso: MatCreateSeqSBAIJ
@*/ 
int MatSeqSBAIJSetColumnIndices(Mat mat,int *indices)
{
  int ierr,(*f)(Mat,int *);

  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_COOKIE);
  ierr = PetscObjectQueryFunction((PetscObject)mat,"MatSeqSBAIJSetColumnIndices_C",(void **)&f);CHKERRQ(ierr);
  if (f) {
    ierr = (*f)(mat,indices);CHKERRQ(ierr);
  } else {
    SETERRQ(1,1,"Wrong type of matrix to set column indices");
  }
  PetscFunctionReturn(0);
}

/* -------------------------------------------------------------------*/
static struct _MatOps MatOps_Values = {MatSetValues_SeqSBAIJ,
       MatGetRow_SeqSBAIJ,
       MatRestoreRow_SeqSBAIJ,
       MatMult_SeqSBAIJ_N,
       MatMultAdd_SeqSBAIJ_N,
       MatMultTranspose_SeqSBAIJ,
       MatMultTransposeAdd_SeqSBAIJ,
       MatSolve_SeqSBAIJ_N,
       0,
       0,
       0,
       0,
       MatCholeskyFactor_SeqSBAIJ,
       0,
       MatTranspose_SeqSBAIJ,
       MatGetInfo_SeqSBAIJ,
       MatEqual_SeqSBAIJ,
       MatGetDiagonal_SeqSBAIJ,
       MatDiagonalScale_SeqSBAIJ,
       MatNorm_SeqSBAIJ,
       0,
       MatAssemblyEnd_SeqSBAIJ,
       0,
       MatSetOption_SeqSBAIJ,
       MatZeroEntries_SeqSBAIJ,
       MatZeroRows_SeqSBAIJ,
       0,
       0,
       MatCholeskyFactorSymbolic_SeqSBAIJ,
       MatCholeskyFactorNumeric_SeqSBAIJ_N,
       MatGetSize_SeqSBAIJ,
       MatGetSize_SeqSBAIJ,
       MatGetOwnershipRange_SeqSBAIJ,
       0,
       MatIncompleteCholeskyFactorSymbolic_SeqSBAIJ,
       0,
       0,
       MatDuplicate_SeqSBAIJ,
       0,
       0,
       0,
       0,
       0,
       MatGetSubMatrices_SeqSBAIJ,
       MatIncreaseOverlap_SeqSBAIJ,
       MatGetValues_SeqSBAIJ,
       0,
       MatPrintHelp_SeqSBAIJ,
       MatScale_SeqSBAIJ,
       0,
       0,
       0,
       MatGetBlockSize_SeqSBAIJ,
       MatGetRowIJ_SeqSBAIJ,
       MatRestoreRowIJ_SeqSBAIJ,
       0,
       0,
       0,
       0,
       0,
       0,
       MatSetValuesBlocked_SeqSBAIJ,
       MatGetSubMatrix_SeqSBAIJ,
       0,
       0,
       MatGetMaps_Petsc};

EXTERN_C_BEGIN
#undef __FUNC__  
#define __FUNC__ "MatStoreValues_SeqSBAIJ"
int MatStoreValues_SeqSBAIJ(Mat mat)
{
  Mat_SeqSBAIJ *aij = (Mat_SeqSBAIJ *)mat->data;
  int         nz = aij->i[aij->m]*aij->bs*aij->bs2;
  int         ierr;

  PetscFunctionBegin;
  if (aij->nonew != 1) {
    SETERRQ(1,1,"Must call MatSetOption(A,MAT_NO_NEW_NONZERO_LOCATIONS);first");
  }

  /* allocate space for values if not already there */
  if (!aij->saved_values) {
    aij->saved_values = (Scalar*)PetscMalloc(nz*sizeof(Scalar));CHKPTRQ(aij->saved_values);
  }

  /* copy values over */
  ierr = PetscMemcpy(aij->saved_values,aij->a,nz*sizeof(Scalar));CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END

EXTERN_C_BEGIN
#undef __FUNC__  
#define __FUNC__ "MatRetrieveValues_SeqSBAIJ"
int MatRetrieveValues_SeqSBAIJ(Mat mat)
{
  Mat_SeqSBAIJ *aij = (Mat_SeqSBAIJ *)mat->data;
  int         nz = aij->i[aij->m]*aij->bs*aij->bs2,ierr;

  PetscFunctionBegin;
  if (aij->nonew != 1) {
    SETERRQ(1,1,"Must call MatSetOption(A,MAT_NO_NEW_NONZERO_LOCATIONS);first");
  }
  if (!aij->saved_values) {
    SETERRQ(1,1,"Must call MatStoreValues(A);first");
  }

  /* copy values over */
  ierr = PetscMemcpy(aij->a,aij->saved_values,nz*sizeof(Scalar));CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END

#undef __FUNC__  
#define __FUNC__ "MatCreateSeqSBAIJ"
/*@C
   MatCreateSeqSBAIJ - Creates a sparse symmetric matrix in block AIJ (block
   compressed row) format.  For good matrix assembly performance the
   user should preallocate the matrix storage by setting the parameter nz
   (or the array nnz).  By setting these parameters accurately, performance
   during matrix assembly can be increased by more than a factor of 50.

   Collective on MPI_Comm

   Input Parameters:
+  comm - MPI communicator, set to PETSC_COMM_SELF
.  bs - size of block
.  m - number of rows, or number of columns
.  nz - number of block nonzeros per block row (same for all rows)
-  nnz - array containing the number of block nonzeros in the various block rows 
         (possibly different for each block row) or PETSC_NULL

   Output Parameter:
.  A - the symmetric matrix 

   Options Database Keys:
.   -mat_no_unroll - uses code that does not unroll the loops in the 
                     block calculations (much slower)
.    -mat_block_size - size of the blocks to use

   Level: intermediate

   Notes:
   The block AIJ format is fully compatible with standard Fortran 77
   storage.  That is, the stored row and column indices can begin at
   either one (as in Fortran) or zero.  See the users' manual for details.

   Specify the preallocated storage with either nz or nnz (not both).
   Set nz=PETSC_DEFAULT and nnz=PETSC_NULL for PETSc to control dynamic memory 
   allocation.  For additional details, see the users manual chapter on
   matrices.

.seealso: MatCreate(), MatCreateSeqAIJ(), MatSetValues(), MatCreateMPIBAIJ()
@*/
int MatCreateSeqSBAIJ(MPI_Comm comm,int bs,int m,int n,int nz,int *nnz,Mat *A)
{
  Mat         B;
  Mat_SeqSBAIJ *b;
  int         i,len,ierr,mbs,bs2,size;
  PetscTruth  flg;
  int         s_nz; 

  PetscFunctionBegin;
  ierr = MPI_Comm_size(comm,&size);CHKERRQ(ierr);
  if (size > 1) SETERRQ(PETSC_ERR_ARG_WRONG,0,"Comm must be of size 1");

  ierr = OptionsGetInt(PETSC_NULL,"-mat_block_size",&bs,PETSC_NULL);CHKERRQ(ierr);
  mbs  = m/bs;
  bs2  = bs*bs;

  if (mbs*bs!=m) {
    SETERRQ(PETSC_ERR_ARG_SIZ,0,"Number rows, cols must be divisible by blocksize");
  }

  if (nz < -2) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,0,"nz cannot be less than -2: value %d",nz);
  if (nnz) {
    for (i=0; i<mbs; i++) {
      if (nnz[i] < 0) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,0,"nnz cannot be less than 0: local row %d value %d",i,nnz[i]);
    }
  }

  *A = 0;
  PetscHeaderCreate(B,_p_Mat,struct _MatOps,MAT_COOKIE,MATSEQSBAIJ,"Mat",comm,MatDestroy,MatView);
  PLogObjectCreate(B);
  B->data = (void*)(b = PetscNew(Mat_SeqSBAIJ));CHKPTRQ(b);
  ierr    = PetscMemzero(b,sizeof(Mat_SeqSBAIJ));CHKERRQ(ierr);
  ierr    = PetscMemcpy(B->ops,&MatOps_Values,sizeof(struct _MatOps));CHKERRQ(ierr);
  ierr    = OptionsHasName(PETSC_NULL,"-mat_no_unroll",&flg);CHKERRQ(ierr);
  if (!flg) {
    switch (bs) {
    case 1:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_1;
      B->ops->solve           = MatSolve_SeqSBAIJ_1;
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_1;
      B->ops->mult            = MatMult_SeqSBAIJ_1;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_1;
      break;
    case 2:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_2;  
      B->ops->solve           = MatSolve_SeqSBAIJ_2;
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_2;
      B->ops->mult            = MatMult_SeqSBAIJ_2;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_2;
      break;
    case 3:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_3;  
      B->ops->solve           = MatSolve_SeqSBAIJ_3;
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_3;
      B->ops->mult            = MatMult_SeqSBAIJ_3;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_3;
      break;
    case 4:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_4;  
      B->ops->solve           = MatSolve_SeqSBAIJ_4;
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_4;
      B->ops->mult            = MatMult_SeqSBAIJ_4;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_4;
      break;
    case 5:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_5;  
      B->ops->solve           = MatSolve_SeqSBAIJ_5; 
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_5;
      B->ops->mult            = MatMult_SeqSBAIJ_5;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_5;
      break;
    case 6:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_6;  
      B->ops->solve           = MatSolve_SeqSBAIJ_6; 
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_6;
      B->ops->mult            = MatMult_SeqSBAIJ_6;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_6;
      break;
    case 7:
      B->ops->mult            = MatMult_SeqSBAIJ_7; 
      B->ops->solve           = MatSolve_SeqSBAIJ_7;
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_7;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_7;
      break;
    }
  }
  B->ops->destroy     = MatDestroy_SeqSBAIJ;
  B->ops->view        = MatView_SeqSBAIJ;
  B->factor           = 0;
  B->lupivotthreshold = 1.0;
  B->mapping          = 0;
  b->row              = 0;
  b->col              = 0;
  b->icol             = 0;
  b->reallocs         = 0;
  b->saved_values     = 0;
  
  b->m       = m; 
  B->m = m; B->M = m;
  b->n       = m; 
  B->n = m; B->N = m;

  ierr = MapCreateMPI(comm,m,m,&B->rmap);CHKERRQ(ierr);
  ierr = MapCreateMPI(comm,m,m,&B->cmap);CHKERRQ(ierr);

  b->mbs     = mbs;
  b->nbs     = mbs; 
  b->imax    = (int*)PetscMalloc((mbs+1)*sizeof(int));CHKPTRQ(b->imax);
  if (!nnz) {
    if (nz == PETSC_DEFAULT) nz = 5;
    else if (nz <= 0)        nz = 1;
    for (i=0; i<mbs; i++) {
      b->imax[i] = (nz+1)/2; 
    }
    nz = nz*mbs; 
  } else {
    nz = 0;
    for (i=0; i<mbs; i++) {b->imax[i] = (nnz[i]+1)/2; nz += nnz[i];}
  }
  s_nz=(nz+mbs)/2;  /* total diagonal and superdiagonal nonzero blocks */

  /* allocate the matrix space */
  len   = s_nz*sizeof(int) + s_nz*bs2*sizeof(MatScalar) + (b->m+1)*sizeof(int);
  b->a  = (MatScalar*)PetscMalloc(len);CHKPTRQ(b->a);
  ierr = PetscMemzero(b->a,s_nz*bs2*sizeof(MatScalar));CHKERRQ(ierr);
  b->j  = (int*)(b->a + s_nz*bs2);
  ierr = PetscMemzero(b->j,s_nz*sizeof(int));CHKERRQ(ierr);
  b->i  = b->j + s_nz;
  b->singlemalloc = PETSC_TRUE;

  /* pointer to beginning of each row */
  b->i[0] = 0;
  for (i=1; i<mbs+1; i++) {
    b->i[i] = b->i[i-1] + b->imax[i-1];
  }
  

  /* b->ilen will count nonzeros in each block row so far. */
  b->ilen = (int*)PetscMalloc((mbs+1)*sizeof(int)); 
  PLogObjectMemory(B,len+2*(mbs+1)*sizeof(int)+sizeof(struct _p_Mat)+sizeof(Mat_SeqSBAIJ));
  for (i=0; i<mbs; i++) { b->ilen[i] = 0;}
  
  b->bs               = bs;
  b->bs2              = bs2;
  b->mbs              = mbs;
  b->s_nz               = 0;
  b->s_maxnz            = s_nz*bs2;
  b->sorted           = PETSC_FALSE;
  b->roworiented      = PETSC_TRUE;
  b->nonew            = 0;
  b->diag             = 0;
  b->solve_work       = 0;
  b->mult_work        = 0;
  b->spptr            = 0;
  B->info.nz_unneeded = (PetscReal)b->s_maxnz;
  b->keepzeroedrows   = PETSC_FALSE;

  *A = B;
  ierr = OptionsHasName(PETSC_NULL,"-help",&flg);CHKERRQ(ierr);
  if (flg) {ierr = MatPrintHelp(B);CHKERRQ(ierr); }

  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatStoreValues_C",
                                     "MatStoreValues_SeqSBAIJ",
                                     (void*)MatStoreValues_SeqSBAIJ);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatRetrieveValues_C",
                                     "MatRetrieveValues_SeqSBAIJ",
                                     (void*)MatRetrieveValues_SeqSBAIJ);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatSeqSBAIJSetColumnIndices_C",
                                     "MatSeqSBAIJSetColumnIndices_SeqSBAIJ",
                                     (void*)MatSeqSBAIJSetColumnIndices_SeqSBAIJ);CHKERRQ(ierr);
   /* printf("In MatCreateSeqSBAIJ, \n");                                   
   for (i=0; i<mbs; i++){
     printf("imax[%d]= %d, ilen[%d]= %d\n", i,b->imax[i], i,b->ilen[i]);
   }       */                           

  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatDuplicate_SeqSBAIJ"
int MatDuplicate_SeqSBAIJ(Mat A,MatDuplicateOption cpvalues,Mat *B)
{
  Mat         C;
  Mat_SeqSBAIJ *c,*a = (Mat_SeqSBAIJ*)A->data;
  int         i,len,mbs = a->mbs,nz = a->s_nz,bs2 =a->bs2,ierr;

  PetscFunctionBegin;
  if (a->i[mbs] != nz) SETERRQ(PETSC_ERR_PLIB,0,"Corrupt matrix");

  *B = 0;
  PetscHeaderCreate(C,_p_Mat,struct _MatOps,MAT_COOKIE,MATSEQSBAIJ,"Mat",A->comm,MatDestroy,MatView);
  PLogObjectCreate(C);
  C->data           = (void*)(c = PetscNew(Mat_SeqSBAIJ));CHKPTRQ(c);
  ierr              = PetscMemcpy(C->ops,A->ops,sizeof(struct _MatOps));CHKERRQ(ierr);
  C->ops->destroy   = MatDestroy_SeqSBAIJ;
  C->ops->view      = MatView_SeqSBAIJ;
  C->factor         = A->factor;
  c->row            = 0;
  c->col            = 0;
  c->icol           = 0;
  c->saved_values   = 0;
  c->keepzeroedrows = a->keepzeroedrows;
  C->assembled      = PETSC_TRUE;

  c->m = C->m   = a->m;
  c->n = C->n   = a->n;
  C->M          = a->m;
  C->N          = a->n;

  c->bs         = a->bs;
  c->bs2        = a->bs2;
  c->mbs        = a->mbs;
  c->nbs        = a->nbs;

  c->imax       = (int*)PetscMalloc((mbs+1)*sizeof(int));CHKPTRQ(c->imax);
  c->ilen       = (int*)PetscMalloc((mbs+1)*sizeof(int));CHKPTRQ(c->ilen);
  for (i=0; i<mbs; i++) {
    c->imax[i] = a->imax[i];
    c->ilen[i] = a->ilen[i]; 
  }

  /* allocate the matrix space */
  c->singlemalloc = PETSC_TRUE;
  len  = (mbs+1)*sizeof(int) + nz*(bs2*sizeof(MatScalar) + sizeof(int));
  c->a = (MatScalar*)PetscMalloc(len);CHKPTRQ(c->a);
  c->j = (int*)(c->a + nz*bs2);
  c->i = c->j + nz;
  ierr = PetscMemcpy(c->i,a->i,(mbs+1)*sizeof(int));CHKERRQ(ierr);
  if (mbs > 0) {
    ierr = PetscMemcpy(c->j,a->j,nz*sizeof(int));CHKERRQ(ierr);
    if (cpvalues == MAT_COPY_VALUES) {
      ierr = PetscMemcpy(c->a,a->a,bs2*nz*sizeof(MatScalar));CHKERRQ(ierr);
    } else {
      ierr = PetscMemzero(c->a,bs2*nz*sizeof(MatScalar));CHKERRQ(ierr);
    }
  }

  PLogObjectMemory(C,len+2*(mbs+1)*sizeof(int)+sizeof(struct _p_Mat)+sizeof(Mat_SeqSBAIJ));  
  c->sorted      = a->sorted;
  c->roworiented = a->roworiented;
  c->nonew       = a->nonew;

  if (a->diag) {
    c->diag = (int*)PetscMalloc((mbs+1)*sizeof(int));CHKPTRQ(c->diag);
    PLogObjectMemory(C,(mbs+1)*sizeof(int));
    for (i=0; i<mbs; i++) {
      c->diag[i] = a->diag[i];
    }
  } else c->diag        = 0;
  c->s_nz               = a->s_nz;
  c->s_maxnz            = a->s_maxnz;
  c->solve_work         = 0;
  c->spptr              = 0;      /* Dangerous -I'm throwing away a->spptr */
  c->mult_work          = 0;
  *B = C;
  ierr = FListDuplicate(A->qlist,&C->qlist);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "MatLoad_SeqSBAIJ"
int MatLoad_SeqSBAIJ(Viewer viewer,MatType type,Mat *A)
{
  Mat_SeqSBAIJ  *a;
  Mat          B;
  int          i,nz,ierr,fd,header[4],size,*rowlengths=0,M,N,bs=1;
  int          *mask,mbs,*jj,j,rowcount,nzcount,k,*browlengths,maskcount;
  int          kmax,jcount,block,idx,point,nzcountb,extra_rows;
  int          *masked,nmask,tmp,bs2,ishift;
  Scalar       *aa;
  MPI_Comm     comm = ((PetscObject)viewer)->comm;

  PetscFunctionBegin;
  ierr = OptionsGetInt(PETSC_NULL,"-matload_block_size",&bs,PETSC_NULL);CHKERRQ(ierr);
  bs2  = bs*bs;

  ierr = MPI_Comm_size(comm,&size);CHKERRQ(ierr);
  if (size > 1) SETERRQ(PETSC_ERR_ARG_WRONG,0,"view must have one processor");
  ierr = ViewerBinaryGetDescriptor(viewer,&fd);CHKERRQ(ierr);
  ierr = PetscBinaryRead(fd,header,4,PETSC_INT);CHKERRQ(ierr);
  if (header[0] != MAT_COOKIE) SETERRQ(PETSC_ERR_FILE_UNEXPECTED,0,"not Mat object");
  M = header[1]; N = header[2]; nz = header[3];

  if (header[3] < 0) {
    SETERRQ(PETSC_ERR_FILE_UNEXPECTED,1,"Matrix stored in special format, cannot load as SeqSBAIJ");
  }

  if (M != N) SETERRQ(PETSC_ERR_SUP,0,"Can only do square matrices");

  /* 
     This code adds extra rows to make sure the number of rows is 
    divisible by the blocksize
  */
  mbs        = M/bs;
  extra_rows = bs - M + bs*(mbs);
  if (extra_rows == bs) extra_rows = 0;
  else                  mbs++;
  if (extra_rows) {
    PLogInfo(0,"MatLoad_SeqSBAIJ:Padding loaded matrix to match blocksize\n");
  }

  /* read in row lengths */
  rowlengths = (int*)PetscMalloc((M+extra_rows)*sizeof(int));CHKPTRQ(rowlengths);
  ierr = PetscBinaryRead(fd,rowlengths,M,PETSC_INT);CHKERRQ(ierr);
  for (i=0; i<extra_rows; i++) rowlengths[M+i] = 1;

  /* read in column indices */
  jj = (int*)PetscMalloc((nz+extra_rows)*sizeof(int));CHKPTRQ(jj);
  ierr = PetscBinaryRead(fd,jj,nz,PETSC_INT);CHKERRQ(ierr);
  for (i=0; i<extra_rows; i++) jj[nz+i] = M+i;

  /* loop over row lengths determining block row lengths */
  browlengths = (int*)PetscMalloc(mbs*sizeof(int));CHKPTRQ(browlengths);
  ierr        = PetscMemzero(browlengths,mbs*sizeof(int));CHKERRQ(ierr);
  mask        = (int*)PetscMalloc(2*mbs*sizeof(int));CHKPTRQ(mask);
  ierr        = PetscMemzero(mask,mbs*sizeof(int));CHKERRQ(ierr);
  masked      = mask + mbs;
  rowcount    = 0; nzcount = 0;
  for (i=0; i<mbs; i++) {
    nmask = 0;
    for (j=0; j<bs; j++) {
      kmax = rowlengths[rowcount];
      for (k=0; k<kmax; k++) {
        tmp = jj[nzcount++]/bs;
        if (!mask[tmp]) {masked[nmask++] = tmp; mask[tmp] = 1;}
      }
      rowcount++;
    }
    browlengths[i] += nmask;
    /* zero out the mask elements we set */
    for (j=0; j<nmask; j++) mask[masked[j]] = 0;
  }

  /* create our matrix */
  ierr = MatCreateSeqSBAIJ(comm,bs,M+extra_rows,N+extra_rows,0,browlengths,A);CHKERRQ(ierr);
  B = *A;
  a = (Mat_SeqSBAIJ*)B->data;

  /* set matrix "i" values */
  a->i[0] = 0;
  for (i=1; i<= mbs; i++) {
    a->i[i]      = a->i[i-1] + browlengths[i-1];
    a->ilen[i-1] = browlengths[i-1];
  }
  a->s_nz         = 0;
  for (i=0; i<mbs; i++) a->s_nz += browlengths[i];

  /* read in nonzero values */
  aa = (Scalar*)PetscMalloc((nz+extra_rows)*sizeof(Scalar));CHKPTRQ(aa);
  ierr = PetscBinaryRead(fd,aa,nz,PETSC_SCALAR);CHKERRQ(ierr);
  for (i=0; i<extra_rows; i++) aa[nz+i] = 1.0;

  /* set "a" and "j" values into matrix */
  nzcount = 0; jcount = 0;
  for (i=0; i<mbs; i++) {
    nzcountb = nzcount;
    nmask    = 0;
    for (j=0; j<bs; j++) {
      kmax = rowlengths[i*bs+j];
      for (k=0; k<kmax; k++) {
        tmp = jj[nzcount++]/bs;
	if (!mask[tmp]) { masked[nmask++] = tmp; mask[tmp] = 1;}
      }
      rowcount++;
    }
    /* sort the masked values */
    ierr = PetscSortInt(nmask,masked);CHKERRQ(ierr);

    /* set "j" values into matrix */
    maskcount = 1;
    for (j=0; j<nmask; j++) {
      a->j[jcount++]  = masked[j];
      mask[masked[j]] = maskcount++; 
    }
    /* set "a" values into matrix */
    ishift = bs2*a->i[i];
    for (j=0; j<bs; j++) {
      kmax = rowlengths[i*bs+j];
      for (k=0; k<kmax; k++) {
        tmp       = jj[nzcountb]/bs ;
        block     = mask[tmp] - 1;
        point     = jj[nzcountb] - bs*tmp;
        idx       = ishift + bs2*block + j + bs*point;
        a->a[idx] = aa[nzcountb++];
      }
    }
    /* zero out the mask elements we set */
    for (j=0; j<nmask; j++) mask[masked[j]] = 0;
  }
  if (jcount != a->s_nz) SETERRQ(PETSC_ERR_FILE_UNEXPECTED,0,"Bad binary matrix");

  ierr = PetscFree(rowlengths);CHKERRQ(ierr);
  ierr = PetscFree(browlengths);CHKERRQ(ierr);
  ierr = PetscFree(aa);CHKERRQ(ierr);
  ierr = PetscFree(jj);CHKERRQ(ierr);
  ierr = PetscFree(mask);CHKERRQ(ierr);

  B->assembled = PETSC_TRUE;

  ierr = MatView_Private(B);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}



