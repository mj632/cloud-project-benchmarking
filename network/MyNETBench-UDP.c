#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

//-----Variable declaration
#define PORT_NUMBER 4311
#define ONE_GB 1000000000
#define ONE_MB 1000000
#define TEST_SIZE 100
#define FREQ 1
#define BYTE 8

struct InputParams
{
	int blockSize;
	int threadCount;
	char neworkProtocol[4];
	int sockID;	//----change name to threadID
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
	ips = malloc(sizeof(struct InputParams));
	if(argc > 2)
	{
		inputFilePointer = fopen(argv[2],"r");
		fscanf(inputFilePointer, "%s\n%d\n%d",(*ips).neworkProtocol,&(*ips).blockSize,&(*ips).threadCount);
		fclose(inputFilePointer);
	}
	else	//-> run all the files and generate output file
	{
		
	}
	
	if(strcmp(argv[1], "server") == 0)
	{
		int socketID,iter;
		struct sockaddr_in serverAddr;
		char *hostAddrName = "0.0.0.0";
		struct timeval process_start_time, process_end_time;
		float total_time_taken;
		double network_speed[FREQ],network_speed_avg;
		network_speed_avg = 0;
		iter = 0;
		socketID = socket(AF_INET, SOCK_DGRAM, 0);
	
		memset(&serverAddr, '\0', sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(PORT_NUMBER);
		
		if (inet_aton(hostAddrName, &serverAddr.sin_addr)==0) {
			fprintf(stderr, "inet_aton() failed\n");
			exit(1);
		}
	
		bind(socketID, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
		/*=============Till This point code is for server creation==================*/
	
		pthread_t p[(*ips).threadCount];
		(*ips).sockID = socketID;
		printf(" Protocol - %s\n Block Size- %d\n Threads - %d\n",(*ips).neworkProtocol,(*ips).blockSize,(*ips).threadCount);	
		if((*ips).blockSize != 1)
		{
			gettimeofday(&process_start_time, NULL);
			for(int i = 0; i < (*ips).threadCount; i++)
			{
				pthread_create(&p[i], NULL, serverSideScript, (void *) ips);
			}
		}
		else
		{
			gettimeofday(&process_start_time, NULL);
			for(int i = 0; i < (*ips).threadCount; i++)
			{
				pthread_create(&p[i], NULL, roundTripServerSideScript, (void *) ips);
			}
		}
		for(int i = 0; i < (*ips).threadCount; i++)
		{
			pthread_join(p[i], NULL);
//			printf("%d joined\n",i+1);
		}

		gettimeofday(&process_end_time, NULL);

		total_time_taken = (float) (process_end_time.tv_usec - process_start_time.tv_usec) / 1000000 + (float) (process_end_time.tv_sec - process_start_time.tv_sec);
		if((*ips).blockSize == 1)
		{
			network_speed[iter] = (double) (total_time_taken * 1000) / (long int)ONE_MB ;
			printf("total_time_taken : %f\n",total_time_taken);
			printf("latency : %fms\n",network_speed[iter]);
		}
		else
		{
			network_speed[iter] = (double) ((long long int)TEST_SIZE * ONE_GB * BYTE)/ ( ONE_MB * total_time_taken);			//---- In Gbps unit  for each size is ONE GB-> 8 Gb
			printf("total_time_taken : %f\n",total_time_taken);
			printf("speed : %f Mbps\n",network_speed[iter]);
		}
		network_speed_avg += network_speed[iter];

			double myNETBenchValue;
			double theoreticalValue;		//----10 Gbps or 0.0007ms
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
				myNETBenchEfficiency = (double)(myNETBenchValue * 100) / theoreticalValue;
			}
			
			printf("AVG is %f\n",network_speed_avg);	
			close(socketID);	


			FILE *outputFilePointer;
			outputFilePointer = fopen("./output/output.out.txt" ,"a");
			char outputSTR[1024];
			sprintf(outputSTR, "%s\t\t %d\t\t\t %d\t\t%f\t\t%f\t\t\t%f\n", (*ips).neworkProtocol, (*ips).threadCount, (*ips).blockSize, myNETBenchValue, theoreticalValue, myNETBenchEfficiency);
			fwrite(outputSTR, 1, strlen(outputSTR), outputFilePointer);
			fclose(outputFilePointer);
		
		close(socketID);
	}
	else
	{
		pthread_t p[(*ips).threadCount];
		(*ips).serverName = calloc(1 + strlen(argv[3]),sizeof(char));
		strcpy((*ips).serverName,argv[3]);

		if((*ips).blockSize != 1)
		{
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_create(&p[i], NULL, clientSideScript, (void *) ips);
		}
		else
		{
			for(int i = 0; i < (*ips).threadCount; i++)
				pthread_create(&p[i], NULL, roundTripClientSideScript, (void *) ips);
		}
		
		for(int i = 0; i < (*ips).threadCount; i++)
			pthread_join(p[i], NULL);
	}
	
    return 0;
}



//-----Server Thread Function declaration (Receiver). Receive data only (UDP connection)
void *serverSideScript(void *arg)
{
	struct InputParams *ips = (struct InputParams *) arg;
	struct sockaddr_in remoteAddr;
	char *message, *recvBuff;
	char *ack = "end";

	message = calloc((long int)ONE_GB, sizeof(char));
	recvBuff = calloc((*ips).blockSize, sizeof(char));
	socklen_t addr_size;
	addr_size = sizeof(remoteAddr);	
	while(strcmp(recvBuff, "end") != 0)
	{
		recvfrom((*ips).sockID, recvBuff, (*ips).blockSize, 0, (struct sockaddr*) &remoteAddr, &addr_size);
		strcpy(message,recvBuff);
		message[strlen(ack)] ='\0';
	}
//printf("Hello\n");	
	return NULL;
}

//-----Client Thread Function declaration (sender)
void *clientSideScript(void *arg)
{
	struct InputParams *ips = (struct InputParams *) arg;
//	printf("Hello\n");	
	int socketID;
	struct sockaddr_in clientAddr;
	char *sendBuff;
	char *ack = "end";
	sendBuff = calloc((*ips).blockSize, sizeof(char));
	struct hostent *sName;
	socketID = socket(AF_INET, SOCK_DGRAM, 0);
	sName = gethostbyname((*ips).serverName);			//----- Assigning hostname/address

 	bzero((char *)&clientAddr, sizeof(clientAddr));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(PORT_NUMBER);
	bcopy((char *)sName->h_addr, (char *)&clientAddr.sin_addr.s_addr, sName->h_length);

	long int totalPackagesCount = (long int) ONE_GB/((*ips).blockSize * (*ips).threadCount);
//	printf("totalPackagesCount : %ld",totalPackagesCount);
	for(int j = 0; j < TEST_SIZE;j++)
	{
		for(long int i = 0; i < totalPackagesCount; i++)
		{
			sendto(socketID, sendBuff, (*ips).blockSize, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
		}
	}	
	for(int i = 0; i < 5; i++)
	{
		sendto(socketID, ack, strlen(ack), 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
	}
	return NULL;
}


//==---------------------RTT---------------===//


void *roundTripServerSideScript(void *arg)
{
	struct InputParams *ips = (struct InputParams *) arg;
	struct sockaddr_in remoteAddr;
	char msg[20],ack[20];		
	socklen_t addrSize;
	addrSize = sizeof(remoteAddr);	

	int totalPackageCount = ONE_MB / (*ips).threadCount;
	
	for(int i = 0; i < totalPackageCount ; i++)
	{
		do
		{
			sprintf(ack,"%d",i);
			recvfrom((*ips).sockID, msg, strlen(ack), 0, (struct sockaddr*) &remoteAddr, &addrSize);
			sendto((*ips).sockID, ack, strlen(ack), 0, (struct sockaddr *) &remoteAddr, sizeof(remoteAddr));
			msg[strlen(ack)] ='\0';
//				printf("ack - %s\tmsg - %s\n",ack,msg);
		}while(strcmp(ack,msg)!=0);
	}
	return NULL;
}

void *roundTripClientSideScript(void *arg)
{
	struct InputParams *ips = (struct InputParams *) arg;
	int socketID;
	struct sockaddr_in clientAddr;
	struct hostent *sName;
	char msg[20],ack[20];

	socklen_t addrSize;
	addrSize = sizeof(clientAddr);	
	
	socketID = socket(AF_INET, SOCK_DGRAM, 0);
	sName = gethostbyname((*ips).serverName);			//----- Assigning hostname/address

 	bzero((char *)&clientAddr, sizeof(clientAddr));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(PORT_NUMBER);
	bcopy((char *)sName->h_addr, (char *)&clientAddr.sin_addr.s_addr, sName->h_length);

	int totalPackageCount = ONE_MB/(*ips).threadCount;

	for(int i = 0; i < totalPackageCount ; i++)
	{
		do
		{
			sprintf(ack,"%d",i);

			sendto(socketID, ack, strlen(ack), 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
			recvfrom(socketID, msg, 1+strlen(ack), 0, (struct sockaddr*) &clientAddr, &addrSize);
			msg[strlen(ack)] ='\0';
//				printf("ack - %s\tmsg - %s\n",ack,msg);
		}while(strcmp(ack,msg)!=0);
	}
	return NULL;
}
