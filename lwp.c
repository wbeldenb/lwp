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
unsigned int nexttid = 0;
thread activeThread = NULL;

/*global scheduler*/
scheduler GLOBAL_SCHEDULAR = NULL;

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
                tq->head == temp->next;

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
    GLOBAL_SCHEDULAR->createQueue(GLOBAL_SCHEDULAR->tq);
}

/*tear down any structures*/
void shutdown_RR(void) {
    free(GLOBAL_SCHEDULAR->tq);
    free(GLOBAL_SCHEDULAR);
}

/* add a thread to the pool*/
void admit_RR(thread new) {
    threadQueue temp = GLOBAL_SCHEDULAR->tq;
    temp->enQueue(temp, new);
}


/* remove a thread from the pool */
void remove_RR(thread victim) {
    threadQueue temp = GLOBAL_SCHEDULAR->tq;
    temp->deQueue(temp, victim);
}
 
/* select a thread to schedule */ 
thread next_RR(void) {
    return GLOBAL_SCHEDULAR->tq->head->t;
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

scheduler set_init_schedular_RR() {
	scheduler newSchedular;
    newSchedular = malloc(sizeof(struct scheduler));

    newSchedular->init = init_RR;
    newSchedular->shutdown = shutdown_RR;
    newSchedular->admit = admit_RR;
    newSchedular->remove = remove_RR;
    newSchedular->next = next_RR;
    newSchedular->createQueue = createQueue_RR;

    newSchedular->init();

    return newSchedular;
}

tid_t lwp_create(lwpfun function, void *argument, size_t stackSize) {
    void *stack = NULL;
    thread newThread;
    GLOBAL_SCHEDULAR = set_init_schedular_RR();

    /* allocate a stack for the LWP */
    if ((stack = malloc(stackSize * __WORDSIZE)) == NULL){
        perror("lwp_create");
        exit(EXIT_FAILURE);
    }

    uintptr_t sp = stack;
    sp += stackSize * __WORDSIZE;
    uintptr_t bsp = sp;
    /* create a stack frame for the LWP */
    /* putting the arguments on first, not sure if that's correct or not */
    /* I reckon that lwp_start is just gonna yank this stuff off anyways so who cares */
    stack[sp] = argument;
    sp += sizeof(void *);
    stack[sp] = function;


    /* store the thread in a struct and put it in the scheduler */
    if (newThread = malloc(sizeof(struct threadinfo_st)) == NULL){
        perror("lwp_create");
        exit(EXIT_FAILURE);
    }

    newThread->tid = nexttid;
    nexttid++;
    newThread->stack = bsp;
    newThread->stacksize = stackSize;
    /* lib_one, lib_two, sched_one, sched_two are undefined and can be used later */
    admit(newThread);

    return newThread->tid;
}

void  lwp_exit(void) {
/* TODO: decrement nexttid */
}

tid_t lwp_gettid(void) {
    if (activeThread == NULL){
        return NULL;
    } else {
        return activeThread->tid;
    }
}

void  lwp_yield(void) {
    
}

void  lwp_start(void) {

}

void  lwp_stop(void) {

}

/*set new schedular*/
void  lwp_set_scheduler(scheduler fun) {
	scheduler newSchedular;

	if (!fun)
		newSchedular = set_init_schedular_RR();

	else {
		newSchedular = fun;
		fun->init();
	}
	
    thread temp = GLOBAL_SCHEDULAR->next();

    /*move all thread from current schedular to new*/
    while (temp) {
    	GLOBAL_SCHEDULAR->remove(temp);
    	newSchedular->admit(temp);
    	temp = GLOBAL_SCHEDULAR->next();
    }

    GLOBAL_SCHEDULAR->shutdown();

    GLOBAL_SCHEDULAR = newSchedular;
}

/*return active scheduler or print error if not set*/
scheduler lwp_get_scheduler(void) {
    if (GLOBAL_SCHEDULAR)
        return GLOBAL_SCHEDULAR;

    else {
        fprintf(stderr, "No scheduler set. Exiting...\n");
        return NULL;
    }
}