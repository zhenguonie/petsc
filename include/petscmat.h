/*
     Include file for the matrix component of PETSc
*/
#ifndef __PETSCMAT_H
#define __PETSCMAT_H
#include "petscvec.h"
PETSC_EXTERN_CXX_BEGIN

/*S
     Mat - Abstract PETSc matrix object

   Level: beginner

  Concepts: matrix; linear operator

.seealso:  MatCreate(), MatType, MatSetType()
S*/
typedef struct _p_Mat*           Mat;

/*E
    MatType - String with the name of a PETSc matrix or the creation function
       with an optional dynamic library name, for example
       http://www.mcs.anl.gov/petsc/lib.a:mymatcreate()

   Level: beginner

.seealso: MatSetType(), Mat
E*/
#define MATSAME            "same"
#define MATSEQMAIJ         "seqmaij"
#define MATMPIMAIJ         "mpimaij"
#define MATMAIJ            "maij"
#define MATIS              "is"
#define MATMPIROWBS        "mpirowbs"
#define MATSEQAIJ          "seqaij"
#define MATMPIAIJ          "mpiaij"
#define MATAIJ             "aij"
#define MATSHELL           "shell"
#define MATSEQBDIAG        "seqbdiag"
#define MATMPIBDIAG        "mpibdiag"
#define MATBDIAG           "bdiag"
#define MATSEQDENSE        "seqdense"
#define MATMPIDENSE        "mpidense"
#define MATDENSE           "dense"
#define MATSEQBAIJ         "seqbaij"
#define MATMPIBAIJ         "mpibaij"
#define MATBAIJ            "baij"
#define MATMPIADJ          "mpiadj"
#define MATSEQSBAIJ        "seqsbaij"
#define MATMPISBAIJ        "mpisbaij"
#define MATSBAIJ           "sbaij"
#define MATDAAD            "daad"
#define MATMFFD            "mffd"
#define MATESI             "esi"
#define MATPETSCESI        "petscesi"
#define MATNORMAL          "normal"
#define MATSEQAIJSPOOLES   "seqaijspooles"
#define MATMPIAIJSPOOLES   "mpiaijspooles"
#define MATSEQSBAIJSPOOLES "seqsbaijspooles"
#define MATMPISBAIJSPOOLES "mpisbaijspooles"
#define MATAIJSPOOLES      "aijspooles"
#define MATSBAIJSPOOLES    "sbaijspooles"
#define MATSUPERLU         "superlu"
#define MATSUPERLU_DIST    "superlu_dist"
#define MATUMFPACK         "umfpack"
#define MATESSL            "essl"
#define MATLUSOL           "lusol"
#define MATAIJMUMPS        "aijmumps"
#define MATSBAIJMUMPS      "sbaijmumps"
#define MATDSCPACK         "dscpack"
#define MATMATLAB          "matlab"
#define MatType char*

/* Logging support */
#define    MAT_FILE_COOKIE 1211216    /* used to indicate matrices in binary files */
extern int MAT_COOKIE;
extern int MATSNESMFCTX_COOKIE;
extern int MAT_FDCOLORING_COOKIE;
extern int MAT_PARTITIONING_COOKIE;
extern int MAT_NULLSPACE_COOKIE;
extern int MAT_Mult, MAT_MultMatrixFree, MAT_Mults, MAT_MultConstrained, MAT_MultAdd, MAT_MultTranspose;
extern int MAT_MultTransposeConstrained, MAT_MultTransposeAdd, MAT_Solve, MAT_Solves, MAT_SolveAdd, MAT_SolveTranspose;
extern int MAT_SolveTransposeAdd, MAT_Relax, MAT_ForwardSolve, MAT_BackwardSolve, MAT_LUFactor, MAT_LUFactorSymbolic;
extern int MAT_LUFactorNumeric, MAT_CholeskyFactor, MAT_CholeskyFactorSymbolic, MAT_CholeskyFactorNumeric, MAT_ILUFactor;
extern int MAT_ILUFactorSymbolic, MAT_ICCFactorSymbolic, MAT_Copy, MAT_Convert, MAT_Scale, MAT_AssemblyBegin;
extern int MAT_AssemblyEnd, MAT_SetValues, MAT_GetValues, MAT_GetRow, MAT_GetSubMatrices, MAT_GetColoring, MAT_GetOrdering;
extern int MAT_IncreaseOverlap, MAT_Partitioning, MAT_ZeroEntries, MAT_Load, MAT_View, MAT_AXPY, MAT_FDColoringCreate;
extern int MAT_FDColoringApply, MAT_Transpose, MAT_FDColoringFunction;

EXTERN int MatInitializePackage(char *);

EXTERN int MatCreate(MPI_Comm,int,int,int,int,Mat*);
EXTERN int MatSetType(Mat,const MatType);
EXTERN int MatSetFromOptions(Mat);
EXTERN int MatSetUpPreallocation(Mat);
EXTERN int MatRegisterAll(const char[]);
EXTERN int MatRegister(const char[],const char[],const char[],int(*)(Mat));

/*MC
   MatRegisterDynamic - Adds a new matrix type

   Synopsis:
   int MatRegisterDynamic(char *name,char *path,char *name_create,int (*routine_create)(Mat))

   Not Collective

   Input Parameters:
+  name - name of a new user-defined matrix type
.  path - path (either absolute or relative) the library containing this solver
.  name_create - name of routine to create method context
-  routine_create - routine to create method context

   Notes:
   MatRegisterDynamic() may be called multiple times to add several user-defined solvers.

   If dynamic libraries are used, then the fourth input argument (routine_create)
   is ignored.

   Sample usage:
.vb
   MatRegisterDynamic("my_mat",/home/username/my_lib/lib/libO/solaris/mylib.a,
               "MyMatCreate",MyMatCreate);
.ve

   Then, your solver can be chosen with the procedural interface via
$     MatSetType(Mat,"my_mat")
   or at runtime via the option
$     -mat_type my_mat

   Level: advanced

   Notes: ${PETSC_ARCH} and ${BOPT} occuring in pathname will be replaced with appropriate values.
         If your function is not being put into a shared library then use VecRegister() instead

.keywords: Mat, register

.seealso: MatRegisterAll(), MatRegisterDestroy()

M*/
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define MatRegisterDynamic(a,b,c,d) MatRegister(a,b,c,0)
#else
#define MatRegisterDynamic(a,b,c,d) MatRegister(a,b,c,d)
#endif

extern PetscTruth MatRegisterAllCalled;
extern PetscFList MatList;

EXTERN int MatCreateSeqDense(MPI_Comm,int,int,PetscScalar[],Mat*);
EXTERN int MatCreateMPIDense(MPI_Comm,int,int,int,int,PetscScalar[],Mat*); 
EXTERN int MatCreateSeqAIJ(MPI_Comm,int,int,int,const int[],Mat*);
EXTERN int MatCreateMPIAIJ(MPI_Comm,int,int,int,int,int,const int[],int,const int[],Mat*); 
EXTERN int MatCreateMPIRowbs(MPI_Comm,int,int,int,const int[],Mat*); 
EXTERN int MatCreateSeqBDiag(MPI_Comm,int,int,int,int,const int[],PetscScalar*[],Mat*); 
EXTERN int MatCreateMPIBDiag(MPI_Comm,int,int,int,int,int,const int[],PetscScalar*[],Mat*); 
EXTERN int MatCreateSeqBAIJ(MPI_Comm,int,int,int,int,const int[],Mat*); 
EXTERN int MatCreateMPIBAIJ(MPI_Comm,int,int,int,int,int,int,const int[],int,const int[],Mat*);
EXTERN int MatCreateMPIAdj(MPI_Comm,int,int,int[],int[],int[],Mat*);
EXTERN int MatCreateSeqSBAIJ(MPI_Comm,int,int,int,int,const int[],Mat*); 
EXTERN int MatCreateMPISBAIJ(MPI_Comm,int,int,int,int,int,int,const int[],int,const int[],Mat*);
EXTERN int MatCreateShell(MPI_Comm,int,int,int,int,void *,Mat*);
EXTERN int MatCreateAdic(MPI_Comm,int,int,int,int,int,void (*)(void),Mat*);
EXTERN int MatCreateNormal(Mat,Mat*);
EXTERN int MatDestroy(Mat);

EXTERN int MatPrintHelp(Mat);
EXTERN int MatGetPetscMaps(Mat,PetscMap*,PetscMap*);

/* ------------------------------------------------------------*/
EXTERN int MatSetValues(Mat,int,const int[],int,const int[],const PetscScalar[],InsertMode);
EXTERN int MatSetValuesBlocked(Mat,int,const int[],int,const int[],const PetscScalar[],InsertMode);

/*S
     MatStencil - Data structure (C struct) for storing information about a single row or
        column of a matrix as index on an associated grid.

   Level: beginner

  Concepts: matrix; linear operator

.seealso:  MatSetValuesStencil(), MatSetStencil(), MatSetValuesBlockStencil()
S*/
typedef struct {
  int k,j,i,c;
} MatStencil;

EXTERN int MatSetValuesStencil(Mat,int,const MatStencil[],int,const MatStencil[],const PetscScalar[],InsertMode);
EXTERN int MatSetValuesBlockedStencil(Mat,int,const MatStencil[],int,const MatStencil[],const PetscScalar[],InsertMode);
EXTERN int MatSetStencil(Mat,int,const int[],const int[],int);

EXTERN int MatSetColoring(Mat,ISColoring);
EXTERN int MatSetValuesAdic(Mat,void*);
EXTERN int MatSetValuesAdifor(Mat,int,void*);

/*E
    MatAssemblyType - Indicates if the matrix is now to be used, or if you plan 
     to continue to add values to it

    Level: beginner

.seealso: MatAssemblyBegin(), MatAssemblyEnd()
E*/
typedef enum {MAT_FLUSH_ASSEMBLY=1,MAT_FINAL_ASSEMBLY=0} MatAssemblyType;
EXTERN int MatAssemblyBegin(Mat,MatAssemblyType);
EXTERN int MatAssemblyEnd(Mat,MatAssemblyType);
EXTERN int MatAssembled(Mat,PetscTruth*);

extern int         MatSetValue_Row, MatSetValue_Column;
extern PetscScalar MatSetValue_Value;

/*MC
   MatSetValue - Set a single entry into a matrix.

   Synopsis:
   int MatSetValue(Mat m,int row,int col,PetscScalar value,InsertMode mode);

   Not collective

   Input Parameters:
+  m - the matrix
.  row - the row location of the entry
.  col - the column location of the entry
.  value - the value to insert
-  mode - either INSERT_VALUES or ADD_VALUES

   Notes: 
   For efficiency one should use MatSetValues() and set several or many
   values simultaneously if possible.

   Level: beginner

.seealso: MatSetValues(), MatSetValueLocal()
M*/
#define MatSetValue(v,i,j,va,mode) \
  (MatSetValue_Row = i,MatSetValue_Column = j,MatSetValue_Value = va, \
   MatSetValues(v,1,&MatSetValue_Row,1,&MatSetValue_Column,&MatSetValue_Value,mode))

#define MatGetValue(v,i,j,va) \
  (MatSetValue_Row = i,MatSetValue_Column = j,\
   MatGetValues(v,1,&MatSetValue_Row,1,&MatSetValue_Column,&va))

#define MatSetValueLocal(v,i,j,va,mode) \
  (MatSetValue_Row = i,MatSetValue_Column = j,MatSetValue_Value = va, \
   MatSetValuesLocal(v,1,&MatSetValue_Row,1,&MatSetValue_Column,&MatSetValue_Value,mode))

/*E
    MatOption - Options that may be set for a matrix and its behavior or storage

    Level: beginner

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

.seealso: MatSetOption()
E*/
typedef enum {MAT_ROW_ORIENTED=1,MAT_COLUMN_ORIENTED=2,MAT_ROWS_SORTED=4,
              MAT_COLUMNS_SORTED=8,MAT_NO_NEW_NONZERO_LOCATIONS=16,
              MAT_YES_NEW_NONZERO_LOCATIONS=32,MAT_SYMMETRIC=64,
              MAT_STRUCTURALLY_SYMMETRIC=65,MAT_NO_NEW_DIAGONALS=66,
              MAT_YES_NEW_DIAGONALS=67,MAT_INODE_LIMIT_1=68,MAT_INODE_LIMIT_2=69,
              MAT_INODE_LIMIT_3=70,MAT_INODE_LIMIT_4=71,MAT_INODE_LIMIT_5=72,
              MAT_IGNORE_OFF_PROC_ENTRIES=73,MAT_ROWS_UNSORTED=74,
              MAT_COLUMNS_UNSORTED=75,MAT_NEW_NONZERO_LOCATION_ERR=76,
              MAT_NEW_NONZERO_ALLOCATION_ERR=77,MAT_USE_HASH_TABLE=78,
              MAT_KEEP_ZEROED_ROWS=79,MAT_IGNORE_ZERO_ENTRIES=80,MAT_USE_INODES=81,
              MAT_DO_NOT_USE_INODES=82,MAT_NOT_SYMMETRIC=83,MAT_HERMITIAN=84,
              MAT_NOT_STRUCTURALLY_SYMMETRIC=85,MAT_NOT_HERMITIAN=86,
              MAT_SYMMETRY_ETERNAL=87,MAT_NOT_SYMMETRY_ETERNAL=88} MatOption;
EXTERN int MatSetOption(Mat,MatOption);
EXTERN int MatGetType(Mat,MatType*);

EXTERN int MatGetValues(Mat,int,const int[],int,const int[],PetscScalar[]);
EXTERN int MatGetRow(Mat,int,int *,const int *[],const PetscScalar*[]);
EXTERN int MatRestoreRow(Mat,int,int *,const int *[],const PetscScalar*[]);
EXTERN int MatGetColumn(Mat,int,int *,int *[],PetscScalar*[]);
EXTERN int MatRestoreColumn(Mat,int,int *,int *[],PetscScalar*[]);
EXTERN int MatGetColumnVector(Mat,Vec,int);
EXTERN int MatGetArray(Mat,PetscScalar *[]);
EXTERN int MatRestoreArray(Mat,PetscScalar *[]);
EXTERN int MatGetBlockSize(Mat,int *);

EXTERN int MatMult(Mat,Vec,Vec);
EXTERN int MatMultAdd(Mat,Vec,Vec,Vec);
EXTERN int MatMultTranspose(Mat,Vec,Vec);
EXTERN int MatIsTranspose(Mat,Mat,PetscReal,PetscTruth*);
EXTERN int MatMultTransposeAdd(Mat,Vec,Vec,Vec);
EXTERN int MatMultConstrained(Mat,Vec,Vec);
EXTERN int MatMultTransposeConstrained(Mat,Vec,Vec);

/*E
    MatDuplicateOption - Indicates if a duplicated sparse matrix should have
  its numerical values copied over or just its nonzero structure.

    Level: beginner

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

.seealso: MatDuplicate()
E*/
typedef enum {MAT_DO_NOT_COPY_VALUES,MAT_COPY_VALUES} MatDuplicateOption;

EXTERN int MatConvertRegister(const char[],const char[],const char[],int (*)(Mat,MatType,Mat*));
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define MatConvertRegisterDynamic(a,b,c,d) MatConvertRegister(a,b,c,0)
#else
#define MatConvertRegisterDynamic(a,b,c,d) MatConvertRegister(a,b,c,d)
#endif
EXTERN int        MatConvertRegisterAll(const char[]);
EXTERN int        MatConvertRegisterDestroy(void);
extern PetscTruth MatConvertRegisterAllCalled;
extern PetscFList MatConvertList;
EXTERN int        MatConvert(Mat,const MatType,Mat*);
EXTERN int        MatDuplicate(Mat,MatDuplicateOption,Mat*);

/*E
    MatStructure - Indicates if the matrix has the same nonzero structure

    Level: beginner

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

.seealso: MatCopy(), KSPSetOperators(), PCSetOperators()
E*/
typedef enum {SAME_NONZERO_PATTERN,DIFFERENT_NONZERO_PATTERN,SAME_PRECONDITIONER,SUBSET_NONZERO_PATTERN} MatStructure;

EXTERN int MatCopy(Mat,Mat,MatStructure);
EXTERN int MatView(Mat,PetscViewer);
EXTERN int MatIsSymmetric(Mat,PetscReal,PetscTruth*);
EXTERN int MatIsSymmetricKnown(Mat,PetscTruth*,PetscTruth*);
EXTERN int MatLoad(PetscViewer,const MatType,Mat*);
EXTERN int MatMerge(MPI_Comm,Mat,Mat*);

EXTERN int MatGetRowIJ(Mat,int,PetscTruth,int*,int *[],int *[],PetscTruth *);
EXTERN int MatRestoreRowIJ(Mat,int,PetscTruth,int *,int *[],int *[],PetscTruth *);
EXTERN int MatGetColumnIJ(Mat,int,PetscTruth,int*,int *[],int *[],PetscTruth *);
EXTERN int MatRestoreColumnIJ(Mat,int,PetscTruth,int *,int *[],int *[],PetscTruth *);

/*S
     MatInfo - Context of matrix information, used with MatGetInfo()

   In Fortran this is simply a double precision array of dimension MAT_INFO_SIZE

   Level: intermediate

  Concepts: matrix^nonzero information

.seealso:  MatGetInfo(), MatInfoType
S*/
typedef struct {
  PetscLogDouble rows_global,columns_global;         /* number of global rows and columns */
  PetscLogDouble rows_local,columns_local;           /* number of local rows and columns */
  PetscLogDouble block_size;                         /* block size */
  PetscLogDouble nz_allocated,nz_used,nz_unneeded;   /* number of nonzeros */
  PetscLogDouble memory;                             /* memory allocated */
  PetscLogDouble assemblies;                         /* number of matrix assemblies called */
  PetscLogDouble mallocs;                            /* number of mallocs during MatSetValues() */
  PetscLogDouble fill_ratio_given,fill_ratio_needed; /* fill ratio for LU/ILU */
  PetscLogDouble factor_mallocs;                     /* number of mallocs during factorization */
} MatInfo;

/*E
    MatInfoType - Indicates if you want information about the local part of the matrix,
     the entire parallel matrix or the maximum over all the local parts.

    Level: beginner

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

.seealso: MatGetInfo(), MatInfo
E*/
typedef enum {MAT_LOCAL=1,MAT_GLOBAL_MAX=2,MAT_GLOBAL_SUM=3} MatInfoType;
EXTERN int MatGetInfo(Mat,MatInfoType,MatInfo*);
EXTERN int MatValid(Mat,PetscTruth*);
EXTERN int MatGetDiagonal(Mat,Vec);
EXTERN int MatGetRowMax(Mat,Vec);
EXTERN int MatTranspose(Mat,Mat*);
EXTERN int MatPermute(Mat,IS,IS,Mat *);
EXTERN int MatPermuteSparsify(Mat,int,PetscReal,PetscReal,IS,IS,Mat *);
EXTERN int MatDiagonalScale(Mat,Vec,Vec);
EXTERN int MatDiagonalSet(Mat,Vec,InsertMode);
EXTERN int MatEqual(Mat,Mat,PetscTruth*);

EXTERN int MatNorm(Mat,NormType,PetscReal *);
EXTERN int MatZeroEntries(Mat);
EXTERN int MatZeroRows(Mat,IS,const PetscScalar*);
EXTERN int MatZeroColumns(Mat,IS,const PetscScalar*);

EXTERN int MatUseScaledForm(Mat,PetscTruth);
EXTERN int MatScaleSystem(Mat,Vec,Vec);
EXTERN int MatUnScaleSystem(Mat,Vec,Vec);

EXTERN int MatGetSize(Mat,int*,int*);
EXTERN int MatGetLocalSize(Mat,int*,int*);
EXTERN int MatGetOwnershipRange(Mat,int*,int*);

/*E
    MatReuse - Indicates if matrices obtained from a previous call to MatGetSubMatrices()
     or MatGetSubMatrix() are to be reused to store the new matrix values.

    Level: beginner

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

.seealso: MatGetSubMatrices(), MatGetSubMatrix(), MatDestroyMatrices()
E*/
typedef enum {MAT_INITIAL_MATRIX,MAT_REUSE_MATRIX} MatReuse;
EXTERN int MatGetSubMatrices(Mat,int,const IS[],const IS[],MatReuse,Mat *[]);
EXTERN int MatDestroyMatrices(int,Mat *[]);
EXTERN int MatGetSubMatrix(Mat,IS,IS,int,MatReuse,Mat *);

EXTERN int MatIncreaseOverlap(Mat,int,IS[],int);

EXTERN int MatAXPY(const PetscScalar *,Mat,Mat,MatStructure);
EXTERN int MatAYPX(const PetscScalar *,Mat,Mat);
EXTERN int MatCompress(Mat);

EXTERN int MatScale(const PetscScalar *,Mat);
EXTERN int MatShift(const PetscScalar *,Mat);

EXTERN int MatSetLocalToGlobalMapping(Mat,ISLocalToGlobalMapping);
EXTERN int MatSetLocalToGlobalMappingBlock(Mat,ISLocalToGlobalMapping);
EXTERN int MatZeroRowsLocal(Mat,IS,const PetscScalar*);
EXTERN int MatSetValuesLocal(Mat,int,const int[],int,const int[],const PetscScalar[],InsertMode);
EXTERN int MatSetValuesBlockedLocal(Mat,int,const int[],int,const int[],const PetscScalar[],InsertMode);

EXTERN int MatStashSetInitialSize(Mat,int,int);
EXTERN int MatStashGetInfo(Mat,int*,int*,int*,int*);

EXTERN int MatInterpolateAdd(Mat,Vec,Vec,Vec);
EXTERN int MatInterpolate(Mat,Vec,Vec);
EXTERN int MatRestrict(Mat,Vec,Vec);
EXTERN int MatGetVecs(Mat,Vec*,Vec*);

/*MC
   MatPreallocInitialize - Begins the block of code that will count the number of nonzeros per
       row in a matrix providing the data that one can use to correctly preallocate the matrix.

   Synopsis:
   int MatPreallocateInitialize(MPI_Comm comm, int nrows, int ncols, int *dnz, int *onz)

   Collective on MPI_Comm

   Input Parameters:
+  comm - the communicator that will share the eventually allocated matrix
.  nrows - the number of rows in the matrix
-  ncols - the number of columns in the matrix

   Output Parameters:
+  dnz - the array that will be passed to the matrix preallocation routines
-  ozn - the other array passed to the matrix preallocation routines


   Level: intermediate

   Notes:
   See the chapter in the users manual on performance for more details

   Do not malloc or free dnz and onz, that is handled internally by these routines

   Use MatPreallocateInitializeSymmetric() for symmetric matrices (MPISBAIJ matrices)

  Concepts: preallocation^Matrix

.seealso: MatPreallocateFinalize(), MatPreallocateSet(), MatPreallocateSymmetricSet(), MatPreallocateSetLocal(),
          MatPreallocateInitializeSymmetric(), MatPreallocateSymmetricSetLocal()
M*/
#define MatPreallocateInitialize(comm,nrows,ncols,dnz,onz) 0; \
{ \
  int _4_ierr,__tmp = (nrows),__ctmp = (ncols),__rstart,__start,__end; \
  _4_ierr = PetscMalloc(2*__tmp*sizeof(int),&dnz);CHKERRQ(_4_ierr);onz = dnz + __tmp;\
  _4_ierr = PetscMemzero(dnz,2*__tmp*sizeof(int));CHKERRQ(_4_ierr);\
  _4_ierr = MPI_Scan(&__ctmp,&__end,1,MPI_INT,MPI_SUM,comm);CHKERRQ(_4_ierr); __start = __end - __ctmp;\
  _4_ierr = MPI_Scan(&__tmp,&__rstart,1,MPI_INT,MPI_SUM,comm);CHKERRQ(_4_ierr); __rstart = __rstart - __tmp;

/*MC
   MatPreallocSymmetricInitialize - Begins the block of code that will count the number of nonzeros per
       row in a matrix providing the data that one can use to correctly preallocate the matrix.

   Synopsis:
   int MatPreallocateSymmetricInitialize(MPI_Comm comm, int nrows, int ncols, int *dnz, int *onz)

   Collective on MPI_Comm

   Input Parameters:
+  comm - the communicator that will share the eventually allocated matrix
.  nrows - the number of rows in the matrix
-  ncols - the number of columns in the matrix

   Output Parameters:
+  dnz - the array that will be passed to the matrix preallocation routines
-  ozn - the other array passed to the matrix preallocation routines


   Level: intermediate

   Notes:
   See the chapter in the users manual on performance for more details

   Do not malloc or free dnz and onz, that is handled internally by these routines

  Concepts: preallocation^Matrix

.seealso: MatPreallocateFinalize(), MatPreallocateSet(), MatPreallocateSymmetricSet(), MatPreallocateSetLocal(),
          MatPreallocateInitialize(), MatPreallocateSymmetricSetLocal()
M*/
#define MatPreallocateSymmetricInitialize(comm,nrows,ncols,dnz,onz) 0; \
{ \
  int _4_ierr,__tmp = (nrows),__ctmp = (ncols),__rstart,__end; \
  _4_ierr = PetscMalloc(2*__tmp*sizeof(int),&dnz);CHKERRQ(_4_ierr);onz = dnz + __tmp;\
  _4_ierr = PetscMemzero(dnz,2*__tmp*sizeof(int));CHKERRQ(_4_ierr);\
  _4_ierr = MPI_Scan(&__ctmp,&__end,1,MPI_INT,MPI_SUM,comm);CHKERRQ(_4_ierr);\
  _4_ierr = MPI_Scan(&__tmp,&__rstart,1,MPI_INT,MPI_SUM,comm);CHKERRQ(_4_ierr); __rstart = __rstart - __tmp;

/*MC
   MatPreallocateSetLocal - Indicates the locations (rows and columns) in the matrix where nonzeros will be
       inserted using a local number of the rows and columns

   Synopsis:
   int MatPreallocateSetLocal(ISLocalToGlobalMappping map,int nrows, int *rows,int ncols, int *cols,int *dnz, int *onz)

   Not Collective

   Input Parameters:
+  map - the mapping between local numbering and global numbering
.  nrows - the number of rows indicated
.  rows - the indices of the rows 
.  ncols - the number of columns in the matrix
.  cols - the columns indicated
.  dnz - the array that will be passed to the matrix preallocation routines
-  ozn - the other array passed to the matrix preallocation routines


   Level: intermediate

   Notes:
   See the chapter in the users manual on performance for more details

   Do not malloc or free dnz and onz, that is handled internally by these routines

  Concepts: preallocation^Matrix

.seealso: MatPreallocateFinalize(), MatPreallocateSet(), MatPreallocateSymmetricSet(), MatPreallocateInitialize(),
          MatPreallocateInitialize(), MatPreallocateSymmetricSetLocal()
M*/
#define MatPreallocateSetLocal(map,nrows,rows,ncols,cols,dnz,onz) 0;\
{\
  int __l;\
  _4_ierr = ISLocalToGlobalMappingApply(map,nrows,rows,rows);CHKERRQ(_4_ierr);\
  _4_ierr = ISLocalToGlobalMappingApply(map,ncols,cols,cols);CHKERRQ(_4_ierr);\
  for (__l=0;__l<nrows;__l++) {\
    _4_ierr = MatPreallocateSet((rows)[__l],ncols,cols,dnz,onz);CHKERRQ(_4_ierr);\
  }\
}
    
/*MC
   MatPreallocateSymmetricSetLocal - Indicates the locations (rows and columns) in the matrix where nonzeros will be
       inserted using a local number of the rows and columns

   Synopsis:
   int MatPreallocateSymmetricSetLocal(ISLocalToGlobalMappping map,int nrows, int *rows,int ncols, int *cols,int *dnz, int *onz)

   Not Collective

   Input Parameters:
+  map - the mapping between local numbering and global numbering
.  nrows - the number of rows indicated
.  rows - the indices of the rows 
.  ncols - the number of columns in the matrix
.  cols - the columns indicated
.  dnz - the array that will be passed to the matrix preallocation routines
-  ozn - the other array passed to the matrix preallocation routines


   Level: intermediate

   Notes:
   See the chapter in the users manual on performance for more details

   Do not malloc or free dnz and onz that is handled internally by these routines

  Concepts: preallocation^Matrix

.seealso: MatPreallocateFinalize(), MatPreallocateSet(), MatPreallocateSymmetricSet(), MatPreallocateInitialize(),
          MatPreallocateInitialize(), MatPreallocateSymmetricSetLocal(), MatPreallocateSetLocal()
M*/
#define MatPreallocateSymmetricSetLocal(map,nrows,rows,ncols,cols,dnz,onz) 0;\
{\
  int __l;\
  _4_ierr = ISLocalToGlobalMappingApply(map,nrows,rows,rows);CHKERRQ(_4_ierr);\
  _4_ierr = ISLocalToGlobalMappingApply(map,ncols,cols,cols);CHKERRQ(_4_ierr);\
  for (__l=0;__l<nrows;__l++) {\
    _4_ierr = MatPreallocateSymmetricSet((rows)[__l],ncols,cols,dnz,onz);CHKERRQ(_4_ierr);\
  }\
}

/*MC
   MatPreallocateSet - Indicates the locations (rows and columns) in the matrix where nonzeros will be
       inserted using a local number of the rows and columns

   Synopsis:
   int MatPreallocateSet(int nrows, int *rows,int ncols, int *cols,int *dnz, int *onz)

   Not Collective

   Input Parameters:
+  nrows - the number of rows indicated
.  rows - the indices of the rows 
.  ncols - the number of columns in the matrix
.  cols - the columns indicated
.  dnz - the array that will be passed to the matrix preallocation routines
-  ozn - the other array passed to the matrix preallocation routines


   Level: intermediate

   Notes:
   See the chapter in the users manual on performance for more details

   Do not malloc or free dnz and onz that is handled internally by these routines

  Concepts: preallocation^Matrix

.seealso: MatPreallocateFinalize(), MatPreallocateSet(), MatPreallocateSymmetricSet(), MatPreallocateInitialize(),
          MatPreallocateInitialize(), MatPreallocateSymmetricSetLocal(), MatPreallocateSetLocal()
M*/
#define MatPreallocateSet(row,nc,cols,dnz,onz) 0;\
{ int __i; \
  for (__i=0; __i<nc; __i++) {\
    if (cols[__i] < __start || cols[__i] >= __end) onz[row - __rstart]++; \
  }\
  dnz[row - __rstart] = nc - onz[row - __rstart];\
}

/*MC
   MatPreallocateSymmetricSet - Indicates the locations (rows and columns) in the matrix where nonzeros will be
       inserted using a local number of the rows and columns

   Synopsis:
   int MatPreallocateSymmetricSet(int nrows, int *rows,int ncols, int *cols,int *dnz, int *onz)

   Not Collective

   Input Parameters:
+  nrows - the number of rows indicated
.  rows - the indices of the rows 
.  ncols - the number of columns in the matrix
.  cols - the columns indicated
.  dnz - the array that will be passed to the matrix preallocation routines
-  ozn - the other array passed to the matrix preallocation routines


   Level: intermediate

   Notes:
   See the chapter in the users manual on performance for more details

   Do not malloc or free dnz and onz that is handled internally by these routines

  Concepts: preallocation^Matrix

.seealso: MatPreallocateFinalize(), MatPreallocateSet(), MatPreallocateSymmetricSet(), MatPreallocateInitialize(),
          MatPreallocateInitialize(), MatPreallocateSymmetricSetLocal(), MatPreallocateSetLocal()
M*/
#define MatPreallocateSymmetricSet(row,nc,cols,dnz,onz) 0;\
{ int __i; \
  for (__i=0; __i<nc; __i++) {\
    if (cols[__i] >= __end) onz[row - __rstart]++; \
    else if (cols[__i] >= row) dnz[row - __rstart]++;\
  }\
}

/*MC
   MatPreallocFinalize - Ends the block of code that will count the number of nonzeros per
       row in a matrix providing the data that one can use to correctly preallocate the matrix.

   Synopsis:
   int MatPreallocateFinalize(int *dnz, int *onz)

   Collective on MPI_Comm

   Input Parameters:
+  dnz - the array that will be passed to the matrix preallocation routines
-  ozn - the other array passed to the matrix preallocation routines


   Level: intermediate

   Notes:
   See the chapter in the users manual on performance for more details

   Do not malloc or free dnz and onz that is handled internally by these routines

  Concepts: preallocation^Matrix

.seealso: MatPreallocateInitialize(), MatPreallocateSet(), MatPreallocateSymmetricSet(), MatPreallocateSetLocal(),
          MatPreallocateSymmetricInitialize(), MatPreallocateSymmetricSetLocal()
M*/
#define MatPreallocateFinalize(dnz,onz) 0;_4_ierr = PetscFree(dnz);CHKERRQ(_4_ierr);}



/* Routines unique to particular data structures */
EXTERN int MatShellGetContext(Mat,void **);

EXTERN int MatBDiagGetData(Mat,int*,int*,int*[],int*[],PetscScalar***);
EXTERN int MatSeqAIJSetColumnIndices(Mat,int[]);
EXTERN int MatSeqBAIJSetColumnIndices(Mat,int[]);
EXTERN int MatCreateSeqAIJWithArrays(MPI_Comm,int,int,int[],int[],PetscScalar[],Mat*);

EXTERN int MatSeqBAIJSetPreallocation(Mat,int,int,const int[]);
EXTERN int MatSeqSBAIJSetPreallocation(Mat,int,int,const int[]);
EXTERN int MatSeqAIJSetPreallocation(Mat,int,const int[]);
EXTERN int MatSeqDensePreallocation(Mat,PetscScalar[]);
EXTERN int MatSeqBDiagSetPreallocation(Mat,int,int,const int[],PetscScalar*[]);
EXTERN int MatSeqDenseSetPreallocation(Mat,PetscScalar[]);

EXTERN int MatMPIBAIJSetPreallocation(Mat,int,int,const int[],int,const int[]);
EXTERN int MatMPISBAIJSetPreallocation(Mat,int,int,const int[],int,const int[]);
EXTERN int MatMPIAIJSetPreallocation(Mat,int,const int[],int,const int[]);
EXTERN int MatMPIDensePreallocation(Mat,PetscScalar[]);
EXTERN int MatMPIBDiagSetPreallocation(Mat,int,int,const int[],PetscScalar*[]);
EXTERN int MatMPIAdjSetPreallocation(Mat,int[],int[],int[]);
EXTERN int MatMPIDenseSetPreallocation(Mat,PetscScalar[]);
EXTERN int MatMPIRowbsSetPreallocation(Mat,int,const int[]);
EXTERN int MatMPIAIJGetSeqAIJ(Mat,Mat*,Mat*,int*[]);
EXTERN int MatMPIBAIJGetSeqBAIJ(Mat,Mat*,Mat*,int*[]);
EXTERN int MatAdicSetLocalFunction(Mat,void (*)(void));

EXTERN int MatSeqDenseSetLDA(Mat,int);

EXTERN int MatStoreValues(Mat);
EXTERN int MatRetrieveValues(Mat);

EXTERN int MatDAADSetCtx(Mat,void*);

/* 
  These routines are not usually accessed directly, rather solving is 
  done through the KSP and PC interfaces.
*/

/*E
    MatOrderingType - String with the name of a PETSc matrix ordering or the creation function
       with an optional dynamic library name, for example 
       http://www.mcs.anl.gov/petsc/lib.a:orderingcreate()

   Level: beginner

.seealso: MatGetOrdering()
E*/
#define MatOrderingType char*
#define MATORDERING_NATURAL   "natural"
#define MATORDERING_ND        "nd"
#define MATORDERING_1WD       "1wd"
#define MATORDERING_RCM       "rcm"
#define MATORDERING_QMD       "qmd"
#define MATORDERING_ROWLENGTH "rowlength"
#define MATORDERING_DSC_ND    "dsc_nd"
#define MATORDERING_DSC_MMD   "dsc_mmd"
#define MATORDERING_DSC_MDF   "dsc_mdf"
#define MATORDERING_CONSTRAINED "constrained"
#define MATORDERING_IDENTITY  "identity"
#define MATORDERING_REVERSE   "reverse"

EXTERN int MatGetOrdering(Mat,const MatOrderingType,IS*,IS*);
EXTERN int MatOrderingRegister(const char[],const char[],const char[],int(*)(Mat,const MatOrderingType,IS*,IS*));

/*MC
   MatOrderingRegisterDynamic - Adds a new sparse matrix ordering to the 
                               matrix package. 

   Synopsis:
   int MatOrderingRegisterDynamic(char *name_ordering,char *path,char *name_create,int (*routine_create)(MatOrdering))

   Not Collective

   Input Parameters:
+  sname - name of ordering (for example MATORDERING_ND)
.  path - location of library where creation routine is 
.  name - name of function that creates the ordering type,a string
-  function - function pointer that creates the ordering

   Level: developer

   If dynamic libraries are used, then the fourth input argument (function)
   is ignored.

   Sample usage:
.vb
   MatOrderingRegisterDynamic("my_order",/home/username/my_lib/lib/libO/solaris/mylib.a,
               "MyOrder",MyOrder);
.ve

   Then, your partitioner can be chosen with the procedural interface via
$     MatOrderingSetType(part,"my_order)
   or at runtime via the option
$     -pc_ilu_mat_ordering_type my_order
$     -pc_lu_mat_ordering_type my_order

   ${PETSC_ARCH} and ${BOPT} occuring in pathname will be replaced with appropriate values.

.keywords: matrix, ordering, register

.seealso: MatOrderingRegisterDestroy(), MatOrderingRegisterAll()
M*/
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define MatOrderingRegisterDynamic(a,b,c,d) MatOrderingRegister(a,b,c,0)
#else
#define MatOrderingRegisterDynamic(a,b,c,d) MatOrderingRegister(a,b,c,d)
#endif

EXTERN int        MatOrderingRegisterDestroy(void);
EXTERN int        MatOrderingRegisterAll(const char[]);
extern PetscTruth MatOrderingRegisterAllCalled;
extern PetscFList MatOrderingList;

EXTERN int MatReorderForNonzeroDiagonal(Mat,PetscReal,IS,IS);

/*S 
   MatFactorInfo - Data based into the matrix factorization routines

   In Fortran these are simply double precision arrays of size MAT_FACTORINFO_SIZE

   Notes: These are not usually directly used by users, instead use PC type of LU, ILU, CHOLESKY or ICC.

   Level: developer

.seealso: MatLUFactorSymbolic(), MatILUFactorSymbolic(), MatCholeskyFactorSymbolic(), MatICCFactorSymbolic(), MatICCFactor()

S*/
typedef struct {
  PetscReal     damping;        /* scaling of identity added to matrix to prevent zero pivots */
  PetscReal     shift;          /* if true, shift until positive pivots */
  PetscReal     shift_fraction; /* record shift fraction taken */
  PetscReal     diagonal_fill;  /* force diagonal to fill in if initially not filled */
  PetscReal     dt;             /* drop tolerance */
  PetscReal     dtcol;          /* tolerance for pivoting */
  PetscReal     dtcount;        /* maximum nonzeros to be allowed per row */
  PetscReal     fill;           /* expected fill; nonzeros in factored matrix/nonzeros in original matrix*/
  PetscReal     levels;         /* ICC/ILU(levels) */ 
  PetscReal     pivotinblocks;  /* for BAIJ and SBAIJ matrices pivot in factorization on blocks, default 1.0 
                                   factorization may be faster if do not pivot */
  PetscReal     zeropivot;      /* pivot is called zero if less than this */
} MatFactorInfo;

EXTERN int MatCholeskyFactor(Mat,IS,MatFactorInfo*);
EXTERN int MatCholeskyFactorSymbolic(Mat,IS,MatFactorInfo*,Mat*);
EXTERN int MatCholeskyFactorNumeric(Mat,Mat*);
EXTERN int MatLUFactor(Mat,IS,IS,MatFactorInfo*);
EXTERN int MatILUFactor(Mat,IS,IS,MatFactorInfo*);
EXTERN int MatLUFactorSymbolic(Mat,IS,IS,MatFactorInfo*,Mat*);
EXTERN int MatILUFactorSymbolic(Mat,IS,IS,MatFactorInfo*,Mat*);
EXTERN int MatICCFactorSymbolic(Mat,IS,MatFactorInfo*,Mat*);
EXTERN int MatICCFactor(Mat,IS,MatFactorInfo*);
EXTERN int MatLUFactorNumeric(Mat,Mat*);
EXTERN int MatILUDTFactor(Mat,MatFactorInfo*,IS,IS,Mat *);
EXTERN int MatGetInertia(Mat,int*,int*,int*);
EXTERN int MatSolve(Mat,Vec,Vec);
EXTERN int MatForwardSolve(Mat,Vec,Vec);
EXTERN int MatBackwardSolve(Mat,Vec,Vec);
EXTERN int MatSolveAdd(Mat,Vec,Vec,Vec);
EXTERN int MatSolveTranspose(Mat,Vec,Vec);
EXTERN int MatSolveTransposeAdd(Mat,Vec,Vec,Vec);
EXTERN int MatSolves(Mat,Vecs,Vecs);

EXTERN int MatSetUnfactored(Mat);

/*E
    MatSORType - What type of (S)SOR to perform

    Level: beginner

   May be bitwise ORd together

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

   MatSORType may be bitwise ORd together, so do not change the numbers 

.seealso: MatRelax(), MatPBRelax()
E*/
typedef enum {SOR_FORWARD_SWEEP=1,SOR_BACKWARD_SWEEP=2,SOR_SYMMETRIC_SWEEP=3,
              SOR_LOCAL_FORWARD_SWEEP=4,SOR_LOCAL_BACKWARD_SWEEP=8,
              SOR_LOCAL_SYMMETRIC_SWEEP=12,SOR_ZERO_INITIAL_GUESS=16,
              SOR_EISENSTAT=32,SOR_APPLY_UPPER=64,SOR_APPLY_LOWER=128} MatSORType;
EXTERN int MatRelax(Mat,Vec,PetscReal,MatSORType,PetscReal,int,int,Vec);
EXTERN int MatPBRelax(Mat,Vec,PetscReal,MatSORType,PetscReal,int,int,Vec);

/* 
    These routines are for efficiently computing Jacobians via finite differences.
*/

/*E
    MatColoringType - String with the name of a PETSc matrix coloring or the creation function
       with an optional dynamic library name, for example 
       http://www.mcs.anl.gov/petsc/lib.a:coloringcreate()

   Level: beginner

.seealso: MatGetColoring()
E*/
#define MatColoringType char*
#define MATCOLORING_NATURAL "natural"
#define MATCOLORING_SL      "sl"
#define MATCOLORING_LF      "lf"
#define MATCOLORING_ID      "id"

EXTERN int MatGetColoring(Mat,const MatColoringType,ISColoring*);
EXTERN int MatColoringRegister(const char[],const char[],const char[],int(*)(Mat,const MatColoringType,ISColoring *));

/*MC
   MatColoringRegisterDynamic - Adds a new sparse matrix coloring to the 
                               matrix package. 

   Synopsis:
   int MatColoringRegisterDynamic(char *name_coloring,char *path,char *name_create,int (*routine_create)(MatColoring))

   Not Collective

   Input Parameters:
+  sname - name of Coloring (for example MATCOLORING_SL)
.  path - location of library where creation routine is 
.  name - name of function that creates the Coloring type, a string
-  function - function pointer that creates the coloring

   Level: developer

   If dynamic libraries are used, then the fourth input argument (function)
   is ignored.

   Sample usage:
.vb
   MatColoringRegisterDynamic("my_color",/home/username/my_lib/lib/libO/solaris/mylib.a,
               "MyColor",MyColor);
.ve

   Then, your partitioner can be chosen with the procedural interface via
$     MatColoringSetType(part,"my_color")
   or at runtime via the option
$     -mat_coloring_type my_color

   $PETSC_ARCH and $BOPT occuring in pathname will be replaced with appropriate values.

.keywords: matrix, Coloring, register

.seealso: MatColoringRegisterDestroy(), MatColoringRegisterAll()
M*/
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define MatColoringRegisterDynamic(a,b,c,d) MatColoringRegister(a,b,c,0)
#else
#define MatColoringRegisterDynamic(a,b,c,d) MatColoringRegister(a,b,c,d)
#endif

EXTERN int        MatColoringRegisterAll(const char[]);
extern PetscTruth MatColoringRegisterAllCalled;
EXTERN int        MatColoringRegisterDestroy(void);
EXTERN int        MatColoringPatch(Mat,int,int,const ISColoringValue[],ISColoring*);

/*S
     MatFDColoring - Object for computing a sparse Jacobian via finite differences
        and coloring

   Level: beginner

  Concepts: coloring, sparse Jacobian, finite differences

.seealso:  MatFDColoringCreate()
S*/
typedef struct _p_MatFDColoring *MatFDColoring;

EXTERN int MatFDColoringCreate(Mat,ISColoring,MatFDColoring *);
EXTERN int MatFDColoringDestroy(MatFDColoring);
EXTERN int MatFDColoringView(MatFDColoring,PetscViewer);
EXTERN int MatFDColoringSetFunction(MatFDColoring,int (*)(void),void*);
EXTERN int MatFDColoringSetParameters(MatFDColoring,PetscReal,PetscReal);
EXTERN int MatFDColoringSetFrequency(MatFDColoring,int);
EXTERN int MatFDColoringGetFrequency(MatFDColoring,int*);
EXTERN int MatFDColoringSetFromOptions(MatFDColoring);
EXTERN int MatFDColoringApply(Mat,MatFDColoring,Vec,MatStructure*,void *);
EXTERN int MatFDColoringApplyTS(Mat,MatFDColoring,PetscReal,Vec,MatStructure*,void *);
EXTERN int MatFDColoringSetRecompute(MatFDColoring);
EXTERN int MatFDColoringSetF(MatFDColoring,Vec);
EXTERN int MatFDColoringGetPerturbedColumns(MatFDColoring,int*,int*[]);
/* 
    These routines are for partitioning matrices: currently used only 
  for adjacency matrix, MatCreateMPIAdj().
*/

/*S
     MatPartitioning - Object for managing the partitioning of a matrix or graph

   Level: beginner

  Concepts: partitioning

.seealso:  MatPartitioningCreate(), MatPartitioningType
S*/
typedef struct _p_MatPartitioning *MatPartitioning;

/*E
    MatPartitioningType - String with the name of a PETSc matrix partitioning or the creation function
       with an optional dynamic library name, for example 
       http://www.mcs.anl.gov/petsc/lib.a:partitioningcreate()

   Level: beginner

.seealso: MatPartitioningCreate(), MatPartitioning
E*/
#define MatPartitioningType char*
#define MAT_PARTITIONING_CURRENT  "current"
#define MAT_PARTITIONING_PARMETIS "parmetis"
#define MAT_PARTITIONING_CHACO    "chaco"
#define MAT_PARTITIONING_JOSTLE   "jostle"
#define MAT_PARTITIONING_PARTY    "party"
#define MAT_PARTITIONING_SCOTCH   "scotch"


EXTERN int MatPartitioningCreate(MPI_Comm,MatPartitioning*);
EXTERN int MatPartitioningSetType(MatPartitioning,const MatPartitioningType);
EXTERN int MatPartitioningSetNParts(MatPartitioning,int);
EXTERN int MatPartitioningSetAdjacency(MatPartitioning,Mat);
EXTERN int MatPartitioningSetVertexWeights(MatPartitioning,const int[]);
EXTERN int MatPartitioningSetPartitionWeights(MatPartitioning,const PetscReal []);
EXTERN int MatPartitioningApply(MatPartitioning,IS*);
EXTERN int MatPartitioningDestroy(MatPartitioning);

EXTERN int MatPartitioningRegister(const char[],const char[],const char[],int(*)(MatPartitioning));

/*MC
   MatPartitioningRegisterDynamic - Adds a new sparse matrix partitioning to the 
   matrix package. 

   Synopsis:
   int MatPartitioningRegisterDynamic(char *name_partitioning,char *path,char *name_create,int (*routine_create)(MatPartitioning))

   Not Collective

   Input Parameters:
+  sname - name of partitioning (for example MAT_PARTITIONING_CURRENT) or parmetis
.  path - location of library where creation routine is 
.  name - name of function that creates the partitioning type, a string
-  function - function pointer that creates the partitioning type

   Level: developer

   If dynamic libraries are used, then the fourth input argument (function)
   is ignored.

   Sample usage:
.vb
   MatPartitioningRegisterDynamic("my_part",/home/username/my_lib/lib/libO/solaris/mylib.a,
               "MyPartCreate",MyPartCreate);
.ve

   Then, your partitioner can be chosen with the procedural interface via
$     MatPartitioningSetType(part,"my_part")
   or at runtime via the option
$     -mat_partitioning_type my_part

   $PETSC_ARCH and $BOPT occuring in pathname will be replaced with appropriate values.

.keywords: matrix, partitioning, register

.seealso: MatPartitioningRegisterDestroy(), MatPartitioningRegisterAll()
M*/
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define MatPartitioningRegisterDynamic(a,b,c,d) MatPartitioningRegister(a,b,c,0)
#else
#define MatPartitioningRegisterDynamic(a,b,c,d) MatPartitioningRegister(a,b,c,d)
#endif

EXTERN int        MatPartitioningRegisterAll(const char[]);
extern PetscTruth MatPartitioningRegisterAllCalled;
EXTERN int        MatPartitioningRegisterDestroy(void);

EXTERN int MatPartitioningView(MatPartitioning,PetscViewer);
EXTERN int MatPartitioningSetFromOptions(MatPartitioning);
EXTERN int MatPartitioningGetType(MatPartitioning,MatPartitioningType*);

EXTERN int MatPartitioningParmetisSetCoarseSequential(MatPartitioning);

EXTERN int MatPartitioningJostleSetCoarseLevel(MatPartitioning,PetscReal);
EXTERN int MatPartitioningJostleSetCoarseSequential(MatPartitioning);

typedef enum { MP_CHACO_MULTILEVEL_KL, MP_CHACO_SPECTRAL, MP_CHACO_LINEAR, 
    MP_CHACO_RANDOM, MP_CHACO_SCATTERED } MPChacoGlobalType;
EXTERN int MatPartitioningChacoSetGlobal(MatPartitioning, MPChacoGlobalType);
typedef enum { MP_CHACO_KERNIGHAN_LIN, MP_CHACO_NONE } MPChacoLocalType;
EXTERN int MatPartitioningChacoSetLocal(MatPartitioning, MPChacoLocalType);
EXTERN int MatPartitioningChacoSetCoarseLevel(MatPartitioning,PetscReal);
typedef enum { MP_CHACO_LANCZOS, MP_CHACO_RQI_SYMMLQ } MPChacoEigenType;
EXTERN int MatPartitioningChacoSetEigenSolver(MatPartitioning,MPChacoEigenType);
EXTERN int MatPartitioningChacoSetEigenTol(MatPartitioning, PetscReal);
EXTERN int MatPartitioningChacoSetEigenNumber(MatPartitioning, int);

#define MP_PARTY_OPT "opt"
#define MP_PARTY_LIN "lin"
#define MP_PARTY_SCA "sca"
#define MP_PARTY_RAN "ran"
#define MP_PARTY_GBF "gbf"
#define MP_PARTY_GCF "gcf"
#define MP_PARTY_BUB "bub"
#define MP_PARTY_DEF "def"
EXTERN int MatPartitioningPartySetGlobal(MatPartitioning, const char*);
#define MP_PARTY_HELPFUL_SETS "hs"
#define MP_PARTY_KERNIGHAN_LIN "kl"
#define MP_PARTY_NONE "no"
EXTERN int MatPartitioningPartySetLocal(MatPartitioning, const char*);
EXTERN int MatPartitioningPartySetCoarseLevel(MatPartitioning,PetscReal);
EXTERN int MatPartitioningPartySetBipart(MatPartitioning,PetscTruth);
EXTERN int MatPartitioningPartySetMatchOptimization(MatPartitioning,PetscTruth);

typedef enum { MP_SCOTCH_GREEDY, MP_SCOTCH_GPS, MP_SCOTCH_GR_GPS } MPScotchGlobalType;
EXTERN int MatPartitioningScotchSetArch(MatPartitioning,const char*);
EXTERN int MatPartitioningScotchSetMultilevel(MatPartitioning);
EXTERN int MatPartitioningScotchSetGlobal(MatPartitioning,MPScotchGlobalType);
EXTERN int MatPartitioningScotchSetCoarseLevel(MatPartitioning,PetscReal);
EXTERN int MatPartitioningScotchSetHostList(MatPartitioning,const char*);
typedef enum { MP_SCOTCH_KERNIGHAN_LIN, MP_SCOTCH_NONE } MPScotchLocalType;
EXTERN int MatPartitioningScotchSetLocal(MatPartitioning,MPScotchLocalType);
EXTERN int MatPartitioningScotchSetMapping(MatPartitioning);
EXTERN int MatPartitioningScotchSetStrategy(MatPartitioning,char*);

/*
    If you add entries here you must also add them to finclude/petscmat.h
*/
typedef enum { MATOP_SET_VALUES=0,
               MATOP_GET_ROW=1,
               MATOP_RESTORE_ROW=2,
               MATOP_MULT=3,
               MATOP_MULT_ADD=4,
               MATOP_MULT_TRANSPOSE=5,
               MATOP_MULT_TRANSPOSE_ADD=6,
               MATOP_SOLVE=7,
               MATOP_SOLVE_ADD=8,
               MATOP_SOLVE_TRANSPOSE=9,
               MATOP_SOLVE_TRANSPOSE_ADD=10,
               MATOP_LUFACTOR=11,
               MATOP_CHOLESKYFACTOR=12,
               MATOP_RELAX=13,
               MATOP_TRANSPOSE=14,
               MATOP_GETINFO=15,
               MATOP_EQUAL=16,
               MATOP_GET_DIAGONAL=17,
               MATOP_DIAGONAL_SCALE=18,
               MATOP_NORM=19,
               MATOP_ASSEMBLY_BEGIN=20,
               MATOP_ASSEMBLY_END=21,
               MATOP_COMPRESS=22,
               MATOP_SET_OPTION=23,
               MATOP_ZERO_ENTRIES=24,
               MATOP_ZERO_ROWS=25,
               MATOP_LUFACTOR_SYMBOLIC=26,
               MATOP_LUFACTOR_NUMERIC=27,
               MATOP_CHOLESKY_FACTOR_SYMBOLIC=28,
               MATOP_CHOLESKY_FACTOR_NUMERIC=29,
               MATOP_SETUP_PREALLOCATION=30,
               MATOP_ILUFACTOR_SYMBOLIC=31,
               MATOP_ICCFACTOR_SYMBOLIC=32,
               MATOP_GET_ARRAY=33,
               MATOP_RESTORE_ARRAY=34,
               MATOP_DUPLCIATE=35,
               MATOP_FORWARD_SOLVE=36,
               MATOP_BACKWARD_SOLVE=37,
               MATOP_ILUFACTOR=38,
               MATOP_ICCFACTOR=39,
               MATOP_AXPY=40,
               MATOP_GET_SUBMATRICES=41,
               MATOP_INCREASE_OVERLAP=42,
               MATOP_GET_VALUES=43,
               MATOP_COPY=44,
               MATOP_PRINT_HELP=45,
               MATOP_SCALE=46,
               MATOP_SHIFT=47,
               MATOP_DIAGONAL_SHIFT=48,
               MATOP_ILUDT_FACTOR=49,
               MATOP_GET_BLOCK_SIZE=50,
               MATOP_GET_ROW_IJ=51,
               MATOP_RESTORE_ROW_IJ=52,
               MATOP_GET_COLUMN_IJ=53,
               MATOP_RESTORE_COLUMN_IJ=54,
               MATOP_FDCOLORING_CREATE=55,
               MATOP_COLORING_PATCH=56,
               MATOP_SET_UNFACTORED=57,
               MATOP_PERMUTE=58,
               MATOP_SET_VALUES_BLOCKED=59,
               MATOP_GET_SUBMATRIX=60,
               MATOP_DESTROY=61,
               MATOP_VIEW=62,
               MATOP_GET_MAPS=63,
               MATOP_USE_SCALED_FORM=64,
               MATOP_SCALE_SYSTEM=65,
               MATOP_UNSCALE_SYSTEM=66,
               MATOP_SET_LOCAL_TO_GLOBAL_MAPPING=67,
               MATOP_SET_VALUES_LOCAL=68,
               MATOP_ZERO_ROWS_LOCAL=69,
               MATOP_GET_ROW_MAX=70,
               MATOP_CONVERT=71,
               MATOP_SET_COLORING=72,
               MATOP_SET_VALUES_ADIC=73,
               MATOP_SET_VALUES_ADIFOR=74,
               MATOP_FD_COLORING_APPLY=75,
               MATOP_SET_FROM_OPTIONS=76,
               MATOP_MULT_CONSTRAINED=77,
               MATOP_MULT_TRANSPOSE_CONSTRAINED=78,
               MATOP_ILU_FACTOR_SYMBOLIC_CONSTRAINED=79,
               MATOP_PERMUTE_SPARSIFY=80,
               MATOP_MULT_MULTIPLE=81,
               MATOP_SOLVE_MULTIPLE=82
             } MatOperation;
EXTERN int MatHasOperation(Mat,MatOperation,PetscTruth*);
EXTERN int MatShellSetOperation(Mat,MatOperation,void(*)(void));
EXTERN int MatShellGetOperation(Mat,MatOperation,void(**)(void));
EXTERN int MatShellSetContext(Mat,void*);

/*
   Codes for matrices stored on disk. By default they are
 stored in a universal format. By changing the format with 
 PetscViewerSetFormat(viewer,PETSC_VIEWER_BINARY_NATIVE); the matrices will
 be stored in a way natural for the matrix, for example dense matrices
 would be stored as dense. Matrices stored this way may only be
 read into matrices of the same time.
*/
#define MATRIX_BINARY_FORMAT_DENSE -1

EXTERN int MatMPIBAIJSetHashTableFactor(Mat,PetscReal);
EXTERN int MatSeqAIJGetInodeSizes(Mat,int *,int *[],int *);
EXTERN int MatMPIRowbsGetColor(Mat,ISColoring *);

EXTERN int MatISGetLocalMat(Mat,Mat*);

/*S
     MatNullSpace - Object that removes a null space from a vector, i.e.
         orthogonalizes the vector to a subsapce

   Level: advanced

  Concepts: matrix; linear operator, null space

  Users manual sections:
.   sec_singular

.seealso:  MatNullSpaceCreate()
S*/
typedef struct _p_MatNullSpace* MatNullSpace;

EXTERN int MatNullSpaceCreate(MPI_Comm,int,int,const Vec[],MatNullSpace*);
EXTERN int MatNullSpaceDestroy(MatNullSpace);
EXTERN int MatNullSpaceRemove(MatNullSpace,Vec,Vec*);
EXTERN int MatNullSpaceAttach(Mat,MatNullSpace);
EXTERN int MatNullSpaceTest(MatNullSpace,Mat);

EXTERN int MatReorderingSeqSBAIJ(Mat A,IS isp);
EXTERN int MatMPISBAIJSetHashTableFactor(Mat,PetscReal);
EXTERN int MatSeqSBAIJSetColumnIndices(Mat,int *);

EXTERN int MatMatMult(Mat A,Mat B, Mat *C);
EXTERN int MatMatMultSymbolic(Mat A,Mat B,Mat *C);
EXTERN int MatMatMultNumeric(Mat A,Mat B,Mat C);

EXTERN int MatCreateMAIJ(Mat,int,Mat*);
EXTERN int MatMAIJRedimension(Mat,int,Mat*);
EXTERN int MatMAIJGetAIJ(Mat,Mat*);

EXTERN int MatComputeExplicitOperator(Mat,Mat*);

EXTERN int MatESISetType(Mat,const char*);
EXTERN int MatESISetFromOptions(Mat);

EXTERN int MatDiagonalScaleLocal(Mat,Vec);

EXTERN int PetscViewerMathematicaPutMatrix(PetscViewer, int, int, PetscReal *);
EXTERN int PetscViewerMathematicaPutCSRMatrix(PetscViewer, int, int, int *, int *, PetscReal *);

EXTERN int MatSeqAIJPtAP(Mat,Mat,Mat*);
EXTERN int MatSeqAIJPtAPSymbolic(Mat,Mat,Mat*);
EXTERN int MatSeqAIJPtAPNumeric(Mat,Mat,Mat);

PETSC_EXTERN_CXX_END
#endif
