# Chat Client

The third assignment of my networking class at UMD. A simple chat client that can connect to a chat server and send and receive messages.

The school had a server hosted and we were given the IP address and port to connect to. Other students could connect to the server with their implementation of the client and chat with each other. There was a certain protocol we needed to follow and had to wrap messages in specific header flags in order to get the server to recognize our messages and process them correctly. 

## How to run

To run the client, use the following command:

```bash
make
./rclient
```

You can give yourself a nickname, create a room, join a room, list users, send a message, and leave a room.