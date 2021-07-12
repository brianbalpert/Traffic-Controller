# Traffic Controller
### A multithreaded server traffic simulation.

![alt text](https://github.com/brianbalpert/Traffic-Controller/blob/main/Traffic-Controller.png?raw=true)

This code simulates data packets arriving to a two server system. The servers can each process a packeta at a constate rate 'mu'. Packet arrival time is defined as the inter-packet-arrival time between two consecutive packets '1/lambda' where 'lambda' is the packet arrival rate. This is not neccissarily constant. Packet arrival to the server system is controlled by a token bucket of depth 'B'. Each packet requires 'P' tokens in order for it to be elligable for transmission. Tokens arrive in the same mannar as packets, at a time defined as the inter-token-arrival time between two consecutive tokens '1/r' where 'r' is the token arrival rate. Again, this is not neccissarily constant.

Packets arrive and are placed in queue 1. When at the head of the queue, the packet will wait for 'P' tokens to be available in the token bucket. It will then be passed to queue 2. When at the head of queue 2, the packet will wait for a server to become available.

if a packet arrives with P > B, the packet is dropped.
if the token bucket is full, and a new token arrives, the token will be lost.

### The System can run in one of two modes:

*Deterministic*	 : 	In this mode, all inter-arrival times are equal to 1/lambda seconds, all packets require exactly P tokens, and all service times are equal to 1/mu seconds (all rounded to the nearest millisecond). All Inter-Arrival-Times are capped at 10 seconds. Parameters can be passed as arguments. Example:
`$ ./trafficController -lambda 1.0 -mu 0.35 -r 1.5 -B 10 -P 3 -n 20`
sets packets to arrive Once every second, tokens to arrive every 1.5 seconds, and the servers to process the tokens ever 0.35 seconds. There token bucket is 10 deep, each packet requires 3 tokens, and 20 packets will arrive in total.


*Trace-driven*	 : 	In this mode, we will drive the emulation using a trace specification file (will be referred to as a "tsfile"). Each line in the trace file specifies the inter-arrival time of a packet, the number of tokens it need in order for it to be eligiable for transmission, and its service time. (Please note that in this mode, it's perfectly fine if an inter-arrival time or a service time is greater than 10 seconds. Also note, the tsfile is not validated before the start of simulation.) to use a tsfile, pass the file in as an argument. Example:
`$ ./trafficController -t tsfile/f0.txt`


### Example of Operating Output:
```
$ ./trafficController -n 3

Emulation Parameters:
number to arrive = 3
lambda = 1.00
mu = 0.35
r = 1.50
B = 10
P = 3
00000000.000ms: emulation begins
00000667.129ms: token t1 arrives, token bucket now has 1 token
00001000.412ms: p1 arrives, needs 3 tokens, inter-arrival time = 1000.412ms
00001000.525ms: p1 enters Q1
00001334.226ms: token t2 arrives, token bucket now has 2 tokens
00002000.898ms: p2 arrives, needs 3 tokens, inter-arrival time = 1000.486ms
00002001.067ms: p2 enters Q1
00002001.290ms: token t3 arrives, token bucket now has 3 tokens
00002001.440ms: p1 leaves Q1, time in Q1 = 1000.915ms, token bucket now has 0 tokens
00002001.539ms: p1 enters Q2
00002001.796ms: p1 leaves Q2, time in Q2 = 0.257ms
00002001.912ms: p1 begins service at S2, requesting 2857ms of service
00002668.645ms: token t4 arrives, token bucket now has 1 token
00003001.292ms: p3 arrives, needs 3 tokens, inter-arrival time = 1000.394ms
00003001.402ms: p3 enters Q1
Packet Thread Closed
00003335.762ms: token t5 arrives, token bucket now has 2 tokens
00004002.881ms: token t6 arrives, token bucket now has 3 tokens
00004002.992ms: p2 leaves Q1, time in Q1 = 2001.925ms, token bucket now has 0 tokens
00004003.006ms: p2 enters Q2
00004003.208ms: p2 leaves Q2, time in Q2 = 0.202ms
00004003.333ms: p2 begins service at S1, requesting 2857ms of service
00004669.887ms: token t7 arrives, token bucket now has 1 token
00004859.171ms: p1 departs from S2, service time = 2857.259ms, time in system = 3858.759ms
00005336.958ms: token t8 arrives, token bucket now has 2 tokens
00006004.057ms: token t9 arrives, token bucket now has 3 tokens
00006004.177ms: p3 leaves Q1, time in Q1 = 3002.775ms, token bucket now has 0 tokens
00006004.194ms: p3 enters Q2
00006004.383ms: p3 leaves Q2, time in Q2 = 0.189ms
00006004.487ms: p3 begins service at S2, requesting 2857ms of service
Token Thread Closed
00006860.581ms: p2 departs from S1, service time = 2857.248ms, time in system = 4859.683ms
First Server Closed
00008862.958ms: p3 departs from S2, service time = 2858.471ms, time in system = 5861.666ms
Second Server Closed
00008863.427ms: emulation ends

Statistics:
        average inter-arrival time = 1.000431
        average service time = 2.857659

        average number of packets in Q1 = 0.677573
        average number of packets in Q2 = 0.000073
        average number of packets in S1 = 0.322364
        average number of packets in S2 = 0.644867

        average time a packet spent in system = 4.860036
        standard deviation for time spent in system = 0.817683

        token drop probability = 0.000000
        packet drop probability = 0.000000
```
