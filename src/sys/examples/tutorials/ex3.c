/*$Id: ex3.c,v 1.39 2001/03/23 23:21:03 balay Exp $*/

static char help[] = "Augmenting PETSc profiling by add events.\n\
Run this program with one of the\n\
following options to generate logging information:  -log, -log_summary,\n\
-log_all.  The PETSc routines automatically log event times and flops,\n\
so this monitoring is intended solely for users to employ in application\n\
codes.  Note that the code must be compiled with the flag -DPETSC_USE_LOG\n\
(the default) to activate logging.\n\n";

/*T
   Concepts: PetscLog^user-defined event profiling
   Concepts: profiling^user-defined event
   Concepts: PetscLog^activating/deactivating events for profiling
   Concepts: profiling^activating/deactivating events
   Processors: n
T*/

/* 
  Include "petsc.h" so that we can use PETSc profiling routines.
*/
#include "petsc.h"

#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc,char **argv)
{
  int i,ierr,imax=10000,icount,USER_EVENT;

  PetscInitialize(&argc,&argv,(char *)0,help);

  /* 
     Create a new user-defined event.
      - Note that PetscLogEventRegister() returns to the user a unique
        integer event number, which should then be used for profiling
        the event via PetscLogEventBegin() and PetscLogEventEnd().
      - The user can also optionally log floating point operations
        with the routine PetscLogFlops().
  */
  ierr = PetscLogEventRegister(&USER_EVENT,"User event","Red:");CHKERRQ(ierr);
  ierr = PetscLogEventBegin(USER_EVENT,0,0,0,0);CHKERRQ(ierr);
  icount = 0;
  for (i=0; i<imax; i++) icount++;
  ierr = PetscLogFlops(imax);CHKERRQ(ierr);
  ierr = PetscSleep(1);CHKERRQ(ierr);
  ierr = PetscLogEventEnd(USER_EVENT,0,0,0,0);CHKERRQ(ierr);

  /* 
     We disable the logging of an event.
      - Note that activation/deactivation of PETSc events and MPE 
        events is handled separately.
      - Note that the user can activate/deactive both user-defined
        events and predefined PETSc events.
  */
  ierr = PetscLogEventMPEDeactivate(USER_EVENT);CHKERRQ(ierr);
  ierr = PetscLogEventDeactivate(USER_EVENT);CHKERRQ(ierr);
  ierr = PetscLogEventBegin(USER_EVENT,0,0,0,0);CHKERRQ(ierr);
  ierr = PetscSleep(1);CHKERRQ(ierr);
  ierr = PetscLogEventEnd(USER_EVENT,0,0,0,0);CHKERRQ(ierr);

  /* 
     We next enable the logging of an event
  */
  ierr = PetscLogEventMPEActivate(USER_EVENT);CHKERRQ(ierr);
  ierr = PetscLogEventActivate(USER_EVENT);CHKERRQ(ierr);
  ierr = PetscLogEventBegin(USER_EVENT,0,0,0,0);CHKERRQ(ierr);
  ierr = PetscSleep(1);CHKERRQ(ierr);
  ierr = PetscLogEventEnd(USER_EVENT,0,0,0,0);CHKERRQ(ierr);

  ierr = PetscFinalize();CHKERRQ(ierr);
  return 0;
}
 
