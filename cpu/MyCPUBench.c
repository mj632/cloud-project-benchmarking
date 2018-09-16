#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

//Variable declaration 
//    Do this for 1 trillion ops
#define TOTAL_ITERATION 1000000000
#define ONE_GB 1000000000
#define TEST_SIZE 1000
#define FREQ_FOR_AVG 1

//-----variables and structures declaration
struct InputValues
{
	char precisionType[3];
	int threadCount;
};

//-----All functions declaration
void *computeArithmeticOps(void *arg);


//Main function
int main(int argc, char *argv[]) {
	
	FILE *detailingFilePointer;
	struct InputValues *iv = malloc(sizeof(struct InputValues));
	double theoGOps,efficiency; 
	if(argc > 1)
	{
		detailingFilePointer = fopen(argv[1],"r");	// opening *.bat file
		if(fscanf(detailingFilePointer, "%s\n%d",(*iv).precisionType,&(*iv).threadCount) < 0)
		{
			return 0;			// error or End of File
		}
		fclose(detailingFilePointer);
		
	}
	else
	{
//		printf("Please re-execute with input file name\n");
		strcpy((*iv).precisionType,"DP");
		(*iv).threadCount = 4;
//		return 0;
	}
//		printf("Type is %s and Threads are %d.\n",(*iv).precisionType,(*iv).threadCount);
	
	//int threadCount = 1;	// for testing
	double total_time_taken[FREQ_FOR_AVG];
	double processor_speed[FREQ_FOR_AVG];	
	
	for(int iter = 0; iter < FREQ_FOR_AVG; iter++)
	{
		struct timeval process_start_time, process_end_time;
//		clock_t process_start_time, process_end_time;
/*
		if((*iv).threadCount > 1)
		{
*/			pthread_t threadIdList[(*iv).threadCount];
//			process_start_time = clock();
			gettimeofday(&process_start_time, NULL);
			for(int i = 0; i < (*iv).threadCount ; ++i)
				pthread_create(&threadIdList[i], NULL, computeArithmeticOps, (void *) iv);

			for(int i = 0; i<(*iv).threadCount ;i++)
				pthread_join(threadIdList[i],NULL);	
				
//			process_end_time = clock();
			gettimeofday(&process_end_time, NULL);
/*		}
		else
		{
			process_start_time = clock();
			computeArithmeticOps((void *)iv);
			process_end_time = clock();
		}
*/	
		total_time_taken[iter] = (float) (process_end_time.tv_usec - process_start_time.tv_usec) / 1000000 + (float) (process_end_time.tv_sec - process_start_time.tv_sec);

//		total_time_taken[iter] = (double)(process_end_time - process_start_time)/CLOCKS_PER_SEC;

//		printf("start time : %f\nEnd time : %f\n",(double) process_start_time,(double) process_end_time);
	
	//----- Calculate processor speed in GigaOps per sec
//		double total_gops = (25.0 * (*iv).threadCount);
		printf("Total time : %f\n",total_time_taken[iter]);
		printf("ops : %lld\n",(long long int)TOTAL_ITERATION * TEST_SIZE);
 		processor_speed[iter] = (double)TEST_SIZE / total_time_taken[iter];
		printf("processor_speed : %f\n",processor_speed[iter]);
	}	
	
	double avg_processor_speed = 0;
	for(int i = 0; i < FREQ_FOR_AVG; i++)
	{
		avg_processor_speed += processor_speed[i];
	}
	avg_processor_speed /= FREQ_FOR_AVG;
	if (strcmp((*iv).precisionType, "QP") == 0) 
	{
		theoGOps = 588.80;
	}
	else if (strcmp((*iv).precisionType, "HP") == 0) 
	{
		theoGOps = 294.40;
	}
	else if (strcmp((*iv).precisionType, "SP") == 0) 
	{
		theoGOps = 147.20;
	}
	else
	{
		theoGOps = 73.60;
	}

	efficiency = avg_processor_speed * 100 / theoGOps;


	
//	printf("Processor Speed : %f Ops/sec(GigaOPS).\n",avg_processor_speed);
	printf("%s\t\t %d\t\t\t %.2f\t\n", (*iv).precisionType, (*iv).threadCount, avg_processor_speed);
	
	FILE *outputFilePointer;
	outputFilePointer = fopen("./output/output.out.txt" ,"a");
	char outputSTR[1024];
	sprintf(outputSTR, "%s %d %f %f %f\n", (*iv).precisionType, (*iv).threadCount, avg_processor_speed, theoGOps, efficiency);
	fwrite(outputSTR, 1, strlen(outputSTR), outputFilePointer);
	fclose(outputFilePointer);
		
    return 0;
}



// Function defination 
void *computeArithmeticOps(void *arg)
{
	struct InputValues *iv = (struct InputValues *) arg;

	long long int total_iterations = (long long int) TOTAL_ITERATION / (25 * (*iv).threadCount);
	if (strcmp((*iv).precisionType, "QP") == 0) 
	{
		char total = '&';
		for(int j = 0; j < TEST_SIZE; j++)
		{	
			for(long long int i = 0; i < total_iterations; i++)
			{
				total += (char) '7' + 'w' + '#' + 'Q' + '.' 
						+'u' + 'R' + '(' + 'z' + '/'
						+'4' + '-' + 'B' + '\n' + ']'
						+'^' + '0' + 'u' + '\b' + ':'
						+'2' + 'P' + ' ' + '\t' + '"';
			}
		}
	}
	else if (strcmp((*iv).precisionType, "HP") == 0) 
	{
		unsigned short int total = 130;
		for(int j = 0; j < TEST_SIZE; j++)
		{	
			for(long long int i = 0; i < total_iterations; i++)
			{
				total += (unsigned short int) 452 + 1932 - 4205 + 1834 
					   + 8734 - 8643 + 6541 + 674 + 8520 
					   + 19085 + 24546 + 8541 + 100 - 7413
					   + 6541 + 452 - 963 + 7523 + 7521
					   + 7536 - 410 + 965 + 120 - 734;
			}
		}
	}
	else if (strcmp((*iv).precisionType, "SP") == 0) 
	{
		int total = 61;
		for(int j = 0; j < TEST_SIZE; j++)
		{	
			for(long long int i = 0; i < total_iterations; i++)
			{
				total += (int) 1452 + 1432 - 4295 + 4134 
					   + 4734 - 6843 + 6531 + 8774 + 520 
					   + 1085 + 1546 + 8561 + 4510 - 17413
					   + 6241 + 452 - 913 + 7923 + 1521
					   + 8536 - 210 + 955 + 1630 - 6734;
			}
		}
	}
	else if (strcmp((*iv).precisionType, "DP") == 0) 
	{
		double total = 40;
		for(int j = 0; j < TEST_SIZE; j++)
		{	
			for(long long int i = 0; i < total_iterations; i++)
			{
				total += (double) 14452.23 + 1432.821 - 14295.1298 + 4134.894 
					   + 34734.33 - 26843.65 + 165312.555 + 1521.8774 + 520.83 
					   + 21085.74 + 1546.153 + 8561.9415 + 4510.4325 - 17413.672;
				total += (double) 6241.65841 + 452.85 - 913.74 + 7923.5684 + 1521.23;
				total += (double) 6.2 - 210.654 + 955.782 + 1630.9874 - 6734.564;
			}
		}
	}
	else
	{
		printf("Something went wrong! Please check the input parameter file.\n");
	}
//	printf("end of function\n");
//	printf("final value is %c and %d. \n",total,total);
	pthread_exit(NULL);
	return NULL;
}

