/*
    *Authors: Will Belden Brown & Miles Chaffin
    *Project: LWP
    *Course: CPE/CSC-453-01
    *Instructor: Dr. Nico
    *Term: Fall 2018
    *Notes: primary files for LWP implementation
*/

#include "lwp.h"

/*global scheduler*/
scheduler *ROUND_ROBIN = NULL;

/*---------------------------------------------------------------------------*/
/*queue functions, maybe move to different file?*/
QNode newNode(thread new) {

}

void enQueue(thread new) {

}

void deQueue(thread victim) {

} 

/*---------------------------------------------------------------------------*/
/*scheduler functions, maybe move to different file?*/

/*initialize any structs*/
void init(void) {

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

}

/*create thread queue*/
threadQueue createQueue(void) {

}

/*---------------------------------------------------------------------------*/
/*thread functions*/

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