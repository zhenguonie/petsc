/* $Id: tr.h,v 1.9 2001/08/07 21:31:09 bsmith Exp $ */

/*
   Context for a Newton trust region method for solving a system 
   of nonlinear equations
 */

#ifndef __SNES_EQTR_H
#define __SNES_EQTR_H
#include "src/snes/snesimpl.h"

typedef struct {
  /* ---- Parameters used by the trust region method  ---- */
  PetscReal mu;		/* used to compute trust region parameter */
  PetscReal eta;		/* used to compute trust region parameter */
  PetscReal delta;		/* trust region parameter */
  PetscReal delta0;	/* used to initialize trust region parameter */
  PetscReal delta1;	/* used to compute trust region parameter */
  PetscReal delta2;	/* used to compute trust region parameter */
  PetscReal delta3;	/* used to compute trust region parameter */
  PetscReal sigma;		/* used to detemine termination */
  int       itflag;	/* flag for convergence testing */
  PetscReal rnorm0,ttol;   /* used for KSP convergence test */
} SNES_EQ_TR;

#endif
