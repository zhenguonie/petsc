/*$Id: dcoor.c,v 1.28 2001/03/23 23:20:08 balay Exp $*/
/*
       Provides the calling sequences for all the basic PetscDraw routines.
*/
#include "src/sys/src/draw/drawimpl.h"  /*I "petscdraw.h" I*/

#undef __FUNCT__  
#define __FUNCT__ "PetscDrawSetCoordinates" 
/*@
   PetscDrawSetCoordinates - Sets the application coordinates of the corners of
   the window (or page).

   Not collective

   Input Parameters:
+  draw - the drawing object
-  xl,yl,xr,yr - the coordinates of the lower left corner and upper
                 right corner of the drawing region.

   Level: advanced

   Concepts: drawing^coordinates
   Concepts: graphics^coordinates

.seealso: PetscDrawGetCoordinates()

@*/
int PetscDrawSetCoordinates(PetscDraw draw,PetscReal xl,PetscReal yl,PetscReal xr,PetscReal yr)
{
  int ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(draw,PETSC_DRAW_COOKIE);
  draw->coor_xl = xl; draw->coor_yl = yl;
  draw->coor_xr = xr; draw->coor_yr = yr;
  if (draw->ops->setcoordinates) {
    ierr = (*draw->ops->setcoordinates)(draw,xl,yl,xr,yr);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

