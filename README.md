# Traffic Controller
### A multithreaded server traffic simulation.

![alt text](https://github.com/brianbalpert/Traffic-Controller/blob/main/Traffic-Controller.png?raw=true)

This code simulates data packets arriving to a two server system. The servers can each process a packeta at a constate rate 'mu'. Packet arrival time is defined as the inter-packet-arrival time between two consecutive packets '1/lambda' where 'lambda' is the packet arrival rate. This is not neccissarily constant. Packet arrival to the server system is controlled by a token bucket of depth 'B'. Each packet requires 'P' tokens in order for it to be elligable for transmission. Tokens arrive in the same mannar as packets, at a time defined as the inter-token-arrival time between two consecutive tokens '1/r' where 'r' is the token arrival rate. Again, this is not neccissarily constant.

Packets arrive and are placed in queue 1. When at the head of the queue, the packet will wait for 'P' tokens to be available in the token bucket. It will then be passed to queue 2. When at the head of queue 2, the packet will wait for a server to become available.

if a packet arrives with P > B, the packet is dropped.
if the token bucket is full, and a new token arrives, the token will be lost.


#### The System can run in one of two modes:


*Deterministic*	 : 	In this mode, all inter-arrival times are equal to 1/lambda seconds, all packets require exactly P tokens, and all service times are equal to 1/mu seconds (all rounded to the nearest millisecond). All Inter-Arrival-Times are capped at 10 seconds. Parameters can be passed as arguments. Example:
`trafficController -lambda 1.0 -mu 0.35 -r 1.5 -B 10 -P 3 -n 20`
sets packets to arrive Once every second, tokens to arrive every 1.5 seconds, and the servers to process the tokens ever 0.35 seconds. There token bucket is 10 deep, each packet requires 3 tokens, and 20 packets will arrive in total.


*Trace-driven*	 : 	In this mode, we will drive the emulation using a trace specification file (will be referred to as a "tsfile"). Each line in the trace file specifies the inter-arrival time of a packet, the number of tokens it need in order for it to be eligiable for transmission, and its service time. (Please note that in this mode, it's perfectly fine if an inter-arrival time or a service time is greater than 10 seconds. Also note, the tsfile is not validated before the start of simulation.) to use a tsfile, pass the file in as an argument. Example:
`trafficController -t tsfile/f0.txt`
