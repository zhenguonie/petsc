/*$Id: ex15.c,v 1.28 2001/08/07 21:30:50 bsmith Exp $*/

static char help[] = "SLES on an operator with a null space.\n\n";

#include "petscsles.h"

#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc,char **args)
{
  Vec          x,b,u;      /* approx solution, RHS, exact solution */
  Mat          A;            /* linear system matrix */
  SLES         sles;         /* SLES context */
  int          ierr,i,n = 10,col[3],its,i1,i2;
  PetscScalar  none = -1.0,value[3],avalue;
  PetscReal    norm;
  PC           pc;

  PetscInitialize(&argc,&args,(char *)0,help);
  ierr = PetscOptionsGetInt(PETSC_NULL,"-n",&n,PETSC_NULL);CHKERRQ(ierr);

  /* Create vectors */
  ierr = VecCreate(PETSC_COMM_WORLD,PETSC_DECIDE,n,&x);CHKERRQ(ierr);
  ierr = VecSetFromOptions(x);CHKERRQ(ierr);
  ierr = VecDuplicate(x,&b);CHKERRQ(ierr);
  ierr = VecDuplicate(x,&u);CHKERRQ(ierr);

  /* create a solution that is orthogonal to the constants */
  ierr = VecGetOwnershipRange(u,&i1,&i2);CHKERRQ(ierr);
  for (i=i1; i<i2; i++) {
    avalue = i;
    VecSetValues(u,1,&i,&avalue,INSERT_VALUES);
  }
  ierr = VecAssemblyBegin(u);CHKERRQ(ierr);
  ierr = VecAssemblyEnd(u);CHKERRQ(ierr);
  ierr = VecSum(u,&avalue);CHKERRQ(ierr);
  avalue = -avalue/(PetscReal)n;
  ierr = VecShift(&avalue,u);CHKERRQ(ierr);

  /* Create and assemble matrix */
  ierr = MatCreate(PETSC_COMM_WORLD,PETSC_DECIDE,PETSC_DECIDE,n,n,&A);CHKERRQ(ierr);
  ierr = MatSetFromOptions(A);CHKERRQ(ierr);
  value[0] = -1.0; value[1] = 2.0; value[2] = -1.0;
  for (i=1; i<n-1; i++) {
    col[0] = i-1; col[1] = i; col[2] = i+1;
    ierr = MatSetValues(A,1,&i,3,col,value,INSERT_VALUES);CHKERRQ(ierr);
  }
  i = n - 1; col[0] = n - 2; col[1] = n - 1; value[1] = 1.0;
  ierr = MatSetValues(A,1,&i,2,col,value,INSERT_VALUES);CHKERRQ(ierr);
  i = 0; col[0] = 0; col[1] = 1; value[0] = 1.0; value[1] = -1.0;
  ierr = MatSetValues(A,1,&i,2,col,value,INSERT_VALUES);CHKERRQ(ierr);
  ierr = MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatMult(A,u,b);CHKERRQ(ierr);

  /* Create SLES context; set operators and options; solve linear system */
  ierr = SLESCreate(PETSC_COMM_WORLD,&sles);CHKERRQ(ierr);
  ierr = SLESSetOperators(sles,A,A,DIFFERENT_NONZERO_PATTERN);CHKERRQ(ierr);

  /* Insure that preconditioner has same null space as matrix */
  ierr = SLESGetPC(sles,&pc);CHKERRQ(ierr);

  ierr = SLESSetFromOptions(sles);CHKERRQ(ierr);
  ierr = SLESSolve(sles,b,x,&its);CHKERRQ(ierr);
  /* ierr = SLESView(sles,PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr); */

  /* Check error */
  ierr = VecAXPY(&none,u,x);CHKERRQ(ierr);
  ierr = VecNorm(x,NORM_2,&norm);CHKERRQ(ierr);
  ierr = PetscPrintf(PETSC_COMM_WORLD,"Norm of error %A, Iterations %d\n",norm,its);CHKERRQ(ierr);

  /* Free work space */
  ierr = VecDestroy(x);CHKERRQ(ierr);ierr = VecDestroy(u);CHKERRQ(ierr);
  ierr = VecDestroy(b);CHKERRQ(ierr);ierr = MatDestroy(A);CHKERRQ(ierr);
  ierr = SLESDestroy(sles);CHKERRQ(ierr);
  ierr = PetscFinalize();CHKERRQ(ierr);
  return 0;
}
