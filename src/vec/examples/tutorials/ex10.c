/*$Id: ex10.c,v 1.19 2001/08/07 03:02:34 balay Exp $*/

/* Program usage:  mpirun ex1 [-help] [all PETSc options] */

static char help[] = "Demonstrates the AMS Memory Snooper viewing.\n\n";

/*T
   Concepts: vectors^basic routines;
   Processors: n
T*/

/* 
  Include "petscvec.h" so that we can use vectors.  Note that this file
  automatically includes:
     petsc.h       - base PETSc routines   petscis.h     - index sets
     petscsys.h    - system routines       petscviewer.h - viewers
*/

#include "petscvec.h"

#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc,char **argv)
{
  Vec      x,y;
  int      n = 20,ierr,i,row;
  PetscScalar   value;

  ierr = PetscInitialize(&argc,&argv,(char*)0,help);CHKERRQ(ierr); 
  ierr = PetscOptionsGetInt(PETSC_NULL,"-n",&n,PETSC_NULL);CHKERRQ(ierr);

  /* 
     Create a vector, specifying only its global dimension.
     When using VecCreate() and VecSetFromOptions(), the vector format (currently parallel,
     shared, or sequential) is determined at runtime.  Also, the parallel
     partitioning of the vector is determined by PETSc at runtime.

     Routines for creating particular vector types directly are:
        VecCreateSeq() - uniprocessor vector
        VecCreateMPI() - distributed vector, where the user can
                         determine the parallel partitioning
        VecCreateShared() - parallel vector that uses shared memory
                            (available only on the SGI); otherwise,
                            is the same as VecCreateMPI()

     With VecCreate() and VecSetFromOptions() the option -vec_type mpi or -vec_type shared causes the 
     particular type of vector to be formed.

  */
  ierr = VecCreate(PETSC_COMM_WORLD,PETSC_DECIDE,n,&x);CHKERRQ(ierr);
  ierr = VecSetFromOptions(x);CHKERRQ(ierr);

  /*
     Duplicate some work vector (of the same format and
     partitioning as the initial vector).
  */
  ierr = VecDuplicate(x,&y);CHKERRQ(ierr);

  ierr = PetscObjectPublish((PetscObject)x);CHKERRQ(ierr);

  for (i=0; i<1000; i++) {

    /*
       Set the vectors to entries to a constant value.
    */
    value = 1;
    row   = i % n;
    ierr = VecSetValues(x,1,&row,&value,ADD_VALUES);CHKERRQ(ierr);
    ierr = VecAssemblyBegin(x);CHKERRQ(ierr);
    ierr = VecAssemblyEnd(x);CHKERRQ(ierr);


    ierr = PetscSleep(5);
  }


  /* 
     Free work space.  All PETSc objects should be destroyed when they
     are no longer needed.
  */
  ierr = VecDestroy(x);CHKERRQ(ierr);
  ierr = VecDestroy(y);CHKERRQ(ierr);
  ierr = PetscFinalize();CHKERRQ(ierr);
  return 0;
}
 
