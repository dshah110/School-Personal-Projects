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
#include <poll.h>

#include "heapless-bencode/bencode.h"
#include "torrent.h"
//#include "utilities/hash.c"
#include "utilities/bigint.c"




char *read_file(char* fileName, int *length) {
    FILE *file;
    char *buffer;
    int fileLen;

    file = fopen(fileName, "rb");
    if(!file) {
        printf("Unable to open file. Please try another file.\n");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = malloc(fileLen + 1);
    fread(buffer, fileLen, 1, file);
    fclose(file);

    *length = fileLen + 1;
    return buffer;
}
/*
uint8_t *hash(char* str, int strlen) {
    unsigned char *digest = (unsigned char*) malloc (SHA_DIGEST_LENGTH);
    SHA1((const unsigned char*)str, strlen, digest);
    return digest;
}
*/
void init_torrent(struct torrent *file) {
    file->info = (metainfo *) malloc(sizeof(metainfo));
    file->info->length = -1;
    file->info->files = NULL;

    file->announce_list = malloc(sizeof(file->announce) * 12);    //will only have 12 different backup trackers
    for(int i = 0; i < TRACKER_NUM; i++) {
        file->announce_list[i] = NULL;
    }
}

struct torrent *parse_torrent(char *buffer, int length) {
    bencode_t ben_dict;// = (bencode_t*) malloc (sizeof(bencode_t));

    bencode_init(&ben_dict, buffer, length);

    struct torrent *file = (torrent *) malloc(sizeof(torrent)); //init torrent with null
    init_torrent(file);

    while(bencode_dict_has_next(&ben_dict)) {
        int klen;
        const char *key;
        bencode_t benk;
        bencode_dict_get_next(&ben_dict, &benk, &key, &klen);

        if (!strncmp(key, "announce", klen)) {

            int len;
            const char *val;
            bencode_string_value(&benk, &val, &len);

            file->announce = (char *) malloc(len + 1);
            strncpy(file->announce, val, len);
            file->announce[len] = '\0';
        }
        else if(!strncmp(key, "announce-list", klen)) {

            assert(bencode_is_list(&benk));

            int i = 0;
            while(bencode_list_has_next(&benk) && i < 12) {
                bencode_t innerlist;
                bencode_list_get_next(&benk, &innerlist);
                while (bencode_list_has_next(&innerlist)) {
                    bencode_t benlitem;
                    const char *backup;
                    int len;

                    bencode_list_get_next(&innerlist, &benlitem);
                    bencode_string_value(&benlitem, &backup, &len);

                    char item[len + 1]; memcpy(item, backup, len);
                    item[len] = '\0';


                    file->announce_list[i] = malloc(len+1);//item;//realloc(file->info->files, sizeof(file->announce_list) + len);

                    strncpy(file->announce_list[i], item, len);
                    file->announce_list[i][len] = '\0';
                    i++;
                }
            }


        }
        else if (!strncmp(key, "comment", klen)) {
            int len;
            const char *val;
            bencode_string_value(&benk, &val, &len);

            file->comment = (char *) malloc(len + 1);
            strncpy(file->comment, val, len);
            file->comment[len] = '\0';

        }
        else if (!strncmp(key, "created by", klen)) {
            int len;
            const char *val;
            bencode_string_value(&benk, &val, &len);

            file->created_by = malloc(len + 1);

            strncpy(file->created_by, val, len);
            file->created_by[len] = '\0';

        }
        else if (!strncmp(key, "creation date", klen)) {
            long int date;

            bencode_int_value(&benk, &date);
            file->creation_time = date;
        }
        else if(!strncmp(key, "encoding", klen)) {
            int len;
            const char *val;
            bencode_string_value(&benk, &val, &len);

            file->encoding = (char *) malloc(len + 1);
            strncpy(file->encoding, val, len);
            file->encoding[len] = '\0';
        }
        else if(!strncmp(key, "info", klen)) {

            parse_info_dict(file, &benk);
        }

    }
    return file;
}

void parse_info_dict(struct torrent *file, bencode_t *dict) {
    {
        const char *val;
        int len;
        bencode_dict_get_start_and_len(dict, &val, &len);

        char *dict_str = (char*) malloc (len + 1);
	    memcpy(dict_str, val, len);
        file->info->infohash = hash(dict_str, len);

    }
    while(bencode_dict_has_next(dict)) {
        const char *key;
        int klen;
        bencode_t benk;

        bencode_dict_get_next(dict, &benk, &key, &klen);

        //single file
        if (!strncmp(key, "length", klen)) {

            long int file_len;
            bencode_int_value(&benk, &file_len);
            bencode_dict_get_next(dict, &benk, &key, &klen);

            if (!strncmp(key, "name", klen)) {
                int len;
                const char *name;
                bencode_string_value(&benk, &name, &len);

                file->info->length = file_len;
                file->info->name = (char *) malloc(len + 1);
                strncpy(file->info->name, name, len);
                file->info->name[len] = '\0';

            }
        }
        else if (!strncmp(key, "piece length", klen)) {
            long int piece_len = 0;

            bencode_int_value(&benk, &piece_len);
            file->info->piece_length = piece_len;
        }
        else if (!strncmp(key, "pieces", klen)){


            const char *piece_hashes;// = (char *) malloc(len);
            int len;
            bencode_string_value(&benk, &piece_hashes, &len);
            file->info->num_pieces = ceil(len/20);
            file->info->pieces = malloc(file->info->num_pieces * sizeof(char *));

            for(int i = 0; i < file->info->num_pieces; i++) {
                file->info->pieces[i] = malloc(20);
                memcpy(file->info->pieces[i], piece_hashes+20*i, 20);
            }
        }

        else if(!strncmp(key, "files", klen)) {
            parse_files_list(file, &benk);
        }
        else if(!strncmp(key, "name", klen)) {

            int len;
            const char *name;
            bencode_string_value(&benk, &name, &len);

            //file->info->length = file_len;
            file->info->name = (char *) malloc(len + 1);
            strncpy(file->info->name, name, len);
            file->info->name[len] = '\0';
        }

    }

    return;
}

void parse_files_list(struct torrent *file, bencode_t *list) {
    int num_files = 1;
    //file->info->files = malloc(sizeof(struct files) * 10);

    while (bencode_list_has_next(list)) {
        bencode_t dict;
        bencode_list_get_next(list, &dict);

        file->info->files = realloc(file->info->files, sizeof(struct files) * num_files);

        while(bencode_dict_has_next(&dict)) {
            bencode_t benk;
            const char *key;
            int klen;
            bencode_dict_get_next(&dict, &benk, &key, &klen);

            //only two keys in this dictionary
            if (!strncmp(key, "length", klen)) {
                long int current_file_len = 0;
                bencode_int_value(&benk, &current_file_len);

                file->info->files[num_files - 1].length = current_file_len;
            }
            else if(!strncmp(key, "path", klen)) {
                char *fullpath = NULL;
                int fp_len = 0; 

                while (bencode_list_has_next(&benk)) {
                    const char *path;
                    bencode_t pathitem;

                    bencode_list_get_next(&benk, &pathitem);
                    bencode_string_value(&pathitem, &path, &klen);

                    if (!fullpath){
                        fullpath = strndup(path, klen);
                        fp_len = klen;
                    }
                    else{
                        fullpath = realloc(fullpath, fp_len + klen + 2);
                        fullpath[fp_len] = '/';
                        strncpy(fullpath + fp_len + 1, path, klen);
                        fp_len += klen + 1;
                    }
                }

                //probably have to strcat
                file->info->files[num_files - 1].path = malloc(fp_len + 1);
                strncpy(file->info->files[num_files - 1].path, fullpath, fp_len);
                file->info->files[num_files - 1].path[fp_len] = '\0';
                free(fullpath);
            }
        }
        num_files++;
    }
    file->info->num_files = num_files - 1;
}

void print_torrent(struct torrent *torrent_file) {
    printf("------Printing Torrent Info------\n");
    printf("\nAnnounce: %s\n", torrent_file->announce);
    printf("Announce-List: [");
    for(int i = 0; i < 12; i++) {
        if(torrent_file->announce_list[i] != NULL)
            printf("%s ", torrent_file->announce_list[i]);
    }
    printf("]\n");
    printf("Created by: %s\n", torrent_file->created_by);
    printf("Creation Date: %ld\n", torrent_file->creation_time);
    printf("Info: \n");
    printf("    Piece Length: %d\n", torrent_file->info->piece_length);
    printf("    Pieces: \n");
    for(int i = 0; i < torrent_file->info->num_pieces; i++) {
        printf("        "); print(torrent_file->info->pieces[i]); printf("\n");
    }
    printf("    Infohash is: "); print(torrent_file->info->infohash); printf("\n");


    if(torrent_file->info->length != -1) {
        printf("    Name: %s\n", torrent_file->info->name);
        printf("    File Length: %ld\n", torrent_file->info->length);
        printf("    Number of Pieces is %d\n", torrent_file->info->num_pieces);
    }   

    if(torrent_file->info->files != NULL) {
        printf("\nThere are multiple files in this torrent.\n");
        printf("The name of the directory in which to store these files is %s\n", torrent_file->info->name);
        printf("The number of files in this torrent is %d\n\n", torrent_file->info->num_files);

        for(int i = 0; i < torrent_file->info->num_files; i++) {
            printf("The length of file[%d] is %ld\n", i+1, torrent_file->info->files[i].length);
            printf("The path corresponding to it is %s\n\n", torrent_file->info->files[i].path);

        }

    }
}

