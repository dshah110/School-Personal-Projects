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
#include "sending.c"
//#include "peer_handler.c"
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

    if(debug) printf("Set up servAddr struct for connection.\n");

    struct timeval timeout;
	timeout.tv_sec  = 1;  
	timeout.tv_usec = 0; //timeout in 0.5 seconds
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)); 

    if(debug) printf("Set up timeout for socket\n");

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

    int bytes_sent = sendBytes(pr.socket, 68, buffer);
    if(bytes_sent <= 0) {
        printf("No bytes sent\n");
    }
    else {
        printf("Sent along %d bytes\n", bytes_sent);
    }
    void *recv_buffer = malloc(sizeof(uint8_t) * 68);
    printf("Socket is %d\n", pr.socket);
    int bytes_read = readBytes(pr.socket, 68, recv_buffer);
    if(bytes_read <= 0) {
        printf("No bytes recieved. Closing Connection\n");
        close(pr.socket);
        return -1;
    }
    //validation check


    uint8_t rlen; char rstr[20]; uint8_t rres[8]; uint8_t rethash[20];// uint8_t retid[20];
    void * retid = malloc(sizeof(uint8_t) * 20);
    memset(rstr, 0, 20);
    memcpy(&rlen, recv_buffer, 1);
    memcpy(&rstr, recv_buffer + 1, 19);
    memcpy(&rres, recv_buffer + 20, 8);
    memcpy(&rethash, recv_buffer + 28, 20);
    memcpy(retid, recv_buffer + 48, 20);

    printf("Our return string and length was %d and %s\n", rlen, rstr);
    printf("The peer id we got was: "); print(retid); printf("\n");

    if(memcmp(rethash, info_hash, 20) != 0) {
        printf("Closing connection because info hash does not match\n");
        close(pr.socket);
        return -1;
    }
    //wont always return same peer id
   /* if(memcmp(retid, pr.id, 20) != 0) {
        printf("The peer id we got was: "); print(retid); printf("\n");
        printf("Closing connection because peer_id does not match\n");
        close(pr.socket);
        return -1;
    } */
    return 1;
}

int peer_has_piece(struct peer *pr, int index) {
    return (pr->pieces[index].status == DOWNLOADED) ? 1 : 0;
}


//information is not modified
void next_piece_to_request(struct peer *pr, int *index, int *offset) {
    int return_index = -1;
    int return_offset = -1;
    for(int i = 0; i < num_pieces; i++) {
        if(return_index != -1 && return_offset != -1) {
            break;
        }
        if(self.pieces[i].status == DOWNLOADED) {
            printf("In downloaded\n");
            continue;
        }
        else if(self.pieces[i].status == IN_PROGRESS) {
            //printf("In in-progress\n");
            if(peer_has_piece(pr, i) == 1) {
                for(int j = 0; j < self.pieces[i].num_blocks; j++) {
                    if(self.pieces[i].blocks[j] == EMPTY) {
                        printf("In this if stmt\n");
                        *index = i;
                        *offset = j;
                        return;
                    }
                }
            }
        }
        else if(self.pieces[i].status == EMPTY && peer_has_piece(pr, i) == 1) {
            printf("In empty\n");
            return_index = i;
            return_offset = 0;
            break;
        }
    }
    if(return_index == -1 && return_offset == -1) {
        printf("Got in this if stmt. Should get in here until later down\n");
        for(int i = 0; i < num_pieces; i++) {
            if(pr->pieces[i].status == IN_PROGRESS) {
                for(int j = 0; j < pr->pieces[i].num_blocks; j++) {
                    if(pr->pieces[i].blocks[j] == IN_PROGRESS) {
                        return_index = i;
                        return_offset = j;
                    }
                }
            }
        }
        //printf("some pieces have not arrived");
    }
    printf("Return index is %d returned offset is %d and num_offsets is %d\n", return_index, return_offset, pr->pieces[0].num_blocks);
    *index = return_index;
    *offset = return_offset;
    return;
}

void request(struct peer *pr) {
            int index = -1;
        int offset = -1;
        next_piece_to_request(pr, &index, &offset);
        printf("Index is %d\n", index);

        if(index >= 0) {
            if(pr->states.am_interested == 0) {
                printf("Sending interested\n");
                
                interested(pr);
            }
            else { //already interested
                if(pr->states.peer_choking == 0) { //and peer not choking us, 
                    printf("About to send request of index %d and offset %d also num_pieces is %d\n", index, offset, num_pieces);
                    printf("self.pieces[index].num_blocks is %d\n", self.pieces[index].num_blocks);
                    if(self.pieces[index].status == EMPTY) {
                        //set it to inprogress
                        self.pieces[index].status = IN_PROGRESS;
                        if(index == num_pieces - 1 && offset == self.pieces[index].num_blocks - 1) {
                            //last piece
                            printf("We are in last piece!!!!!!\n");
                            printf("going to send a request of index %d and offset %d\n", index, offset);
                            printf("Bytes in the last offset is %d\n", self.pieces[index].bytes_last_block);
                            //printf("Going to send a request to this peer of index %d, offset %d (should be 0) and bytes %d", index, offset, self.pieces[index].bytes_last_block);



                            send_piece_request(pr, index, offset * BLOCK_SIZE, self.pieces[index].bytes_last_block);
                        }
                        else {
                            send_piece_request(pr, index, offset * BLOCK_SIZE, BLOCK_SIZE);
                        }


                    }
                    //if(index == num_pieces - 1 && offset > self.pieces[index].num_blocks) {
                        //do something
                    //} //it self.pieces[index].status = IN_PROGRESS then wait till we get it twice more after which we resend.
                    //send_piece_request(pr, index, offset * BLOCK_SIZE, BLOCK_SIZE); //move this in first if stmt
                    //send_piece_request();
                }
            }
        }
}


void handle_peer(struct peer *pr, int socket) {
    uint32_t length; uint8_t id;
    readBytes(socket, 4, &length); length = ntohl(length);
    if(length == 0) {
        printf("Keep Alive Message\n");
        return;
    }
    readBytes(socket, 1, &id);
    if(id == 0) {
        printf("Recieved a choke message\n");

        pr->states.peer_choking = 1; 
        return; //think we retrun

    }
    else if(id == 1) {
        printf("Recieved an unchoke message\n");

        pr->states.peer_choking = 0; 
        //if we are not already interested in them and this peer has a piece we want send them an interested message.

    }
    else if(id == 2) {
        printf("Recieved an interested message\n");

        pr->states.peer_interested = 1;

    }
    else if(id == 3) {
        printf("Recieved a not-interested message\n");

        pr->states.peer_interested = 0;
    
    }
    else if(id == 4) {
        printf("Recieved a have message\n");
        uint32_t index;
        readBytes(socket, 4, &index); index = ntohl(index);

        pr->pieces[index].status = DOWNLOADED;
    }
    else if(id == 5) {
        printf("Recieved bitfield\n");
        int byte_size = 8;
        int num_bytes = length - 1;
        uint8_t payload[num_bytes]; uint8_t temp[8*num_bytes];
        readBytes(socket, num_bytes, payload);
        uint8_t bits[8*num_bytes]; 
        for (int j=0; (long unsigned int) j < sizeof(payload)*8; j++) {
            temp[j] = ((1 << (j % 8)) & (payload[j/8])) >> (j % 8);
        }
        for(int idx = 0; idx < num_bytes; idx ++) {
            for(int jdx = 0, k = byte_size - 1; jdx < byte_size; jdx++, k--) {
                bits[idx * byte_size + jdx]  = temp[byte_size * idx + k];
            }
        }
        //memcpy(pr->pieces, bits, num_pieces);
        for(int j = 0; j < num_pieces; j++) {
            if(bits[j] == 1) {
                pr->pieces[j].status = DOWNLOADED;
                printf("%d,", 1);
            }
            else {
                pr->pieces[j].status = EMPTY;
                printf("%d", 0);
            }
        }
        printf("]\n"); 
    }
    else if(id == 6) {
        printf("Recieved a request message\n");

    }
    else if(id == 7) {
        printf("Recieved a piece message\n");

        handle_piece(pr, length);

    }
    //else {


        int index = -1;
        int offset = -1;
        next_piece_to_request(pr, &index, &offset);
        printf("Index is %d\n", index);

        if(index >= 0) {
            if(pr->states.am_interested == 0) {
                printf("Sending interested\n");
                interested(pr);
            }
            else { //already interested
                if(pr->states.peer_choking == 0) { //and peer not choking us, 
                    printf("About to send request of index %d and offset %d also num_pieces is %d\n", index, offset, num_pieces);
                    printf("self.pieces[index].num_blocks is %d\n", self.pieces[index].num_blocks);
                    if(self.pieces[index].status == EMPTY) {
                        //set it to inprogress
                        self.pieces[index].status = IN_PROGRESS;
                        if(index == num_pieces - 1 && offset == self.pieces[index].num_blocks - 1) {
                            //last piece
                            printf("We are in last piece!!!!!!\n");
                            printf("going to send a request of index %d and offset %d\n", index, offset);
                            printf("Bytes in the last offset is %d\n", self.pieces[index].bytes_last_block);
                            //printf("Going to send a request to this peer of index %d, offset %d (should be 0) and bytes %d", index, offset, self.pieces[index].bytes_last_block);



                            send_piece_request(pr, index, offset * BLOCK_SIZE, self.pieces[index].bytes_last_block);
                        }
                        else {
                            send_piece_request(pr, index, offset * BLOCK_SIZE, BLOCK_SIZE);
                        }


                    }
                    //if(index == num_pieces - 1 && offset > self.pieces[index].num_blocks) {
                        //do something
                    //} //it self.pieces[index].status = IN_PROGRESS then wait till we get it twice more after which we resend.
                    //send_piece_request(pr, index, offset * BLOCK_SIZE, BLOCK_SIZE); //move this in first if stmt
                    //send_piece_request();
                }
            }
        }
        
        
        //request(pr);
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
    strncpy(file_name, file->info->name, strlen(file->info->name));
    file_name[strlen(file->info->name)] = '\0';

    memcpy(&num_pieces, &(file->info->num_pieces), 4);
    memcpy(&piece_length, &(file->info->piece_length), 4);
    fp = fopen(file_name, "w+b");
    /*END OF CREATING FILE AND STORING DATA*/

     /*SETTING UP COMMUNICATION STATE AND SELF'S STATE*/
    int num_offsets = piece_length / BLOCK_SIZE;
    printf("USUALLY NUM_OFFSETS IS %d\n", num_offsets);
    printf("Num of pieces is %d\n", num_pieces);
    self.pieces = (struct piece *) malloc(sizeof(struct piece) * num_pieces); //modify piece length here and offset values here
    printf("Past self.pieces\n");
    for(int i = 0 ; i < num_pieces - 1;  i++) { //setting up all but last piece
        self.pieces[i].status = EMPTY;
        self.pieces[i].num_blocks = num_offsets;
        self.pieces[i].blocks_recieved = 0;
        self.pieces[i].blocks = malloc(sizeof(uint8_t) * num_offsets);
        self.pieces[i].bytes_last_block = BLOCK_SIZE;
        memset(self.pieces[i].blocks, EMPTY, num_offsets);
    }
    
    //setting up last piece

    int num_bytes_last_piece = file->info->length - (num_pieces - 1) * piece_length;
    int offsets_last = floor(num_bytes_last_piece / BLOCK_SIZE) + 1;
    int bytes_last_offset = num_bytes_last_piece -  (offsets_last - 1) * BLOCK_SIZE;
    self.pieces[num_pieces - 1].status = EMPTY;
    self.pieces[num_pieces - 1].num_blocks = offsets_last;
    self.pieces[num_pieces - 1].blocks_recieved = 0;
    self.pieces[num_pieces - 1].blocks = malloc(sizeof(uint8_t) * num_offsets);
    self.pieces[num_pieces - 1].bytes_last_block = bytes_last_offset;

    printf("THE LAST PIECE INFO WE HAVE IS:\nTHE NUM_BYTES IN LAST PIECE IS% d\nTHE NUM_OFFSETS IN LAST PIECE IS %d\nTHE NUM OF BYTES IN LAST OFFSET IS %d\n", num_bytes_last_piece, offsets_last, bytes_last_offset);




    int port = 33333;
    struct tracker_response *response = get_response(0, port, file); 
    if(response == NULL) {
        printf("Error in parsing tracker response\n");
        return 1;
    }
    int responsive_peers = 0;
    struct peer *tracker_peers = response->peers; //sometimes we will get bad peers. 
    struct peer *peers = get_responsive_peers(tracker_peers, response->num_peers, &responsive_peers);
    /*END OF PARSING TRACKER RESPONSE*/

    if (debug){ //debug information
        print_response(response);printf("\n");
        printf("The number of responsive peers is %d\n", responsive_peers);
        printf("-----Printing Responsive Peers-----\n\n");
        for(int i = 0; i < responsive_peers; i++) {
            if(peers[i].id != NULL) {printf("Peer ID is "); print((uint8_t*)peers[i].id);}//, resp->peers[i].id);// print((uint8_t*) resp->peers[i].id);//%s IP address is %s and port is %d", resp->peers[i].id, resp->peers[i].addr, resp->peers[i].port);
            printf(" IP address is %s and port is %d", peers[i].addr, peers[i].port);
            printf("\n");
        }
    }


    struct pollfd sockets[51]; //will be communicating with at most 50 different peers. No need for anymore.
    sockets[0].fd = setup_sock(9903); sockets[0].events = POLLIN;
    for(int i = 1; i <= responsive_peers; i++) { //
        sockets[i].fd = peers[i-1].socket;
        sockets[i].events = POLLIN;
    }

    printf("About to handshake\n");


    /*HANDSHAKING TO BEGIN PROTOCOL*/
    for(int i = 1; i <= responsive_peers; i++) {
        printf("Handhshaking with peer(addr=%s and port = %d)\n", peers[i-1].addr, peers[i-1].port);
        printf("Their peer id is: "); print((uint8_t *)peers[i-1].id); printf("\n");
        int success = handshake(peers[i-1], file->info->infohash, self.id);
        if(success < 0) {
            peers[i-1].socket = -1;
        }
    }
    printf("Got past handshaking\n");
    for(int i = 1; i <= responsive_peers; i++) {
        if(peers[i-1].socket != -1) {
            printf("We are connected to peer %s and port %d\n", peers[i-1].addr, peers[i-1].port);
        }
    }

    for(;;) {
        if(poll(sockets, responsive_peers + 1, 0) > 0) {
            if(sockets[0].revents && POLLIN) {  
                printf("New Incoming Connection\n");
            }
            else {
                for(int i = 1; i <= responsive_peers; i++) {
                    if((sockets[i].revents && POLLIN) && peers[i-1].socket != -1) {
                        //incoming peer message
                        // uint32_t length;
                        // readBytes(sockets[i].fd, 4, &length); 
                        // length = ntohl(length);
                        handle_peer(&peers[i-1], sockets[i].fd);
                    }

                }
            }
        }
       /* for(int i = 1; i <= responsive_peers; i++) {
            if(peers[i-1].socket != -1) {
                //request(&peers[i-1]);
            }
        } */
    }




}