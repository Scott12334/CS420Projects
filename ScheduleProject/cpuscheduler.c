#include<string.h>
#include<stdio.h>
#include<stdlib.h>

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
void readInQueue(Link ** head, FILE * inputFile);
void RR();
void SJF();
void PR_noPREMP();
void PR_withPREMP();
int main(int argc, char * argv[]){
	FILE * inputFile = fopen("input.txt", "r");
	if(inputFile == NULL){
		printf("Error opening input file");
	}
	//13 is the max size of the schedule type +1 for terminating 0
	char * scheduleType = (char *)calloc(sizeof(char),13); 
	fgets(scheduleType,13,inputFile);
	Link * head = (Link *)malloc(sizeof(Link));
	Link * end = (Link *)malloc(sizeof(Link));
	head->next = end;
	end->prev = head;
	if(strncmp("RR",scheduleType,2) == 0){
		char * numberString = strtok(scheduleType," ");
		numberString = strtok(NULL, " ");
		int number = atoi(numberString);
		printf("RR %d\n",number);
		readInQueue(&end, inputFile);
		RR(&head,&end,number);
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
	(*end)->prev->next = newLink;
	(*end)->prev = newLink;
	newLink->next = (*end);
}
Job * dequeue(Link ** head){
	Link *nodeToRemove = (*head)->next;
    Job *nextOut = nodeToRemove->value;

    (*head)->next = nodeToRemove->next;
    nodeToRemove->next->prev = (*head);

    free(nodeToRemove);  

	return nextOut;
}
void readInQueue(Link ** end, FILE * inputFile){
	char * currentLine = (char *)calloc(sizeof(char),15); 
	while(fgets(currentLine,15,inputFile)){
		printf("%s",currentLine);
		Job * newJob = (Job *)malloc(sizeof(Job));
		newJob->number = atoi(strtok(currentLine," "));
		newJob->arrivalTime = atoi(strtok(NULL," "));
		newJob->cpuBurst = atoi(strtok(NULL," "));
		newJob->priority= atoi(strtok(NULL," "));
		enqueue(end,newJob);
		//printf("%d %d %d %d\n", newJob->number, newJob->arrivalTime, newJob->cpuBurst, newJob->priority);
	}
}
void RR(Link ** head, Link ** end,int timeQuantum){
	int time = 0;
	while((*head)->next != *(end)){
		printf("%d %d\n",time,(*head)->next->value->number);
		//Pop off first queue object
		Job * nextJob = dequeue(head);
		if(nextJob->cpuBurst < timeQuantum){time += nextJob->cpuBurst;}
		else{time += timeQuantum;}
		//Subtract time quantum from CPU burst
		nextJob->cpuBurst -= timeQuantum;
		printf("%d\n",nextJob->cpuBurst);
		//If CPU burst is > 0, add back to queue
		if(nextJob->cpuBurst > 0){enqueue(end,nextJob);}
	}
}
void SJF(){}
void PR_noPREMP(){}
void PR_withPREMP(){}
