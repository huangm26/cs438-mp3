#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <algorithm> 
#include <time.h> 
#include <string>
#include <cstring>
#include <pthread.h>
#include <functional>
#include <queue>
#include <list>

#define MAX_BUFFER		2000
#define BUFFER_SIZE		1400

char file_to_send[MAX_BUFFER][BUFFER_SIZE];
int Sequence_number;

int recv_sockfd;
struct addrinfo recv_hints, *recv_servinfo, *recv_p;
int recv_rv;
struct sockaddr_storage recv_their_addr;	socklen_t recv_addr_len;
char recv_s[INET6_ADDRSTRLEN];
char myport[100];


typedef struct message_struct{
	char messages[BUFFER_SIZE];
	int Sequence_number;
	int final;
} message;

typedef struct ack_struct{
	int request_sequence_number;
} ack;



int send_package(char* hostname, unsigned short int hostUDPport, message msg){
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
    char theirport[100];
	sprintf(theirport, "%d", hostUDPport);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
    
	if ((rv = getaddrinfo(hostname, theirport, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
    
	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
        
		break;
	}
    
	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}
    

	if((numbytes = sendto(sockfd, &msg, sizeof(msg), 0, p->ai_addr, p->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	close(sockfd);
    return numbytes;
}

int read_file(char* filename, unsigned long long int bytesToTransfer){
	FILE * pFile;
	long lSize;
	char * buffer;
	size_t result;
	int i,j,k;
	j = 0;
	k = 0;

	pFile = fopen (filename, "rb" );
	if (pFile==NULL) {
		fputs ("File error",stderr); 
		exit (1);
	}

	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	lSize = ftell (pFile);
	rewind (pFile);

	// allocate memory to contain the whole file:
	buffer = (char*) malloc (sizeof(char)*lSize);
	if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

	// copy the file into the buffer:
	result = fread (buffer,1,lSize,pFile);
	//if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

	// terminate
	fclose (pFile);
	
	while(bytesToTransfer > BUFFER_SIZE){
		for(i = 0; i < BUFFER_SIZE; i++){
			file_to_send[j][i] = buffer[k];
			k++;
		}
		j++;
		bytesToTransfer -= BUFFER_SIZE;
	}
	for(i = 0; i < (long long int)bytesToTransfer; i++){
		file_to_send[j][i] = buffer[k];
		k++;
	}
	
	free (buffer);
	return 0;
}

int bind_udp(unsigned short int port){
	sprintf(myport, "%d", port);
	memset(&recv_hints, 0, sizeof recv_hints);
	recv_hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	recv_hints.ai_socktype = SOCK_DGRAM;
	recv_hints.ai_flags = AI_PASSIVE; // use my IP
    
	if ((recv_rv = getaddrinfo(NULL, myport, &recv_hints, &recv_servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(recv_rv));
		return 1;
	}
    
	// loop through all the results and bind to the first we can
	for(recv_p = recv_servinfo; recv_p != NULL; recv_p = recv_p->ai_next) {
		if ((recv_sockfd = socket(recv_p->ai_family, recv_p->ai_socktype,
                             recv_p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
        
		if (bind(recv_sockfd, recv_p->ai_addr, recv_p->ai_addrlen) == -1) {
			close(recv_sockfd);
			perror("listener: bind");
			continue;
		}
        
		break;
	}
    
	if (recv_p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
    
	freeaddrinfo(recv_servinfo);
    
	printf("listener: waiting to recvfrom...\n");
	//fcntl(recv_sockfd, F_SETFL, fcntl(recv_sockfd, F_GETFL) | O_NONBLOCK);
	return 0;

}

int recv_ack(ack* recv_ack){
	return recvfrom(recv_sockfd, recv_ack, sizeof(*recv_ack), 0, (struct sockaddr *)&recv_their_addr, &recv_addr_len);
}

void* accept_ack(void *identifier){
	while(1){
		ack receiver_ack;
		recv_ack(&receiver_ack);
	}
	return NULL;
}

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer)
{
	sleep(20);
	int total_sequence;
	int i, j;
	//read the file into buffer array
	read_file(filename, bytesToTransfer);
	
	//initialize Sequence_number and total_sequence
	Sequence_number = 0;
	total_sequence = bytesToTransfer/BUFFER_SIZE+1;
	
	//create thread for accept ack
	pthread_t t;
	pthread_create(&t, NULL, &accept_ack, NULL);				
	
	//sending first package
	for(j = 0; j < total_sequence; j++){
		message msg;
		for(i = 0; i < BUFFER_SIZE; i++){
			msg.messages[i] = file_to_send[Sequence_number][i];
		}
		msg.Sequence_number = Sequence_number;
		if(Sequence_number == total_sequence-1){
			msg.final = 1;
		}else{
			msg.final = 0;
		}
		send_package(hostname, hostUDPport, msg);
		Sequence_number++;
	}

}

int main(int argc, char** argv)
{
	unsigned short int udpPort;
	unsigned long long int numBytes;
	
	if(argc != 5)
	{
		fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
		exit(1);
	}
	
	udpPort = (unsigned short int)atoi(argv[2]);
	numBytes = atoll(argv[4]);
	bind_udp(udpPort-1);
	reliablyTransfer(argv[1], udpPort, argv[3], numBytes);
} 







