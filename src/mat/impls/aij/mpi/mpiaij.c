
#include "mpiaij.h"
#include "vec/vecimpl.h"


#define CHUNCKSIZE   100
/*
   This is a simple minded stash. Do a linear search to determine if
 in stash, if not add to end.
*/
static int StashValues(Stash *stash,int row,int n, int *idxn,
                       Scalar *values,InsertMode addv)
{
  int    i,j,N = stash->n,found,*n_idx, *n_idy;
  Scalar val,*n_array;

  for ( i=0; i<n; i++ ) {
    found = 0;
    val = *values++;
    for ( j=0; j<N; j++ ) {
      if ( stash->idx[j] == row && stash->idy[j] == idxn[i]) {
        /* found a match */
        if (addv == AddValues) stash->array[j] += val;
        else stash->array[j] = val;
        found = 1;
        break;
      }
    }
    if (!found) { /* not found so add to end */
      if ( stash->n == stash->nmax ) {
        /* allocate a larger stash */
        n_array = (Scalar *) MALLOC( (stash->nmax + CHUNCKSIZE)*(
                                     2*sizeof(int) + sizeof(Scalar)));
        CHKPTR(n_array);
        n_idx = (int *) (n_array + stash->nmax + CHUNCKSIZE);
        n_idy = (int *) (n_idx + stash->nmax + CHUNCKSIZE);
        MEMCPY(n_array,stash->array,stash->nmax*sizeof(Scalar));
        MEMCPY(n_idx,stash->idx,stash->nmax*sizeof(int));
        MEMCPY(n_idy,stash->idy,stash->nmax*sizeof(int));
        if (stash->array) FREE(stash->array);
        stash->array = n_array; stash->idx = n_idx; stash->idy = n_idy;
        stash->nmax += CHUNCKSIZE;
      }
      stash->array[stash->n]   = val;
      stash->idx[stash->n]     = row;
      stash->idy[stash->n++]   = idxn[i];
    }
  }
  return 0;
}

static int MatiAIJInsertValues(Mat mat,int m,int *idxm,int n,
                            int *idxn,Scalar *v,InsertMode addv)
{
  Matimpiaij *aij = (Matimpiaij *) mat->data;
  int        ierr,i,j, rstart = aij->rstart, rend = aij->rend;
  int        cstart = aij->cstart, cend = aij->cend,row,col;

  if (aij->insertmode != NotSetValues && aij->insertmode != addv) {
    SETERR(1,"You cannot mix inserts and adds");
  }
  aij->insertmode = addv;
  for ( i=0; i<m; i++ ) {
    if (idxm[i] >= rstart && idxm[i] < rend) {
      row = idxm[i] - rstart;
      for ( j=0; j<n; j++ ) {
        if (idxn[j] >= cstart && idxn[j] < cend){
          col = idxn[j] - cstart;
          ierr = MatSetValues(aij->A,1,&row,1,&col,v+i*n+j,addv);CHKERR(ierr);
        }
        else {
          col = idxn[j];
          ierr = MatSetValues(aij->B,1,&row,1,&col,v+i*n+j,addv);CHKERR(ierr);
        }
      }
    } 
    else {
      ierr = StashValues(&aij->stash,idxm[i],n,idxn,v+i*n,addv);CHKERR(ierr);
    }
  }
  return 0;
}

/*
    the assembly code is alot like the code for vectors, we should 
    sometime derive a single assembly code that can be used for 
    either case.
*/

static int MatiAIJBeginAssemble(Mat mat)
{ 
  Matimpiaij  *aij = (Matimpiaij *) mat->data;
  MPI_Comm    comm = aij->comm;
  int         ierr, numtids = aij->numtids, *owners = aij->rowners;
  int         mytid = aij->mytid;
  MPI_Request *send_waits,*recv_waits;
  int         *nprocs,i,j,n,idx,*procs,nsends,nreceives,nmax,*work;
  int         tag = 50, *owner,*starts,count;
  InsertMode  addv;
  Scalar      *rvalues,*svalues;

  /* make sure all processors are either in INSERTMODE or ADDMODE */
  MPI_Allreduce((void *) &aij->insertmode,(void *) &addv,numtids,MPI_INT,
                MPI_BOR,comm);
  if (addv == (AddValues|InsertValues)) {
    SETERR(1,"Some processors have inserted while others have added");
  }
  aij->insertmode = addv; /* in case this processor had no cache */

  /*  first count number of contributors to each processor */
  nprocs = (int *) MALLOC( 2*numtids*sizeof(int) ); CHKPTR(nprocs);
  MEMSET(nprocs,0,2*numtids*sizeof(int)); procs = nprocs + numtids;
  owner = (int *) MALLOC( (aij->stash.n+1)*sizeof(int) ); CHKPTR(owner);
  for ( i=0; i<aij->stash.n; i++ ) {
    idx = aij->stash.idx[i];
    for ( j=0; j<numtids; j++ ) {
      if (idx >= owners[j] && idx < owners[j+1]) {
        nprocs[j]++; procs[j] = 1; owner[i] = j; break;
      }
    }
  }
  nsends = 0;  for ( i=0; i<numtids; i++ ) { nsends += procs[i];} 

  /* inform other processors of number of messages and max length*/
  work = (int *) MALLOC( numtids*sizeof(int) ); CHKPTR(work);
  MPI_Allreduce((void *) procs,(void *) work,numtids,MPI_INT,MPI_SUM,comm);
  nreceives = work[mytid]; 
  MPI_Allreduce((void *) nprocs,(void *) work,numtids,MPI_INT,MPI_MAX,comm);
  nmax = work[mytid];
  FREE(work);

  /* post receives: 
       1) each message will consist of ordered pairs 
     (global index,value) we store the global index as a double 
     to simply the message passing. 
       2) since we don't know how long each individual message is we 
     allocate the largest needed buffer for each receive. Potentially 
     this is a lot of wasted space.


       This could be done better.
  */
  rvalues = (Scalar *) MALLOC(3*(nreceives+1)*nmax*sizeof(Scalar));
  CHKPTR(rvalues);
  recv_waits = (MPI_Request *) MALLOC((nreceives+1)*sizeof(MPI_Request));
  CHKPTR(recv_waits);
  for ( i=0; i<nreceives; i++ ) {
    MPI_Irecv((void *)(rvalues+3*nmax*i),3*nmax,MPI_SCALAR,MPI_ANY_SOURCE,tag,
              comm,recv_waits+i);
  }

  /* do sends:
      1) starts[i] gives the starting index in svalues for stuff going to 
         the ith processor
  */
  svalues = (Scalar *) MALLOC( 3*(aij->stash.n+1)*sizeof(Scalar) );
  CHKPTR(svalues);
  send_waits = (MPI_Request *) MALLOC( (nsends+1)*sizeof(MPI_Request));
  CHKPTR(send_waits);
  starts = (int *) MALLOC( numtids*sizeof(int) ); CHKPTR(starts);
  starts[0] = 0; 
  for ( i=1; i<numtids; i++ ) { starts[i] = starts[i-1] + nprocs[i-1];} 
  for ( i=0; i<aij->stash.n; i++ ) {
    svalues[3*starts[owner[i]]]       = (Scalar)  aij->stash.idx[i];
    svalues[3*starts[owner[i]]+1]     = (Scalar)  aij->stash.idy[i];
    svalues[3*(starts[owner[i]]++)+2] =  aij->stash.array[i];
  }
  FREE(owner);
  starts[0] = 0;
  for ( i=1; i<numtids; i++ ) { starts[i] = starts[i-1] + nprocs[i-1];} 
  count = 0;
  for ( i=0; i<numtids; i++ ) {
    if (procs[i]) {
      MPI_Isend((void*)(svalues+3*starts[i]),3*nprocs[i],MPI_SCALAR,i,tag,
                comm,send_waits+count++);
    }
  }
  FREE(starts); FREE(nprocs);

  /* Free cache space */
  aij->stash.nmax = aij->stash.n = 0;
  if (aij->stash.array){ FREE(aij->stash.array); aij->stash.array = 0;}

  aij->svalues    = svalues;       aij->rvalues = rvalues;
  aij->nsends     = nsends;         aij->nrecvs = nreceives;
  aij->send_waits = send_waits; aij->recv_waits = recv_waits;
  aij->rmax       = nmax;

  return 0;
}
extern int MPIAIJSetUpMultiply(Mat);

static int MatiAIJEndAssemble(Mat mat)
{ 
  int        ierr;
  Matimpiaij *aij = (Matimpiaij *) mat->data;

  MPI_Status  *send_status,recv_status;
  int         index,idx,nrecvs = aij->nrecvs, count = nrecvs, i, n;
  int         row,col;
  Scalar      *values,val;
  InsertMode  addv = aij->insertmode;

  /*  wait on receives */
  while (count) {
    MPI_Waitany(nrecvs,aij->recv_waits,&index,&recv_status);
    /* unpack receives into our local space */
    values = aij->rvalues + 3*index*aij->rmax;
    MPI_Get_count(&recv_status,MPI_SCALAR,&n);
    n = n/3;
    for ( i=0; i<n; i++ ) {
      row = (int) PETSCREAL(values[3*i]) - aij->rstart;
      col = (int) PETSCREAL(values[3*i+1]);
      val = values[3*i+2];
      if (col >= aij->cstart && col < aij->cend) {
          col -= aij->cstart;
        MatSetValues(aij->A,1,&row,1,&col,&val,addv);
      } 
      else {
        MatSetValues(aij->B,1,&row,1,&col,&val,addv);
      }
    }
    count--;
  }
  FREE(aij->recv_waits); FREE(aij->rvalues);
 
  /* wait on sends */
  if (aij->nsends) {
    send_status = (MPI_Status *) MALLOC( aij->nsends*sizeof(MPI_Status) );
    CHKPTR(send_status);
    MPI_Waitall(aij->nsends,aij->send_waits,send_status);
    FREE(send_status);
  }
  FREE(aij->send_waits); FREE(aij->svalues);

  aij->insertmode = NotSetValues;
  ierr = MatBeginAssembly(aij->A); CHKERR(ierr);
  ierr = MatEndAssembly(aij->A); CHKERR(ierr);

  ierr = MPIAIJSetUpMultiply(mat); CHKERR(ierr);
  ierr = MatBeginAssembly(aij->B); CHKERR(ierr);
  ierr = MatEndAssembly(aij->B); CHKERR(ierr);
  return 0;
}

static int MatiZero(Mat A)
{
  Matimpiaij *l = (Matimpiaij *) A->data;

  MatZeroEntries(l->A); MatZeroEntries(l->B);
  return 0;
}

/* again this uses the same basic stratagy as in the assembly and 
   scatter create routines, we should try to do it systemamatically 
   if we can figure out the proper level of generality. */

/* the code does not do the diagonal entries correctly unless the 
   matrix is square and the column and row owerships are identical.
   This is a BUG. The only way to fix it seems to be to access 
   aij->A and aij->B directly and not through the MatZeroRows() 
   routine. 
*/
static int MatiZerorows(Mat A,IS is,Scalar *diag)
{
  Matimpiaij     *l = (Matimpiaij *) A->data;
  int            i,ierr,N, *rows,*owners = l->rowners,numtids = l->numtids;
  int            *localrows,*procs,*nprocs,j,found,idx,nsends,*work;
  int            nmax,*svalues,*starts,*owner,nrecvs,mytid = l->mytid;
  int            *rvalues,tag = 67,count,base,slen,n,len,*source;
  int            *lens,index,*lrows,*values;
  MPI_Comm       comm = l->comm;
  MPI_Request    *send_waits,*recv_waits;
  MPI_Status     recv_status,*send_status;
  IS             istmp;

  ierr = ISGetLocalSize(is,&N); CHKERR(ierr);
  ierr = ISGetIndices(is,&rows); CHKERR(ierr);

  /*  first count number of contributors to each processor */
  nprocs = (int *) MALLOC( 2*numtids*sizeof(int) ); CHKPTR(nprocs);
  MEMSET(nprocs,0,2*numtids*sizeof(int)); procs = nprocs + numtids;
  owner = (int *) MALLOC((N+1)*sizeof(int)); CHKPTR(owner); /* see note*/
  for ( i=0; i<N; i++ ) {
    idx = rows[i];
    found = 0;
    for ( j=0; j<numtids; j++ ) {
      if (idx >= owners[j] && idx < owners[j+1]) {
        nprocs[j]++; procs[j] = 1; owner[i] = j; found = 1; break;
      }
    }
    if (!found) SETERR(1,"Index out of range");
  }
  nsends = 0;  for ( i=0; i<numtids; i++ ) { nsends += procs[i];} 

  /* inform other processors of number of messages and max length*/
  work = (int *) MALLOC( numtids*sizeof(int) ); CHKPTR(work);
  MPI_Allreduce((void *) procs,(void *) work,numtids,MPI_INT,MPI_SUM,comm);
  nrecvs = work[mytid]; 
  MPI_Allreduce((void *) nprocs,(void *) work,numtids,MPI_INT,MPI_MAX,comm);
  nmax = work[mytid];
  FREE(work);

  /* post receives:   */
  rvalues = (int *) MALLOC((nrecvs+1)*nmax*sizeof(int)); /*see note */
  CHKPTR(rvalues);
  recv_waits = (MPI_Request *) MALLOC((nrecvs+1)*sizeof(MPI_Request));
  CHKPTR(recv_waits);
  for ( i=0; i<nrecvs; i++ ) {
    MPI_Irecv((void *)(rvalues+nmax*i),nmax,MPI_INT,MPI_ANY_SOURCE,tag,
              comm,recv_waits+i);
  }

  /* do sends:
      1) starts[i] gives the starting index in svalues for stuff going to 
         the ith processor
  */
  svalues = (int *) MALLOC( (N+1)*sizeof(int) ); CHKPTR(svalues);
  send_waits = (MPI_Request *) MALLOC( (nsends+1)*sizeof(MPI_Request));
  CHKPTR(send_waits);
  starts = (int *) MALLOC( (numtids+1)*sizeof(int) ); CHKPTR(starts);
  starts[0] = 0; 
  for ( i=1; i<numtids; i++ ) { starts[i] = starts[i-1] + nprocs[i-1];} 
  for ( i=0; i<N; i++ ) {
    svalues[starts[owner[i]]++] = rows[i];
  }
  ISRestoreIndices(is,&rows);

  starts[0] = 0;
  for ( i=1; i<numtids+1; i++ ) { starts[i] = starts[i-1] + nprocs[i-1];} 
  count = 0;
  for ( i=0; i<numtids; i++ ) {
    if (procs[i]) {
      MPI_Isend((void*)(svalues+starts[i]),nprocs[i],MPI_INT,i,tag,
                comm,send_waits+count++);
    }
  }
  FREE(starts);

  base = owners[mytid];

  /*  wait on receives */
  lens = (int *) MALLOC( 2*(nrecvs+1)*sizeof(int) ); CHKPTR(lens);
  source = lens + nrecvs;
  count = nrecvs; slen = 0;
  while (count) {
    MPI_Waitany(nrecvs,recv_waits,&index,&recv_status);
    /* unpack receives into our local space */
    MPI_Get_count(&recv_status,MPI_INT,&n);
    source[index]  = recv_status.MPI_SOURCE;
    lens[index]  = n;
    slen += n;
    count--;
  }
  FREE(recv_waits); 
  
  /* move the data into the send scatter */
  lrows = (int *) MALLOC( slen*sizeof(int) ); CHKPTR(lrows);
  count = 0;
  for ( i=0; i<nrecvs; i++ ) {
    values = rvalues + i*nmax;
    for ( j=0; j<lens[i]; j++ ) {
      lrows[count++] = values[j] - base;
    }
  }
  FREE(rvalues); FREE(lens);
  FREE(owner); FREE(nprocs);
    
  /* actually zap the local rows */
  ierr = ISCreateSequential(slen,lrows,&istmp); CHKERR(ierr);  FREE(lrows);
  ierr = MatZeroRows(l->A,istmp,diag); CHKERR(ierr);
  ierr = MatZeroRows(l->B,istmp,0); CHKERR(ierr);
  ierr = ISDestroy(istmp); CHKERR(ierr);

  /* wait on sends */
  if (nsends) {
    send_status = (MPI_Status *) MALLOC( nsends*sizeof(MPI_Status) );
    CHKPTR(send_status);
    MPI_Waitall(nsends,send_waits,send_status);
    FREE(send_status);
  }
  FREE(send_waits); FREE(svalues);


  return 0;
}

static int MatiAIJMult(Mat aijin,Vec xx,Vec yy)
{
  Matimpiaij *aij = (Matimpiaij *) aijin->data;
  int        ierr;

  ierr = VecScatterBegin(xx,0,aij->lvec,0,InsertValues,&aij->Mvctx);
  CHKERR(ierr);
  ierr = MatMult(aij->A,xx,yy); CHKERR(ierr);
  ierr = VecScatterEnd(xx,0,aij->lvec,0,InsertValues,&aij->Mvctx); CHKERR(ierr);
  ierr = MatMultAdd(aij->B,aij->lvec,yy,yy); CHKERR(ierr);
  return 0;
}

/*
  This only works correctly for square matrices where the subblock A->A is the 
   diagonal block
*/
static int MatiAIJgetdiag(Mat Ain,Vec v)
{
  Matimpiaij *A = (Matimpiaij *) Ain->data;
  return MatGetDiagonal(A->A,v);
}

static int MatiAIJdestroy(PetscObject obj)
{
  Mat        mat = (Mat) obj;
  Matimpiaij *aij = (Matimpiaij *) mat->data;
  int        ierr;
  FREE(aij->rowners); 
  ierr = MatDestroy(aij->A); CHKERR(ierr);
  ierr = MatDestroy(aij->B); CHKERR(ierr);
  FREE(aij); FREE(mat);
  if (aij->lvec) VecDestroy(aij->lvec);
  if (aij->Mvctx) VecScatterCtxDestroy(aij->Mvctx);
  return 0;
}

static int MatiView(PetscObject obj,Viewer viewer)
{
  Mat        mat = (Mat) obj;
  Matimpiaij *aij = (Matimpiaij *) mat->data;
  int        ierr;

  MPE_Seq_begin(aij->comm,1);
  printf("[%d] rows %d starts %d ends %d cols %d starts %d ends %d\n",
          aij->mytid,aij->m,aij->rstart,aij->rend,aij->n,aij->cstart,
          aij->cend);
  ierr = MatView(aij->A,viewer); CHKERR(ierr);
  ierr = MatView(aij->B,viewer); CHKERR(ierr);
  MPE_Seq_end(aij->comm,1);
  return 0;
}

/*
    This has to provide several versions.

     1) per sequential 
     2) a) use only local smoothing updating outer values only once.
        b) local smoothing updating outer values each inner iteration
     3) color updating out values betwen colors. (this imples an 
        ordering that is sort of related to the IS argument, it 
        is not clear a IS argument makes the most sense perhaps it 
        should be dropped.
*/
static int MatiAIJrelax(Mat matin,Vec bb,double omega,int flag,IS is,
                        int its,Vec xx)
{
  Matimpiaij *mat = (Matimpiaij *) matin->data;
  Scalar     zero = 0.0;
  int        ierr,one = 1, tmp, *idx, *diag;
  int        n = mat->n, m = mat->m, i, j;

  if (is) SETERR(1,"No support for ordering in relaxation");
  if (flag & SOR_ZERO_INITIAL_GUESS) {
    if (ierr = VecSet(&zero,xx)) return ierr;
  }

  /* update outer values from other processors*/
 
  /* smooth locally */
  return 0;
} 
/* -------------------------------------------------------------------*/
static struct _MatOps MatOps = {MatiAIJInsertValues,
       0, 0,
       MatiAIJMult,0,0,0,
       0,0,0,0,
       0,0,
       MatiAIJrelax,
       0,
       0,0,0,
       0,
       MatiAIJgetdiag,0,0,
       MatiAIJBeginAssemble,MatiAIJEndAssemble,
       0,
       0,MatiZero,MatiZerorows,0,
       0,0,0,0 };



/*@

      MatCreateMPIAIJ - Creates a sparse parallel matrix 
                                 in AIJ format.

  Input Parameters:
.   comm - MPI communicator
.   m,n - number of local rows and columns (or -1 to have calculated)
.   M,N - global rows and columns (or -1 to have calculated)
.   d_nz - total number nonzeros in diagonal portion of matrix
.   d_nzz - number of nonzeros per row in diagonal portion of matrix or null
.           You must leave room for the diagonal entry even if it is zero.
.   o_nz - total number nonzeros in off-diagonal portion of matrix
.   o_nzz - number of nonzeros per row in off-diagonal portion of matrix
.           or null. You must have at least one nonzero per row.

  Output parameters:
.  newmat - the matrix 

  Keywords: matrix, aij, compressed row, sparse, parallel
@*/
int MatCreateMPIAIJ(MPI_Comm comm,int m,int n,int M,int N,
                 int d_nz,int *d_nnz, int o_nz,int *o_nnz,Mat *newmat)
{
  Mat          mat;
  Matimpiaij   *aij;
  int          ierr, i,rl,len,sum[2],work[2];
  *newmat         = 0;
  CREATEHEADER(mat,_Mat);
  mat->data       = (void *) (aij = NEW(Matimpiaij)); CHKPTR(aij);
  mat->cookie     = MAT_COOKIE;
  mat->ops        = &MatOps;
  mat->destroy    = MatiAIJdestroy;
  mat->view       = MatiView;
  mat->type       = MATAIJMPI;
  mat->factor     = 0;
  mat->row        = 0;
  mat->col        = 0;
  aij->comm       = comm;
  aij->insertmode = NotSetValues;
  MPI_Comm_rank(comm,&aij->mytid);
  MPI_Comm_size(comm,&aij->numtids);

  if (M == -1 || N == -1) {
    work[0] = m; work[1] = n;
    MPI_Allreduce((void *) work,(void *) sum,1,MPI_INT,MPI_SUM,comm );
    if (M == -1) M = sum[0];
    if (N == -1) N = sum[1];
  }
  if (m == -1) {m = M/aij->numtids + ((M % aij->numtids) > aij->mytid);}
  if (n == -1) {n = N/aij->numtids + ((N % aij->numtids) > aij->mytid);}
  aij->m       = m;
  aij->n       = n;
  aij->N       = N;
  aij->M       = M;

  /* build local table of row and column ownerships */
  aij->rowners = (int *) MALLOC(2*(aij->numtids+2)*sizeof(int)); 
  CHKPTR(aij->rowners);
  aij->cowners = aij->rowners + aij->numtids + 1;
  MPI_Allgather(&m,1,MPI_INT,aij->rowners+1,1,MPI_INT,comm);
  aij->rowners[0] = 0;
  for ( i=2; i<=aij->numtids; i++ ) {
    aij->rowners[i] += aij->rowners[i-1];
  }
  aij->rstart = aij->rowners[aij->mytid]; 
  aij->rend   = aij->rowners[aij->mytid+1]; 
  MPI_Allgather(&n,1,MPI_INT,aij->cowners+1,1,MPI_INT,comm);
  aij->cowners[0] = 0;
  for ( i=2; i<=aij->numtids; i++ ) {
    aij->cowners[i] += aij->cowners[i-1];
  }
  aij->cstart = aij->cowners[aij->mytid]; 
  aij->cend   = aij->cowners[aij->mytid+1]; 


  ierr = MatCreateSequentialAIJ(m,n,d_nz,d_nnz,&aij->A); CHKERR(ierr);
  ierr = MatCreateSequentialAIJ(m,N,o_nz,o_nnz,&aij->B); CHKERR(ierr);

  /* build cache for off array entries formed */
  aij->stash.nmax = CHUNCKSIZE; /* completely arbratray number */
  aij->stash.n    = 0;
  aij->stash.array = (Scalar *) MALLOC( aij->stash.nmax*(2*sizeof(int) +
                            sizeof(Scalar))); CHKPTR(aij->stash.array);
  aij->stash.idx = (int *) (aij->stash.array + aij->stash.nmax);
  aij->stash.idy = (int *) (aij->stash.idx + aij->stash.nmax);

  /* stuff used for matrix vector multiply */
  aij->lvec      = 0;
  aij->Mvctx     = 0;

  *newmat = mat;
  return 0;
}
