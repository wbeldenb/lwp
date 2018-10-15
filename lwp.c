/*
    *Authors: Will Belden Brown & Miles Chaffin
    *Project: LWP
    *Course: CPE/CSC-453-01
    *Instructor: Dr. Nico
    *Term: Fall 2018
    *Notes: primary files for LWP implementation
*/

#include <stdio.h>
#include "lwp.h"

/*---------------------------------------------------------------------------*/
/*GLOBAL's Space*/
//id of the next thread to be created
unsigned int nexttid = 1;

//saves the active thread
thread activeThread = NULL;

//global queue to store threads
threadQueue GLOBAL_THREAD_QUEUE = NULL;

/*global scheduler*/
scheduler GLOBAL_SCHEDULER = NULL;

// Main system Thread
thread mainSystemThread;

/*---------------------------------------------------------------------------*/
/*queue functions, maybe move to different file?*/

QNode newNode(thread new) {
    QNode temp = (QNode)malloc(sizeof(QNode)); 
    temp->t = new; 
    temp->next = NULL; 
    temp->prev = NULL;
    return temp;
}

void enQueue(threadQueue tq, thread new) {
    //Create a new Qnode 
    QNode temp = newNode(new); 
  
    //If queue is empty, then new node is both the head and tail
    if (tq->tail == NULL)
       tq->head = tq->tail = temp;
  
    //Add the new node at the end of queue and change tail 
    else {
        tq->tail->next = temp; 
        tq->tail = temp; 
    }
}

void deQueue(threadQueue tq, thread victim) {
    if (tq->head == NULL)
        return;

    QNode temp = tq->head;

    /*go through queue until match is found*/
    while (temp != NULL) {
        if (temp->t->tid == victim->tid) {
            if (temp->prev)
                temp->prev->next = temp->next;

            if (temp->next)
                temp->next->prev = temp->prev;

            if (temp == tq->head)
                tq->head = temp->next;

            if(temp == tq->tail)
                tq->tail = temp->prev;

            break;
        }

        temp = temp->next;
    }
} 

/*---------------------------------------------------------------------------*/
/*RR scheduler functions, maybe move to different file?*/

/*initialize any structs*/
void init_RR(void) {
}

/*tear down any structures*/
void shutdown_RR(void) {
    free(GLOBAL_SCHEDULER);
}

/* add a thread to the pool*/
void admit_RR(thread new) {
    threadQueue temp = GLOBAL_THREAD_QUEUE;
    temp->enQueue(temp, new);
}


/* remove a thread from the pool */
void remove_RR(thread victim) {
    threadQueue temp = GLOBAL_THREAD_QUEUE;
    temp->deQueue(temp, victim);
}
 
/* select a thread to schedule */ 
thread next_RR(void) {
	thread temp = GLOBAL_THREAD_QUEUE->head->t;
	GLOBAL_THREAD_QUEUE->head = GLOBAL_THREAD_QUEUE->head->next;
	GLOBAL_THREAD_QUEUE->head->prev = NULL;
	remove_RR(temp);
	admit_RR(temp);
	return temp;
}

/*create thread queue*/
void createQueue_RR(threadQueue tq) {
    tq = (threadQueue)malloc(sizeof(threadQueue)); 
    tq->head = tq->tail = NULL;

    tq->newNode = newNode;
    tq->enQueue = enQueue;
    tq->deQueue = deQueue;   
}

/*---------------------------------------------------------------------------*/
/* thread functions */

scheduler set_init_scheduler_RR() {
	scheduler newScheduler;
    newScheduler = malloc(sizeof(struct scheduler));

    newScheduler->init = init_RR;
    newScheduler->shutdown = shutdown_RR;
    newScheduler->admit = admit_RR;
    newScheduler->remove = remove_RR;
    newScheduler->next = next_RR;

    newScheduler->init();

    return newScheduler;
}

// A simple function to take care of pushing things on to stacks
// puts the void* onto the stack and shifts to the sp down to compensate
uintptr_t stack_pusher(uintptr_t sp, void* pushMe){
    void** loc = (void **)sp;
    *loc = pushMe;
    sp -= sizeof(void *);
    return sp;
}

tid_t lwp_create(lwpfun function, void *argument, size_t stackSize) {
    void *stack = NULL;
    thread newThread;

    //if not scheduler, create one
    if(!GLOBAL_SCHEDULER)
    	GLOBAL_SCHEDULER = set_init_scheduler_RR();

    //if no thread list, create one
    if (!GLOBAL_THREAD_QUEUE)
   		createQueue_RR(GLOBAL_THREAD_QUEUE);

    /* allocate a stack for the LWP */
    if ((stack = malloc(stackSize * sizeof(unsigned long))) == NULL){
        perror("lwp_create");
        exit(EXIT_FAILURE);
    }

    uintptr_t sp = (uintptr_t)stack;
    sp += stackSize * sizeof(unsigned long);
    /* create a stack frame for the LWP */
    /* we're putting on lwp_exit, then the client function */
    /* this is so that when we swap to this, the client func gets called */
    /* and then when that returns it will return to lwp_exit() */
    sp = stack_pusher(sp, &lwp_exit);
    sp = stack_pusher(sp, function);


    /* store the thread in a struct and put it in the scheduler */
    if ((newThread = malloc(sizeof(struct threadinfo_st))) == NULL){
        perror("lwp_create");
        exit(EXIT_FAILURE);
    }

    newThread->tid = nexttid;
    nexttid++;
    newThread->stack = stack;
    newThread->stacksize = stackSize;
    newThread->state.fxsave = FPU_INIT;
    newThread->state.rbp = sp;
    newThread->state.rdi = (unsigned long) argument;
    /* lib_one, lib_two, sched_one, sched_two are undefined and can be used later */
    GLOBAL_SCHEDULER->admit(newThread);

    return newThread->tid;
}

void actuallyExit(thread oldThread, thread nextThread){
    // destroy the old thread
    free(oldThread->stack);
	free(oldThread);

	/*swap registers of next thread with active thread*/
	activeThread = nextThread;  
	swap_rfiles(&oldThread->state, &nextThread->state);
}


/*Terminates the current LWP and frees its resources. Calls sched->next()
  to get the next thread. If there are no other threads, 
  restores the original system thread.*/
void  lwp_exit(void) {
    thread oldThread, nextThread = GLOBAL_SCHEDULER->next(); 
	if(!nextThread)
		lwp_stop();

    oldThread = activeThread;
	/*finds the active thread in the scheduler and removes it,
	  then frees the thread*/
	GLOBAL_SCHEDULER->remove(oldThread);

    SetSP(mainSystemThread->state.rsp);
    actuallyExit(oldThread, nextThread);
}

tid_t lwp_gettid(void) {
    if (activeThread == NULL){
        return NO_THREAD;
    } else {
        return activeThread->tid;
    }
}

/*Yields control to another LWP. Which one depends on the scheduler. 
  Saves the current LWP’s context, picks the next one, restores
  that thread’s context, and returns.*/
void  lwp_yield(void) {
	thread oldThread, newThread = GLOBAL_SCHEDULER->next();
	if(!newThread)
		lwp_stop();

    // preform the thread swap
    oldThread = activeThread;
    activeThread = newThread;
    swap_rfiles(&oldThread->state, &newThread->state);
}

/* Starts the LWP system. Saves the original context (for lwp stop()
to use later), picks a LWP and starts it running. If there are no
LWPs, returns immediately */
void  lwp_start(void) {
	thread firstThread;

    firstThread = GLOBAL_SCHEDULER->next();
    if (firstThread == NULL){
        return;
    }
    // if we haven't created memory for the system's context, create it now.
    if (mainSystemThread == NULL){
        if ((mainSystemThread = malloc(sizeof(struct threadinfo_st))) == NULL){
            perror("lwp_start");
            exit(EXIT_FAILURE);
        }
    }

    mainSystemThread->state.fxsave = FPU_INIT;
    
    swap_rfiles(&(mainSystemThread->state), &(firstThread->state));
    // When lwp_stop is executed, it should return to right here
    return;
}

/* Stops the LWP system, restores the original stack pointer and returns
to that context. (Wherever lwp start() was called from.
lwp stop() does not destroy any existing contexts, and thread
processing will be restarted by a call to lwp start(). */
void  lwp_stop(void) {
    thread finalThread = activeThread;
    activeThread = NULL; // There isn't an active thread anymore
    swap_rfiles(&(finalThread->state), &(mainSystemThread->state));
}

/*set new scheduler*/
void  lwp_set_scheduler(scheduler fun) {
	scheduler newScheduler;

	/*if passed NULL, set default RR scheduler*/
	if (!fun)
		newScheduler = set_init_scheduler_RR();

	/*otherwise setup new scheduler*/
	else {
		newScheduler = fun;
		fun->init();
	}

    QNode temp = GLOBAL_THREAD_QUEUE->head;

    /*move all thread from current scheduler to new*/
    while (temp) {
    	GLOBAL_SCHEDULER->remove(temp->t);
    	newScheduler->admit(temp->t);
    	temp = temp->next;
    }

    GLOBAL_SCHEDULER->shutdown();

    GLOBAL_SCHEDULER = newScheduler;
}

/*return active scheduler or print error if not set*/
scheduler lwp_get_scheduler(void) {
    if (GLOBAL_SCHEDULER)
        return GLOBAL_SCHEDULER;

    else {
        fprintf(stderr, "No scheduler set.\n");
        return NULL;
    }
}

/*Returns the thread corresponding to the given thread ID, or
NULL if the ID is invalid*/
thread tid2thread(tid_t tid) {
	if (tid < 0)
		return NULL;

	QNode temp = GLOBAL_THREAD_QUEUE->head;

	while(temp) {
		if (temp->t->tid == tid)
			return temp->t;

		temp = temp->next;
	}

	return NULL;
}
