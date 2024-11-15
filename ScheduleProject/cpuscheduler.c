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
void readInQueue(Link ** end, FILE * inputFile);
void RR(Link ** head, Link ** end,int timeQuantum, FILE * outputFile);
void SJF(Link ** head, Link ** end, int jobCount, FILE * outputFile);
Link * sortedProcesses(Link ** queueHead, int numProcesses);
void PR_noPREMP();
void PR_withPREMP();
int main(int argc, char * argv[]){
	FILE * inputFile = fopen("input.txt", "r");
	FILE * outputFile = fopen("output.txt", "w");
	if(inputFile == NULL || outputFile == NULL){
		printf("Error opening input/output file");
	}
	//13 is the max size of the schedule type +1 for terminating 0
	char * scheduleType = (char *)calloc(sizeof(char),13); 
	fgets(scheduleType,13,inputFile);
	char * processCount = (char *)calloc(sizeof(char),5); 
	fgets(processCount,5,inputFile);
	int numProcess = atoi(processCount);

	Link * head = (Link *)malloc(sizeof(Link));
	Link * end = (Link *)malloc(sizeof(Link));
	head->next = end;
	end->prev = head;
	if(strncmp("RR",scheduleType,2) == 0){
		fwrite(scheduleType,sizeof(char),strlen(scheduleType),outputFile);
		char * numberString = strtok(scheduleType," ");
		numberString = strtok(NULL, " ");
		int number = atoi(numberString);
		readInQueue(&end, inputFile);
		RR(&head,&end,number,outputFile);
	}else if(strncmp("SJF",scheduleType,3) == 0){
		fwrite(scheduleType,sizeof(char),strlen(scheduleType),outputFile);
		readInQueue(&end, inputFile);
		SJF(&head, &end, numProcess,outputFile);
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
Link * dequeue(Link ** head){
	Link *nodeToRemove = (*head)->next;

    (*head)->next = nodeToRemove->next;
    nodeToRemove->next->prev = (*head);

	return nodeToRemove;
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
void RR(Link ** head, Link ** end,int timeQuantum, FILE * outputFile){
	int time = 0;
	char * result = (char *)calloc(sizeof(char),100);
	while((*head)->next != *(end)){
		sprintf(result,"%d %d\n",time,(*head)->next->value->number);
		fwrite(result,sizeof(char),strlen(result),outputFile);
		//Pop off first queue object
		Job * nextJob = (dequeue(head))->value;
		if(nextJob->cpuBurst < timeQuantum){time += nextJob->cpuBurst;}
		else{time += timeQuantum;}
		//Subtract time quantum from CPU burst
		nextJob->cpuBurst -= timeQuantum;
		//If CPU burst is > 0, add back to queue
		if(nextJob->cpuBurst > 0){enqueue(end,nextJob);}
	}
}
void SJF(Link ** head, Link ** end, int jobCount, FILE * outputFile){
	int time = 0;
	//Create sorted linked list of processes
	Link * sortedHead = sortedProcesses(head, jobCount);
	sortedHead = sortedHead->next;
	while(sortedHead->next != NULL){
		printf("%d", sortedHead->value->cpuBurst);
		sortedHead = sortedHead->next;
	}
	//Move through array, if the process has arrived by the time, run it, if not move to next shortest, add to time
	//If reach end and nothing has run, add one to time 
}
void PR_noPREMP(){}
void PR_withPREMP(){}
Link * sortedProcesses(Link ** queueHead, int numProcesses){
	Link * sortedHead = (Link *)malloc(sizeof(Link));
	Link * storedHead = sortedHead;
	sortedHead->next = dequeue(queueHead);
	sortedHead->next->prev = sortedHead;
	for(int i = 1; i < numProcesses; i++){
		Link * nextLink = dequeue(queueHead);
		sortedHead = storedHead->next;
		//Loop through the new list until you either reach the end or find a spot where the cpu burst is less then the next one
		while(sortedHead->next != NULL){
			if(nextLink->value->cpuBurst < sortedHead->value->cpuBurst){
				sortedHead->prev->next = nextLink;
				nextLink->prev = sortedHead->prev;
				sortedHead->prev = nextLink;
				nextLink->next = sortedHead;
				break;
			}
			sortedHead = sortedHead->next;
			if(sortedHead->next == NULL){
				sortedHead->next = nextLink;
				nextLink->prev = sortedHead;
				break;
			}
		}
	}
	return sortedHead;
}
