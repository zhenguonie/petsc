/*$Id: viewregall.c,v 1.19 2001/04/10 19:34:10 bsmith Exp $*/

#include "src/sys/src/viewer/viewerimpl.h"  /*I "petsc.h" I*/  

EXTERN_C_BEGIN
EXTERN int PetscViewerCreate_Socket(PetscViewer);
EXTERN int PetscViewerCreate_ASCII(PetscViewer);
EXTERN int PetscViewerCreate_Binary(PetscViewer);
EXTERN int PetscViewerCreate_String(PetscViewer);
EXTERN int PetscViewerCreate_Draw(PetscViewer);
EXTERN int PetscViewerCreate_AMS(PetscViewer);
EXTERN_C_END
  
#undef __FUNCT__  
#define __FUNCT__ "PetscViewerRegisterAll" 
/*@C
  PetscViewerRegisterAll - Registers all of the graphics methods in the PetscViewer package.

  Not Collective

   Level: developer

.seealso:  PetscViewerRegisterDestroy()
@*/
int PetscViewerRegisterAll(char *path)
{
  int ierr;

  PetscFunctionBegin;
  
  ierr = PetscViewerRegisterDynamic(PETSC_VIEWER_ASCII,    path,"PetscViewerCreate_ASCII",      PetscViewerCreate_ASCII);CHKERRQ(ierr);
  ierr = PetscViewerRegisterDynamic(PETSC_VIEWER_BINARY,   path,"PetscViewerCreate_Binary",     PetscViewerCreate_Binary);CHKERRQ(ierr);
  ierr = PetscViewerRegisterDynamic(PETSC_VIEWER_STRING,   path,"PetscViewerCreate_String",     PetscViewerCreate_String);CHKERRQ(ierr);
  ierr = PetscViewerRegisterDynamic(PETSC_VIEWER_DRAW,     path,"PetscViewerCreate_Draw",       PetscViewerCreate_Draw);CHKERRQ(ierr);
  ierr = PetscViewerRegisterDynamic(PETSC_VIEWER_SOCKET,   path,"PetscViewerCreate_Socket",     PetscViewerCreate_Socket);CHKERRQ(ierr);
#if defined(PETSC_HAVE_AMS)
  ierr = PetscViewerRegisterDynamic(PETSC_VIEWER_AMS,      path,"PetscViewerCreate_AMS",        PetscViewerCreate_AMS);CHKERRQ(ierr); 
#endif
  PetscFunctionReturn(0);
}

