#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <argp.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <poll.h>
#include <signal.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <curl/curl.h>
#include <assert.h>
#include <openssl/evp.h>

#include "torrent.c"
#include "heapless-bencode/bencode.c"
#include "client.h"
//#include "sending.c"
#include "peer_handler.c"
//#include "recieving.c"

int connect_to(struct peer p) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        printf("Socket creation failed\n");
		return -1;
	}

    struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr)); //zero out structure
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(p.addr);
	servAddr.sin_port = htons(p.port);

   // if(debug) printf("Set up servAddr struct for connection.\n");

    struct timeval timeout;
	timeout.tv_sec  = 1;  
	timeout.tv_usec = 0; //timeout in 0.5 seconds
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

   // if(debug) printf("Set up timeout for socket\n");

    if(connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        if(debug) printf("Connection refused\n");
		return -1; //instead of the whole perror thing
	}
    else {
        if(debug) printf("Connection success!\n");
    }
    return sock;

}


int setup_sock(int port) { //this is our read socket
	struct sockaddr_in servAddr;
	int servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//constructed local address struture
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    if(bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        perror("bind() failed");
        return -1;
    }
    if(listen(servSock, 50) < 0) { //listen to at most 50 at once
        perror("listen failed\n");
        return -1;
    }
	return servSock;
}


/*if connection does't return -1, initilaize a set of peers that are responsive.*/
struct peer *get_responsive_peers(struct peer *tracker_peers, int num_peers, int *responsive_peers) {
    struct peer *peers = NULL;
    int responsive = 0;
    for(int i = 0; i < num_peers; i++) {
        int socket = connect_to(tracker_peers[i]); //if connection to peer[i] succeeds, copy that over to a responsive peers list
        if(socket != -1) {
            peers = realloc(peers, sizeof(peer) * (responsive + 1));
            peers[responsive].socket = socket;

            //malloc space for address and copy it over, including port
            peers[responsive].addr = malloc(strlen(tracker_peers[i].addr) + 1);
            strcpy(peers[responsive].addr, tracker_peers[i].addr);
            peers[responsive].addr[strlen(tracker_peers[i].addr)] = '\0';
            peers[responsive].port = tracker_peers[i].port;

            //malloc space for id and copy it over
            peers[responsive].id = malloc(20);
            memcpy(peers[responsive].id, tracker_peers[i].id, 20);

            //initializing state information
            peers[responsive].pieces = NULL;
            peers[responsive].states.am_choking = 1;
            peers[responsive].states.am_interested = 0;
            peers[responsive].states.peer_choking = 1;
            peers[responsive].states.peer_interested = 0;
            peers[responsive].current_piece = -1;

            peers[responsive].pieces = (struct piece *) malloc(sizeof(struct piece) * num_pieces);
            for(int j = 0; j < num_pieces; j++) {
                int num_offsets = piece_length / BLOCK_SIZE;
                peers[responsive].pieces[j].status = EMPTY;
                peers[responsive].pieces[j].num_blocks = num_offsets;
                peers[responsive].pieces[j].blocks_recieved = 0;
                peers[responsive].pieces[j].blocks = malloc(sizeof(uint8_t) * num_offsets);
                memset(peers[responsive].pieces[j].blocks, 0, num_offsets);
            }
            responsive++;

        }
    }
    *responsive_peers = responsive;
    return peers;
}

int all_pieces_downloaded(){
    for(int i = 0; i < num_pieces; i++) {
        if(self.pieces[i].status != DOWNLOADED) {
            return 0;
        }
    }
    return 1;
}



int handshake(struct peer pr, unsigned char *infohash, uint8_t *our_id) {
    uint8_t pstrlen = 19; //no need for htonl/s bc it is a raw byte
    char *pstr = "BitTorrent protocol";
    uint8_t reserved[8]; memset(reserved, 0, 8);
    uint8_t info_hash[20]; memcpy(info_hash, infohash, 20);
    uint8_t id[20]; memcpy(id, our_id, 20);
    //packing
    uint8_t buffer[68];
    memcpy(buffer, &pstrlen, 1);
    memcpy(buffer + 1, pstr, 19);
    memcpy(buffer + 20, reserved, 8);
    memcpy(buffer + 28, info_hash, 20);
    memcpy(buffer + 48, id, 20);

    sendBytes(pr.socket, 68, buffer);
    uint8_t recv_buffer[68];
    int bytes_read = readBytes(pr.socket, 68, recv_buffer);
    if(bytes_read <= 0) {
        printf("No bytes recieved. Closing Connection\n");
        close(pr.socket);
        return -1;
    }
    //validation check
    uint8_t rlen; char rstr[20]; uint8_t rres[8]; uint8_t rethash[20]; uint8_t retid[20];
    memset(rstr, 0, 20);
    memcpy(&rlen, recv_buffer, 1);
    memcpy(rstr, recv_buffer + 1, 19);
    memcpy(rres, recv_buffer + 20, 8);
    memcpy(rethash, recv_buffer + 28, 20);
    memcpy(retid, recv_buffer + 48, 20);

    if(memcmp(rethash, info_hash, 20) != 0) {
        printf("Closing connection because info hash does not match\n");
        close(pr.socket);
        return -1;
    }
    return 1;
}

int peer_has_piece(struct peer *pr, int index) {
    return (pr->pieces[index].status == DOWNLOADED) ? 1 : 0;
}

//returns true if all blocks in a piece have been downloaded
int all_blocks_downloaded(struct peer *pr, int index) {
    for(int i = 0; i < pr->pieces[index].num_blocks; i++) {
        if(pr->pieces[index].blocks[i] != DOWNLOADED) {
            return -1;
        }
    }
    return 1;
}

int none_empty(struct peer *pr) {
    for(int i = 0; i < num_pieces; i++) {
        if(pr->pieces[i].status == EMPTY) {
            return -1;
        }
    }
    return 1;
}
int main(int argc, char *argv[]) {
    char *file_str; int file_len;
    char *file_buffer;
    if(argc < 2) {
        printf("Need to supply a torrent file\n");
        return 1;
    }
    file_str = argv[1];
    file_buffer = read_file(file_str, &file_len);
    if(file_buffer == NULL) {
        return 1;
    }
    struct torrent *file = parse_torrent(file_buffer, file_len);
    print_torrent(file);
    /*END OF PARSING TORRENT*/


    file_name = (char *) malloc(strlen(file->info->name) + 1);
    strcpy(file_name, file->info->name);    //changed from strncpy to strcpy
    file_name[strlen(file->info->name)] = '\0';

    memcpy(&num_pieces, &(file->info->num_pieces), 4);
    memcpy(&piece_length, &(file->info->piece_length), 4);
    fp = fopen(file_name, "w+b"); 
    /*END OF CREATING FILE AND STORING DATA*/

    //gonna request pieces instead of blocks
    int num_blocks = piece_length / BLOCK_SIZE;
    self.pieces = (struct piece *) malloc(sizeof(struct piece) * num_pieces); 
    for(int i = 0 ; i < num_pieces - 1;  i++) { //setting up all but last piece
        self.pieces[i].status = EMPTY;
        self.pieces[i].num_blocks = num_blocks;
        self.pieces[i].blocks_recieved = 0;
        self.pieces[i].blocks = malloc(sizeof(uint8_t) * num_blocks);
        self.pieces[i].bytes_last_block = BLOCK_SIZE;
        memset(self.pieces[i].blocks, EMPTY, num_blocks);
    }
    long int temp = (num_pieces - 1)*piece_length;
    int bytes_last_piece = file->info->length - temp;
    int blocks_last = floor(bytes_last_piece / BLOCK_SIZE) + 1;
    int bytes_last_block = bytes_last_piece - (blocks_last - 1) * BLOCK_SIZE;

    self.pieces[num_pieces - 1].status = EMPTY;
    self.pieces[num_pieces - 1].bytes_last_block = bytes_last_block;
    self.pieces[num_pieces - 1].num_blocks = blocks_last;
    self.pieces[num_pieces - 1].blocks_recieved = 0;
    self.pieces[num_pieces - 1].blocks = malloc(sizeof(uint8_t) * blocks_last); //maybe change to * .offsets
    memset(self.pieces[num_pieces - 1].blocks, EMPTY, blocks_last);

    //printf("Num of bytes in last block is %d\n", bytes_last_block);
    if(debug) {
        printf("Num of pieces is %d\n", num_pieces);
        printf("The number of bytes in the last piece is %d\n", bytes_last_piece);
        printf("The number of blocks in the last piece is %d\n", blocks_last);
        printf("The number of blocks usually is %d\n", num_blocks);
    }
    /*END OF STORING PIECE INFORMATION*/

    int port = 22222;
    struct tracker_response *response = get_response(0, port, file); 
    if(response == NULL) {
        printf("Error in parsing tracker response\n");
        return 1;
    }
    int responsive_peers = 0;
    struct peer *tracker_peers = response->peers; //sometimes we will get bad peers. 
    struct peer *peers = get_responsive_peers(tracker_peers, response->num_peers, &responsive_peers);
    /*END OF PARSING TRACKER RESPONSE*/

    struct pollfd sockets[51]; //will be communicating with at most 50 different peers. No need for anymore.
    sockets[0].fd = setup_sock(9903); sockets[0].events = POLLIN;

    for(int i = 1; i <= responsive_peers; i++) { //
        sockets[i].fd = peers[i-1].socket;
        sockets[i].events = POLLIN;
    }
    /*HANDSHAKING TO BEGIN PROTOCOL*/
    for(int i = 1; i <= responsive_peers; i++) {
        int success = handshake(peers[i-1], file->info->infohash, self.id);
        if(success < 0) {
            peers[i-1].socket = -1;
        }
    }
    
    for(int i = 1; i <= responsive_peers; i++) {
        if(peers[i-1].socket != -1) {
            printf("We are connected to peer %s and port %d\n", peers[i-1].addr, peers[i-1].port);
        }
    }

    struct timeval timeBefore;
	gettimeofday(&timeBefore, NULL);
    for(;;) {
        if(poll(sockets, responsive_peers + 1, 0) > 0) {    //CHANGE BACK RESPONSIVE_PEERS + 1
            if(sockets[0].revents && POLLIN) {  
                printf("New Incoming Connection\n");
            }
            else{
                for(int i = 1; i <= responsive_peers; i++) {
                    if((sockets[i].revents && POLLIN) && peers[i-1].socket != -1) {
                        //handle_peer(&peers[i-1], sockets[i].fd);
                        handle(sockets[i].fd, &peers[i-1]);
                    }

                }
            }
        }

        for(int i = 0; i < responsive_peers; i++) {
            int index;
            if(peers[i].states.am_interested == 0) {//if we are not interested {
                //and if they have a piece that is empty for us, send them interested.
                index = has_piece_for_me(&peers[i]);
                if(index >= 0) {
                    send_interested(peers[i].socket);
                    peers[i].states.am_interested = 1;
                }
            }
            else { //if we are already interested
                if(peers[i].states.peer_choking == 0) { //and peer is not choking us
                    //send request. But what request do we send...
                    if(peers[i].current_piece >= 0) { //if this current peer is working on a piece continue
                        int status = request_next_block(&peers[i], peers[i].current_piece);
                        if(status == -1) { //all pieces currently in progress or downloaded so we will mark it as downloaded.
                            //validation check. if there are still some in progress, remark them as empty.
                            //self.pieces[peers[i].current_piece].status = DOWNLOADED;
                            //peers[i].pieces[peers[i].current_piece].status = DOWNLOADED;
                            peers[i].current_piece = -1; //move onto next piece
                        } 
                    }
                    else {//current peer is not working on a piece
                        index = has_piece_for_me(&peers[i]); //index is set to a piece the peer has and I currently dont have.
                        if(index >= 0) {
                            peers[i].current_piece = index; //if they dont have a piece, then its still negativo 1;
                        }

                    }
                    //if This current peer working on a piece, then request next block for piece.
                    //else if current peer not working on a piece, give them a piece to work on.
                }
            }
        } 
/*
        struct timeval timeNow;
		gettimeofday(&timeNow, NULL);
        if(diff(timeBefore, timeNow) > 20000){
            for(int i = 0; i < num_pieces; i++) {
                if(self.pieces[i].status == IN_PROGRESS) {
                    //printf("Inside this if statement. Index we're changing is %d\n", i);
                    self.pieces[i].status = EMPTY;
                    empty_piece(i);

                }
                gettimeofday(&timeBefore, NULL);
            } 

        } */




        if(all_pieces_downloaded()) {
            for(int i = 0; i < responsive_peers; i++) {
                if(peers[i].socket != -1) {
                    if(peers[i].states.am_interested == 1) {
                        send_not_interested(peers[i].socket);
                    }
                    if(peers[i].states.am_choking == 1) {
                        send_not_choke(peers[i].socket);
                    }
                }
            }
            break;
        }

    }
    printf("All pieces have been downloaded! You can open your file. Number of blocks recieved is %d\n", blocks_recieved);
    return 0;
}