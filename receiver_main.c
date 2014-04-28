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

typedef struct message_struct{
	char messages[BUFFER_SIZE];
	int Sequence_number;
	int final;
} message;

typedef struct ack_struct{
	int request_sequence_number;
} ack;

int recv_sockfd;
struct addrinfo recv_hints, *recv_servinfo, *recv_p;
int recv_rv;
struct sockaddr_storage recv_their_addr;	socklen_t recv_addr_len;
char recv_s[INET6_ADDRSTRLEN];
char myport[100];

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

int recv_message(message* msg){
	return recvfrom(recv_sockfd, msg, sizeof(*msg), 0, (struct sockaddr *)&recv_their_addr, &recv_addr_len);
}

void reliablyReceive(unsigned short int myUDPport, char* destinationFile){
  FILE * pFile;
  pFile = fopen (destinationFile, "wb");
  

	while(1){
		message msg;
		recv_message(&msg);
		fwrite (msg.messages , sizeof(char), sizeof(msg.messages), pFile);
		if(msg.final == 1){
			break;
		}
	}
	fclose (pFile);
}

int main(int argc, char** argv)
{
	unsigned short int udpPort;
	
	if(argc != 3)
	{
		fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
		exit(1);
	}
	
	udpPort = (unsigned short int)atoi(argv[1]);
	bind_udp(udpPort);
	reliablyReceive(udpPort, argv[2]);
}
