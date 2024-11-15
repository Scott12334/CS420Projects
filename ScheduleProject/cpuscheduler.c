#include<string.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct{
	int number;
	int arrivalTime;
	int cpuBurst;
	int remainingBurst;
	int priority;
}Job;
typedef struct Link{
	Job * value;
	struct Link * next;
	struct Link * prev;
}Link;
void readInQueue(Link ** end, FILE * inputFile);
void RR(Link ** head, Link ** end,int timeQuantum, int jobCount,FILE * outputFile);
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
		RR(&head,&end,number,numProcess,outputFile);
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
		newJob->remainingBurst = newJob->cpuBurst;
		newJob->priority= atoi(strtok(NULL," "));
		enqueue(end,newJob);
		//printf("%d %d %d %d\n", newJob->number, newJob->arrivalTime, newJob->cpuBurst, newJob->priority);
	}
}
void RR(Link ** head, Link ** end,int timeQuantum, int jobCount,FILE * outputFile){
	int time = 0;
	char * result = (char *)calloc(sizeof(char),100);
	double totalWaitTime = 0;
	while((*head)->next != *(end)){
		sprintf(result,"%d %d\n",time,(*head)->next->value->number);
		fwrite(result,sizeof(char),strlen(result),outputFile);
		//Pop off first queue object
		Job * nextJob = (dequeue(head))->value;
		if(nextJob->remainingBurst<= timeQuantum){
			//This is the time the job is finished
			time += nextJob->remainingBurst;
			totalWaitTime += time - nextJob->cpuBurst;
		}
		else{time += timeQuantum;}
		//Subtract time quantum from CPU burst
		nextJob->remainingBurst-= timeQuantum;
		//If CPU burst is > 0, add back to queue
		if(nextJob->remainingBurst > 0){enqueue(end,nextJob);}
	}
	sprintf(result,"AVG Waiting Time: %f",totalWaitTime/jobCount);
	fwrite(result,sizeof(char),strlen(result),outputFile);
}
void SJF(Link ** head, Link ** end, int jobCount, FILE * outputFile){
	int time = 0;
	double totalWaitTime = 0;
	char * result = (char *)calloc(sizeof(char),100);
	//Create sorted linked list of processes
	Link * sortedHead = sortedProcesses(head, jobCount);
	Link * storedHead = sortedHead;
	sortedHead = sortedHead->next;
	//Move through array, if the process has arrived by the time, run it, if not move to next shortest, add to time
	int jobsDone = 0;
	while(jobsDone < jobCount){
		int increaseTime = 0;
		sortedHead = storedHead->next;
		while(sortedHead != NULL){
			if(sortedHead->value->arrivalTime <= time){
				//Next job to run
				sprintf(result,"%d %d\n",time,sortedHead->value->number);
				fwrite(result,sizeof(char),strlen(result),outputFile);
				time += sortedHead->value->cpuBurst;
				totalWaitTime += time - sortedHead->value->cpuBurst - sortedHead->value->arrivalTime; 
				//Remove job, not at end
				if(sortedHead->next != NULL){
					sortedHead->prev->next = sortedHead->next;
					sortedHead->next->prev = sortedHead->prev;
				}else{
					sortedHead->prev->next = NULL;
				}
				jobsDone ++;
				increaseTime = 1;
				break;
			}
			sortedHead = sortedHead->next;
		}
		//If reach end and nothing has run, add one to time 
		if(increaseTime == 0){time++;}
	}
	sprintf(result,"AVG Waiting Time: %f",totalWaitTime/jobCount);
	fwrite(result,sizeof(char),strlen(result),outputFile);
}
void PR_noPREMP(){}
void PR_withPREMP(){}
Link * sortedProcesses(Link ** queueHead, int numProcesses){
	Link * sortedHead = (Link *)malloc(sizeof(Link));
	Link * storedHead = sortedHead;
	sortedHead->next = dequeue(queueHead);
	sortedHead->next->next = NULL;
	sortedHead->next->prev = sortedHead;
	for(int i = 1; i < numProcesses; i++){
		Link * nextLink = dequeue(queueHead);
		Link * lastLink;
		sortedHead = storedHead->next;
		int added = 0;
		//Loop through the new list until you either reach the end or find a spot where the cpu burst is less then the next one
		while(sortedHead != NULL){
			if(nextLink->value->cpuBurst < sortedHead->value->cpuBurst){
				sortedHead->prev->next = nextLink;
				nextLink->prev = sortedHead->prev;
				sortedHead->prev = nextLink;
				nextLink->next = sortedHead;
				added = 1;
				break;
			}
			lastLink = sortedHead;
			sortedHead = sortedHead->next;
		}
		if(added == 0){
			lastLink->next = nextLink;
			nextLink->prev = lastLink;
			nextLink->next = NULL;
		}
		sortedHead = storedHead->next;
	}
	sortedHead = storedHead;
	return sortedHead;
}
