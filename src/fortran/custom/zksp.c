#ifndef lint
static char vcid[] = "$Id: zoptions.c,v 1.1 1995/08/21 19:56:20 bsmith Exp bsmith $";
#endif

#include "zpetsc.h"
#include "draw.h"
#include "ksp.h"
#if defined(HAVE_STRING_H)
#include <string.h>
#endif
#include "pinclude/petscfix.h"

#ifdef FORTRANCAPS
#define kspregisterdestroy_      KSPREGISTERDESTROY
#define kspregisterall_          KSPREGISTERALL
#define kspgetmethodfromcontext_ KSPGETMETHODFROMCONTEXT
#define kspdestroy_              KSPDESTROY
#define ksplgmonitordestroy_     KSPLGMONITORDESTROY
#define ksplgmonitorcreate_      KSPLGMONITORCREATE
#define kspgetrhs_               KSPGETRHS
#define kspgetsolution_          KSPGETSOLUTION
#define kspgetbinv_              KSPGETBINV
#define kspsetmonitor_           KSPSETMONITOR
#define kspsetconvergencetest_   KSPSETCONVERGENCETEST
#define kspcreate_               KSPCREATE
#define kspsetoptionsprefix_     KSPSETOPTIONSPREFIX
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspregisterdestroy_      kspregisterdestroy
#define kspregisterall_          kspregisterall
#define kspgetmethodfromcontext_ kspgetmethodfromcontext
#define kspdestroy_              kspdestroy
#define ksplgmonitordestroy_     ksplgmonitordestroy
#define ksplgmonitorcreate_      ksplgmonitorcreate
#define kspgetrhs_               kspgetrhs
#define kspgetsolution_          kspgetsolution
#define kspgetbinv_              kspgetbinv
#define kspsetmonitor_           kspsetmonitor
#define kspsetconvergencetest_   kspsetconvergencetest
#define kspcreate_               kspcreate
#define kspsetoptionsprefix_     kspsetoptionsprefix
#endif

void kspsetoptionsprefix_(KSP ksp,char *prefix, int *__ierr,int len ){
  char *t;
  if (prefix[len] != 0) {
    t = (char *) PETSCMALLOC( (len+1)*sizeof(char) ); 
    strncpy(t,prefix,len);
    t[len] = 0;
  }
  else t = prefix;
  *__ierr = KSPSetOptionsPrefix((KSP)MPIR_ToPointer( *(int*)(ksp) ),t);
}

void kspcreate_(MPI_Comm comm,KSP *ksp, int *__ierr ){
  KSP tmp;
  *__ierr = KSPCreate((MPI_Comm)MPIR_ToPointer( *(int*)(comm) ),&tmp);
  *(int*)ksp =  MPIR_FromPointer(tmp);
}

static int (*f2)(int*,int*,double*,void*,int*);
static int ourtest(KSP ksp,int i,double d,void* ctx)
{
  int s1, ierr = 0;
  s1 = MPIR_FromPointer(ksp);
  (*f2)(&s1,&i,&d,ctx,&ierr); CHKERRQ(ierr);
  MPIR_RmPointer(s1);
  return 0;
}
void kspsetconvergencetest_(KSP itP,
      int (*converge)(int*,int*,double*,void*,int*),void *cctx, int *__ierr){
  f2 = converge;
  *__ierr = KSPSetConvergenceTest(
	(KSP)MPIR_ToPointer( *(int*)(itP) ),ourtest,cctx);
}

static int (*f1)(int*,int*,double*,void*,int*);
static int ourmonitor(KSP ksp,int i,double d,void* ctx)
{
  int s1, ierr = 0;
  s1 = MPIR_FromPointer(ksp);
  (*f1)(&s1,&i,&d,ctx,&ierr); CHKERRQ(ierr);
  MPIR_RmPointer(s1);
  return 0;
}
void kspsetmonitor_(KSP itP,int (*monitor)(int*,int*,double*,void*,int*),
                    void *mctx, int *__ierr ){
  f1 = monitor;
  *__ierr = KSPSetMonitor((KSP)MPIR_ToPointer(*(int*)(itP)),ourmonitor,mctx);
}

void kspgetbinv_(KSP itP,PC *B, int *__ierr ){
  PC pc;
  *__ierr = KSPGetBinv((KSP)MPIR_ToPointer( *(int*)(itP) ),&pc);
  *(int*) B = MPIR_FromPointer(pc);
}

void kspgetsolution_(KSP itP,Vec *v, int *__ierr ){
  Vec vv;
  *__ierr = KSPGetSolution((KSP)MPIR_ToPointer( *(int*)(itP) ),&vv);
  *(int*) v =  MPIR_FromPointer(vv);
}
void kspgetrhs_(KSP itP,Vec *r, int *__ierr ){
  Vec vv;
  *__ierr = KSPGetRhs((KSP)MPIR_ToPointer( *(int*)(itP) ),&vv);
  *(int*) r =  MPIR_FromPointer(vv);
}

/*
   Possible bleeds memory but cannot be helped.
*/
void ksplgmonitorcreate_(char *host,char *label,int *x,int *y,int *m,
                       int *n,DrawLGCtx *ctx, int *__ierr,int len1,int len2){
  char *t1,*t2;
  DrawLGCtx lg;
  if (host[len1] != 0) {
    t1 = (char *) PETSCMALLOC( (len1+1)*sizeof(char) ); 
    strncpy(t1,host,len1);
    t1[len1] = 0;
  }
  else t1 = host;
  if (label[len2] != 0) {
    t2 = (char *) PETSCMALLOC( (len2+1)*sizeof(char) ); 
    strncpy(t2,label,len2);
    t2[len2] = 0;
  }
  else t2 = label;
  *__ierr = KSPLGMonitorCreate(t1,t2,*x,*y,*m,*n,&lg);
  *(int*) ctx = MPIR_FromPointer(lg);
}
void ksplgmonitordestroy_(DrawLGCtx ctx, int *__ierr ){
  *__ierr = KSPLGMonitorDestroy((DrawLGCtx)MPIR_ToPointer( *(int*)(ctx) ));
  MPIR_RmPointer(*(int*)(ctx) );
}


void kspdestroy_(KSP itP, int *__ierr ){
  *__ierr = KSPDestroy((KSP)MPIR_ToPointer( *(int*)(itP) ));
  MPIR_RmPointer(*(int*)(itP) );
}

void kspgetmethodfromcontext_(KSP itP,KSPMethod *method, int *__ierr ){
  *__ierr = KSPGetMethodFromContext((KSP)MPIR_ToPointer(*(int*)(itP)),method);
}

void kspregisterdestroy_(int* MPIR_ierr)
{
  *MPIR_ierr = KSPRegisterDestroy();
}


void kspregisterall_(int* MPIR_ierr)
{
  *MPIR_ierr = KSPRegisterAll();
}
