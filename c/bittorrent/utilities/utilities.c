#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "utilities.h"

int debug = 1;
struct peer self;

int readBytes(int socket, size_t length, void* buffer) {
	size_t recieved = 0;
	while(recieved < length) {
        recieved += recv(socket, buffer + recieved, length - recieved, 0);
	}
	return recieved;
}

int sendBytes(int socket, size_t length, void* buffer) {
	size_t sent = 0;
	while(sent < length) {
		sent += send(socket, buffer + sent, length - sent, 0);
	}
	return sent;
}


uint8_t *hash(char* str, int strlen) {
    unsigned char *digest = (unsigned char*) malloc (SHA_DIGEST_LENGTH);
    SHA1((const unsigned char*)str, strlen, digest);
    return digest;
}

/*Lib curl documentation*/
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

uint8_t *gen_peerid() {
    uint8_t *id = (uint8_t *)malloc(21); time_t t;
    memset(id, 0 , 21);
    srand((unsigned) time(&t));
    strcat((char *)id, "-DS1159-");
    for(int i = 0; i < 12; i++) {
        uint8_t temp[2];
        sprintf((char *)temp, "%1d", rand()%10);
        strncat((char *)id, (char *)temp, 1);
    }
    return id;

}

void parse_peer_list(bencode_t *peers, struct tracker_response *resp) {

  int num_peers = 1;
  //bencode_list_get_next(peers, &dict);
  if(bencode_is_list(peers)) {
      while(bencode_list_has_next(peers)) {
          bencode_t dict;
          bencode_list_get_next(peers, &dict);
          resp->peers = realloc(resp->peers, sizeof(peer) * num_peers); //allocates memory for each additional peer
          while(bencode_dict_has_next(&dict)) {
              bencode_t benk;
              const char *key;
              int klen;
              bencode_dict_get_next(&dict, &benk, &key, &klen);

              if (!strncmp(key, "peer id", klen)) {
                  int len;
                  const char *id;
                  bencode_string_value(&benk, &id, &len);

                  resp->peers[num_peers - 1].id = malloc(len);
                  memcpy(resp->peers[num_peers-1].id, id, len); //change back to len. should be 20 bytes but bad peers
                  //print(resp->peers[num_peers-1].id); printf("\n");
                  printf("The peer is %s AND THE BYTE VERSION IS: ", resp->peers[num_peers - 1].addr);print(resp->peers[num_peers - 1].id); printf("\n");

              }   
              else if(!strncmp(key, "ip", klen)) {
                  int len;
                  const char *val;
                  bencode_string_value(&benk, &val, &len);

                  resp->peers[num_peers - 1].addr = malloc(len + 1);
                  strncpy(resp->peers[num_peers - 1].addr, val, len);
                  resp->peers[num_peers - 1].addr[len] = '\0'; 
              }
              else if(!strncmp(key, "port", klen)) {
                  int port = 0;
                  bencode_int_value(&benk, (long int *)&port);
                  resp->peers[num_peers - 1].port = port;
              }
          }

          num_peers++;
      }
  }
  else if(bencode_is_string(peers)) {
      int len;
      const char *val;
      bencode_string_value(peers, &val, &len);

      uint8_t *peer_str = malloc(len);
      memcpy(peer_str, val, len);
      
      /*Converting to string and original port number for consistencies sake*/
      for(int i = 0; i < len; i+= 6) {
          resp->peers = realloc(resp->peers, sizeof(peer) * num_peers);
          resp->peers->id = NULL;
          uint32_t ip;
          uint16_t port;
          memcpy(&ip, peer_str + i, 4);
          memcpy(&port, peer_str + i + 4, 2);

          struct in_addr ip_addr;
          ip_addr.s_addr = ip; 
        
          char *val = inet_ntoa(ip_addr);
          int len = strlen(val);

          resp->peers[num_peers - 1].addr = (char *) malloc(len + 1);
          strncpy(resp->peers[num_peers - 1].addr, val, len);
          resp->peers[num_peers - 1].addr[len] = '\0'; 

          resp->peers[num_peers - 1].port = ntohs(port);
          num_peers++;
      }

  }
  resp->num_peers = num_peers - 1;//resp->complete + resp->incomplete;//num_peers - 1;
}

struct tracker_response *parse_tracker_response(int compact, char *response, int length) {
  bencode_t ben;
  bencode_init(&ben, response, length);

  struct tracker_response *resp = (tracker_response *) malloc(sizeof(tracker_response));
  resp->compact = compact;

  if (!bencode_is_dict(&ben)) {
      printf("\n\nresponse is not bencode dictionary\n");
      return NULL;
  }

  while(bencode_dict_has_next(&ben)) {
      int klen;
      const char *key;
      bencode_t benk;

      bencode_dict_get_next(&ben, &benk, &key, &klen);
      //printf("Our key is %s and \nkey length is %d\n", key, klen);
      if (!strncmp(key, "failure reason", klen)) {
          int len;
          const char *val;
          bencode_string_value(&benk, &val, &len);

          char reason[len + 1];
          strncpy(reason, val, len);
          reason[len] = '\0';
          printf("Parsing Tracker Response Failed: %s\n", reason);
          return NULL;
      }
      else if(!strncmp(key, "interval", klen)) {
          //printf("We are in interval\n");
          int interval = 0;
          bencode_int_value(&benk, (long int *)&interval);
          //printf("Interval value is %d\n", interval);

          resp->interval = interval;
          //store interval somehow

      }
      else if(!strncmp(key, "tracker id", klen)) {
          int len;
          const char *val;
          bencode_string_value(&benk, &val, &len);

          resp->tracker_id = (char *) malloc(len + 1);
          strncpy(resp->tracker_id, val, len);
          resp->tracker_id[len] = '\0';
      }
      else if(!strncmp(key, "complete", klen)) {
          long int complete = 0;
          bencode_int_value(&benk, &complete);

          resp->complete = complete;
      }
      else if(!strncmp(key, "incomplete", klen)) {
          long int incomplete = 0;
          bencode_int_value(&benk, &incomplete);

          resp->incomplete = incomplete;
      }
      else if(!strncmp(key, "peers", klen)) {
        //printf("We are in peers\n");
          resp->peers = NULL;
          //printf("about to parse peers\n");
          //bencode_dict_get_next(&ben, &benk, &key, &klen);
          parse_peer_list(&benk, resp);
      }

  }
  return resp;
}

struct tracker_response *get_response(int compact, int port, struct torrent *file) {

  uint8_t *response;
  int size;
  {
      CURL *curl;
      CURLcode res;

      struct MemoryStruct chunk;
      chunk.memory = malloc(1);  
      chunk.size = 0;

      curl_global_init(CURL_GLOBAL_ALL);
      curl = curl_easy_init();
      if(!curl) {
          printf("Something failed, about to exit.\n");
          return NULL;
      }   
      struct timeval curr_time;
      gettimeofday(&curr_time, NULL);

      uint8_t *id = gen_peerid();
      printf("Our peer id is %s\n", (char *) id);

      self.id = id; 
      self.port = port;
      self.addr = (char *) malloc(16); self.addr = "10.104.212.219";
      
      char *escaped_id = curl_easy_escape(curl, (char *)id, 20);
      char *info_hash = curl_easy_escape(curl, (char *)file->info->infohash, 20);
      char query[500];
      sprintf(query, "%s?info_hash=%s&peer_id=%s&port=%d&uploaded=0&downloaded=0&left=%ld&compact=%d&event=started",
                      file->announce, info_hash, escaped_id, port, file->info->length, compact);   //unsure how to proceed in the case of multiple files

      if(debug == 1) {printf("Our query to the tracker is: \n%s\n", query);}

      curl_easy_setopt(curl, CURLOPT_URL, query);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
      res = curl_easy_perform(curl);
      //sticks after this line if no tracker present.

      if(res != CURLE_OK) {
          fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
          return NULL;
      }

      response = (uint8_t *) malloc(chunk.size);//[chunk.size];
      size = chunk.size;
      memcpy(response, chunk.memory, chunk.size);
      /*At this point we have stored our response in memory*/
      curl_easy_cleanup(curl);

  }
  if(debug == 1) printf("Our response is:\n%s\n\n", (char *)response);


  struct tracker_response *resp = parse_tracker_response(compact, (char *)response, size);
  //printf("Interval here is %ld\n", resp->interval);
  return resp;

}

void print_response(struct tracker_response *resp) {
  printf("------Printing Tracker Response Info------\n");
  if(resp->tracker_id != NULL) printf("Tracker id is %s\n", resp->tracker_id);
  printf("Interval is %ld\n", resp->interval);
  printf("Num of complete peers is %ld and Num of incomplete peers is %ld\n", resp->complete, resp->incomplete);

  printf("Number of peers is %d\n", resp->num_peers);

  for(int i = 0; i < resp->num_peers; i++) {
      if(resp->peers[i].id != NULL) {printf("Peer ID is "); print((uint8_t*) resp->peers[i].id);}//, resp->peers[i].id);// print((uint8_t*) resp->peers[i].id);//%s IP address is %s and port is %d", resp->peers[i].id, resp->peers[i].addr, resp->peers[i].port);
      printf(" IP address is %s and port is %d", resp->peers[i].addr, resp->peers[i].port);
      printf("\n");
  } 

}

float diff(struct timeval t0, struct timeval t1) {
    return ((t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f);
}