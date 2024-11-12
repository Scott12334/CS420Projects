#include<string.h>
#include<stdio.h>
#include<stdlib.h>

void RR();
void SJF();
void PR_noPREMP();
void PR_withPREMP();
typedef struct{
	int number;
	int arrivalTime;
	int cpuBurst;
	int priority;
}Job;
typedef struct Link{
	Job * value;
	struct Link * next;
	struct Link * prev;
}Link;
int main(int argc, char * argv[]){
	FILE * inputFile = fopen("input.txt", "r");
	if(inputFile == NULL){
		printf("Error opening input file");
	}
	//13 is the max size of the schedule type +1 for terminating 0
	char * scheduleType = (char *)calloc(sizeof(char),13); 
	fgets(scheduleType,13,inputFile);
	if(strncmp("RR",scheduleType,2) == 0){
		char * numberString = strtok(scheduleType," ");
		numberString = strtok(NULL, " ");
		int number = atoi(numberString);
		printf("RR %d",number);
	}else if(strncmp("SJF",scheduleType,3) == 0){
		printf("SJF");
	}else if(strncmp("PR_noPREMP",scheduleType,10) == 0){
		printf("PR_noPREMP");
	}else if(strncmp("PR_withPREMP",scheduleType,12) == 0){
		printf("PR_withPREMP");
	}
	return 0;
}
void enqueue(Link ** end, Job * newJob){
	Link * newLink = (Link *)malloc(sizeof(Link));
	newLink->value = newJob;
	*end->prev->next = newLink;
	*end->prev = newLink;
}
Job * dequeue(Link ** head){
	Job * nextOut = *head->value;
	*head->next = *head->next->next;
	return nextOut;
}
void RR(){}
void SJF(){}
void PR_noPREMP(){}
void PR_withPREMP(){}
