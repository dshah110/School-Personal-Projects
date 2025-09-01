# Chord

Chord is a protocol and algorithm for a peer-to-peer (P2P) distributed hash table (DHT). Algorithms like this are used to store data in a distributed manner and allow for efficient lookups. 

IIRC, each node has something called a finger table where it knows the id of each node up to halfway around the ring (n + 2^i-1). It also has a successor and predecessor. 

If we want to find an element, we see if the current node as it, if not it will check for the closest predecessor to the key and ask it to find the direct successor.

The code is quite messy and if I could do it again, I would make it more strutured and organized. I'd design my solution first without implementing it.

## How to run

To run the server, use the following command:

```bash
make all
./chord
```