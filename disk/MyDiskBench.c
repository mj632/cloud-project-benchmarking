#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>


//-----Global Variables
#define ONE_MB 		1000000
#define ONE_GB 		1000000000
#define TEST_SIZE 	10000000000
#define FREQ 1
#define BYTE 8

pthread_mutex_t pmt = PTHREAD_MUTEX_INITIALIZER;
long long int sharedFilePointerPos = 0;

struct InputParams
{
	char modeOfOps[3];
	int threadCount;
	long long int blockSize;
};

//-----Function Declaration
void *readOnlySeq(void *arg);
void *writeOnlySeq(void *arg);
void *readOnlyRand(void *arg);
void *writeOnlyRand(void *arg);
void *readOnlyRandLatency(void *arg);
void *writeOnlyRandLatency(void *arg);


//-----Main function code
int main(int argc, char **argv)
{
	FILE *inputValuesFP;
	struct InputParams *ips;
	double diskThroughput[FREQ],total_time_taken[FREQ],avg_throughput;
	double diskLatency[FREQ],avg_latency, iops[FREQ],avg_iops;
	avg_throughput = 0;
	avg_latency = 0;
	avg_iops = 0;
	if(argc>1)
	{
		ips = malloc(sizeof(struct InputParams));
		inputValuesFP = fopen(argv[1], "r");
		if(fscanf(inputValuesFP, "%s\n%lld\n%d",(*ips).modeOfOps,&(*ips).blockSize,&(*ips).threadCount) < 0)
			return 0;
		fclose(inputValuesFP);
		(*ips).blockSize *= 1000;	//block size is given in KB
		
//		printf("type: %s\nsize: %lld\ncount: %d\n",(*ips).modeOfOps,(*ips).blockSize,(*ips).threadCount);
	}
	else
	{
		//----- do for all files
//		printf("For calculation of CPU Benchmark, Please Enter the File name : \n");
		ips = malloc(sizeof(struct InputParams));
		strcpy((*ips).modeOfOps, "WR");
		(*ips).blockSize = 1000;
		(*ips).threadCount = 4;
		printf("type: %s\nsize: %lld\ncount: %d\n",(*ips).modeOfOps,(*ips).blockSize,(*ips).threadCount);
//		scanf("");
//		return 0;
	}
	
	for(int iter= 0; iter < FREQ ; iter++)
	{
		pthread_t diskBEnchmarkThread[(*ips).threadCount];
		struct timeval process_start_time, process_end_time;
//		clock_t process_start_time, process_end_time;

		FILE *writeFilePointer;
		writeFilePointer = fopen("/tmp/mpat.txt","w");
		fseek(writeFilePointer, (long long int)TEST_SIZE, SEEK_SET);
		fwrite("\0", 1, sizeof("\0"), writeFilePointer);
		fclose(writeFilePointer);

		if(strcmp((*ips).modeOfOps,"RS") == 0)
		{
//			printf("HEY\n");
			gettimeofday(&process_start_time, NULL);
//			process_start_time = clock();
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_create(&diskBEnchmarkThread[i], NULL, readOnlySeq, (void *) ips);
			
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_join(diskBEnchmarkThread[i],NULL);
			gettimeofday(&process_end_time, NULL);
//			process_end_time = clock();
		}
		else if(strcmp((*ips).modeOfOps,"WS") == 0)
		{
			gettimeofday(&process_start_time, NULL);
//			process_start_time = clock();
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_create(&diskBEnchmarkThread[i], NULL, writeOnlySeq, (void *) ips);
			
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_join(diskBEnchmarkThread[i],NULL);
			gettimeofday(&process_end_time, NULL);
//			process_end_time = clock();
		}
		else if(strcmp((*ips).modeOfOps,"RR") == 0)
		{
//			process_start_time = clock();
			gettimeofday(&process_start_time, NULL);
			if((*ips).blockSize != 1000)
			{
				for(int i = 0; i < (*ips).threadCount; i++)
					pthread_create(&diskBEnchmarkThread[i], NULL, readOnlyRand, (void *) ips);
			}
			else
			{
				FILE *writeFilePointer;
//				writeFilePointer = fopen("/tmp/mpat.txt","w");
				writeFilePointer = fopen("/tmp/mpat.txt","w");
				fseek(writeFilePointer, (long long int)ONE_GB, SEEK_SET);
				fwrite("\0", 1, sizeof("\0"), writeFilePointer);
				fclose(writeFilePointer);

				for(int i = 0; i < (*ips).threadCount; i++)
					pthread_create(&diskBEnchmarkThread[i], NULL, readOnlyRandLatency, (void *) ips);
			}
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_join(diskBEnchmarkThread[i],NULL);
			gettimeofday(&process_end_time, NULL);
//			process_end_time = clock();
		}
		else
		{
//			process_start_time = clock();
			gettimeofday(&process_start_time, NULL);
			if((*ips).blockSize != 1000)
			{
				for(int i = 0; i < (*ips).threadCount; i++)
					pthread_create(&diskBEnchmarkThread[i], NULL, writeOnlyRand, (void *) ips);
			}
			else
			{
				FILE *writeFilePointer;
				writeFilePointer = fopen("/tmp/mpat.txt","w");
				fseek(writeFilePointer, (long long int)ONE_GB, SEEK_SET);
				fwrite("\0", 1, sizeof("\0"), writeFilePointer);
				fclose(writeFilePointer);

				for(int i = 0; i < (*ips).threadCount; i++)
					pthread_create(&diskBEnchmarkThread[i], NULL, writeOnlyRandLatency, (void *) ips);
			}
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_join(diskBEnchmarkThread[i],NULL);
			gettimeofday(&process_end_time, NULL);
//			process_end_time = clock();
		}
	
		total_time_taken[iter] = (float) (process_end_time.tv_usec - process_start_time.tv_usec) / 1000000 + (float) (process_end_time.tv_sec - process_start_time.tv_sec);
//		printf("start time : %f\nEnd time : %f\n",(double) process_start_time.tv_sec,(double) process_end_time.tv_sec);
	//	double diskThroughput = 
//		printf("TEST_SIZE : %lld\n",(long long int)TEST_SIZE);

		if((*ips).blockSize != 1000)
		{
			diskThroughput[iter] = (long long int)TEST_SIZE / (total_time_taken[iter] * ONE_MB);
			avg_throughput += diskThroughput[iter];
		}
		else
		{
			diskLatency[iter] = (double) (total_time_taken[iter] * 1000)/ (long int) ONE_MB;  //for milisec
			avg_latency += diskLatency[iter];
			iops[iter] = (double)(long long int)TEST_SIZE / (total_time_taken[iter]);
			avg_iops += iops[iter];
		}

//		printf("Time taken : %f\n",total_time_taken[iter]);
	}	
	
	double efficiency;
	char outputSTR[2048];
	
		if(strcmp((*ips).modeOfOps,"RS") == 0)
		{
			double theoreticalThroughput = 372;		//--------get this value
			avg_throughput /= BYTE * FREQ;
			efficiency = (avg_throughput * 100)/theoreticalThroughput;			//----------------Get this value
			
			sprintf(outputSTR, "%s\t\t %d\t\t\t %lld\t\t %f\t\t\t %f\t\t\t%f\n", (*ips).modeOfOps, (*ips).threadCount,(long long int)(*ips).blockSize/ONE_MB, avg_throughput,theoreticalThroughput,efficiency);
		}
		else if(strcmp((*ips).modeOfOps,"WS") == 0)
		{
			double theoreticalThroughput = 172;		//--------get this value
			avg_throughput /= FREQ;
			efficiency = (avg_throughput * 100)/theoreticalThroughput;			//----------------Get this value
			
			sprintf(outputSTR, "%s\t\t %d\t\t\t %lld\t\t %f\t\t\t %f\t\t\t%f\n", (*ips).modeOfOps, (*ips).threadCount,(long long int)(*ips).blockSize/ONE_MB, avg_throughput,theoreticalThroughput,efficiency);
		}
		else if(strcmp((*ips).modeOfOps,"RR") == 0)
		{
			if((*ips).blockSize != 1000)
			{
				double theoreticalThroughput = 540;		//--------get this value
				avg_throughput /=  BYTE * FREQ;
				efficiency = (avg_throughput * 100)/theoreticalThroughput;			//----------------Get this value
				
				sprintf(outputSTR, "%s\t\t %d\t\t\t %lld\t\t %f\t\t\t %f\t\t\t%f\n", (*ips).modeOfOps, (*ips).threadCount,(long long int)(*ips).blockSize/ONE_MB, avg_throughput,theoreticalThroughput,efficiency);

//				printf("avg_throughput : %f\n",avg_throughput);
			}
			else
			{
				double theoreticalLatency = 0.5;		//--------get this value
				avg_latency /= FREQ;
				avg_iops /= FREQ;
	
				efficiency = 100 - ((avg_latency * 100)/theoreticalLatency);			//----------------Get this value
	
				sprintf(outputSTR, "%s\t\t %d\t\t\t %lld\t\t %f\t\t\t %f\t\t\t%f\n", (*ips).modeOfOps, (*ips).threadCount,(long long int)(*ips).blockSize/1000, avg_latency,theoreticalLatency,efficiency);
				double theoreticalIops = 93000 * 8;
				efficiency = avg_iops * 100/theoreticalIops;			//----------------Get this value

				sprintf(outputSTR, "%s%s\t\t %d\t\t\t %lld\t\t %f\t\t\t %f\t\t\t%f\n",outputSTR , (*ips).modeOfOps, (*ips).threadCount,(long long int)(*ips).blockSize/1000, avg_iops,theoreticalLatency,efficiency);
//				printf("avg_latency : %f\n",avg_latency);
			}
		}
		else			//===================WS
		{
			if((*ips).blockSize != 1000)
			{
				double theoreticalThroughput = 410;		//--------get this value
				avg_throughput /= FREQ;
				efficiency = (avg_throughput * 100)/theoreticalThroughput;			//----------------Get this value
				
				sprintf(outputSTR, "%s\t\t %d\t\t\t %lld\t\t %f\t\t\t %f\t\t\t%f\n", (*ips).modeOfOps, (*ips).threadCount,(long long int)(*ips).blockSize/ONE_MB, avg_throughput,theoreticalThroughput,efficiency);

//				printf("avg_throughput : %f\n",avg_throughput);
			}
			else
			{
				double theoreticalLatency = 0.5;		//--------get this value
				avg_latency /= FREQ;
				avg_iops /= FREQ;
	
				efficiency = 100 - ((avg_latency * 100)/theoreticalLatency);			//----------------Get this value
	
				sprintf(outputSTR, "%s\t\t %d\t\t\t %lld\t\t %f\t\t\t %f\t\t\t%f\n", (*ips).modeOfOps, (*ips).threadCount,(long long int)(*ips).blockSize/1000, avg_latency,theoreticalLatency,efficiency);
				double theoreticalIops = 43000 * 8;
				efficiency = avg_iops * 100/theoreticalIops;			//----------------Get this value

				sprintf(outputSTR, "%s%s\t\t %d\t\t\t %lld\t\t %f\t\t\t %f\t\t\t%f\n",outputSTR , (*ips).modeOfOps, (*ips).threadCount,(long long int)(*ips).blockSize/1000, avg_iops,theoreticalLatency,efficiency);
//				printf("avg_latency : %f\n",avg_latency);
			}
		}	
	
	
	
	
	
	
//------  Output saving in file
	FILE *outputFilePointer;
	outputFilePointer = fopen("./output/output.out.txt" ,"a");
	fwrite(outputSTR, 1, strlen(outputSTR), outputFilePointer);
	fclose(outputFilePointer);

    return 0;
}


//-----Function Defination
void *readOnlySeq(void *arg)
{
	struct InputParams *ips = (struct InputParams *) arg;
	FILE *readFilePointer;
	readFilePointer = fopen("/tmp/mpat.txt","r");
	char *inputBuff;
	inputBuff = malloc((*ips).blockSize *sizeof(char));
	long long int filePointerPOS;
	int flag = 1;
	memset(inputBuff, 0, (*ips).blockSize);
	while(flag)
	{
		pthread_mutex_lock(&pmt);
		filePointerPOS = sharedFilePointerPos;
		sharedFilePointerPos += (*ips).blockSize; 			// Sequential  read block by block
		if(sharedFilePointerPos >= (long long int)TEST_SIZE)
		{
			flag = 0;
//			printf("sharedFilePointerPos : %lld\n",sharedFilePointerPos);
		}	
		pthread_mutex_unlock(&pmt);
		if(filePointerPOS < (long long int)TEST_SIZE)
		{
			memset(inputBuff, '\0', (*ips).blockSize);
			fseek(readFilePointer, filePointerPOS, SEEK_SET);
			if ( fread(inputBuff, (*ips).blockSize, 1, readFilePointer) < 0)
				printf("reading out of bound!!\n");
		}
	}
	fclose(readFilePointer);
	pthread_exit(NULL);
	return NULL;
}

void *writeOnlySeq(void *arg)
{
	struct InputParams *ips = (struct InputParams *) arg;
	FILE *writeFilePointer;
	writeFilePointer = fopen("/tmp/mpat.txt","a");
	char *inputBuff;
	inputBuff = malloc((*ips).blockSize *sizeof(char));
	long long int filePointerPOS;
	int flag = 1;
	
	memset(inputBuff, 0, (*ips).blockSize);
	while(flag)					
	{
		pthread_mutex_lock(&pmt);
		filePointerPOS = sharedFilePointerPos;
		sharedFilePointerPos += (*ips).blockSize; 			// Sequential  write block by block
		if(sharedFilePointerPos >= (long long int)TEST_SIZE)
			flag = 0;
		pthread_mutex_unlock(&pmt);
		if(filePointerPOS < (long long int)TEST_SIZE)
		{
			fseek(writeFilePointer, filePointerPOS, SEEK_SET);
			fwrite(inputBuff, 1, (*ips).blockSize, writeFilePointer);
			memset(inputBuff, 0, (*ips).blockSize);
		}
	}
	fclose(writeFilePointer);
	pthread_exit(NULL);
	return NULL;
}

void *readOnlyRand(void *arg)
{
	struct InputParams *ips = (struct InputParams *) arg;
	FILE *readFilePointer;
	readFilePointer = fopen("/tmp/mpat.txt","r");
	char *inputBuff;
	inputBuff = malloc((*ips).blockSize *sizeof(char));
	long long int filePointerPOS;
	time_t tm;
	srand((unsigned) time(&tm));

	memset(inputBuff, 0, (*ips).blockSize);
	long int totalIter = (long long int) TEST_SIZE / ((*ips).blockSize * (*ips).threadCount);

	for(long int i = 0; i < totalIter; i++)
	{
		filePointerPOS = (*ips).blockSize * (rand() % totalIter);
		memset(inputBuff, '\0', (*ips).blockSize);
		fseek(readFilePointer, filePointerPOS, SEEK_SET);
		if( fread(inputBuff, (*ips).blockSize, 1, readFilePointer) < 0)
			printf("Reading out of bound!!!\n");
	}
	fclose(readFilePointer);
	pthread_exit(NULL);
	return NULL;
}

void *writeOnlyRand(void *arg)
{
	struct InputParams *ips = (struct InputParams *) arg;
	FILE *writeFilePointer;
	writeFilePointer = fopen("/tmp/mpat.txt","a");
	char *inputBuff;
	inputBuff = malloc((*ips).blockSize *sizeof(char));
	long long int filePointerPOS;
	time_t tm;
	srand((unsigned) time(&tm));

	memset(inputBuff, '\0', (*ips).blockSize);
	long int totalIter = (long long int) TEST_SIZE / ((*ips).blockSize * (*ips).threadCount);
	for(long int i = 0; i < totalIter; i++)
	{
		filePointerPOS = (*ips).blockSize * (rand() % totalIter);
		fseek(writeFilePointer, filePointerPOS, SEEK_SET);
		fwrite(inputBuff, 1, (*ips).blockSize, writeFilePointer);
		memset(inputBuff, '\0', (*ips).blockSize);
	}
	fclose(writeFilePointer);
	pthread_exit(NULL);
	return NULL;
}

//-----  For Latency calculation
void *readOnlyRandLatency(void *arg)
{
	struct InputParams *ips = (struct InputParams *) arg;
	FILE *readFilePointer;
	readFilePointer = fopen("/tmp/mpat.txt","r");
	char *inputBuff;
	inputBuff = malloc((*ips).blockSize *sizeof(char));
	long long int filePointerPOS;
	time_t tm;
	srand((unsigned) time(&tm));

	memset(inputBuff, 0, (*ips).blockSize);
	long int totalIter  = (long long int)ONE_MB / (*ips).threadCount;

	for(long int i = 0; i < totalIter; i++)
	{
		filePointerPOS = (rand() % ONE_MB);
//		fflush(inputBuff);
		memset(inputBuff, '\0', (*ips).blockSize);
		fseek(readFilePointer, filePointerPOS, SEEK_SET);
		if( fread(inputBuff, (*ips).blockSize, 1, readFilePointer) < 0)
			printf("Reading out of bound!!!\n");
	}
	fclose(readFilePointer);
	pthread_exit(NULL);
	return NULL;
}


void *writeOnlyRandLatency(void *arg)
{
	struct InputParams *ips = (struct InputParams *) arg;
	FILE *writeFilePointer;
	writeFilePointer = fopen("/tmp/mpat.txt","a");
	char *inputBuff;
	inputBuff = malloc((*ips).blockSize *sizeof(char));
	long long int filePointerPOS;
	time_t tm;
	srand((unsigned) time(&tm));

	memset(inputBuff, '\0', (*ips).blockSize);
	long int totalIter  = (long long int)ONE_MB / (*ips).threadCount;

//	printf("totalIter : %ld\n",totalIter);
	for(long int i = 0; i < totalIter; i++)
	{
		filePointerPOS = (rand() % (long int)totalIter) * (*ips).blockSize;
//		printf("filePointerPOS : %ld\n",filePointerPOS);
//		fflush(inputBuff);
		fseek(writeFilePointer, (long long int) filePointerPOS, SEEK_SET);
		fwrite(inputBuff, 1, (*ips).blockSize, writeFilePointer);
		memset(inputBuff, '\0', (*ips).blockSize);
	}
	fclose(writeFilePointer);
	pthread_exit(NULL);
	return NULL;
}

