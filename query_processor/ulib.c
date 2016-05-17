/*
  /-------------------------------------------------------------\
  |                     FILE : ulib.c                           |
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
  |  Modification by :  Bora Ucar                               |
  |  Date            :  20/4/1999                               |
  |  Description     :  Pool names added                        |
  \-------------------------------------------------------------/
  */

#include "ulib.h"

long _u_seed=847449649;
long _u_g_jobno=-1;

#define U_MB     (1024*1024)
#define MAXTABSPACELINE  300
#define TABSIZE          3



#ifdef _DEBUG_MALLOC_LOW

#define MAX_ALLOC_CNT    10000

typedef struct UmallocTTag
{
  pvoid  ptr;
  int    size;
  char   *msg;
} UmallocT;

typedef UmallocT    UmallocList[MAX_ALLOC_CNT];

static UmallocList __umallst;

static int __umallstcnt=0;

#endif

void uprintf(pchar f_str,...)
{
va_list argp;

fflush(stdout);
fflush(stderr);
va_start(argp, f_str);
vfprintf(stdout, f_str, argp);
va_end(argp);
fflush(stdout);
}

void iswap(pint p1, pint p2)
{
int t=*p1;
*p1 = *p2;
*p2 = t;
}

int minindexof(pint lst, int size)
{
int i, min;

min = 0;
for (i=1; i<size; i++)
    if (lst[i]<lst[min])
    min = i;
return min;
}

int maxindexof(pint lst, int size)
{
int i, max;

max = 0;
for (i=1; i<size; i++)
    if (lst[i]>lst[max])
    max = i;
return max;
}


int minof(pint lst, int size)
{
int i, min;

min = lst[0];
for (i=1; i<size; i++)
    if (lst[i]<min)
    min = lst[i];
return min;
}

int maxof(pint lst, int size)
{
int i, max;

max = lst[0];
for (i=1; i<size; i++)
    if (lst[i]>max)
    max = lst[i];
return max;
}

int sumof(pint lst, int size)
{
int i, sum=0;

for (i=0; i<size; i++)
    sum += lst[i];
return sum;
}



/*************************************************************************
 * --------------------------- Time ---------------------------------------
 **************************************************************************/

void u_cleartimer(ptimer tmr)
{
tmr->time = 0.0;
tmr->count = 0L;
tmr->status = TMR_CLEAR;
}

void u_addtimer(ptimer t, ptimer a)
{
t->time += a->time;
t->count++;
}

void u_starttimer(ptimer tmr)
{
if (tmr->status == TMR_START)
    errexit("Timer already started!\n");

tmr->time -= u_seconds();
tmr->count++;
tmr->status = TMR_START;
}


void u_stoptimer(ptimer tmr)
{
if (tmr->status != TMR_START)
    errexit("Timer has not been started!\n");

tmr->time += u_seconds();
tmr->status = TMR_STOP;
}


void u_printtimer(ptimer tmr, pchar msg)
{
if (tmr->status == TMR_START)
    errexit("Timing is in progress!\n");

printf("\n%s: %11.3lf", msg, tmr->time);
fflush(stdout);
}

double u_gettimer(ptimer tmr)
{
if (tmr->status == TMR_START)
    errexit("Timing is in progress!\n");

return tmr->time;
}

char *u_gettimestr(ptimer tmr)
{
static char st[20];
double h, m, s;


if (tmr->status == TMR_START)
    errexit("Timing is in progress!\n");

m = tmr->time / 60.0;
s = tmr->time - (int)m*60;
h = m / 60.0;
m = m - (int)h*60;

sprintf(st, "%d:%02d:%02d", (int) h, (int) m, (int) s);
return st;
}


double u_seconds(void)
{
#ifdef _SUN
#ifdef _USE_RUSAGE
double time;                         /* elapsed time in seconds */
struct rusage rusage;

getrusage(RUSAGE_SELF, &rusage);
time = ((rusage.ru_utime.tv_sec + rusage.ru_stime.tv_sec) +
    1.0e-6*(rusage.ru_utime.tv_usec + rusage.ru_stime.tv_usec));
return(time);
#else
return((double) clock()*1.0e-6);
#endif
#endif

#ifdef _DOS
return (double) clock() / (double) CLOCKS_PER_SEC;
#endif
}


void PrintRUsageInfo()
{
#ifdef _SUN
struct rusage rusage;

getrusage(RUSAGE_SELF, &rusage);

printf("%ld\t%ld\t%ld\t", rusage.ru_maxrss, rusage.ru_minflt, rusage.ru_majflt);
#endif

#ifdef _DOS
printf("N/A\tN/A\tN/A\t");
#endif
}


/*************************************************************************
 * --------------------------- File ---------------------------------------
 **************************************************************************/

FILE *ufopen(pchar fname, pchar mode, pchar msg)
{
FILE *fp;

fp = fopen(fname, mode);
if (fp != NULL)
    return fp;

errexit("file: %s, mode: %s, [%s]", fname, mode, msg);
return NULL;
}


void ufclose(FILE *fp)
{
if (fp)
    fclose(fp);
}

int uopen(pchar fname, int flags, pchar msg)
{
int fid;

fid = open(fname, flags);
if (fid != -1)
    return fid;

errexit("file: %s, flags: %x, [%s]", fname, flags, msg);
return -1;
}


void uclose(int fid)
{
close(fid);
}


/*************************************************************************
 * -------------------------- Memory --------------------------------------
 **************************************************************************/
pint imalloc(long size, pchar msg)
{
pint ptr;

ptr = (pint )umalloc(sizeof(int)*size, msg);
return ptr;
}

static long __u_tot_size=0;

#ifdef _DEBUG_MALLOC_LOW
static long __u_maxtot_size=0;
#endif


pvoid umalloc(long size, pchar msg)
{
pvoid ptr;
#ifdef _DEBUG_MALLOC_LOW
int i;

if (size<=0)
    errexit("umalloc: invalid alloc request in '%s' with size: %ld", msg, size);
#endif

if (size == 0)
    return NULL;

ptr = (pvoid) malloc(size);
if (!ptr)
    errexit("Memory allocation failed for '%s'.\nRequested size: %ld byte(s) [%.2lf MB]. Total allocated : %ld byte(s) [%.2lf MB]\n", msg, size, (double)size/(double)U_MB, __u_tot_size, (double)__u_tot_size/(double)U_MB);
__u_tot_size += size;

#ifdef _DEBUG_MALLOC_LOW
if (ptr<(pvoid)1000)
    errexit("Possible Memory allocation failed?? (ptr=%ld) for '%s'.\nRequested size: %ld byte(s). Total allocated : %ld \n", ptr, msg, size, __u_tot_size);

if (__u_tot_size>__u_maxtot_size)
    __u_maxtot_size = __u_tot_size;
for (i=0; i<__umallstcnt && __umallst[i].ptr ; i++);
if (i>=__umallstcnt)
    if (i>MAX_ALLOC_CNT)
    errexit("MAX_ALLOC_CNT not big enough");
    else
    __umallstcnt++;
__umallst[i].ptr = ptr;
__umallst[i].size = size;
__umallst[i].msg = strdup(msg);
#endif

return ptr;
}

pvoid urealloc(pvoid ptr, long size, pchar msg)
{
pvoid nptr;
#ifdef _DEBUG_MALLOC_LOW
int i;

for (i=0; i<__umallstcnt && __umallst[i].ptr!=ptr ; i++);
if (i>__umallstcnt)
    errexit("urealloc: ptr (%ld) not allocated before", ptr);
#endif

nptr = (pvoid) realloc(ptr, size);
if (!nptr)
    errexit("\nRe-alloc failure for '%s'.\nRequested size: %d byte(s).\nTotal allocated: %ld", msg, size, __u_tot_size);

#ifdef _DEBUG_MALLOC_LOW
__u_tot_size -= __umallst[i].size;
__u_tot_size += size;
__umallst[i].ptr = nptr;
__umallst[i].size = size;
free(__umallst[i].msg);
__umallst[i].msg = strdup(msg);
#endif
return nptr;
}

void ReportAllocMem(void)
{
#ifdef _DEBUG_MALLOC_LOW
int i, cnt=0;

for (i=0; i<__umallstcnt; i++)
    if (__umallst[i].ptr)
    cnt++;
uprintf("\n total %d (%d emptied out of %d) ptr allocated with size %dK (maxsize=%dK)", cnt, __umallstcnt-cnt, __umallstcnt,__u_tot_size/1024, __u_maxtot_size/1024);
#else
uprintf("\n Total allocated size = %ldK", __u_tot_size/1024);
#endif
}

#ifdef _DEBUG_MALLOC_LOW
void DetailedReportAllocMem()
{
int i, cnt=0;

uprintf("\n Allocations:");
for (i=0; i<__umallstcnt; i++)
    if (__umallst[i].ptr)
    {
    cnt += __umallst[i].size;
    uprintf("\n%3d. size=%6dK (CurTot:%6dK) %s", i, __umallst[i].size/1024, cnt/1024, __umallst[i].msg);
    /*
    uprintf("\n%3d. ptr=%9ld size=%6dK (CurTot:%6dK) %s", i, __umallst[i].ptr, __umallst[i].size/1024, __umallst[i].msg);
    */
    }
uprintf("\n---------------------------------------------");
if (cnt != __u_tot_size)
    errexit("DetailedReportAllocMem: cnt=%d tot_size=%d", cnt, __u_tot_size);
ReportAllocMem();
uprintf("\n---------------------------------------------");
}
#endif


void usfree(pvoid ptr)    /* Safe free checks if parameter is not null*/
{
if (ptr)
    {
#ifdef _DEBUG_MALLOC_LOW
    int i;

    for (i=0; i<__umallstcnt && __umallst[i].ptr!=ptr ; i++);
    if (i>__umallstcnt)
    errexit("usfree: ptr (%ld) not allocated before", ptr);
    uprintf(", %d", i);
    __u_tot_size -= __umallst[i].size;
    __umallst[i].ptr = 0;
    free(__umallst[i].msg);
#endif
    free(ptr);
    }
}

void ufree(pvoid ptr1,...)
{
va_list plist;
pvoid ptr;

va_start(plist, ptr1);

while ((ptr = va_arg(plist, pvoid )) != NULL)
    usfree(ptr);

va_end(plist);
}


void icopy(pint dest, pint src, long size)
{
if (!dest || !src)
    errexit("icopy : either src or dest is null");
else if (size<=0)
    return;
memcpy((pvoid )dest, (pvoid )src, sizeof(int)*size);
}

void dcopy(pdouble dest, pdouble src, long size)
{
if (!dest || !src)
    errexit("icopy : either src or dest is null");
else if (size<=0)
    return;
memcpy((pvoid )dest, (pvoid )src, sizeof(double)*size);
}

static AllocationPools _u_pools;

static AllocationPools _u_pools_check[100];
static int poolcheckcnt=0;

void SavePools2Check(void)
{
int i;
if (poolcheckcnt==100)
    errexit("poolcheck array is too small to save checks");
for (i=0; i<MAXALLOCPOOL; ++i)
    _u_pools_check[poolcheckcnt][i] = _u_pools[i];
++poolcheckcnt;
#ifdef _DEBUG_MEM
uprintf("\n ******** !!!!!!!!!!!!!!  Pools Saved !!!!!!!!!!! ********");
#endif
}

void CheckPools(void)
{
int i, cnt=0;
--poolcheckcnt;
if (poolcheckcnt<0)
    errexit("poolcheckcnt=%d", poolcheckcnt);
for (i=0; i<MAXALLOCPOOL; ++i)
    {
    if (_u_pools_check[poolcheckcnt][i].base != _u_pools[i].base)
    uprintf("\n ErrCnt:%4d in pool %d check-base=%x org-base=%x diff=%d", ++cnt, i, _u_pools_check[poolcheckcnt][i].base, _u_pools[i].base, _u_pools_check[poolcheckcnt][i].base-_u_pools[i].base);
    if (_u_pools_check[poolcheckcnt][i].ptr != _u_pools[i].ptr)
    uprintf("\n ErrCnt:%4d in pool %d check-ptr=%x org-ptr=%x diff=%d", ++cnt, i, _u_pools_check[poolcheckcnt][i].ptr, _u_pools[i].ptr, _u_pools_check[poolcheckcnt][i].ptr-_u_pools[i].ptr);
    if (_u_pools_check[poolcheckcnt][i].last != _u_pools[i].last)
    uprintf("\n ErrCnt:%4d in pool %d check-last=%x org-last=%x diff=%d", ++cnt, i, _u_pools_check[poolcheckcnt][i].last, _u_pools[i].last, _u_pools_check[poolcheckcnt][i].last-_u_pools[i].last);
    if (_u_pools_check[poolcheckcnt][i].lastptr != _u_pools[i].lastptr)
    uprintf("\n ErrCnt:%4d in pool %d check-lastptr=%x org-lastptr=%x diff=%d", ++cnt, i, _u_pools_check[poolcheckcnt][i].lastptr, _u_pools[i].lastptr, _u_pools_check[poolcheckcnt][i].lastptr-_u_pools[i].lastptr);
    }
if (cnt)
    errexit("pool check error");
}


void InitAllocationPools()
{
int i;
for (i=0; i<MAXALLOCPOOL; i++)
    _u_pools[i].isallocated = 0;
}

int  IsPoolAllocated(int pool)
{
return _u_pools[pool].isallocated;
}

void AllocatePool(int pool, long size, char *msg)
{
if (!IsPoolAllocated(pool))
    {
    _u_pools[pool].base = _u_pools[pool].ptr = (pchar) umalloc(size, msg);
    _u_pools[pool].last = _u_pools[pool].lastptr = _u_pools[pool].ptr + size;
    _u_pools[pool].isallocated = 1;
#ifdef _DEBUG_MEM
    _u_pools[pool].maxused = 0;
#endif
    }
}

void ReInitPools()
{
int pool;

for (pool=0; pool<MAXALLOCPOOL; pool++)
    if (IsPoolAllocated(pool))
    {
    _u_pools[pool].ptr = _u_pools[pool].base;
    _u_pools[pool].lastptr = _u_pools[pool].last;
#ifdef _DEBUG_MEM
    _u_pools[pool].maxused = 0;
#endif
    }
}

void ReportMaxPoolUsages(void)
{
#ifdef _DEBUG_MEM
int i;
uprintf("\n Max pool usages in Kbyte(s)");
for (i=0; i<MAXALLOCPOOL; i++)
     uprintf("\nPool[%d] = %6ld  of  %6ld  (%3ld%%)", i, _u_pools[i].maxused/1024,
     (_u_pools[i].last-_u_pools[i].base)/1024,
     100*_u_pools[i].maxused/(_u_pools[i].last-_u_pools[i].base));
uprintf("\n-------------------------\n");
#else
uprintf("\n----- Max usage not available--------\n");
#endif
}

void ShortReportMaxPoolUsages(void)
{
#ifdef _DEBUG_MEM
int i;
uprintf("\tMax Pool Usage (MB):");
for (i=0; i<MAXALLOCPOOL; i++)
     uprintf("\t%5.2lf", (double)_u_pools[i].maxused/(double)(1024*1024));
uprintf("\tMax Pool Usage %%:");
for (i=0; i<MAXALLOCPOOL; i++)
     uprintf("\t%5.2lf", 100.0*(double)_u_pools[i].maxused/(double)(_u_pools[i].last-_u_pools[i].base));
#else
uprintf("\t----- Max usage not available--------");
#endif
}


void FreePool(int pool)
{
if (IsPoolAllocated(pool))
    {
    usfree(_u_pools[pool].base);
    _u_pools[pool].isallocated = 0;
    }
}

void FreePools()
{
int i;
#ifdef _DEBUG_MEM
ShortReportMaxPoolUsages();
#endif
for (i=0; i<MAXALLOCPOOL; i++)
    FreePool(i);
}


static char *_g_pool_names[MAXALLOCPOOL] =
{
"CellLstPtr",
"NetLstPtr",
"Pins",
"Pins",
"InvMap",
"HyGr",
"PartVec",
"General",
"cellToNetPW",   /*Bora UCar*/
"netToCellPW"    /*Bora UCar*/
};

void CheckPool(int pool, char *msg)
{
if (_u_pools[pool].ptr>=_u_pools[pool].lastptr)
    {
    ShortReportMaxPoolUsages();
    errexit("%s : pool %d\n allocated %ld byte(s) but used %d (%d+%d) byte(s).\nPlease increase MemMul_%s parameter.", msg,
        pool, _u_pools[pool].last-_u_pools[pool].base, (_u_pools[pool].ptr-_u_pools[pool].base)+(_u_pools[pool].last-_u_pools[pool].lastptr), _u_pools[pool].ptr-_u_pools[pool].base, _u_pools[pool].last-_u_pools[pool].lastptr, _g_pool_names[pool]);
    }
#ifdef _DEBUG_MEM
used = (_u_pools[pool].ptr-_u_pools[pool].base)+(_u_pools[pool].last-_u_pools[pool].lastptr);
if (used > _u_pools[pool].maxused)
    _u_pools[pool].maxused = used;
#endif
}

pchar poolalloc(int pool, long size, char *msg)
{
pchar rp=_u_pools[pool].ptr;
#ifdef _DEBUG_MEM
long used;

if (!IsPoolAllocated(pool))
    errexit("Pool %d is not allocated yet", pool);
#endif
_u_pools[pool].ptr += size;
CheckPool(pool, msg);
return rp;
}

pchar poolalloclast(int pool, long size, char *msg)
{
#ifdef _DEBUG_MEM
long used;
if (!IsPoolAllocated(pool))
    errexit("Pool %d is not allocated yet", pool);
#endif
_u_pools[pool].lastptr -= size;
CheckPool(pool, msg);
return _u_pools[pool].lastptr;
}


int ispoolavailable(int pool, long size)
{
return _u_pools[pool].lastptr-_u_pools[pool].ptr>size;
}

void poolfree(int pool, long size, char *msg)
{
#ifdef _DEBUG_MEM
if (!IsPoolAllocated(pool))
    errexit("Pool %d is not allocated yet", pool);
if (size<0)
    errexit("%s : poolfree: pool=%d size=%d", msg, pool, size);
#endif
_u_pools[pool].ptr -= size;
}

void poolfreelast(int pool, long size, char *msg)
{
#ifdef _DEBUG_MEM
if (!IsPoolAllocated(pool))
    errexit("Pool %d is not allocated yet", pool);
if (size<0)
    errexit("%s : poolfree: pool=%d size=%d", msg, pool, size);
#endif
_u_pools[pool].lastptr += size;
}


pchar poolgetptr(int pool)
{
#ifdef _DEBUG_MEM
if (!IsPoolAllocated(pool))
    errexit("Pool %d is not allocated yet", pool);
#endif
return _u_pools[pool].ptr;
}

void  poolsetptr(int pool, pchar ptr)
{
#ifdef _DEBUG_MEM
if (!IsPoolAllocated(pool))
    errexit("Pool %d is not allocated yet", pool);
#endif
_u_pools[pool].ptr = ptr;
}


void poolfree_upto(int pool, pchar ptr, char *msg)
{
#ifdef _DEBUG_MEM
long size=_u_pools[pool].ptr-ptr;

if (!IsPoolAllocated(pool))
    errexit("Pool %d is not allocated yet", pool);

if (size<0)
    warning("%s : poolfree_upto invalid free size=%ld in pool %d", msg, size, pool);
#endif

if (ptr < _u_pools[pool].ptr)
    _u_pools[pool].ptr = ptr;
#ifdef _DEBUG_MEM
if (_u_pools[pool].ptr < _u_pools[pool].base)
    errexit("%s : poolfree_upto invalid free ptr < base in pool %d", msg, pool);
#endif
}


/*************************************************************************
* -------------------------- String --------------------------------------
**************************************************************************/

int iswhitespace(char c)
{
return c==' ' || c=='\t' || c=='\n' || c=='\r';
}

pchar ltrim(pchar src)
{
int     i;
pchar   p;

for (p=src, i=0; iswhitespace(*p); p++, i++);
if (i>0)
    {
    for (; *p; p++)
    *(p-i) = *p;
    *(p-i) = 0;
    }
return src;
}

pchar rtrim(pchar src)
{
pchar   p;
int     i=strlen(src)-1;

for (p=src+i; i>=0 && iswhitespace(*p); p--, i--)
    *p = 0;
return src;
}

pchar trim(pchar src)
{
return ltrim(rtrim(src));
}

pchar ustrcpy(pchar dest, pchar src)
{
if (!dest || !src) /* either one of them is null */
    return NULL;
else
    return (strcpy(dest, src));
}

pchar ustrncpy(pchar dest, pchar src, long n)
{
if (!dest || !src) /* either one of them is null */
    return NULL;
else
    return (strncpy(dest, src, n));
}

pchar ustrdup(pchar st)
{
pchar p;

if (!st)
    return NULL;
else
    {
    p = umalloc(strlen(st)+1, "ustrdup");
    ustrcpy(p, st);
    return p;
    }
}


pchar GetTabSpace(int tabcnt)
{
static char spaces[MAXTABSPACELINE];
int i;

for (i=0; i<tabcnt*TABSIZE; i++)
    spaces[i] = ' ';
spaces[i] = 0;
return spaces;
}

/*************************************************************************
* -------------------------- Sort ----------------------------------------
**************************************************************************/

void isorttwo(pint i, pint j)
{
int t=*i;

if (*i>*j)
    {
    *i = *j;
    *j = t;
    }
}


/*************************************************************************
 * This function compares 2 ints for sorting in inc order
 **************************************************************************/
static int incint(const void *v1, const void *v2)
{
return (*((pint )v1) - *((pint )v2));
}

/*************************************************************************
 * This function compares 2 ints for sorting in dec order
 **************************************************************************/
static int decint(const void *v1, const void *v2)
{
return (*((pint )v2) - *((pint )v1));
}

void iincsort(long n, pint list)
{
qsort((pvoid )list, (size_t)n, (size_t)sizeof(int), incint);
}


void idecsort(long n, pint list)
{
qsort((pvoid )list, (size_t)n, (size_t)sizeof(int), decint);
}



static int incitem(const void *v1, const void *v2)
{
return (((PSortItem)v1)->val - ((PSortItem)v2)->val);
}

static int decitem(const void *v1, const void *v2)
{
return (((PSortItem)v2)->val - ((PSortItem)v1)->val);
}

void SortDecItems(int size, PSortItem list)
{
qsort((pvoid )list, (size_t)size, (size_t)sizeof(SortItem), decitem);
}

void SortIncItems(int size, PSortItem list)
{
qsort((pvoid )list, (size_t)size, (size_t)sizeof(SortItem), incitem);
}


/*************************************************************************
* -------------------------- Error Exit ----------------------------------
**************************************************************************/
void errexit(pchar f_str,...)
{
va_list argp;

fflush(stdout);
fflush(stderr);
fprintf(stderr, "\n****** Error:\n");
va_start(argp, f_str);
vfprintf(stderr, f_str, argp);
va_end(argp);

fprintf(stderr," ******\n");
fflush(stderr);

#ifdef _SUN
if (_u_g_jobno!=-1)
    {
    char hostname[200], donefname[300];
    FILE *donef;

    gethostname(hostname, sizeof(hostname));
    sprintf(donefname, "DONE.%s", hostname);
    donef = fopen(donefname, "w");
    fprintf(donef, "%ld", _u_g_jobno);
    fclose(donef);
    }
#endif


exit(1);
}

void warning(pchar f_str,...)
{
va_list argp;

fflush(stdout);
fprintf(stderr, "\n*** Warning:");
va_start(argp, f_str);
vfprintf(stderr, f_str, argp);
va_end(argp);
fprintf(stderr, " ***\n");
fflush(stderr);
}


void udbgprint(pchar f_str,...)
{
int static cnt=0;
va_list argp;
FILE *ofp;

ofp = ufopen("UMIT-DBG.TXT", "a", "dbgprint");
if (cnt++==0)
    {
    time_t curtime;
    char    tst[26];

    time(&curtime);
    strcpy(tst, ctime(&curtime));
    tst[24] = 0;
    fprintf(ofp, "\n\n\n----------------- %s -----------------\n\n", tst);
    }
va_start(argp, f_str);
vfprintf(ofp, f_str, argp);
va_end(argp);
ufclose(ofp);
}


#define SWAP(a, b, tmp)  \
                 do {(tmp) = (a); (a) = (b); (b) = (tmp);} while(0)


void PermuteInPlace(pint a, int n)
{
#ifdef _USE_DRAND
int i, u, v, tmp;

if (n <= 4)
    return;

for (i=0; i<n; i+=16)
    {
    u = uRandom(n-4);
    v = uRandom(n-4);
    SWAP(a[v], a[u], tmp);
    SWAP(a[v+1], a[u+1], tmp);
    SWAP(a[v+2], a[u+2], tmp);
    SWAP(a[v+3], a[u+3], tmp);
    }

#else
int i, end = n / 4;

for (i=1; i<end; i++)
    {
    int j=uRandom(n-i);
    int t=a[j];
    a[j] = a[n-i];
    a[n-i] = t;
    }
#endif
}


int ispowerof2(int n)
{
int pwr=2;

while (n>pwr)
    pwr *= 2;
return n==pwr;
}
