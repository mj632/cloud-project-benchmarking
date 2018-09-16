#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>

//-----Variable declaration
#define PORT_NUMBER 5311
#define ONE_GB 1000000000
#define ONE_MB 1000000
#define TEST_SIZE 100		//----for server
#define FREQ 1
#define BYTE 8
//#define TEST_SIZE 1

pthread_mutex_t pmt = PTHREAD_MUTEX_INITIALIZER;
int sharedSockIndex = 0;
int sharedCount = 0;

struct InputParams
{
	int blockSize;
	int threadCount;
	char neworkProtocol[4];
	int *sockID;	
	char *serverName;
};

//-----Functions declaration
void *serverSideScript(void *arg);
void *clientSideScript(void *arg);
void *roundTripServerSideScript(void *arg);
void *roundTripClientSideScript(void *arg);

//-----Main Function
int main(int argc, char **argv)
{

	FILE *inputFilePointer;
	struct InputParams *ips;
	float total_time_taken;

	if(argc > 2)
	{		//------  check whether it is a client or server
		inputFilePointer = fopen(argv[2],"r");
		ips = malloc(sizeof(struct InputParams));
		fscanf(inputFilePointer, "%s\n%d\n%d",(*ips).neworkProtocol,&(*ips).blockSize,&(*ips).threadCount);
		fclose(inputFilePointer);
	}
	else	//-> run all the files and generate output file
	{
		ips = malloc(sizeof(struct InputParams));
		(*ips).blockSize = 32000;
		(*ips).threadCount = 4;

	}
	
//-----Check for Node type(client or server)?
	char nodeType[10];
	if(argv[1] != NULL)
		strcpy(nodeType, argv[1]);
//	printf("here\nNode type : %s\n",nodeType);

	struct timeval process_start_time, process_end_time;
	
	do
	{
		if(strcmp(nodeType,"client") == 0)
		{
			(*ips).serverName = calloc(1 + strlen(argv[3]),sizeof(char));
			strcpy((*ips).serverName,argv[3]);
			
			for(int iter = 0; iter < FREQ; iter++)					//----- Code for Client
			{
				//-----Create threads and connections as per thread count
				pthread_t netBenchmarkThread[(*ips).threadCount];
			
				if((*ips).blockSize == 1)
				{
					for(int i = 0; i < (*ips).threadCount; i++)
						pthread_create(&netBenchmarkThread[i], NULL, roundTripClientSideScript, (void *) ips);
				}
				else
				{			
					for(int i = 0; i < (*ips).threadCount; i++)
						pthread_create(&netBenchmarkThread[i], NULL, clientSideScript, (void *) ips);
				}
				
				for(int i = 0; i < (*ips).threadCount; i++)
					pthread_join(netBenchmarkThread[i], NULL);
				
			}
			break;
		}
		else if(strcmp(nodeType,"server") == 0)
		{
			double network_speed[FREQ],network_speed_avg;
			network_speed_avg = 0;
			//-----Create Socket
			int socketID = socket(AF_INET, SOCK_STREAM, 0);	//--- (IPv4,TCP,0)
			if(socketID == 0)
			{
				printf("Socket creation failed.\n");
				return 0;	//-----Think a about while loop for successful creation of Socket
			}

			struct sockaddr_in sockAddrs;
			sockAddrs.sin_family = AF_INET;
			sockAddrs.sin_port = htons(PORT_NUMBER);

			char *hostAddrName = "0.0.0.0";
			if (inet_aton(hostAddrName, &sockAddrs.sin_addr)==0) {
				fprintf(stderr, "inet_aton() failed\n");
				exit(1);
			}

			if(bind(socketID, (struct sockaddr*) &sockAddrs, sizeof(sockAddrs)) < 0)
			{
				printf("Binding failed.\n");
				return 0;
			}
			else
			{
				listen(socketID, 10);
 				struct sockaddr_in clientAddrs;
 				socklen_t clientAddrSize = sizeof(clientAddrs);

				for(int iter = 0; iter < FREQ; iter++)					//----- Code for Server
				{
					pthread_t netBenchmarkThread[(*ips).threadCount];

					(*ips).sockID = malloc((*ips).threadCount * sizeof(int));	//----- Socket ID from clients who requested for connection
					if((*ips).blockSize == 1)
					{
						for(int i = 0; i < (*ips).threadCount && ((*ips).sockID[i] = accept(socketID, (struct sockaddr *)&clientAddrs, &clientAddrSize)) >= 0; i++)
						{
							if( i == 0)
								gettimeofday(&process_start_time, NULL);
							pthread_create(&netBenchmarkThread[i], NULL, roundTripServerSideScript, (void *) ips);
						}
					}
					else
					{
						for(int i = 0; i < (*ips).threadCount && ((*ips).sockID[i] = accept(socketID, NULL, NULL)) >= 0; i++)
						{					
							if( i == 0)
								gettimeofday(&process_start_time, NULL);
							pthread_create(&netBenchmarkThread[i], NULL, serverSideScript, (void *) ips);
						}
					}
					for(int i = 0; i < (*ips).threadCount; i++)
					{
						pthread_join(netBenchmarkThread[i], NULL);
					}		
					gettimeofday(&process_end_time, NULL);

					//-----connection close
		
					//-----  CHECK for socket count. Is it same as Thread count?
		
					total_time_taken = (float) (process_end_time.tv_usec - process_start_time.tv_usec) / 1000000 + (float) (process_end_time.tv_sec - process_start_time.tv_sec);
					if((*ips).blockSize == 1)
					{
						network_speed[iter] = (double) (total_time_taken * 1000) / (long int)ONE_MB;
						printf("total_time_taken : %f\n",total_time_taken);
						printf("latency : %fms\n",network_speed[iter]);
					}
					else
					{
						network_speed[iter] = (double) ((long long int)TEST_SIZE * ONE_GB * BYTE)/ (ONE_MB * total_time_taken);			//---- In Gbps unit  for each size is ONE GB-> 8 Gb
						printf("total_time_taken : %f\n",total_time_taken);
						printf("speed : %f Mbps\n",network_speed[iter]);
					}
					network_speed_avg += network_speed[iter];
				}
			}
			double myNETBenchValue;
			double theoreticalValue;		//----10 GBps or 0.0007ms
			double myNETBenchEfficiency;
			network_speed_avg /= FREQ;	
			myNETBenchValue = network_speed_avg;
			if((*ips).blockSize == 1)
			{
				theoreticalValue = 0.0007;
				myNETBenchEfficiency = (double) 100 - ((myNETBenchValue * 100) / theoreticalValue);
			}
			else
			{
				theoreticalValue = 10000;
				myNETBenchEfficiency = (myNETBenchValue * 100) / theoreticalValue;
			}
			
			printf("AVG is %f\n",network_speed_avg);	
			close(socketID);	


			FILE *outputFilePointer;
			outputFilePointer = fopen("./output/output.out.txt" ,"a");
			char outputSTR[1024];
			sprintf(outputSTR, "%s\t\t %d\t\t\t %d\t\t%f\t\t%f\t\t\t%f\n", (*ips).neworkProtocol, (*ips).threadCount, (*ips).blockSize, myNETBenchValue, theoreticalValue, myNETBenchEfficiency);
			fwrite(outputSTR, 1, strlen(outputSTR), outputFilePointer);
			fclose(outputFilePointer);
			break;
		}
		else
		{
			printf("How do you want to use this node? client or server : ");
			scanf("%s",nodeType);
		}
	}while(1);
	pthread_mutex_destroy(&pmt);
    return 0;
}

//-----Server Thread Function declaration (Receiver). Receive data only (UDP connection)
void *serverSideScript(void *arg)
{
	int socketID;	//----- Client's socket ID
	struct InputParams *ips = (struct InputParams *) arg;
	char msg[(*ips).blockSize];
	int totalPackageCount = ONE_GB / ((*ips).blockSize * (*ips).threadCount);
	
	pthread_mutex_lock(&pmt);
		socketID = (*ips).sockID[sharedSockIndex++];
	pthread_mutex_unlock(&pmt);

//	printf("socket ID : %d\n", socketID);	

//----- Receive message from sender
	for(int j = 0; j < TEST_SIZE; j++)
	{
		for(int i = 0; i < totalPackageCount ; i++)
		{
			recv(socketID, &msg, sizeof(msg), 0);
		}
	}

	pthread_exit(NULL);
	return NULL;
}


//-----Client Thread Function declaration (sender)
void *clientSideScript(void *arg)
{
	int socketID;	//----- Client's socket ID
	struct InputParams *ips = (struct InputParams *) arg;
	socketID = socket(AF_INET, SOCK_STREAM, 0);	//--- (IPv4,TCP,0)
	if(socketID == 0)
	{
		printf("Socket creation failed.\n");
		return 0;	//-----Think a about while loop for successful creation of Socket
	}

	struct hostent *sName;
	sName = gethostbyname((*ips).serverName);
	
	struct sockaddr_in sockAddrs;

 	bzero((char *) &sockAddrs, sizeof(sockAddrs));

	sockAddrs.sin_family = AF_INET;
	sockAddrs.sin_port = htons(PORT_NUMBER);

	bcopy((char *)sName->h_addr, (char *)&sockAddrs.sin_addr.s_addr, sName->h_length);

	int connectionStatus = connect(socketID, (struct sockaddr *) &sockAddrs, sizeof(sockAddrs));
	if(connectionStatus < 0)
	{
		printf("connection refused.\n");
		//---- wait for a while and retry to connect. CODE HERE
	}
	//------send data  
	char msg[(*ips).blockSize] ;
	msg[(*ips).blockSize] = '\0';
	int msgPackageCount = (int) ONE_GB / ((*ips).blockSize * (*ips).threadCount);
	for(int j = 0; j < TEST_SIZE; j++)
	{
		for(int i = 0; i < msgPackageCount ; i++)
		{
			send(socketID, msg, (*ips).blockSize, 0);
		}
//		printf("%d)  ONE GB data completed\n",j+1);
	}	
//-----connection close
	close(socketID);	

	pthread_exit(NULL);
	return NULL;
}

void *roundTripServerSideScript(void *arg)
{
	int socketID;	//----- Client's socket ID
	char ack[20];

	struct InputParams *ips = (struct InputParams *) arg;
	char msg[20];
	int totalPackageCount = ONE_MB / (*ips).threadCount;
	
	pthread_mutex_lock(&pmt);
		socketID = (*ips).sockID[sharedSockIndex++];
	pthread_mutex_unlock(&pmt);

//	printf("ping pong\n");
//----- Ping Pong from sender
	for(int i = 0; i < totalPackageCount ; i++)
	{
		do
		{
			sprintf(ack,"%d",i);
			send(socketID, ack, sizeof(ack), 0);
			recv(socketID, &msg, sizeof(msg), 0);
		}while(strcmp(ack,msg)!=0);
	}
	pthread_exit(NULL);
	
	return NULL;
}
void *roundTripClientSideScript(void *arg)
{
	int socketID;	//----- Client's socket ID
	struct InputParams *ips = (struct InputParams *) arg;
	char ack[20];

	socketID = socket(AF_INET, SOCK_STREAM, 0);	//--- (IPv4,TCP,0)
	if(socketID == 0)
	{
		printf("Socket creation failed.\n");
		return 0;	//-----Think a about while loop for successful creation of Socket
	}
	struct hostent *sName;
	sName = gethostbyname((*ips).serverName);
	
	struct sockaddr_in sockAddrs;

 	bzero((char *)&sockAddrs, sizeof(sockAddrs));

	sockAddrs.sin_family = AF_INET;
	sockAddrs.sin_port = htons(PORT_NUMBER);

	bcopy((char *)sName->h_addr, (char *)&sockAddrs.sin_addr.s_addr, sName->h_length);

	int connectionStatus = connect(socketID, (struct sockaddr *) &sockAddrs, sizeof(sockAddrs));
	if(connectionStatus < 0)
	{
		printf("connection refused.\n");
	}
	//------send data  
	char msg[20] ;
	int totalPackageCount = ONE_MB / (*ips).threadCount;
//	printf("client ping pong\n");
	for(int i = 0; i < totalPackageCount ; i++)
	{
		do
		{
			sprintf(ack,"%d",i);
			recv(socketID, &msg, sizeof(msg), 0);
			send(socketID, ack, sizeof(ack), 0);
		}while(strcmp(ack,msg)!=0);
	}
//-----connection close
	close(socketID);
//	printf("End ping pong\n");
	

	pthread_exit(NULL);
	return NULL;
}

