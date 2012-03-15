#ifndef _SNES_FASIMPLS
#define _SNES_FASIMPLS

#include <private/snesimpl.h>
#include <private/linesearchimpl.h>
#include <private/dmimpl.h>
#include <petscsnesfas.h>

typedef struct {

  /* flags for knowing the global place of this FAS object */
  PetscInt       level;                        /* level = 0 coarsest level */
  PetscInt       levels;                       /* if level + 1 = levels; we're the last turtle */

  PetscViewer    monitor;                      /* debuggging output for FAS */

  /* smoothing objects */
  SNES           upsmooth;                     /* the SNES for presmoothing */
  SNES           downsmooth;                   /* the SNES for postsmoothing */

  PetscLineSearch     linesearch_smooth;            /* the line search for default upsmoothing */

  /* coarse grid correction objects */
  SNES           next;                         /* the SNES instance for the next coarser level in the hierarchy */
  SNES           previous;                     /* the SNES instance for the next finer level in the hierarchy */
  Mat            interpolate;                  /* interpolation */
  Mat            inject;                       /* injection operator (unscaled) */
  Mat            restrct;                      /* restriction operator */
  Vec            rscale;                       /* the pointwise scaling of the restriction operator */

  /* method parameters */
  PetscInt       n_cycles;                     /* number of cycles on this level */
  SNESFASType    fastype;                      /* FAS type */
  PetscInt       max_up_it;                    /* number of pre-smooths */
  PetscInt       max_down_it;                  /* number of post-smooth cycles */
  PetscBool      usedmfornumberoflevels;       /* uses a DM to generate a number of the levels */

  /* Galerkin FAS state */
  PetscBool      galerkin;                     /* use Galerkin formation of the coarse problem */
  Vec            Xg;                           /* Galerkin solution projection */
  Vec            Fg;                           /* Galerkin function projection */

} SNES_FAS;

#endif
