/*$Id: snesmfjdef.c,v 1.31 2001/08/30 21:37:18 balay Exp $*/
/*
  Implements the default PETSc approach for computing the h 
  parameter used with the finite difference based matrix-free 
  Jacobian-vector products.

  To make your own: clone this file and modify for your needs.

  Mandatory functions:
  -------------------
      MatSNESMFCompute_  - for a given point and direction computes h

      MatSNESMFCreate_ - fills in the MatSNESMF data structure
                           for this particular implementation

      
   Optional functions:
   -------------------
      MatSNESMFView_ - prints information about the parameters being used.
                       This is called when SNESView() or -snes_view is used.

      MatSNESMFSetFromOptions_ - checks the options database for options that 
                               apply to this method.

      MatSNESMFDestroy_ - frees any space allocated by the routines above

*/

/*
    This include file defines the data structure  MatSNESMF that 
   includes information about the computation of h. It is shared by 
   all implementations that people provide
*/
#include "src/snes/mf/snesmfj.h"   /*I  "petscsnes.h"   I*/
#include "src/mat/matimpl.h"

/*
      The default method has one parameter that is used to 
   "cutoff" very small values. This is stored in a data structure
   that is only visible to this file. If your method has no parameters
   it can omit this, if it has several simply reorganize the data structure.
   The data structure is "hung-off" the MatSNESMF data structure in
   the void *hctx; field.
*/
typedef struct {
  PetscReal umin;          /* minimum allowable u'a value relative to |u|_1 */
} MatSNESMFDefault;

#undef __FUNCT__  
#define __FUNCT__ "MatSNESMFCompute_Default"
/*
   MatSNESMFCompute_Default - Standard PETSc code for computing the
   differencing paramter (h) for use with matrix-free finite differences.

   Input Parameters:
+  ctx - the matrix free context
.  U - the location at which you want the Jacobian
-  a - the direction you want the derivative

   Output Parameter:
.  h - the scale computed

*/
static int MatSNESMFCompute_Default(MatSNESMFCtx ctx,Vec U,Vec a,PetscScalar *h)
{
  MatSNESMFDefault *hctx = (MatSNESMFDefault*)ctx->hctx;
  PetscReal        norm,sum,umin = hctx->umin;
  PetscScalar      dot;
  int              ierr;

  PetscFunctionBegin;
  if (!(ctx->count % ctx->recomputeperiod)) {
    /*
     This algorithm requires 2 norms and 1 inner product. Rather than
     use directly the VecNorm() and VecDot() routines (and thus have 
     three separate collective operations, we use the VecxxxBegin/End() routines
    */
    ierr = VecDotBegin(U,a,&dot);CHKERRQ(ierr);
    ierr = VecNormBegin(a,NORM_1,&sum);CHKERRQ(ierr);
    ierr = VecNormBegin(a,NORM_2,&norm);CHKERRQ(ierr);
    ierr = VecDotEnd(U,a,&dot);CHKERRQ(ierr);
    ierr = VecNormEnd(a,NORM_1,&sum);CHKERRQ(ierr);
    ierr = VecNormEnd(a,NORM_2,&norm);CHKERRQ(ierr);

    /* 
      Safeguard for step sizes that are "too small"
    */
    if (!sum) {dot = 1.0; norm = 1.0;}
#if defined(PETSC_USE_COMPLEX)
    else if (PetscAbsScalar(dot) < umin*sum && PetscRealPart(dot) >= 0.0) dot = umin*sum;
    else if (PetscAbsScalar(dot) < 0.0 && PetscRealPart(dot) > -umin*sum) dot = -umin*sum;
#else
    else if (dot < umin*sum && dot >= 0.0) dot = umin*sum;
    else if (dot < 0.0 && dot > -umin*sum) dot = -umin*sum;
#endif
    *h = ctx->error_rel*dot/(norm*norm);
  } else {
    *h = ctx->currenth;
  }
  if (*h != *h) SETERRQ3(1,"Differencing parameter is not a number sum = %g dot = %g norm = %g",sum,PetscRealPart(dot),norm);
  ctx->count++;
  PetscFunctionReturn(0);
} 

#undef __FUNCT__  
#define __FUNCT__ "MatSNESMFView_Default"
/*
   MatSNESMFView_Default - Prints information about this particular 
   method for computing h. Note that this does not print the general
   information about the matrix-free method, as such info is printed
   by the calling routine.

   Input Parameters:
+  ctx - the matrix free context
-  viewer - the PETSc viewer
*/   
static int MatSNESMFView_Default(MatSNESMFCtx ctx,PetscViewer viewer)
{
  MatSNESMFDefault *hctx = (MatSNESMFDefault *)ctx->hctx;
  int              ierr;
  PetscTruth       isascii;

  PetscFunctionBegin;
  /*
     Currently this only handles the ascii file viewers, others
     could be added, but for this type of object other viewers
     make less sense
  */
  ierr = PetscTypeCompare((PetscObject)viewer,PETSC_VIEWER_ASCII,&isascii);CHKERRQ(ierr);
  if (isascii) {
    ierr = PetscViewerASCIIPrintf(viewer,"    umin=%g (minimum iterate parameter)\n",hctx->umin);CHKERRQ(ierr); 
  } else {
    SETERRQ1(1,"Viewer type %s not supported for this SNES matrix free matrix",((PetscObject)viewer)->type_name);
  }    
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatSNESMFSetFromOptions_Default"
/*
   MatSNESMFSetFromOptions_Default - Looks in the options database for 
   any options appropriate for this method.

   Input Parameter:
.  ctx - the matrix free context

*/
static int MatSNESMFSetFromOptions_Default(MatSNESMFCtx ctx)
{
  int              ierr;
  MatSNESMFDefault *hctx = (MatSNESMFDefault*)ctx->hctx;

  PetscFunctionBegin;
  ierr = PetscOptionsHead("Default matrix free parameters");CHKERRQ(ierr);
    ierr = PetscOptionsReal("-snes_mf_umin","umin","MatSNESMFDefaultSetUmin",hctx->umin,&hctx->umin,0);CHKERRQ(ierr);
  ierr = PetscOptionsTail();CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatSNESMFDestroy_Default"
/*
   MatSNESMFDestroy_Default - Frees the space allocated by 
   MatSNESMFCreate_Default(). 

   Input Parameter:
.  ctx - the matrix free context

   Notes: 
   Does not free the ctx, that is handled by the calling routine
*/
static int MatSNESMFDestroy_Default(MatSNESMFCtx ctx)
{
  int ierr;

  PetscFunctionBegin;
  ierr = PetscFree(ctx->hctx);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "MatSNESMFDefaultSetUmin_Private"
/*
   The following two routines use the PetscObjectCompose() and PetscObjectQuery()
   mechanism to allow the user to change the Umin parameter used in this method.
*/
int MatSNESMFDefaultSetUmin_Private(Mat mat,PetscReal umin)
{
  MatSNESMFCtx     ctx = (MatSNESMFCtx)mat->data;
  MatSNESMFDefault *hctx;

  PetscFunctionBegin;
  if (!ctx) {
    SETERRQ(1,"MatSNESMFDefaultSetUmin() attached to non-shell matrix");
  }
  hctx = (MatSNESMFDefault*)ctx->hctx;
  hctx->umin = umin;
  PetscFunctionReturn(0);
} 
EXTERN_C_END

#undef __FUNCT__  
#define __FUNCT__ "MatSNESMFDefaultSetUmin"
/*@
    MatSNESMFDefaultSetUmin - Sets the "umin" parameter used by the default
    PETSc routine for computing the differencing parameter, h, which is used
    for matrix-free Jacobian-vector products.

   Input Parameters:
+  A - the matrix created with MatCreateSNESMF()
-  umin - the parameter

   Level: advanced

   Notes:
   See the manual page for MatCreateSNESMF() for a complete description of the
   algorithm used to compute h.

.seealso: MatSNESMFSetFunctionError(), MatCreateSNESMF()

@*/
int MatSNESMFDefaultSetUmin(Mat A,PetscReal umin)
{
  int ierr,(*f)(Mat,PetscReal);

  PetscFunctionBegin;
  PetscValidHeaderSpecific(A,MAT_COOKIE);
  ierr = PetscObjectQueryFunction((PetscObject)A,"MatSNESMFDefaultSetUmin_C",(void (**)())&f);CHKERRQ(ierr);
  if (f) {
    ierr = (*f)(A,umin);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "MatSNESMFCreate_Default"
/*
   MatSNESMFCreate_Default - Standard PETSc code for 
   computing h with matrix-free finite differences.

   Input Parameter:
.  ctx - the matrix free context created by MatSNESMFCreate()

*/
int MatSNESMFCreate_Default(MatSNESMFCtx ctx)
{
  MatSNESMFDefault *hctx;
  int              ierr;

  PetscFunctionBegin;

  /* allocate my own private data structure */
  ierr       = PetscNew(MatSNESMFDefault,&hctx);CHKERRQ(ierr);
  ctx->hctx  = (void*)hctx;
  /* set a default for my parameter */
  hctx->umin = 1.e-6;

  /* set the functions I am providing */
  ctx->ops->compute        = MatSNESMFCompute_Default;
  ctx->ops->destroy        = MatSNESMFDestroy_Default;
  ctx->ops->view           = MatSNESMFView_Default;  
  ctx->ops->setfromoptions = MatSNESMFSetFromOptions_Default;  

  ierr = PetscObjectComposeFunctionDynamic((PetscObject)ctx->mat,"MatSNESMFDefaultSetUmin_C",
                            "MatSNESMFDefaultSetUmin_Private",
                             MatSNESMFDefaultSetUmin_Private);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END







