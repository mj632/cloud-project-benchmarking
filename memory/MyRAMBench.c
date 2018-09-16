#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

//-----variables declarations
//#define DATA_BLOCK_SIZE 1000000000
#define ONE_GB 1000000000
#define ONE_US 1000000
#define ONE_MB 1000000
#define TEST_SIZE 100
#define FREQ 3
struct InputParams
{
	int threadCount;
	long int blockSize;
	char benchmarkType[4];
	char *msg;				//-----ONE_GB
};

long int packageCount;
int totalIter = 0;

//-----function declarations
void *read_write_data_sequential(void *arg);
void *read_write_data_random(void *arg);


//-----Main function
int main(int argc, char *argv[]) {
	
	FILE *inputValuesFP;
	struct InputParams *ips;
	double throughput[FREQ],latency[FREQ];
	float total_time_taken;
	double avg_throughput,avg_latency;
	avg_throughput = 0;
	avg_latency = 0;
	ips = malloc(sizeof(struct InputParams));
	if(argc>1)
	{
		inputValuesFP = fopen(argv[1], "r");
		
		if(fscanf(inputValuesFP, "%s\n%ld\n%d",(*ips).benchmarkType,&(*ips).blockSize,&(*ips).threadCount) < 0)
		{
			return 0;
		}
		fclose(inputValuesFP);
//		printf("type: %s\nsize: %ld\ncount: %d\n",(*ips).benchmarkType,(*ips).blockSize,(*ips).threadCount);
	}
	else			//-----   For testing -----//
	{
		strcpy((*ips).benchmarkType, "RWS");
		(*ips).blockSize = 1;
		(*ips).threadCount = 1;
	}
	
	printf("type: %s\nsize: %ld\ncount: %d\n",(*ips).benchmarkType,(*ips).blockSize,(*ips).threadCount);

	for(int fNo = 0; fNo < FREQ; fNo++)
	{
//		inputDataBlock = malloc((long int) ONE_GB  * sizeof(char));			//---- Read block
//		outputDataBlock = malloc((long int) ONE_GB  * sizeof(char));			//---- Read block
//		memset(inputDataBlock, 0, (long int) ONE_GB);
		(*ips).msg = malloc((long int)ONE_GB * sizeof(char));
		memset((*ips).msg , '\0', (long int) ONE_GB);
		
	
		struct timeval process_start_time, process_end_time;

		if( strcmp((*ips).benchmarkType , "RWS") == 0)				//----------------RWS
		{	
			pthread_t ramBenchmarkThreads[(*ips).threadCount];
			gettimeofday(&process_start_time, NULL);
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_create(&ramBenchmarkThreads[i], NULL, read_write_data_sequential, (void *) ips);
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_join(ramBenchmarkThreads[i], NULL);
			gettimeofday(&process_end_time, NULL);
		
			total_time_taken = (float) (process_end_time.tv_usec - process_start_time.tv_usec) / 1000000 + (float) (process_end_time.tv_sec - process_start_time.tv_sec);
		}
		else														//-----------------RWR
		{
			pthread_t ramBenchmarkThreads[(*ips).threadCount];
			gettimeofday(&process_start_time, NULL);
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_create(&ramBenchmarkThreads[i], NULL, read_write_data_random, (void *) ips);
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_join(ramBenchmarkThreads[i], NULL);
			gettimeofday(&process_end_time, NULL);

			total_time_taken = (float) (process_end_time.tv_usec - process_start_time.tv_usec) / 1000000 + (float) (process_end_time.tv_sec - process_start_time.tv_sec);	// in seconds
		}

		if((*ips).blockSize == 1)
		{
			latency[fNo] = (double) (total_time_taken * ONE_US) / (TEST_SIZE * ONE_MB);			//  micro seconds
			printf("Total time taken : %f\nLatency : %f\n",total_time_taken,latency[fNo]);
			avg_latency += latency[fNo];
		}
		else
		{
			throughput[fNo] = (double) ONE_GB / ( total_time_taken * ONE_GB);
			throughput[fNo] *= TEST_SIZE; //-----100GB data processed

//			printf("Total time taken : %f\nThroughput : %f\n",total_time_taken,throughput[fNo]);
			avg_throughput += throughput[fNo];
		}
		free((*ips).msg);
	}

	double myRAMBenchValue;
	double theoreticalValue;
	if((*ips).blockSize == 1)
	{
		avg_latency /= FREQ;
//		printf("AVG_Latency : %f\n",avg_latency);
		myRAMBenchValue = avg_latency;
		theoreticalValue = 0.015;		//---------   CL is 15. i.e., 15 ns
	}
	else
	{
		avg_throughput /= FREQ;
//		printf("AVG_Throughput : %f\n",avg_throughput);
		myRAMBenchValue = avg_throughput;
		theoreticalValue = 8 * 4 * 2.133;			
	}
	
	double myRAMBenchEfficiency = myRAMBenchValue / theoreticalValue;
	printf("%s %d %ld %f %f %f\n", (*ips).benchmarkType, (*ips).threadCount, (*ips).blockSize, myRAMBenchValue, theoreticalValue, myRAMBenchEfficiency);
	FILE *outputFilePointer;
	outputFilePointer = fopen("./output/output.out.txt" ,"a");
	char outputSTR[1024];
	sprintf(outputSTR, "%s %d %ld %f %f %f\n", (*ips).benchmarkType, (*ips).threadCount, (*ips).blockSize, myRAMBenchValue, theoreticalValue, myRAMBenchEfficiency);
	fwrite(outputSTR, 1, strlen(outputSTR), outputFilePointer);
	fclose(outputFilePointer);
	return 0;
}


//----- Functions defination for RWS and RWR methods  
void *read_write_data_sequential(void *arg)
{
	struct InputParams *ips = (struct InputParams*) arg;
	char *outputDataBlock;
	long long int totalIterations;
	
	if((*ips).blockSize != 1)
	{
		totalIterations = (long long int) ONE_GB / (long long int) ((*ips).blockSize * (*ips).threadCount);	//-----check for throughput calc
		outputDataBlock = malloc((long long int) ONE_GB  * sizeof(char));
	}
	else
	{
		totalIterations = (long long int) ONE_MB / (long long int) ((*ips).blockSize * (*ips).threadCount);	//-----check for latency calc
		outputDataBlock = malloc((long long int) ONE_MB  * sizeof(char));
	}
	for(int j = 0; j < TEST_SIZE; j++)
	{
		if((*ips).blockSize != 1)
			outputDataBlock = malloc((long long int) ONE_GB  * sizeof(char));
		else
			outputDataBlock = malloc((long long int) ONE_MB  * sizeof(char));
//		memset(outputDataBlock, 0, (long long int) ONE_GB);
		for(long long int i = 0; i < totalIterations; i++)
		{
//			long long int l = rand() % totalIterations;
			memcpy(outputDataBlock + (i * (*ips).blockSize), (*ips).msg + (i * (*ips).blockSize) , (*ips).blockSize);
//			printf("POS = %lld\n",(i * (*ips).blockSize));
		}
		free(outputDataBlock);	
	}
	pthread_exit(NULL);
	return NULL;
}

void *read_write_data_random(void *arg)
{
	struct InputParams *ips = (struct InputParams*) arg;
	char *outputDataBlock;
	outputDataBlock = malloc((long long int) ONE_GB  * sizeof(char));
	long long int totalIterations;
	
	if((*ips).blockSize != 1)
		totalIterations = (long long int) ONE_GB / (long long int) ((*ips).blockSize * (*ips).threadCount);	//-----check for throughput calc
	else
		totalIterations = (long long int) ONE_MB / (long long int) ((*ips).blockSize * (*ips).threadCount);	//-----check for latency calc

	for(int j = 0; j < TEST_SIZE; j++)
	{
	outputDataBlock = malloc((long long int) ONE_GB  * sizeof(char));
//		memset(outputDataBlock, 0, (long long int) ONE_GB);
		for(long long int i = 0; i < totalIterations; i++)
		{
			long long int l = rand() % totalIterations;
			memcpy(outputDataBlock + (l * (*ips).blockSize), (*ips).msg + (l * (*ips).blockSize) , (*ips).blockSize);
		}
	free(outputDataBlock);	
	}
	pthread_exit(NULL);
	return NULL;
}


