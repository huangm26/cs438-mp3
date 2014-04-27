#include <stdio.h>
#include <stdlib.h>


typedef struct message_struct{
	bool isMessage;
	char messages[MAX_MESSAGE_SIZE];
	int from;
	int to;
} message;

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer);

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
	
	reliablyTransfer(argv[1], udpPort, argv[3], numBytes);
} 


void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer)
{

	///////////////input the message 




    	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
    	char theirport[100];
	sprintf(theirport, "%d", hostUDPport);
 //   	printf("destination PORT: %c \n", theirport[3]);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
    
	if ((rv = getaddrinfo(destIP, theirport, &hints, &servinfo)) != 0) {
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
//    	printf("size of message %lu \n", sizeof(msg));
	if ((numbytes = sendto(sockfd, &msg, sizeof(msg), 0,
                           p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}
    
	freeaddrinfo(servinfo);
    
//	printf("talker: sent %d bytes to %s\n", numbytes, destIP);
	close(sockfd);
    	return ;

}
