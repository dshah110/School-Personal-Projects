#include <argp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sysexits.h>
#include <unistd.h>
#include <strings.h>
#include <math.h>
#include <netdb.h>
#include "time.h"

#define MAXCLIENTS 10000

struct server_arguments {
	int port;
    int drop;
};

struct __attribute__((packed)) client {
    struct sockaddr_in clientaddr;
    int highseqNum;
    time_t tou; //can place current time
};


error_t server_parser(int key, char *arg, struct argp_state *state) {
	struct server_arguments *args = state->input;
	error_t ret = 0;
	switch(key) {
	case 'p':
		/* Validate that port is correct and a number, etc!! */
		args->port = atoi(arg);
		if (0 /* port is invalid */) {
			argp_error(state, "Invalid option for a port, must be a number");
		}
		break;
	case 'd':
        args->drop = atoi(arg);
		break;
	default:
		ret = ARGP_ERR_UNKNOWN;
		break;
	}
	return ret;
}

struct server_arguments server_parseopt(int argc, char *argv[]) {
	struct server_arguments args;

	/* bzero ensures that "default" parameters are all zeroed out */
	bzero(&args, sizeof(args));



	struct argp_option options[] = {
		{ "port", 'p', "port", 0, "The port to be used for the server" ,0},
		{ "drop", 'd', "drop", 0, "The drop percentage", 0},
		{0}
	};
	struct argp argp_settings = { options, server_parser, 0, 0, 0, 0, 0 };
	if (argp_parse(&argp_settings, argc, argv, 0, NULL, &args) != 0) {
		printf("Got an error condition when parsing\n");
	}

	/* What happens if you don't pass in parameters? Check args values
	 * for sanity and required parameters being filled in */

	/* If they don't pass in all required settings, you should detect
	 * this and return a non-zero value from main */
	printf("Got port %d and drop percentage %d.\n", args.port, args.drop);
	//free(args.salt);
    return args;
}


int contains(struct sockaddr_in clad, struct client cls[], int length) {
    for(int i = 0; i < length; i++) {
        if(clad.sin_addr.s_addr == cls[i].clientaddr.sin_addr.s_addr && clad.sin_port == cls[i].clientaddr.sin_port) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in servaddr;
    struct server_arguments sargs = server_parseopt(argc,argv);
    int sock;
    int numOfClients = 0;
    srand(time(NULL));
    unsigned int clientaddrLen;


    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(sargs.port);

    if (bind(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("bind() failed");
    }

    struct client cls[MAXCLIENTS];
    calloc(MAXCLIENTS, sizeof(struct client));
    
    for(;;) {
        struct sockaddr_in clientaddr;
        clientaddrLen = sizeof(clientaddr);

        struct timespec time;
		clock_gettime(CLOCK_REALTIME, &time);
        int random = (rand() % 101); //will give us a random from 0 - 100;
        uint8_t response[38];

        if(recvfrom(sock, response, 22, 0, (struct sockaddr *)&clientaddr, &clientaddrLen) < 0) {
            perror("recvfrom() failed");
        }
        if(sargs.drop == 0 || random > sargs.drop)  {
            int seqNum;        
            memcpy(&seqNum, response, 4);
            seqNum = ntohl(seqNum);

            struct timespec time;
            clock_gettime(CLOCK_REALTIME, &time); //timestamp

            char clientaddrstr[1025];
            char clientport[32];
            getnameinfo((struct sockaddr *)&clientaddr, clientaddrLen, clientaddrstr, sizeof(clientaddrstr), clientport, sizeof(clientport), NI_NUMERICHOST | NI_NUMERICSERV);

            if(!contains(clientaddr, cls, numOfClients)) {
                cls[numOfClients].clientaddr = clientaddr;
                cls[numOfClients].highseqNum = seqNum;
                cls[numOfClients].tou = time.tv_sec;
                numOfClients++;
            }
            
            else {
                for(int i = 0; i < numOfClients; i++) {
                    if(clientaddr.sin_addr.s_addr == cls[i].clientaddr.sin_addr.s_addr && clientaddr.sin_port == cls[i].clientaddr.sin_port) {
                        if(difftime(time.tv_sec, cls[i].tou) >= 120) {
                            cls[i].highseqNum = -1;
                        }
                        if(seqNum < cls[i].highseqNum) {
                            printf("%s:%s %d %d\n", clientaddrstr, clientport, seqNum, cls[i].highseqNum);
                        }
                        else {
                            cls[i].highseqNum = seqNum;
                            cls[i].tou = time.tv_sec;
                        }
                    }
                }

            }

            uint64_t ssec = htobe64(time.tv_sec);
            uint64_t snsec = htobe64(time.tv_nsec);
            memcpy(response + 22, &ssec, 8);
            memcpy(response + 30, &snsec, 8);
            sendto(sock, response, 38, 0, (struct sockaddr *) &clientaddr, sizeof(clientaddr));

        }

    }
}
