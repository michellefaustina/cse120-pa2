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
//	int arrival;	/* temporary - hopefully.... */
} proctab[MAXPROCS];

static struct {
	int valid;
	int pid;
} q[MAXPROCS];

int qptr = 0;
int sptr = 0;

typedef struct {
	int valid;
	int pid;
	struct Node *next;
} Node;

Node *head;
Node *tail;

static struct Node rrq[MAXPROCS];


int x;


/* Queue functions */

void initq()
{
	Node *n;
	x = 0;
	n->pid = x;
	n->next = &n;
	head = &n;
	tail = &n;
}

void push(pid) 
	int pid;
{
	Node *n;
	n->pid = pid;
	n->next = &n;
	tail->next = &n;
	tail = &n;

//	x = x % MAXPROCS;

//	rrq[x] = n;
//	x++;

	
}

int pop() 
{
	Node *tmp;
	tmp = *head;
	*head = head->next;
	return tmp;
}




/*	InitSched () is called when kernel starts up.  Initialize data
 *	structures (such as the process table) here.  Also, call any
 *	initialization procedures, such as SetSchedPolicy to set the
 *	scheduling policy (see sys.h) and SetTimer which sets the
 *	timer to interrupt after a specified number of ticks.
 */

void InitSched ()
{
	int i;

	initq();

	for (i = 0; i < MAXPROCS; i++) {
		proctab[i].valid = 0;
		q[i].valid = 0;
		rrq[i].valid = 0;
	}
	//SetSchedPolicy (LIFO);
	SetSchedPolicy (ROUNDROBIN);
        /* CHANGE HERE */
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

	push(pid);

	for (i = 0; i < MAXPROCS; i++){
		if (! proctab[i].valid) {
			proctab[i].valid = 1;
			proctab[i].pid = pid;

//			proctab[i].arrival = a;
//			a++;

			/* insert into queue */
			q[qptr].valid = 1;
			q[qptr].pid = pid;

			qptr++; /* increment queue pointer */
			sptr++; /* increment stack pointer */

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
//			Printf("q[%d].pid = %d ended\n",i,pid);
			q[i].valid = 0;
//			qptr--;
			sptr--;
		}
	}

	for (i = 0; i < MAXPROCS; i++) {
		if (rrq[i].valid && rrq[i].pid == pid) {
//			Printf("rrq[%d].pid = %d ended\n",i,pid);
//			rrq[i].valid = 0;
			pop(pid);
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
	int i, n, m;
	struct node *tmp;

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
/*
		for (i = 0; i < MAXPROCS; i++) {
			n = proctab[i].arrival;
			if (proctab[n].valid) {
				return (proctab[n].pid);
			}
		}
*/

		/* uses queue */
		for (i = 0; i < MAXPROCS; i++) {
			if (q[i].valid) {
				return (q[i].pid);
			}
		}
		break;

	case LIFO:

		/* your code here */
/*
		for (i = MAXPROCS - 1; i >=0; i--) {
			n = proctab[i].arrival;
			if (proctab[n].valid) {
				return (proctab[n].pid);
			}
		}
*/

		for (i = MAXPROCS - 1; i >= 0; i--) {
			if (q[i].valid) {
				sptr--;
				return (q[i].pid);
			}
		}
		break;

	case ROUNDROBIN:

		/* your code here */

		//run each program a little for TIMERINTERVAL

/*
		for (i = 0; i < MAXPROCS; i++) {
			i = x % MAXPROCS;
			HandleTimerIntr();
			if (q[head].valid) {
				Printf("calling q[%d].pid = %d\n",i,q[i].pid);
				
				push(q[head].pid);
				return (q[head].pid);
			}

		}
*/
		pop();
		HandleTimerInter();
		return (head->pid);

	
		
		break;

	case PROPORTIONAL:

		/* your code here */

		break;

	}
	
	return (0);
}


/*	HandleTimerIntr () is called by the kernel whenever a timer
 *	interrupt occurs.
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
	
	/* error check */
	if (m < 1 || n < 1 || m > n)
		return (-1);

	/* allocate m/n quantums to pid */
	

	return (0);
}
