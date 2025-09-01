#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <math.h>
//#include <curl/curl.h>
#include <poll.h>
#include "heapless-bencode/bencode.h"
//#include "utilities.h"
//#include "utilities/utilities.c"

#define TRACKER_NUM 12

typedef struct files{
    long int length;
    char *path;     //is a list of strings but will store entire list as a single string

} files;

typedef struct metainfo {
    int piece_length;
    unsigned char **pieces;
    char *name;
    
    long int length;
    int num_files;
    struct files *files; //list of dictionaries. file[0] is a dict => length, path file[1] is a dict
    unsigned char *infohash;        //20 byte bencoded form of info dictionary
    int num_pieces;
} metainfo;


typedef struct torrent {
    char *announce; 
    char **announce_list;
    struct metainfo *info;

    long int creation_time;
    char *comment;
    char *created_by;
    char *encoding;
} torrent;



/*Reads a file and puts its contents into memory. Used for storing torrent file information*/
char *read_file(char* fileName, int *length);

/*Returns the sha1 hash of the string str*/
uint8_t *hash(char* str, int strlen);

/*Initializes our torrent structure*/
void init_torrent(struct torrent *file);

/*Parses our torrent file and stores contents into our struct*/
struct torrent *parse_torrent(char *buffer, int length);

/*Helper to parse torrent. Deals with parsing info_dict*/
void parse_info_dict(struct torrent *file, bencode_t *dict);

/*Helper to parse info_dict. Deals with parsing the files list (if contained)*/
void parse_files_list(struct torrent *file, bencode_t *list);

/*Prints Out the torrent info*/
void print_torrent(struct torrent *file);
