#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct Node{
	int number;
	struct Node* next;
};

int main(){
	//init
	FILE * txtFile;
	pid_t pid; 		//process id to create child process
	int numbersOf = 0; 	//divisible 3 and 2
	struct Node* head = (struct Node*)malloc(sizeof(struct Node));
	char buffer[1024];
	
	//read file
	txtFile = fopen("numbers.txt","r");
	
	if(txtFile == NULL) {
		printf("null file\n");
		free(head);
		return 0;
	}
	
	if(fgets(buffer,1024,txtFile)){
		head->number = atoi(buffer);
		head->next = NULL;
	}
	else{
		printf("null content\n");
		free(head);
		fclose(txtFile);
		return 0;
	}
	struct Node* tmp = head;
	
	while(fgets(buffer,1024,txtFile)){
		tmp->next = (struct Node*)malloc(sizeof(struct Node));
		tmp->next->number = atoi(buffer);
		tmp = tmp->next;
		tmp->next = NULL;			
	}
	fclose(txtFile);
	tmp = head;
	//create child process
	pid = fork();
	
	if(pid < 0){
		printf("ERROR fork\n");
		return 0;
	}
	else if (pid == 0){		//Child process
		while(tmp != NULL){
			if(tmp->number %3 == 0) numbersOf++;
			tmp = tmp->next;
		}
		printf("Child process : the numbers of integer that are divisible by 3 is ");	
	}
	else{				//parent process
		struct Node* tmp2 = head;
		while(tmp != NULL){
			if(tmp->number %2 == 0) numbersOf++;
			tmp= tmp->next;
		}
		printf("Parent process : the numbers of integer that are divisible by 2 is ");
	}
	printf("%d\n",numbersOf);
	while(head != NULL){
		struct Node* tmp1 = head->next;
		free(head);
		head = tmp1;
	}
	getc(stdin);
	return 0;
}
