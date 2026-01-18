#ifndef PTHREAD_CREATE
#define PTHREAD_CREATE
#include <linux/sched.h>

#define STACK_SIZE 4096

int pthread_create(int ∗threadid, void ∗(∗start_routine)(void∗), void ∗argument); {
	void ** child = (void ** ) malloc(STACK_SIZE);
	void * stackbase = child + STACK_SIZE;
	clone(start_routine,stackbase, CLONE_VM|CLONE_FILES, argument);
	return *threadid;
}


#endif
