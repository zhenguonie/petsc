#ifndef lint
static char vcid[] = "$Id: sles.c,v 1.66 1996/08/08 14:44:27 bsmith Exp curfman $";
#endif

#include "src/sles/slesimpl.h"     /*I  "sles.h"    I*/
#include "pinclude/pviewer.h"

/*@ 
   SLESView - Prints the SLES data structure.

   Input Parameters:
.  SLES - the SLES context
.  viewer - optional visualization context

   Options Database Key:
$  -sles_view : calls SLESView() at end of SLESSolve()

   Note:
   The available visualization contexts include
$     VIEWER_STDOUT_SELF - standard output (default)
$     VIEWER_STDOUT_WORLD - synchronized standard
$       output where only the first processor opens
$       the file.  All other processors send their 
$       data to the first processor to print. 

   The user can open alternative vistualization contexts with
$    ViewerFileOpenASCII() - output to a specified file

.keywords: SLES, view

.seealso: ViewerFileOpenASCII()
@*/
int SLESView(SLES sles,Viewer viewer)
{
  KSP         ksp;
  PC          pc;
  int         ierr;

  SLESGetPC(sles,&pc);
  SLESGetKSP(sles,&ksp);
  ierr = KSPView(ksp,viewer); CHKERRQ(ierr);
  ierr = PCView(pc,viewer); CHKERRQ(ierr);
  return 0;
}

/*@
   SLESPrintHelp - Prints SLES options.

   Input Parameter:
.  sles - the SLES context

.keywords: SLES, help

.seealso: SLESSetFromOptions()
@*/
int SLESPrintHelp(SLES sles)
{
  char    *prefix = "-";
  if (sles->prefix) prefix = sles->prefix;
  PetscValidHeaderSpecific(sles,SLES_COOKIE);
  PetscPrintf(sles->comm,"SLES options:\n");
  PetscPrintf(sles->comm," %ssles_view: view SLES info after each linear solve\n",prefix);
  KSPPrintHelp(sles->ksp);
  PCPrintHelp(sles->pc);
  return 0;
}

/*@C
   SLESSetOptionsPrefix - Sets the prefix used for searching for all 
   SLES options in the database.

   Input Parameter:
.  sles - the SLES context
.  prefix - the prefix to prepend to all option names

   Notes:
   This prefix is particularly useful for nested use of SLES.  For
   example, the block Jacobi and block diagonal preconditioners use
   the prefix "sub" for options relating to the individual blocks.  

.keywords: SLES, set, options, prefix, database
@*/
int SLESSetOptionsPrefix(SLES sles,char *prefix)
{
  int ierr;
  PetscValidHeaderSpecific(sles,SLES_COOKIE);
  ierr = PetscObjectSetPrefix((PetscObject)sles, prefix); CHKERRQ(ierr);
  ierr = KSPSetOptionsPrefix(sles->ksp,prefix);CHKERRQ(ierr);
  ierr = PCSetOptionsPrefix(sles->pc,prefix); CHKERRQ(ierr);
  return 0;
}
/*@C
   SLESAppendOptionsPrefix - Appends to the prefix used for searching for all 
   SLES options in the database.

   Input Parameter:
.  sles - the SLES context
.  prefix - the prefix to prepend to all option names

   Notes:
   This prefix is particularly useful for nested use of SLES.  For
   example, the block Jacobi and block diagonal preconditioners use
   the prefix "sub" for options relating to the individual blocks.  

.keywords: SLES, append, options, prefix, database
@*/
int SLESAppendOptionsPrefix(SLES sles,char *prefix)
{
  int ierr;
  PetscValidHeaderSpecific(sles,SLES_COOKIE);
  ierr = PetscObjectAppendPrefix((PetscObject)sles, prefix); CHKERRQ(ierr);
  ierr = KSPAppendOptionsPrefix(sles->ksp,prefix);CHKERRQ(ierr);
  ierr = PCAppendOptionsPrefix(sles->pc,prefix); CHKERRQ(ierr);
  return 0;
}
/*@
   SLESGetOptionsPrefix - Gets the prefix used for searching for all 
   SLES options in the database.

   Input Parameter:
.  sles - the SLES context

   Output Parameter:
.  prefix - pointer to the prefix string used

   Notes:
   This prefix is particularly useful for nested use of SLES.  For
   example, the block Jacobi and block diagonal preconditioners use
   the prefix "sub" for options relating to the individual blocks.  

.keywords: SLES, get, options, prefix, database
@*/
int SLESGetOptionsPrefix(SLES sles,char **prefix)
{
  PetscValidHeaderSpecific(sles,SLES_COOKIE);
  return PetscObjectGetPrefix((PetscObject)sles, prefix); 
}

/*@
   SLESSetFromOptions - Sets various SLES parameters from user options.
   Also takes all KSP and PC options.

   Input Parameter:
.  sles - the SLES context

.keywords: SLES, set, options, database

.seealso: SLESPrintHelp()
@*/
int SLESSetFromOptions(SLES sles)
{
  int ierr;

  PetscValidHeaderSpecific(sles,SLES_COOKIE);
  ierr = KSPSetFromOptions(sles->ksp); CHKERRQ(ierr);
  ierr = PCSetFromOptions(sles->pc); CHKERRQ(ierr);
  return 0;
}
/*@C
   SLESCreate - Creates a linear equation solver context.

   Input Parameter:
.  comm - MPI communicator

   Output Parameter:
.  sles - the newly created SLES context

.keywords: SLES, create, context

.seealso: SLESSolve(), SLESDestroy()
@*/
int SLESCreate(MPI_Comm comm,SLES *outsles)
{
  int ierr;
  SLES sles;
  *outsles = 0;
  PetscHeaderCreate(sles,_SLES,SLES_COOKIE,0,comm);
  PLogObjectCreate(sles);
  ierr = KSPCreate(comm,&sles->ksp); CHKERRQ(ierr);
  ierr = PCCreate(comm,&sles->pc); CHKERRQ(ierr);
  PLogObjectParent(sles,sles->ksp);
  PLogObjectParent(sles,sles->pc);
  sles->setupcalled = 0;
  *outsles = sles;
  return 0;
}

/*@C
   SLESDestroy - Destroys the SLES context.

   Input Parameters:
.  sles - the SLES context

.keywords: SLES, destroy, context

.seealso: SLESCreate(), SLESSolve()
@*/
int SLESDestroy(SLES sles)
{
  int ierr;
  PetscValidHeaderSpecific(sles,SLES_COOKIE);
  ierr = KSPDestroy(sles->ksp); CHKERRQ(ierr);
  ierr = PCDestroy(sles->pc); CHKERRQ(ierr);
  PLogObjectDestroy(sles);
  PetscHeaderDestroy(sles);
  return 0;
}
extern int PCPreSolve(PC,KSP),PCPostSolve(PC,KSP);
/*@
   SLESSetUp - Performs set up required for solving a linear system.

   Input Parameters:
.  sles - the SLES context
.  b - the right hand side

   Output Parameters:
.  x - the approximate solution

   Note:
   For basic use of the SLES solvers the user need not explicitly call
   SLESSetUp(), since these actions will automatically occur during
   the call to SLESSolve().  However, if one wishes to generate
   performance data for this computational phase (for example, for
   incomplete factorization using the ILU preconditioner) using the 
   PETSc log facilities, calling SLESSetUp() is required.

.keywords: SLES, solve, linear system

.seealso: SLESCreate(), SLESDestroy(), SLESDestroy()
@*/
int SLESSetUp(SLES sles,Vec b,Vec x)
{
  int ierr;
  KSP ksp;
  PC  pc;

  PetscValidHeaderSpecific(sles,SLES_COOKIE);
  PetscValidHeaderSpecific(b,VEC_COOKIE);
  PetscValidHeaderSpecific(x,VEC_COOKIE);
  ksp = sles->ksp; pc = sles->pc;
  KSPSetRhs(ksp,b);
  KSPSetSolution(ksp,x);
  KSPSetPC(ksp,pc);
  if (!sles->setupcalled) {
    PLogEventBegin(SLES_SetUp,sles,b,x,0);
    ierr = PCSetVector(pc,b); CHKERRQ(ierr);
    ierr = KSPSetUp(sles->ksp); CHKERRQ(ierr);
    ierr = PCSetUp(sles->pc); CHKERRQ(ierr);
    sles->setupcalled = 1;
    PLogEventEnd(SLES_SetUp,sles,b,x,0);
  }
  return 0;
}
/*@
   SLESSolve - Solves a linear system.

   Input Parameters:
.  sles - the SLES context
.  b - the right hand side

   Output Parameters:
.  x - the approximate solution
.  its - the number of iterations until termination

   Notes:
   On return, the parameter "its" contains
$   - the iteration number at which convergence
$     was successfully reached, 
$   - or the negative of the iteration at which
$      divergence or breakdown was detected.

   If using a direct method (e.g., via the KSP solver
   KSPPREONLY and a preconditioner such as PCLU/PCILU),
   then its=1.  See KSPSetTolerances() and KSPDefaultConverged()
   for more details.
   
.keywords: SLES, solve, linear system

.seealso: SLESCreate(), SLESDestroy(), KSPSetTolerances(), KSPDefaultConverged()
@*/
int SLESSolve(SLES sles,Vec b,Vec x,int *its)
{
  int ierr, flg;
  KSP ksp;
  PC  pc;

  PetscValidHeaderSpecific(sles,SLES_COOKIE);
  if (b == x) SETERRQ(1,"SLESSolve:b and x must be different vectors");
  ksp = sles->ksp; pc = sles->pc;
  KSPSetRhs(ksp,b);
  KSPSetSolution(ksp,x);
  KSPSetPC(ksp,pc);
  if (!sles->setupcalled) {
    ierr = SLESSetUp(sles,b,x); CHKERRQ(ierr);
  }
  ierr = PCSetUpOnBlocks(pc); CHKERRQ(ierr);
  PLogEventBegin(SLES_Solve,sles,b,x,0);
  ierr = PCPreSolve(pc,ksp); CHKERRQ(ierr);
  ierr = KSPSolve(ksp,its); CHKERRQ(ierr);
  ierr = PCPostSolve(pc,ksp); CHKERRQ(ierr);
  PLogEventEnd(SLES_Solve,sles,b,x,0);
  ierr = OptionsHasName(sles->prefix,"-sles_view", &flg); CHKERRQ(ierr); 
  if (flg) { ierr = SLESView(sles,VIEWER_STDOUT_WORLD); CHKERRQ(ierr); }
  return 0;
}

/*@C
   SLESGetKSP - Returns the KSP context for a SLES solver.

   Input Parameter:
.  sles - the SLES context

   Output Parameter:
.  ksp - the Krylov space context

   Notes:  
   The user can then directly manipulate the KSP context to set various 
   options, etc.
   
.keywords: SLES, get, KSP, context

.seealso: SLESGetPC()
@*/
int SLESGetKSP(SLES sles,KSP *ksp)
{
  PetscValidHeaderSpecific(sles,SLES_COOKIE);
  *ksp = sles->ksp;
  return 0;
}
/*@C
   SLESGetPC - Returns the preconditioner (PC) context for a SLES solver.

   Input Parameter:
.  sles - the SLES context

   Output Parameter:
.  pc - the preconditioner context

   Notes:  
   The user can then directly manipulate the PC context to set various 
   options, etc.

.keywords: SLES, get, PC, context

.seealso: SLESGetKSP()
@*/
int SLESGetPC(SLES sles,PC *pc)
{
  PetscValidHeaderSpecific(sles,SLES_COOKIE);
  *pc = sles->pc;
  return 0;
}

#include "src/mat/matimpl.h"
/*@
   SLESSetOperators - Sets the matrix associated with the linear system
   and a (possibly) different one associated with the preconditioner. 

   Input Parameters:
.  sles - the sles context
.  Amat - the matrix associated with the linear system
.  Pmat - matrix to be used in constructing preconditioner, usually the same
          as Amat. 
.  flag - flag indicating information about the preconditioner matrix structure
   during successive linear solves. When solving just one linear system, this
   flag is ignored.

   Notes: 
   The flag can be used to eliminate unnecessary work in the preconditioner 
   during the repeated solution of linear systems of the same size.  The
   available options are
$    SAME_PRECONDITIONER -
$      Pmat is identical during successive linear solves.
$      This option is intended for folks who are using
$      different Amat and Pmat matrices and want to reuse the
$      same preconditioner matrix.  For example, this option
$      saves work by not recomputing incomplete factorization
$      for ILU/ICC preconditioners.
$    SAME_NONZERO_PATTERN -
$      Pmat has the same nonzero structure during
$      successive linear solves. 
$    DIFFERENT_NONZERO_PATTERN -
$      Pmat does not have the same nonzero structure.

    If in doubt about whether your preconditioner matrix has changed
    structure or not, use the flag DIFFERENT_NONZERO_PATTERN.

.keywords: SLES, set, operators, matrix, preconditioner, linear system

.seealso: SLESSolve()
@*/
int SLESSetOperators(SLES sles,Mat Amat,Mat Pmat,MatStructure flag)
{
  PetscValidHeaderSpecific(sles,SLES_COOKIE);
  PetscValidHeaderSpecific(Amat,MAT_COOKIE);
  PetscValidHeaderSpecific(Pmat,MAT_COOKIE);
  PCSetOperators(sles->pc,Amat,Pmat,flag);
  sles->setupcalled = 0;  /* so that next solve call will call setup */
  return 0;
}

/*@
   SLESSetUpOnBlocks - Sets up the preconditioner for each block in
   the block Jacobi, block Gauss-Seidel, and overlapping Schwarz 
   methods.

   Input parameters:
.  pc - the preconditioner context

   Notes:
   SLESSetUpOnBlocks() is a routine that the user can optinally call for
   more precise profiling (via -log_summary) of the setup phase for these
   block preconditioners.  If the user does not call SLESSetUpOnBlocks(),
   it will automatically be called from within SLESSolve().
   
   Calling SLESSetUpOnBlocks() is the same as calling PCSetUpOnBlocks()
   on the PC context within the SLES context.

.keywords: SLES, setup, blocks

.seealso: PCSetUpOnBlocks(), SLESSetUp(), PCSetUp()
@*/
int SLESSetUpOnBlocks(SLES sles)
{
  int ierr;
  PC  pc;

  ierr = SLESGetPC(sles,&pc); CHKERRQ(ierr);
  ierr = PCSetUpOnBlocks(pc); CHKERRQ(ierr);
  return 0;
}
