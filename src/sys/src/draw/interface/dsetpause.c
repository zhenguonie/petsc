#ifdef PETSC_RCS_HEADER
static char vcid[] = "$Id: dsetpause.c,v 1.13 1998/04/13 17:46:34 bsmith Exp curfman $";
#endif
/*
       Provides the calling sequences for all the basic Draw routines.
*/
#include "src/draw/drawimpl.h"  /*I "draw.h" I*/

#undef __FUNC__  
#define __FUNC__ "DrawSetPause" 
/*@
   DrawSetPause - Sets the amount of time that program pauses after 
   a DrawPause() is called. 

   Collective on Draw

   Input Parameters:
+  draw - the drawing object
-  pause - number of seconds to pause, -1 implies until user input

   Note:
   By default the pause time is zero unless the -draw_pause option is given 
   during DrawOpenX().

.keywords: draw, set, pause

.seealso: DrawGetPause(), DrawPause()
@*/
int DrawSetPause(Draw draw,int pause)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(draw,DRAW_COOKIE);
  if (draw->type == DRAW_NULLWINDOW) PetscFunctionReturn(0);
  draw->pause = pause;
  PetscFunctionReturn(0);
}
