/*
  /-------------------------------------------------------------\
  |                     File : ulib.h                           |
  +-------------------------------------------------------------+
  |           Written by : Umit V. CATALYUREK                   |
  +-------------------------------------------------------------+
  |           Date       : 27.9.95                              |
  +-------------------------------------------------------------+
  |  Description : Some useful library functions, such as;      |
  |                                                             |
  |     1. File open/close                                      |
  |     2. Timer functions                                      |
  |     3. Safe memory allocation functions                     |
  |     4. Sorting functions                                    |
  |     5. Safe string functions                                |
  |     6. Nice error exit function                             |
  +-------------------------------------------------------------+
  |  Modification by :     Bora Ucar                            |
  |  Date            :     20/4/1999                            |
  |  Description     :     two pools added.                     |
  \-------------------------------------------------------------/
*/
#ifndef _ulib_H_
#define _ulib_H_

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <fcntl.h>

/*
#define _DEBUG_MEM
#define _DEBUG_MALLOC_LOW
*/

/* Do not FORGET to comment other operating systems. */

#define _SUN

//#define _DOS

#ifdef _SUN
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

#ifdef _DOS
#include <io.h>
#include <time.h>
#include <dos.h>
#endif

/*************************************************************************
* --------------------- Some Useful Types --------------------------------
**************************************************************************/
typedef unsigned short int ussint;
typedef ussint         *pussint;
typedef short int      sint;
typedef sint           *psint;
typedef int        *pint;
typedef char           *pchar;
typedef float          *pfloat;
typedef double         *pdouble;
typedef void               *pvoid;


/*************************************************************************
* --------------------- Some Useful Macros -------------------------------
**************************************************************************/
/*
#ifndef NULL
#define NULL    0
#endif
*/

#define U_MAXINT        2000000000
#define u_fEPSILON  1e-6
#define u_dEPSILON  1e-12
#define NONE    0

//#define log2(x)     ((int) (log((double)(x)) / log(2.0)))
#define umax(a, b)  ((a) >= (b) ? (a) : (b))
#define umin(a, b)  ((a) >= (b) ? (b) : (a))
#define uabs(a)     (((a) < 0) ? (-(a)) : (a))
#define usqr(x)     ((x)*(x))
#define unearest(x) (((x)-floor(x) < (ceil(x) - (x))) ? floor(x) : ceil(x))

#define u_fNEARZERO(x)   (uabs(x) < u_fEPSILON)
#define u_dNEARZERO(x)   (uabs(x) < u_dEPSILON)


/*
#define _USE_DRAND
*/

#ifdef _USE_DRAND

#define usRandom(s)     srand48(s)
#define usRandomize     usRandom(time(NULL))


static long uRandom(long l)
{
return (long) (drand48()*(double)l);
}


static long uGetSeed(void)
{
return uRandom(U_MAXINT);
}

#else

extern long _u_seed;

#define usRandom(s)     _u_seed = s
#define usRandomize     usRandom(time(NULL))

static long uGetSeed(void)
{
return _u_seed;
}

static long uRandom(long l)
{
long   a = 16807;
long   m = 2147483647;
long   q = 127773;
long   r = 2836;
long   lo, hi, test;
double d;

lo = _u_seed % q;
hi = _u_seed / q;
test = (a*lo)-(r*hi);
if (test>0)
    _u_seed = test;
else
    _u_seed = test + m;
d = (double) ((double) _u_seed / (double) m);
return (long) (d*(double)l);
}
#endif

/*************************************************************************
* --------------------------- Useful -------------------------------------
**************************************************************************/
void Wait(void);
void uprintf(pchar fmt,...);
void iswap(pint p1, pint p2);
int minindexof(pint lst, int size);
int maxindexof(pint lst, int size);
int minof(pint lst, int size);
int maxof(pint lst, int size);
int sumof(pint lst, int size);


/*************************************************************************
* --------------------------- Time ---------------------------------------
**************************************************************************/

/* --------------------- Time : Constants ------------------------------ */

/* Cause to use getrusage function to get time */
#define _USE_RUSAGE


#define TMR_CLEAR   1
#define TMR_START   2
#define TMR_STOP    8

/* --------------------- Time : Data structures ------------------------ */

struct timer
    {
    int    status;    /* current status of timer look TMR_xxxx constants */
    double time;      /* time in seconds */
    long   count;     /* counts how many times the counter was started */
    };

typedef struct timer    timer;
typedef timer           *ptimer;

/* --------------------- Time : Functions ------------------------------ */

void u_cleartimer(ptimer);
void u_starttimer(ptimer);
void u_addtimer(ptimer t, ptimer a);
void u_stoptimer(ptimer);
void u_printtimer(ptimer, pchar msg);
char *u_gettimestr(ptimer tmr);
double u_gettimer(ptimer);
double u_seconds(void);

void PrintRUsageInfo(void);


/*************************************************************************
* --------------------------- File ---------------------------------------
**************************************************************************/

/* --------------------- File : Functions ------------------------------ */

FILE *ufopen(pchar fname, pchar mode, pchar msg);
void ufclose(FILE *);

int  uopen(pchar fname, int flags, pchar msg);
void uclose(int fid);

/*************************************************************************
* -------------------------- Memory --------------------------------------
**************************************************************************/

/* --------------------- Memory : Functions ---------------------------- */
void ReportAllocMem();
#ifdef _DEBUG_MALLOC_LOW
void DetailedReportAllocMem();
#endif
pint imalloc(long size, pchar msg);
pvoid umalloc(long size, pchar msg);
pvoid urealloc(pvoid ptr, long size, pchar msg);
void usfree(pvoid );    /* Safe free checks if parameter is not null*/
void ufree(pvoid ,...);  /* IMPORTANT : Last parameter must be NULL */
void icopy(pint dest, pint src, long size);
void dcopy(pdouble dest, pdouble src, long size);

/* allocation pool indicies */
#define CLST      0
#define NLST      1
#define CELLS     2
#define NETS      3
#define INVP      4
#define GR        5
#define PART      6
#define GENERAL   7
#define C2N_PINW  8           /* Bora Ucar: date 20/4/1999*/
#define N2C_PINW  9           /* Bora Ucar: date 20/4/1999*/

#define MAXALLOCPOOL    10    /* Bora Ucar: date 20/4/1999*/

typedef struct _AllocPoolInfo
    {
    pchar          base, ptr, last, lastptr;
    int            isallocated;
#ifdef _DEBUG_MEM
    long       maxused;
#endif
    } AllocPoolInfo;

typedef AllocPoolInfo AllocationPools[MAXALLOCPOOL];

void InitAllocationPools(void);
int  IsPoolAllocated(int pool);
void AllocatePool(int pool, long size, char *msg);
void ReInitPools(void);
void FreePool(int pool);
void FreePools(void);
void ReportMaxPoolUsages(void);
void ShortReportMaxPoolUsages(void);
void SavePools2Check(void);
void CheckPools(void);
pchar poolalloc(int pool, long size, char *msg);
pchar poolalloclast(int pool, long size, char *msg);
pchar poolgetptr(int pool);
void  poolsetptr(int pool, pchar ptr);
void poolfree(int pool, long size, char *msg);
void poolfreelast(int pool, long size, char *msg);
void poolfree_upto(int pool, pchar ptr, char *msg);
int ispoolavailable(int pool, long size);

/*************************************************************************
* -------------------------- String --------------------------------------
**************************************************************************/

/* --------------------- String : Functions ---------------------------- */
int iswhitespace(char c);
pchar ltrim(pchar src);
pchar rtrim(pchar src);
pchar trim(pchar src);
pchar ustrcpy(pchar dest, pchar src);
pchar ustrncpy(pchar dest, pchar src, long n);
pchar ustrdup(pchar st);
pchar GetTabSpace(int tabcnt);


/*************************************************************************
* -------------------------- Sort ----------------------------------------
**************************************************************************/

typedef struct TagSortItem
{
int id;
int val;
} SortItem;

typedef SortItem *PSortItem;


/* --------------------- Sort : Functions ---------------------------- */
void isorttwo(pint i, pint j);
void iincsort(long size, pint list);  /* Integer sort in increasing order */
void idecsort(long size, pint list);  /* Integer sort in decreasing order */

void SortDecItems(int size, PSortItem list);
void SortIncItems(int size, PSortItem list);

/*************************************************************************
* -------------------------- Error Exit ----------------------------------
**************************************************************************/

/* --------------------- Exit : Functions ---------------------------- */
void errexit(pchar fmt,...);
void warning(pchar f_str,...);
void udbgprint(pchar f_str,...);


/*************************************************************************
* -------------------------- Usefuls   ----------------------------------
**************************************************************************/

/* randomly permutes n integers stored in a[0]..a[n-1] */
void PermuteInPlace(pint a, int n);
int ispowerof2(int n);

#endif
