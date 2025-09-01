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
#include <pthread.h>

#include "peer_handler.h"
#define UNUSED(x) (void)(x)

struct peer_thread *p;


//each peer works on their own piece.
//A peer does not start working on another piece until all the blocks in a piece are downloaded

int has_piece_for_me(struct peer *pr) {
    for(int i = 0; i < num_pieces; i++) {
        if(self.pieces[i].status == EMPTY && pr->pieces[i].status == DOWNLOADED) {
            return i;
        }
    }
    return -1;
}
void empty_piece(int index) {
    for(int i = 0 ; i <  self.pieces[index].num_blocks; i++) {
        self.pieces[index].blocks[i] = EMPTY;
    }
}


int request_next_block(struct peer *pr, int index) {
    //self.pieces[index] is what this peer is working on
    if(index == -1) { //indicating we dont need anymore blocks
        return -1;
    }
  //  if(self.pieces[index].status == EMPTY) { //indicating that a peer is already working on giving us this piece so we dont ask it of another peer
   //     self.pieces[index].status = IN_PROGRESS;
   // }
    int num_blocks = self.pieces[index].num_blocks;
    for(int i = 0; i < num_blocks; i++) {
        if(self.pieces[index].blocks[i] == EMPTY) { //will check first if empty.
            //printf("Sending request for piece#%d and block#%d\n", index, i);
            //quick check to see if this is the last block.
            if(index == num_pieces - 1 && i == num_blocks - 1) {
                self.pieces[index].status = IN_PROGRESS; //index and offset within index
                send_request(pr->socket, index, i * BLOCK_SIZE, self.pieces[index].bytes_last_block);
                self.pieces[index].blocks[i] = IN_PROGRESS;
                return i;
            }

            self.pieces[index].status = IN_PROGRESS; //keep setting it i guess cuz idk what to do with it
            send_request(pr->socket, index, i * BLOCK_SIZE, BLOCK_SIZE);
            self.pieces[index].blocks[i] = IN_PROGRESS;
            return i;
        }
    }


    return -1;
}


void handle(int socket, struct peer *pr){
    uint32_t length;
    uint8_t id;
    readBytes(socket, 4, &length);
    length = ntohl(length);
    if(length == 0) {
        printf("Keep Alive\n");
        return;
    }
    readBytes(socket, 1, &id);

    if(id == 0) {
        printf("Recieved a choke\n");
        handle_choke(pr);

    }
    else if(id == 1) {
        printf("Recieved an unchoke\n");
        handle_unchoke(pr);

    }
    else if(id == 2) {
        printf("Recieved an interested message\n"); 
        handle_interested(pr);

    }
    else if(id == 3) {
        printf("Recieved a not-interested message\n");
        handle_not_interested(pr);
    
    }
    else if(id == 4) {
        printf("Recieved a have message\n");
        handle_have(socket, pr);
    }
    else if(id == 5) {
        printf("Recieved bitfield\n");
        handle_bitfield(socket, length, pr);
    }
    else if(id == 6) {
        printf("Recieved a request message\n");
        handle_request(socket);

    }
    else if(id == 7) {
        //printf("Recieved a piece message\n");
        handle_piece(pr, length);
        blocks_recieved++;
    }
    else {
        printf("Got an unknown id %d\n", id);
    }

}

/*SENDING FUNCTIONS*/

//sends the socket a keep alive message
//keep-alive: <len=0000>
int send_keep_alive(int socket){
    UNUSED(socket);

	return 0;
}

//sends choking msg
//choke: <len=0001><id=0>
int send_choke(int socket){
	uint32_t slength = htonl(1); 
    uint8_t sid = 0;
    uint8_t buffer[5]; memcpy(buffer, &slength, 4);
    memcpy(buffer + 4, &sid, 1);
    if(sendBytes(socket, 5, buffer) < 0) {
        return -1;
    }
    //pr->states.am_choking = 1;
    return 0;
}

//sends not choking msg
//unchoke: <len=0001><id=1>
int send_not_choke(int socket){
    uint32_t slength = htonl(1); uint8_t sid = 1;
    uint8_t buffer[5]; memcpy(buffer, &slength, 4);
    memcpy(buffer + 4, &sid, 1);
    if(sendBytes(socket, 5, buffer) < 0) {
        return -1;
    } 
	return 0;
}

//sends interested msg
//interested: <len=0001><id=2>
int send_interested(int socket){
    uint32_t slength = htonl(1); uint8_t sid = 2;
    uint8_t buffer[5]; memcpy(buffer, &slength, 4);
    memcpy(buffer + 4, &sid, 1);
    if(sendBytes(socket, 5, buffer) < 0) {
        return -1;
    }
    //pr->states.am_interested = 1;
	return 0;
}

//sends not interested msg
//not interested: <len=0001><id=3>
int send_not_interested(int socket){
    uint32_t slength = htonl(1); uint8_t sid = 3;
    uint8_t buffer[5]; memcpy(buffer, &slength, 4);
    memcpy(buffer + 4, &sid, 1);
    if(sendBytes(socket, 5, buffer) < 0) {
        return -1;
    }
	return 0;
}

//sends have piece msg
//have: <len=0005><id=4><piece index>
int send_have_piece(int socket, int index) {
    UNUSED(socket);
    UNUSED(index);
	return 0;
}

//sends bitfield of all pieces, each bit is a piece we have or don't have
//bitfield: <len=0001+X><id=5><bitfield>
int send_bitfield(int socket, int length, uint8_t *payload){
    UNUSED(socket);
    UNUSED(length);
    UNUSED(payload);
	return 0;
}

//sends request msg for a particular piece 
//request: <len=0013><id=6><index><begin><length>
int send_request(int socket, int index, int begin, int length){
    //printf("Sending Request for piece index %d and offset within piece %d\n", index, begin);
    uint32_t slength = htonl(13);
    uint8_t sid = 6;
    uint32_t tindex = htonl(index); 
    uint32_t tbegin = htonl(begin);
    uint32_t tlength = htonl(length);
    uint8_t buffer[17]; 
    memcpy(buffer, &slength, 4);
    memcpy(buffer + 4, &sid, 1);
    memcpy(buffer + 5, &tindex, 4);
    memcpy(buffer + 9, &tbegin, 4);
    memcpy(buffer + 13, &tlength, 4);
    sendBytes(socket, 17, buffer);
	return 0;
}

//sends a piece to the peer
//piece: <len=0009+X><id=7><index><begin><block>
int send_piece(int socket, int index, int begin, uint8_t *block){
    UNUSED(socket);
    UNUSED(index);
    UNUSED(begin);
    UNUSED(block);
	return 0;
}

//sends cancel message for a requested block
//cancel: <len=0013><id=8><index><begin><length>
int send_cancel(int socket, int index, int begin, int length){
    UNUSED(socket);
    UNUSED(index);
    UNUSED(begin);
    UNUSED(length);
	return 0;
}



/*RECIEVING FUNCTIONS*/

//uint32_t length; uint8_t id;
//readBytes(socket, 4, &length); length = ntohl(length);

void handle_choke(struct peer *pr) {
    pr->states.peer_choking = 1; 
}

void handle_unchoke(struct peer *pr) {
    pr->states.peer_choking = 0; 
}

void handle_interested(struct peer *pr) {
    pr->states.peer_interested = 1;
}

void handle_not_interested(struct peer *pr) {
    pr->states.peer_interested = 0;
}

void handle_have(int socket, struct peer *pr) {
    uint32_t index;
    readBytes(socket, 4, &index); 
    index = ntohl(index);

    pr->pieces[index].status = DOWNLOADED;
}

void handle_bitfield(int socket, int length, struct peer *pr) {
    int byte_size = 8;
    int num_bytes = length - 1;
    uint8_t payload[num_bytes]; 
    uint8_t temp[8*num_bytes];
    uint8_t bits[8*num_bytes]; 
    readBytes(socket, num_bytes, payload);
    for (int j=0; (long unsigned int) j < sizeof(payload)*8; j++) {
        temp[j] = ((1 << (j % 8)) & (payload[j/8])) >> (j % 8);
    }
    for(int idx = 0; idx < num_bytes; idx ++) {
        for(int jdx = 0, k = byte_size - 1; jdx < byte_size; jdx++, k--) {
            bits[idx * byte_size + jdx]  = temp[byte_size * idx + k];
        }
    }
    for(int j = 0; j < num_pieces; j++) {
        if(bits[j] == 1) {
            pr->pieces[j].status = DOWNLOADED;
            printf("1");
        }
        else {
            pr->pieces[j].status = EMPTY;
            printf("0");
        }
    }
    printf("\n");
    return;
}

void handle_request(int socket) {
    UNUSED(socket);
    return;
}


void handle_piece(struct peer *pr, int length) {
    uint32_t idx; 
    uint32_t begin; 
    uint8_t buffer[length - 9];
    readBytes(pr->socket, 4, &idx);
    readBytes(pr->socket, 4, &begin);
    readBytes(pr->socket, length - 9, buffer);

    int index = ntohl(idx);
    int block = ntohl(begin)/BLOCK_SIZE;

    if(self.pieces[index].blocks[block] != DOWNLOADED) {
        printf("Downloaded piece #%d and block #%d and begin was %d\n", index, block, ntohl(begin));
        write_to_file(fp, index, block, length - 9, buffer);
        self.pieces[index].blocks[block] = DOWNLOADED;
        self.pieces[index].blocks_recieved++;
    }

    int blocks_downloaded = 0;
    for(int i = 0; i < self.pieces[index].num_blocks; i++) {
        if(self.pieces[index].blocks[i] == DOWNLOADED) {
            blocks_downloaded++;
        }
    }
    if(blocks_downloaded == self.pieces[index].num_blocks) {
        self.pieces[index].status = DOWNLOADED;
        pr->current_piece = -1;
    } 
}

void write_to_file(FILE *fp, int index, int offset, int length, uint8_t *payload) {
    fseek(fp, index * piece_length + offset * BLOCK_SIZE, SEEK_SET);
    fwrite(payload, 1, length, fp);
    rewind(fp);
}
