/*
CS 420 
Assignment 2: Multithreading and Synchronization
Group 19 <- just your group number in this line
Section 2 <- just your section number
OSs Tested on: Kali Linux, Arch Linux - AMD64
cpu specs Arch
Architecture:             x86_64
  CPU op-mode(s):         32-bit, 64-bit
  Address sizes:          43 bits physical, 48 bits virtual
  Byte Order:             Little Endian
CPU(s):                   16
  On-line CPU(s) list:    0-15
Vendor ID:                AuthenticAMD
  Model name:             AMD Ryzen 7 2700X Eight-Core Processor
    CPU family:           23
    Model:                8
    Thread(s) per core:   2
    Core(s) per socket:   8
    Socket(s):            1
    Stepping:             2
    Frequency boost:      enabled
    CPU(s) scaling MHz:   59%
    CPU max MHz:          3700.0000
    CPU min MHz:          2200.0000
    BogoMIPS:             7403.73
    Flags:                fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflu
                          sh mmx fxsr sse sse2 ht syscall nx mmxext fxsr_opt pdpe1gb rdtscp lm constant
                          _tsc rep_good nopl nonstop_tsc cpuid extd_apicid aperfmperf rapl pni pclmulqd
                          q monitor ssse3 fma cx16 sse4_1 sse4_2 movbe popcnt aes xsave avx f16c rdrand
                           lahf_lm cmp_legacy svm extapic cr8_legacy abm sse4a misalignsse 3dnowprefetc
                          h osvw skinit wdt tce topoext perfctr_core perfctr_nb bpext perfctr_llc mwait
                          x cpb hw_pstate ssbd ibpb vmmcall fsgsbase bmi1 avx2 smep bmi2 rdseed adx sma
                          p clflushopt sha_ni xsaveopt xsavec xgetbv1 clzero irperf xsaveerptr arat npt
                           lbrv svm_lock nrip_save tsc_scale vmcb_clean flushbyasid decodeassists pause
                          filter pfthreshold avic v_vmsave_vmload vgif overflow_recov succor smca sev s
                          ev_es
Virtualization features:
  Virtualization:         AMD-V
Caches (sum of all):
  L1d:                    256 KiB (8 instances)
  L1i:                    512 KiB (8 instances)
  L2:                     4 MiB (8 instances)
  L3:                     16 MiB (2 instances)
NUMA:
  NUMA node(s):           1
  NUMA node0 CPU(s):      0-15
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_SIZE 100000000
#define MAX_THREADS 16
#define RANDOM_SEED 8631
#define MAX_RANDOM_NUMBER 3000
#define NUM_LIMIT 9973

// Global variables
long gRefTime; //For timing
int gData[MAX_SIZE]; //The array that will hold the data

int gThreadCount; //Number of threads

struct sharedBlock{

	int gDoneThreadCount; //Number of threads that are done at a certain point. Whenever a thread is done, it increments this. Used with the semaphore-based solution
	volatile int gThreadProd[MAX_THREADS]; //The modular product for each array division that a single thread is responsible for
	volatile bool gThreadDone[MAX_THREADS]; //Is this thread done? Used when the parent is continually checking on child threads

	// Semaphores
	sem_t completed; //To notify parent that all threads have completed or one of them found a zero
	sem_t mutex; //Binary semaphore to protect the shared variable gDoneThreadCount
};
struct sharedBlock* block;

int SqFindProd(int size); //Sequential FindProduct (no threads) computes the product of all the elements in the array mod NUM_LIMIT
void *ThFindProd(void *param); //Thread FindProduct but without semaphores
void *ThFindProdWithSemaphore(void *param); //Thread FindProduct with semaphores
int ComputeTotalProduct(); // Multiply the division products to compute the total modular product 
void InitSharedVars();
void GenerateInput(int size, int indexForZero); //Generate the input array
void CalculateIndices(int arraySize, int thrdCnt, int indices[MAX_THREADS][3]); //Calculate the indices to divide the array into T divisions, one division per thread
int GetRand(int min, int max);//Get a random number between min and max
int InitShm();
//Timing functions
long GetMilliSecondTime(struct timeb timeBuf);
long GetCurrentTime(void);
void SetTime(void);
long GetTime(void);

int main(int argc, char *argv[]){
	pid_t pid[MAX_THREADS];
	pthread_attr_t attr[MAX_THREADS];
	int indices[MAX_THREADS][3];
	int i, indexForZero, arraySize, prod;

	// Code for parsing and checking command-line arguments
	if(argc != 3){
		fprintf(stderr, "Invalid number of arguments!\n");
		exit(-1);
	}
	if((arraySize = atoi(argv[1])) <= 0 || arraySize > MAX_SIZE){
		fprintf(stderr, "Invalid Array Size\n");
		exit(-1);
	}
	gThreadCount = atoi(argv[2]);
	if(gThreadCount > MAX_THREADS || gThreadCount <=0){
		fprintf(stderr, "Invalid Thread Count\n");
		exit(-1);
	}
	InitShm();
	//indexForZero=-1;
	GenerateInput(arraySize, indexForZero);
	CalculateIndices(arraySize, gThreadCount, indices);
	/* 
	Code to check that Calculate indices work
	for(int i =0; i < gThreadCount; i++){
		printf("Division %d: Start %d, End %d\n",indices[i][0],indices[i][1],indices[i][2]);
	}
	*/

	// Code for the sequential part
	SetTime();
	prod = SqFindProd(arraySize);
	printf("Sequential multiplication completed in %ld ms. Product = %d\n", GetTime(), prod);

	// Threaded with parent waiting for all child threads
	InitSharedVars();
	SetTime();

	// Write your code here
	// Initialize threads, create threads, and then let the parent wait for all threads using pthread_join
	// The thread start function is ThFindProd
	// Don't forget to properly initialize shared variables
	for(i = 0;i<gThreadCount;i++){
		pid[i]= fork();
		if(pid[i]<0)
		{
			fprintf(stderr, "Fork Failed\n");
			exit(1);
		}
		else if (pid[i]==0) {
			ThFindProd(indices[i]);
			exit(0);
		}
	}
	for(int i = 0;i<gThreadCount;i++){
		int status;
		waitpid(pid[i], &status, 0);
		if(status == 0){
		}else{
			printf("process failed\n");
			exit(1);
		}
	}



	prod = ComputeTotalProduct();
	printf("Threaded multiplication with parent waiting for all children completed in %ld ms. Product = %d\n", GetTime(), prod);

	// Multi-threaded with busy waiting (parent continually checking on child threads without using semaphores)
	InitSharedVars();
	SetTime();

	// Write your code here
	// Don't use any semaphores in this part
	// Initialize threads, create threads, and then make the parent continually check on all child threads
	// The thread start function is ThFindProd
	// Don't forget to properly initialize shared variables

	for(i = 0;i<gThreadCount;i++){
		pid[i]= fork();
		if(pid[i]<0)
		{
			fprintf(stderr, "Fork Failed\n");
			exit(1);
		}
		else if (pid[i]==0) {//end on child
			ThFindProd(indices[i]);
			exit(0);
		}
	}
	bool done = false;
	while(!done){
		done = true;
		for(int i =0;i<gThreadCount;i++){
			if(block->gThreadDone[i]==false){
				done = false;
			}
		}
	}

	prod = ComputeTotalProduct();
	printf("Threaded multiplication with parent continually checking on children completed in %ld ms. Product = %d\n", GetTime(), prod);

	// Multi-threaded with semaphores

	InitSharedVars();
	// Initialize your semaphores here
	sem_init(&(block->mutex), 1, 1);
	sem_init(&(block->completed), 1, 0);


	SetTime();

	// Write your code here
	// Initialize threads, create threads, and then make the parent wait on the "completed" semaphore
	// The thread start function is ThFindProdWithSemaphore
	// Don't forget to properly initialize shared variables and semaphores using sem_init

	for(i = 0;i<gThreadCount;i++){
		pid[i]= fork();
		if(pid[i]<0)
		{
			fprintf(stderr, "Fork Failed\n");
			exit(1);
		}
		else if (pid[i]==0) {
			ThFindProdWithSemaphore(indices[i]);
			exit(0);
		}
	}

	sem_wait(&(block->completed));
	for(int i = 0;i<gThreadCount;i++){
		kill(SIGTERM,pid[i]);
	}
	prod = ComputeTotalProduct();
	printf("Threaded multiplication with parent waiting on a semaphore completed in %ld ms. Min = %d\n", GetTime(), prod);
	sem_destroy(&(block->mutex));
	sem_destroy(&(block->completed));
}

int InitShm(){

	int fd = shm_open("OS_HW2EC_19",O_CREAT | O_RDWR,S_IRUSR | S_IWUSR);
	if(fd==-1){
		printf("error creating shared memory shm_open\n");
	}
	if(ftruncate(fd, sizeof(struct sharedBlock))==-1){
		printf("error creating shared memory ftruncate\n");
	}
	block = mmap(NULL,sizeof(struct sharedBlock),PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);



	return 0;
}
// Write a regular sequential function to multiply all the elements in gData mod NUM_LIMIT
// REMEMBER TO MOD BY NUM_LIMIT AFTER EACH MULTIPLICATION TO PREVENT YOUR PRODUCT VARIABLE FROM OVERFLOWING
int SqFindProd(int size) {
	int product = 1;
	for(int i = 0; i < size; i++){
		product *= gData[i];
		product = product % NUM_LIMIT;
	}
	return product;
}

// Write a thread function that computes the product of all the elements in one division of the array mod NUM_LIMIT
// REMEMBER TO MOD BY NUM_LIMIT AFTER EACH MULTIPLICATION TO PREVENT YOUR PRODUCT VARIABLE FROM OVERFLOWING
// When it is done, this function should store the product in gThreadProd[threadNum] and set gThreadDone[threadNum] to true
void* ThFindProd(void *param) {
	int threadNum = ((int*)param)[0];//load params according to CalculateIndicies
	int startIndex = ((int*)param)[1];
	int endIndex = ((int*)param)[2];
	int prod=1;

	for(int i = startIndex;i<endIndex;i++){
		prod *= gData[i];
		prod = prod % NUM_LIMIT;
	}
	block->gThreadProd[threadNum]=prod;
	block->gThreadDone[threadNum]=true;
}

// Write a thread function that computes the product of all the elements in one division of the array mod NUM_LIMIT
// REMEMBER TO MOD BY NUM_LIMIT AFTER EACH MULTIPLICATION TO PREVENT YOUR PRODUCT VARIABLE FROM OVERFLOWING
// When it is done, this function should store the product in gThreadProd[threadNum]
// If the product value in this division is zero, this function should post the "completed" semaphore
// If the product value in this division is not zero, this function should increment gDoneThreadCount and
// post the "completed" semaphore if it is the last thread to be done
// Don't forget to protect access to gDoneThreadCount with the "mutex" semaphore
void* ThFindProdWithSemaphore(void *param) {
	int threadNum = ((int*)param)[0];//load params according to CalculateIndicies
	int startIndex = ((int*)param)[1];
	int endIndex = ((int*)param)[2];
	int *arr = (int*)param;
	int prod=1;

	for(int i = startIndex;i<endIndex;i++){
		prod *= gData[i];
		prod = prod % NUM_LIMIT;
	}
	if(prod==0){//done if zero
		sem_post(&(block->completed));
	}
	block->gThreadProd[threadNum]=prod;

	sem_wait(&(block->mutex));//mutex protection
	block->gDoneThreadCount++;

	if(block->gDoneThreadCount==gThreadCount){//done if last thread
		sem_post(&(block->completed));
	}
	sem_post(&(block->mutex));

}

int ComputeTotalProduct() {
    int i, prod = 1;

	for(i=0; i<gThreadCount; i++)
	{
		prod *= block->gThreadProd[i];
		prod %= NUM_LIMIT;
	}

	return prod;
}

void InitSharedVars() {
	int i;

	for(i=0; i<gThreadCount; i++){
		block->gThreadDone[i] = false;
		block->gThreadProd[i] = 1;
	}
	block->gDoneThreadCount = 0;
}

// Write a function that fills the gData array with random numbers between 1 and MAX_RANDOM_NUMBER
// If indexForZero is valid and non-negative, set the value at that index to zero
void GenerateInput(int size, int indexForZero) {
	for(int i = 0; i < size; i++){
		gData[i] = GetRand(1, MAX_RANDOM_NUMBER);
	}
	if(indexForZero >= 0 && indexForZero < size){
		printf("zero here\n");
		gData[indexForZero] = 0;
	}
}

// Write a function that calculates the right indices to divide the array into thrdCnt equal divisions
// For each division i, indices[i][0] should be set to the division number i,
// indices[i][1] should be set to the start index, and indices[i][2] should be set to the end index
void CalculateIndices(int arraySize, int thrdCnt, int indices[MAX_THREADS][3]) {
	//Correctly rounds the division
	int count = (arraySize + (thrdCnt-1))/thrdCnt;
	int i = 0;
	for(i =0; i < thrdCnt-1; i++){
		indices[i][0] = i;
		indices[i][1] = 0 + (count*i);
		indices[i][2] = (count - 1) + (count*i);
	}
	indices[i][0] = i;
	indices[i][1] = 0 + (count*i);
	indices[i][2] = arraySize - 1;
}

// Get a random number in the range [x, y]
int GetRand(int x, int y) {
    int r = rand();
    r = x + r % (y-x+1);
    return r;
}

long GetMilliSecondTime(struct timeb timeBuf){
	long mliScndTime;
	mliScndTime = timeBuf.time;
	mliScndTime *= 1000;
	mliScndTime += timeBuf.millitm;
	return mliScndTime;
}

long GetCurrentTime(void){
	long crntTime=0;
	struct timeb timeBuf;
	ftime(&timeBuf);
	crntTime = GetMilliSecondTime(timeBuf);
	return crntTime;
}

void SetTime(void){
	gRefTime = GetCurrentTime();
}

long GetTime(void){
	long crntTime = GetCurrentTime();
	return (crntTime - gRefTime);
}

