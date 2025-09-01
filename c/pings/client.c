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
#include "time.h"

struct __attribute__((packed)) client_arguments {
	char ip_address[16]; /* You can store this as a string, but I probably wouldn't */
	int port; /* is there already a structure you can store the address
	           * and port in instead of like this? */
    int reqNum;
    int timOut;
};

struct __attribute__((packed)) timeResponse {
	uint32_t seqNum;
	uint16_t version;
	uint64_t clseconds;
	uint64_t clnano;
	uint64_t sseconds;
	uint64_t snano;
	float theta;
	float delta;
};


error_t client_parser(int key, char *arg, struct argp_state *state) {
	struct client_arguments *args = state->input;
	error_t ret = 0;
	switch(key) {
	case 'a':
		/* validate that address parameter makes sense */
		strncpy(args->ip_address, arg, 16);
		if (0 /* ip address is goofytown */) {
			argp_error(state, "Invalid address");
		}
		break;
	case 'p':
		/* Validate that port is correct and a number, etc!! */
		args->port = atoi(arg);
		if (0 /* port is invalid */) {
			argp_error(state, "Invalid option for a port, must be a number");
		}
		break;
	case 'n':
		/* validate argument makes sense */
		args->reqNum = atoi(arg);
		break;
	case 't':
		/* validate file */
        args->timOut = atoi(arg);
		/*len = strlen(arg);
		args->filename = malloc(len + 1);
		strcpy(args->filename, arg); */
		break;
	default:
		ret = ARGP_ERR_UNKNOWN;
		break;
	}
	return ret;
}

//was void
struct client_arguments client_parseopt(int argc, char *argv[]) {
	struct argp_option options[] = {
		{ "addr", 'a', "addr", 0, "The IP address the server is listening at", 0},
		{ "port", 'p', "port", 0, "The port that is being used at the server", 0},
        {"reqNum", 'n', "reqNum", 0, "The number of time requests to send to the server", 0},
        {"timOut", 't', "timOut", 0, "The time the client will wait before sending out last request", 0},
		{0}
	};

	struct argp argp_settings = { options, client_parser, 0, 0, 0, 0, 0 };

	struct client_arguments args;
	bzero(&args, sizeof(args));

	if (argp_parse(&argp_settings, argc, argv, 0, NULL, &args) != 0) {
		printf("Got error in parse\n");
	}

	/* If they don't pass in all required settings, you should detect
	 * this and return a non-zero value from main */
/*	printf("Got %s on port %d with n=%d smin=%d smax=%d filename=%s\n",
	       args.ip_address, args.port, args.hashnum, args.smin, args.smax, args.filename); */
	return args;
}

int main(int argc, char *argv[]){
    struct client_arguments cargs = client_parseopt(argc, argv);
    int sock; /* Socket descriptor */
    struct sockaddr_in servAddr; /* server address */
    struct sockaddr_in srcAddr;
	struct timeResponse responses[cargs.reqNum];
	memset(responses, 0, sizeof(responses));

	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket failed");
    } 
    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr)); /* Zero out structure */
    servAddr.sin_family = AF_INET; /* Internet addr family */
    servAddr.sin_addr.s_addr = inet_addr(cargs.ip_address); /* Server IP address */
    servAddr.sin_port = htons(cargs.port); /* Server port */


    //want to send out time responses
    for(int i = 0; i < cargs.reqNum; i++) {
		responses[i].seqNum = -1; //for later
        uint8_t timeRequest[22];
        struct timespec time;
        clock_gettime(CLOCK_REALTIME, &time);

		uint32_t seq = htonl(i+1);
		uint16_t version = htons(7);
		uint64_t sec = htobe64(time.tv_sec);
		uint64_t nsec = htobe64(time.tv_nsec);

		memcpy(timeRequest, &seq, 4);
		memcpy(timeRequest+4, &version, 2);
		memcpy(timeRequest+6, &sec, 8);
		memcpy(timeRequest+14, &nsec, 8);

		sendto(sock, timeRequest, 22, 0, (struct sockaddr *)&servAddr, sizeof(servAddr));
    }


	if(cargs.timOut != 0) {
		struct timeval timeout;
		timeout.tv_sec = cargs.timOut;
		timeout.tv_usec = 0;
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));
	}

	int recvd = 0;
	unsigned int srcSize = sizeof(srcAddr);
	for(int i = 0; i < cargs.reqNum; i++) {
		uint8_t response[38];
		recvd = recvfrom(sock, response, 38, 0, (struct sockaddr *)&srcAddr, &srcSize);
		if(recvd > 0) {
			int index;
			struct timespec time;
			clock_gettime(CLOCK_REALTIME, &time);

			memcpy(&index, response, 4);
			index = ntohl(index) - 1;

			memcpy(&responses[index].seqNum, response, 4);
			memcpy(&responses[index].version, response+4, 2);
			memcpy(&responses[index].clseconds, response+6, 8);
			memcpy(&responses[index].clnano, response+14, 8);
			memcpy(&responses[index].sseconds, response+22, 8);
			memcpy(&responses[index].snano, response + 30, 8);

			//have them all in responses now we just wanna convert

			responses[index].seqNum = ntohl(responses[index].seqNum);
			responses[index].version = ntohs(responses[index].version);
			responses[index].clseconds = be64toh(responses[index].clseconds);
			responses[index].clnano = be64toh(responses[index].clnano);
			responses[index].sseconds = be64toh(responses[index].sseconds);
			responses[index].snano = be64toh(responses[index].snano);


			long double t0 = responses[index].clseconds + (responses[index].clnano)/pow(10,9);
			long double t1 = responses[index].sseconds + (responses[index].snano)/pow(10,9);
			long double t2 = time.tv_sec + (time.tv_nsec)/pow(10,9);

			responses[index].theta = ((t1 - t0) + (t1 - t2))/2;
			responses[index].delta = t2 - t0;

		}
		else {
			break;
		}

	}
	close(sock);


	for(int i = 0; i < cargs.reqNum; i++) {
		if((int) responses[i].seqNum == -1) {
			printf("%d: Dropped\n", i+1);
		}
		else {
			printf("%d: %.4f %.4f\n", i+1 , responses[i].theta, responses[i].delta);
		}
	}
}