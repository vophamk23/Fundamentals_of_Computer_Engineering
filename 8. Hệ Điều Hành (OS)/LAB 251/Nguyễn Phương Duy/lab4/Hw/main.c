#include "pthread_create.h"
#include <stdio.h>

#define MAX_THREAD 2

int count;
void* function (void* arg){
	int id = *(int*)(arg);
	printf("This is thread %d - count = %d\n",id,++count);
}

int main(int argc, char * argv[]){
	int tId[MAX_THREAD];
	count = 0;
	for(int i = 0; i < MAX_THREAD; i++){
		tId[i] = i;
		
	}
	for(int i = 0; i < MAX_THREAD; i++){
		pthread_create(&tId[i], function, &tId[i]);
	}
	getc(stdin);
	return 0;

}
