/*
    *Authors: Will Belden Brown & Miles Chaffin
    *Project: LWP
    *Course: CPE/CSC-453-01
    *Instructor: Dr. Nico
    *Term: Fall 2018
    *Notes: primary files for LWP implementation
*/

#include "lwp.h"

/*---------------------------------------------------------------------------*/
/*GLOBAL's Space*/

/*global scheduler*/
scheduler *ROUND_ROBIN = NULL;

/*---------------------------------------------------------------------------*/
/*queue functions, maybe move to different file?*/

QNode newNode(thread new) {
    QNode temp = (struct QNode*)malloc(sizeof(struct QNode)); 
    temp->t = new; 
    temp->next = NULL; 
    temp->prev = NULL;
    return temp;
}

void enQueue(threadQueue tq, thread new) {
    //Create a new Qnode 
    QNode *temp = newNode(new); 
  
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

    QNode temp = threadQueue->head;

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
/*scheduler functions, maybe move to different file?*/

/*initialize any structs*/
void init(void) {
    createQueue(ROUND_ROBIN->tq);
}

/*tear down any structures*/
void shutdown(void) {

}


/* add a thread to the pool*/
void admit (thread new) {

}


/* remove a thread from the pool */
void remove(thread victim) {

}
 
/* select a thread to schedule*/ 
thread next(void) {
    return ROUND_ROBIN->tq->head->t;
}

/*create thread queue*/
void createQueue(threadQueue tq) {
    *tq = (struct threadQueue*)malloc(sizeof(struct threadQueue)); 
    tq->front = tq->rear = NULL;

    tq->newNode = newNode;
    tq->enQueue = enQueue;
    tq->deQueue = deQueue;   
}

/*---------------------------------------------------------------------------*/
/*thread functions*/

tid_t lwp_create(lwpfun,void *,size_t) {

}

void  lwp_exit(void) {

}

tid_t lwp_gettid(void) {

}

void  lwp_yield(void) {

}

void  lwp_start(void) {

}

void  lwp_stop(void) {

}

void  lwp_set_scheduler(scheduler fun) {

}

/*set ROUND_ROBIN*/
void  lwp_set_scheduler(scheduler fun) {
    ROUND_ROBIN = malloc(sizeof(struct scheduler));

    ROUND_ROBIN->init = init;
    ROUND_ROBIN->shutdown = shutdown;
    ROUND_ROBIN->admit = admit;
    ROUND_ROBIN->remove = remove;
    ROUND_ROBIN->next = next;
    ROUND_ROBIN->createQueue = createQueue;

    ROUND_ROBIN->init();
}

/*return active scheduler or print error if not set*/
scheduler lwp_get_scheduler(void) {
    if (ROUND_ROBIN)
        return ROUND_ROBIN;

    else {
        printf(stderr, "No scheduler set. Exiting...\n");
        return NULL;
    }
}