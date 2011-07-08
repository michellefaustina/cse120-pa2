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
//		rrpt[i].valid = 0;


	}
	//SetSchedPolicy (FIFO);
	SetSchedPolicy (ROUNDROBIN);
        //SetSchedPolicy (PROPORTIONAL);
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

	if ( ! rrq[numProcs].valid ) {
		rrq[numProcs].valid = 1;
		rrq[numProcs].pid = pid;
//		Printf("just added pid %d\n",rrq[numProcs].pid);

		numProcs = (numProcs+1) % MAXPROCS;
	}


//	for (prev = MAXPROCS-1, i = 0, next = 1; i < MAXPROCS; prev++, i++, next++){
	for (i = 0; i < MAXPROCS; i++) {
		if (! proctab[i].valid) {
			proctab[i].valid = 1;
			proctab[i].pid = pid;

			//break;
			return (1);
		}
	}

/*
	if (i < MAXPROCS) {
		for (prev = i-1; prev % MAXPROCS != i; prev--) {
			if (rrpt[prev % MAXPROCS].valid) {
				rrpt[prev % MAXPROCS].pid = pid;
				break;
			}
		}
		for (next = i+1; next % MAXPROCS != prev % MAXPROCS+1; next++) {
			if (proctab[next%MAXPROCS].valid) {
				rrpt[i].valid = 1;
				rrpt[i].pid = proctab[next].pid;
				break;
			}
		}
	}
	else {
*/
		Printf ("Error in StartingProc: no free table entries\n");
		return (0);
/*
	}
	return (1);
*/
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
		//	qptr--;
		}
	}
	
	for (i = 0; i < MAXPROCS; i++) {
		if (s[i].valid && s[i].pid == pid) {
			s[i].valid = 0;
			sptr--;
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
	float m, r;
	int nextProc, nextInd;

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

		/* uses queue, pops from front */
		for (i = 0; i < MAXPROCS; i++) {
			if (q[i].valid) {
				return (q[i].pid);
			}
		}
		break;

	case LIFO:

		/* your code here */

		/* uses stack, pops from end */
//		i = sptr - 1;
		for (i = MAXPROCS - 1; i >= 0; i--) {
			if (s[i].valid) {
				return (s[i].pid);
			}
		}
		break;

	case ROUNDROBIN:

		/* your code here */
/*		
		if(rrq[curInd].valid && numProcs!=0) {
			curProc = rrq[curInd].pid;
			curInd = (curInd+1) % numProcs;
		//	Printf("numProcs is %d\n",numProcs);
			return curProc;
		}
		else {
			return 0;
		}	
*/


		
		for ( i = curInd, j = 0; j < MAXPROCS; i = (i+1) % MAXPROCS, j++) {


			if(rrq[i].valid) {
				curProc = rrq[i].pid;
				curInd = (i+1) % MAXPROCS;
			//	Printf("about to run p[%d] =  %d\n",i,curProc);
				return curProc;
			}
			

		}
			
		break;

	case PROPORTIONAL:

		/* your code here */
		//cputime++;
		//Printf("cputime %f\n",(float)cputime);

		m = 50.0; // min ratio

		// find the process with the lowest ratio, needs CPU most
		for(i = 0; i < MAXPROCS; i++) {
			if(proctab[i].valid) {
				proctab[i].acc++;

				r = proctab[i].util / proctab[i].promised;
			Printf("p[%d]=%d has u %f\n",i,proctab[i].pid,proctab[i].util);

//		Printf("p[%d]=%d has r %f\n",i,proctab[i].pid,r);

				if(r < m) {
					m = proctab[i].ratio;
					curInd = i;
//		Printf("p[%d]=%d has r %f\n",curInd,proctab[curInd].pid,r);
				}
			}
		}
		// n is the index of the process with the lowest current ratio
		if(proctab[curInd].valid) {
			// fraction of cpu time received
			
			proctab[curInd].use++;
	proctab[curInd].util = (float) proctab[curInd].use / (float) proctab[curInd].acc;
			
//	Printf("p[%d]=%d has u %f\n",curInd,proctab[curInd].pid,proctab[curInd].util);
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
//			Printf("proc %d is promised %f\n",pid,proctab[i].promised);
		}
	}

	return (0);
}
