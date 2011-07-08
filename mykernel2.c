/* mykernel2.c: your portion of the kernel (last modified 10/18/09)
 *
 *	Below are procedures that are called by other parts of the kernel.
 *	Your ability to modify the kernel is via these procedures.  You may
 *	modify the bodies of these procedures any way you wish (however,
 *	you cannot change the interfaces).
 * 
 */

#include "aux.h"
#include "sys.h"
#include "mykernel2.h"

//#define TIMERINTERVAL 100000	/* in ticks (tick = 10 msec) */
#define TIMERINTERVAL 1

/*	A sample process table.  You may change this any way you wish.
 */

static struct {
	int valid;		/* is this entry valid: 1 = yes, 0 = no */
	int pid;		/* process id (as provided by kernel) */
	float promised;
	float util;		/* fration of cpu time received */
	float ratio;
	int start;		/* for calculating total cpu time since starting */
	int acc;		/* accumulated cpu time since starting */
	int use;		/* counter for cpu time received */
} proctab[MAXPROCS];

static struct {
	int valid;
	int pid;
} q[MAXPROCS];

int qptr = 0;

static struct {
	int valid;
	int pid;
} s[MAXPROCS];

int sptr = 0;

static struct {
	int valid;
	int pid;
} rrq[MAXPROCS];

int numProcs;

int curInd;
int curProc;

int cputime;

float min;



/*	InitSched () is called when kernel starts up.  Initialize data
 *	structures (such as the process table) here.  Also, call any
 *	initialization procedures, such as SetSchedPolicy to set the
 *	scheduling policy (see sys.h) and SetTimer which sets the
 *	timer to interrupt after a specified number of ticks.
 */

void InitSched ()
{
	int i;
	curInd = 0;
	curProc = 0;
	numProcs = 0;
	min = 50.0;

	for (i = 0; i < MAXPROCS; i++) {
		proctab[i].valid = 0;
		proctab[i].util = 0.0;
		proctab[i].promised = 1.0;
		proctab[i].ratio = 1.0;

		proctab[i].acc = 0;
		proctab[i].use = 0;
	

		q[i].valid = 0;
		s[i].valid = 0;
	}
        SetSchedPolicy (PROPORTIONAL);
	SetTimer (TIMERINTERVAL);
}


/*	StartingProc (pid) is called by the kernel when the process
 *	identified by pid is starting.  This allows you to record the
 *	arrival of a new process in the process table, and allocate
 *	any resources (if necessary).  Returns 1 if successful, 0 otherwise.
 */

int StartingProc (pid)
	int pid;
{
	int i;
	int prev, next;

	/* put process into queue as soon as created */
	if ( ! q[qptr].valid ) {
		q[qptr].valid = 1;
		q[qptr].pid = pid;
		//qptr = (qptr+1) % MAXPROCS; 
		qptr++;
	}

	/* also add to stack */
	if ( ! s[sptr].valid ) {
		s[sptr].valid = 1;
		s[sptr].pid = pid;
		sptr++;
	}

	/* add to round robin queue */
	if ( ! rrq[numProcs].valid ) {
		rrq[numProcs].valid = 1;
		rrq[numProcs].pid = pid;
		numProcs = (numProcs+1) % MAXPROCS;
	}

	for (i = 0; i < MAXPROCS; i++) {
		if (! proctab[i].valid) {
			proctab[i].valid = 1;
			proctab[i].pid = pid;
			proctab[i].start = cputime;
			return (1);
		}
	}

	Printf ("Error in StartingProc: no free table entries\n");
	return (0);

}
			

/*	EndingProc (pid) is called by the kernel when the process
 *	identified by pid is ending.  This allows you to update the
 *	process table accordingly, and deallocate any resources (if
 *	necessary).  Returns 1 if successful, 0 otherwise.
 */


int EndingProc (pid)
	int pid;
{
	int i;


	for (i = 0; i < MAXPROCS; i++) {
		if (q[i].valid && q[i].pid == pid) {
			q[i].valid = 0;
		}
	}
	
	for (i = 0; i < MAXPROCS; i++) {
		if (s[i].valid && s[i].pid == pid) {
			s[i].valid = 0;
		}
	}

	for (i = 0; i < MAXPROCS; i++) {
		if (rrq[i].valid && rrq[i].pid == pid) {
			rrq[i].valid = 0;
			numProcs--;
		}
	}

	for (i = 0; i < MAXPROCS; i++) {
		if (proctab[i].valid && proctab[i].pid == pid) {
			proctab[i].valid = 0;
			return (1);
		}
	}

	Printf ("Error in EndingProc: can't find process %d\n", pid);
	return (0);
}


/*	SchedProc () is called by kernel when it needs a decision for
 *	which process to run next.  It calls the kernel function
 *	GetSchedPolicy () which will return the current scheduling policy
 *	which was previously set via SetSchedPolicy (policy). SchedProc ()
 *	should return a process id, or 0 if there are no processes to run.
 */

int SchedProc ()
{
	int i, j, n;
	float r, tmp, acc, mr;


	switch (GetSchedPolicy ()) {

	case ARBITRARY:

		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				return (proctab[i].pid);
			}
		}
		break;

	case FIFO:

		/* your code here */

		/* uses queue, pops from front (iterate in order) */
		for (i = 0; i < MAXPROCS; i++) {
			if (q[i].valid) {
				return (q[i].pid);
			}
		}
		break;

	case LIFO:

		/* your code here */
		
		/* uses stack, pops from end (go backwards from FIFO)*/
		for (i = MAXPROCS-1; i >= 0; i--) {
			if (s[i].valid) {
				return (s[i].pid);
			}
		}
		break;

	case ROUNDROBIN:

		/* your code here */

		/* uses a circular queue
		 * i is the index that cycles around, starting at the current process
		 * j ensures that the loop ends at after MAXPROCS iterations
		 */
		for ( i = curInd, j = 0; j < MAXPROCS; i = (i+1) % MAXPROCS, j++) {
			if(rrq[i].valid) {
				curProc = rrq[i].pid;
				curInd = (i+1) % MAXPROCS;
				return curProc;
			}
		}
		break;

	case PROPORTIONAL:

		/* your code here */
		
		cputime++;

		

		mr = proctab[curInd].ratio;

		// find the process with the lowest utilization ratio (needs CPU most)
		for ( i = curInd, j = 0; j < MAXPROCS; i = (i+1) % MAXPROCS, j++) { 
			if(proctab[i].valid) {
				// increment the number of total quantums passed
				// proctab[i].acc++;
				acc = (float) cputime - (float) proctab[i].start;

				// fraction of cpu time received
				tmp = (float) proctab[i].use / acc;
				proctab[i].util = tmp;

				// find the ratio of this process
				proctab[i].ratio = proctab[i].util / proctab[i].promised;

				if(proctab[i].ratio < mr) {
					mr = proctab[i].ratio;
					curInd = i;
				}
			}
		}

		// n is the index of the process with the lowest current ratio
		if(proctab[curInd].valid) {
			proctab[curInd].use++; // increment the number of quantums used
			return (proctab[curInd].pid);
		}
	

		break;

	}
	
	return (0);
}


/*	HandleTimerIntr () is called by the kernel whenever a timer
 *	interrupt occurs.r
 */

void HandleTimerIntr ()
{
	SetTimer (TIMERINTERVAL);
	DoSched ();
}

/*	MyRequestCPUrate (pid, m, n) is called by the kernel whenever a process
 *	identified by pid calls RequestCPUrate (m, n).  This is a request for
 *	a fraction m/n of CPU time, effectively running on a CPU that is m/n
 *	of the rate of the actual CPU speed.  m of every n quantums should
 *	be allocated to the calling process.  Both m and n must be greater
 *	than zero, and m must be less than or equal to n.  MyRequestCPUrate
 *	should return 0 if successful, i.e., if such a request can be
 *	satisfied, otherwise it should return -1, i.e., error (including if
 *	m < 1, or n < 1, or m > n).  If MyRequestCPUrate fails, it should
 *	have no effect on scheduling of this or any other process, i.e., as
 *	if it were never called.
 */

int MyRequestCPUrate (pid, m, n)
	int pid;
	int m;
	int n;
{
	/* your code here */
	
	// error check
	if (m < 1 || n < 1 || m > n) {
		return (-1);
	}

	/* allocate m/n quantums to pid */
	for(int i = 0; i < MAXPROCS; i++) {
		if(proctab[i].pid == pid && proctab[i].valid) {
			proctab[i].promised = (float)m / (float)n;
		}
	}

	return (0);
}
