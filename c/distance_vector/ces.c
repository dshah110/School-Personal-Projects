#ifndef _ES_C_
#define _ES_C_

/* $Id: es.c,v 1.1 2000/03/01 14:09:09 bobby Exp bobby $
 * ----
 * $Revision: 1.1 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include "queue.h"
#include "common.h"
#include "es.h"
#include "ls.h"
#include "rt.h"
#include "n2h.h"

static struct el *g_el;
//static struct link *g_ls;

void runDVA(int *, int);

void listenForInput(int *, int *, int);

int init_new_el() {
    InitDQ(g_el,
    struct el);
    assert (g_el);

    g_el->es_head = 0x0;
    return (g_el != 0x0);
}

void add_new_es() {
    struct es *n_es;
    struct el *n_el = (struct el *)
            getmem(sizeof(struct el));

    struct el *tail = g_el->prev;
    InsertDQ(tail, n_el);

    // added new es to tail
    // lets start a new queue here

    {
        struct es *nhead = tail->es_head;
        InitDQ(nhead,
        struct es);

        tail = g_el->prev;
        tail->es_head = nhead;

        n_es = nhead;

        n_es->ev = _es_null;
        n_es->peer0 = n_es->peer1 =
        n_es->port0 = n_es->port1 =
        n_es->cost = -1;
        n_es->name = 0x0;
    }
}

void add_to_last_es(e_type ev,
                    node peer0, int port0,
                    node peer1, int port1,
                    int cost, char *name) {
    struct el *tail = g_el->prev;
    bool local_event = false;

    assert (tail->es_head);

    // check for re-defined link (for establish)
    // check for local event (for tear-down, update)
    switch (ev) {
        case _es_link:
            // a local event?
            if ((peer0 == get_myid()) || peer1 == get_myid())
                local_event = true;
            break;
        case _ud_link:
            // a local event?
            if (geteventbylink(name))
                local_event = true;
            break;
        case _td_link:
            // a local event?
            if (geteventbylink(name))
                local_event = true;
            break;
        default:
            printf("[es]\t\tUnknown event!\n");
            break;
    }

    if (!local_event) {
        printf("[es]\t Not a local event, skip\n");
        return;
    }

    printf("[es]\t Adding into local event\n");

    {
        struct es *es_tail = (tail->es_head)->prev;

        struct es *n_es = (struct es *)
                getmem(sizeof(struct es));

        n_es->ev = ev;
        n_es->peer0 = peer0;
        n_es->port0 = port0;
        n_es->peer1 = peer1;
        n_es->port1 = port1;
        n_es->cost = cost;
        n_es->name = (char *)
                getmem(strlen(name) + 1);
        strcpy(n_es->name, name);

        InsertDQ(es_tail, n_es);
    }
}

/*
 * A simple walk of event sets: dispatch and print a event SET every 2 sec
 */
void walk_el(int update_time, int time_between, int verb) {
    struct el *el;
    struct es *es_hd;
    struct es *es;

    assert (g_el->next);
    assert (get_myid >= 0);

    print_el();

    /* initialize link set, routing table, and routing table */
    create_ls();
    create_rt();
    init_rt_from_n2h();

    for (el = g_el->next; el != g_el; el = el->next) {
        assert(el);
        es_hd = el->es_head;
        assert (es_hd);

        int *updated = (int *) calloc(256, sizeof(int));
        struct es *myCurrES = es_hd->next;
        while (myCurrES->ev != _es_null) {
            if (myCurrES->ev == _es_link) {
                if ((int) get_myid() == myCurrES->peer0) {
                    update_rte(myCurrES->peer1, myCurrES->cost, myCurrES->peer1);
                    updated[myCurrES->peer1] = 1;
                } else {
                    update_rte(myCurrES->peer0, myCurrES->cost, myCurrES->peer0);
                    updated[myCurrES->peer0] = 1;
                }
            } else if (myCurrES->ev == _td_link) {
                struct link *linkToTd = find_link(myCurrES->name);
                if (get_myid() == linkToTd->peer0) {
                    update_rte(linkToTd->peer1, -1, linkToTd->peer1);
                    updated[myCurrES->peer1] = 1;
                } else {
                    update_rte(linkToTd->peer0, -1, linkToTd->peer0);
                    updated[myCurrES->peer0] = 1;
                }
            } else if (myCurrES->ev == _ud_link) {
                struct link *linkToUd = find_link(myCurrES->name);
                if (get_myid() == linkToUd->peer0) {
                    update_rte(linkToUd->peer1, myCurrES->cost, linkToUd->peer1);
                    updated[myCurrES->peer1] = 1;
                } else {
                    update_rte(linkToUd->peer0, myCurrES->cost, linkToUd->peer0);
                    updated[myCurrES->peer0] = 1;
                }
            }
            myCurrES = myCurrES->next;
        }

        printf("[es] >>>>>>>>>> Dispatch next event set <<<<<<<<<<<<<\n");
        for (es = es_hd->next; es != es_hd; es = es->next) {
            printf("[es] Dispatching next event ... \n");
            dispatch_event(es);
        }

        //routing table has been updated at this point



        runDVA(updated, 1);


        sleep(3);
        printf("[es] >>>>>>> Start dumping data stuctures <<<<<<<<<<<\n");
        print_n2h();
        print_ls();
        print_rt();
    }
}

void runDVA(int *updated, int isFirst) {
    char *advertise = calloc(1032, 1);
    char type = 0x7;
    char version = 0x1;
    short numUpdates = 0;

    char *tempPoint = (char *) &numUpdates;
    advertise[0] = type;
    advertise[1] = version;
    advertise[2] = tempPoint[0];
    advertise[3] = tempPoint[1];

    advertise[4] = '\0';
    //Build update
    int itt = 0;
    int entry = 1;
    while (itt < 256) {
        if (updated[itt]) {
            unsigned char node = itt;
            struct rte *thisRte = find_rte(itt);
            tempPoint = (char *) &thisRte->c;
            advertise[entry * 4] = node;
            advertise[entry * 4 + 1] = tempPoint[0];
            advertise[entry * 4 + 2] = tempPoint[1];
            advertise[entry * 4 + 3] = tempPoint[2];

            advertise[entry * 4 + 4] = 0;
            updated[itt] = 0;
            numUpdates += 1;
            tempPoint = (char *) &numUpdates;
            advertise[2] = tempPoint[0];
            advertise[3] = tempPoint[1];

            entry += 1;
        }
        itt += 1;
    }
	

    //Create a set of all the sockets on this node 
    int *socketSet = (int *) calloc(256, sizeof(int));
    int *portSet = (int *) calloc(256, sizeof(int));

    //Pointing to the first element of the list of links
    struct link *myLs = get_ls()->next;
	

    itt = 0;
    int maxDescriptor = -1;
    while (myLs->name != NULL) {
        if (get_myid() == myLs->peer0) {
            socketSet[itt] = myLs->sockfd0;
            portSet[itt] = myLs->port1;
        } else if (get_myid() == myLs->peer1) {
            socketSet[itt] = myLs->sockfd1;
            portSet[itt] = myLs->port0;
        }
        if (socketSet[itt] > maxDescriptor) {
            maxDescriptor = socketSet[itt];
        }

        itt += 1;
        myLs = myLs->next;
    }
    itt = 0;
	

    while (socketSet[itt] > 0) {
        struct addrinfo *myAddr;
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;

        char *portBuff = calloc(6, 1);


        sprintf(portBuff, "%d", portSet[itt]);
        getaddrinfo("localhost", portBuff, &hints, &myAddr);

        sendto(socketSet[itt], advertise, entry * 4, 0,
               myAddr->ai_addr, myAddr->ai_addrlen);


        itt += 1;
    }

    if (isFirst) {
        listenForInput(socketSet, portSet, maxDescriptor);
    }
}


void listenForInput(int *socketSet, int *portSet, int maxDescriptor) {
    int running = 1;
    fd_set sockSet;
    int *updated = calloc(256, sizeof(int));
    while (running) {
        FD_ZERO(&sockSet);

        int updatedSomething = 0;
        int itt = 0;
        while (socketSet[itt] > 0) {
            FD_SET(socketSet[itt], &sockSet);
            itt += 1;
        }
        struct timeval selTimeout;
        selTimeout.tv_sec = 3;
        selTimeout.tv_usec = 0;

        if (select(maxDescriptor + 1, &sockSet, NULL, NULL, &selTimeout) == 0) {
            running = 0;
        }
        itt = 0;
        while (socketSet[itt] > 0) {
            if (FD_ISSET(socketSet[itt], &sockSet)) {
                struct sockaddr_storage clntAddr;
                socklen_t clntAddrLen = sizeof(clntAddr);
                char rcvd[1032];
                recvfrom(socketSet[itt], rcvd, 1032, 0,
                         (struct sockaddr *) &clntAddr, &clntAddrLen);
                struct link *tempLs = get_ls()->next;//g_ls->next;
                int sender;
                while (tempLs != get_ls()) {//g_ls) {
                    if (tempLs->sockfd0 == socketSet[itt]) {
                        sender = tempLs->peer1;
                    } else if (tempLs->sockfd1 == socketSet[itt]) {
                        sender = tempLs->peer0;
                    }
                    tempLs = tempLs->next;
                }


                short updateSize = rcvd[2];
                int tempItt = 1;
                while (tempItt <= updateSize) {
                    int node = rcvd[tempItt * 4];
                    if (node == (int) get_myid()) {
                        tempItt += 1;
                        continue;
                    }
                    int distance = rcvd[tempItt * 4 + 1];
                    distance <<= 8;
                    distance >>= 8;
                    struct rte *currentBest = find_rte(node);
                    int distanceToSender = find_rte(sender)->c;
                    if ((int) currentBest->c == -1) {
                        update_rte(node, distanceToSender + distance, sender);
                        updated[node] = 1;
                        updatedSomething = 1;
                    } else if (distanceToSender + distance < (int) currentBest->c) {
                        update_rte(node, distanceToSender + distance, sender);
                        updated[node] = 1;
                        updatedSomething = 1;
                    } else if ((int)currentBest->d == node && (int)currentBest->nh == sender) {
                        update_rte(node, distanceToSender + distance, sender);
                        updated[node] = 1;
                        updatedSomething = 1;
                    }
                    tempItt += 1;
                }
            }

            itt += 1;
        }
        if (updatedSomething) {
            runDVA(updated, 0);
        }
    }
}


/*
 * -------------------------------------
 * Dispatch one event
 * -------------------------------------
 */
void dispatch_event(struct es *es) {
    assert(es);

    switch (es->ev) {
        case _es_link:
            add_link(es->peer0, es->port0, es->peer1, es->port1,
                     es->cost, es->name);
            break;
        case _ud_link:
            ud_link(es->name, es->cost);
            break;
        case _td_link:
            del_link(es->name);
            break;
        default:
            printf("[es]\t\tUnknown event!\n");
            break;
    }

}

/*
 * print out the whole event LIST
 */
void print_el() {
    struct el *el;
    struct es *es_hd;
    struct es *es;

    assert (g_el->next);

    printf("\n\n");
    printf("[es] >>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    printf("[es] >>>>>>>>>> Dumping all event sets  <<<<<<<<<<<<<\n");
    printf("[es] >>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<\n");

    for (el = g_el->next; el != g_el; el = el->next) {

        assert(el);
        es_hd = el->es_head;
        assert (es_hd);

        printf("\n[es] ***** Dumping next event set *****\n");

        for (es = es_hd->next; es != es_hd; es = es->next)
            print_event(es);
    }
}

/*
 * print out one event: establish, update, or, teardown
 */
void print_event(struct es *es) {
    assert(es);

    switch (es->ev) {
        case _es_null:
            printf("[es]\t----- NULL event -----\n");
            break;
        case _es_link:
            printf("[es]\t----- Establish event -----\n");
            break;
        case _ud_link:
            printf("[es]\t----- Update event -----\n");
            break;
        case _td_link:
            printf("[es]\t----- Teardown event -----\n");
            break;
        default:
            printf("[es]\t----- Unknown event-----\n");
            break;
    }
    printf("[es]\t link-name(%s)\n", es->name);
    printf("[es]\t node(%d)port(%d) <--> node(%d)port(%d)\n",
           es->peer0, es->port0, es->peer1, es->port1);
    printf("[es]\t cost(%d)\n", es->cost);
}

struct es *geteventbylink(char *lname) {
    struct el *el;
    struct es *es_hd;
    struct es *es;

    assert (g_el->next);
    assert (lname);

    for (el = g_el->next; el != g_el; el = el->next) {

        assert(el);
        es_hd = el->es_head;
        assert (es_hd);

        for (es = es_hd->next; es != es_hd; es = es->next)
            if (!strcmp(lname, es->name))
                return es;
    }
    return 0x0;
}

#endif