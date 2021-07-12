#include "my402list.h"
#include "initializeParameters.h"
#include "cs402.h"

typedef struct Packet
{
	int tokReq, num, service_time;
	struct timeval packet_arrives, enters_Q1, leaves_Q1,
		enters_Q2, leaves_Q2, enters_S, leaves_S;
} Packet;

/** time_diff
 *  Returns time difference between x and y in microseconds
 *  x occuring first, and y occuring second.
 */
double time_diff(struct timeval x, struct timeval y)
{
	double x_us, y_us, diff;
	x_us = 1000000 * (double)x.tv_sec + (double)x.tv_usec;
	y_us = 1000000 * (double)y.tv_sec + (double)y.tv_usec;
	diff = (double)y_us - (double)x_us;
	return diff;
}

void printStatistics(SharedData *s)
{
	double emulationTime = time_diff(s->start,s->end);	
	double varTime = s->stats->avg_system_t2 - pow(s->stats->avg_system_t,2);

	printf("\nStatistics:\n");
	printf("\taverage inter-arrival time = %.6lf\n",s->stats->avg_inter_arival_t/1000000);
	printf("\taverage service time = %.6lf\n\n",s->stats->avg_pkt_service_t/1000000);

	printf("\taverage number of packets in Q1 = %.6lf\n", s->stats->avg_num_pkts_Q1/emulationTime);
	printf("\taverage number of packets in Q2 = %.6lf\n", s->stats->avg_num_pkts_Q2/emulationTime);
	printf("\taverage number of packets in S1 = %.6lf\n", s->stats->avg_num_pkts_S1/emulationTime);
	printf("\taverage number of packets in S2 = %.6lf\n\n", s->stats->avg_num_pkts_S2/emulationTime);

	printf("\taverage time a packet spent in system = %.6lf\n",s->stats->avg_system_t);
	printf("\tstandard deviation for time spent in system = ");
	varTime > 0 ? printf("%.6lf\n\n", sqrt(varTime)) : printf("N/A\n\n");

	printf("\ttoken drop probability = %.6lf\n",s->stats->t_drop_prob);
	printf("\tpacket drop probability = %.6lf\n",s->stats->p_drop_prob);
}

/**
 * @brief packetRoutine. The thread that controls the flow of data packets into
 * the traffic controller.
 * 
 * @param arg 
 * @return void* 
 */
void *packetRoutine(void *arg)
{
	SharedData *threadData = (SharedData *)(arg);
	char line[1025];
	int arrival_time, service_time, required_tokens;
	int d = 0;

	gettimeofday(&(threadData->start),NULL);
	printf("%012.3lfms: emulation begins\n", time_diff(threadData->start, threadData->start) / 1000);
	struct timeval last_packet_arrival = threadData->start;

	for (int p = 0; p < threadData->params->num; p++)
	{
		//If tsfile was specified, Run in Trace-Driven Mode.
		if (threadData->params->t != NULL)
		{
			if (fgets(line, sizeof(line), threadData->fp) != NULL)
			{
				if (sscanf(line, "%d %d %d", &arrival_time, &required_tokens, &service_time) == 3)
					;
				else
				{
					//NEEDS TO TERMINATE HERE. MEANS FILE WAS BAD
					printf("Input does not contain 3 integers!\n");
					return 0;
				}
			}
		}
		//If tsfile was not specified, Run in Deterministic Mode.
		else if(p == 0)
		{
			if ((arrival_time = 1.0 / threadData->params->lambda * 1000) > 10000)
				arrival_time = 10000;
			if ((service_time = 1.0 / threadData->params->mu * 1000) > 10000)
				service_time = 10000;
			required_tokens = threadData->params->P;
		}

		//Sleep for inter_arrival time between packets.
		usleep(arrival_time * 1000);

		//Then make a new packet.
		Packet *packet = (Packet *)malloc(sizeof(Packet));
		packet->service_time = service_time;
		packet->tokReq = required_tokens;
		packet->num = p + 1;

		//Lock
		pthread_mutex_lock(threadData->m);
		//Print packet arrival
		gettimeofday(&(packet->packet_arrives), NULL);
		printf("%012.3lfms: ", time_diff(threadData->start, packet->packet_arrives) / 1000);
		printf("p%d arrives, needs %d tokens, inter-arrival time = %.3lfms",
			   packet->num, required_tokens, time_diff(last_packet_arrival, packet->packet_arrives) / 1000);
		
		threadData->stats->avg_inter_arival_t = (p * threadData->stats->avg_inter_arival_t + time_diff(last_packet_arrival, packet->packet_arrives)) / (p+1);
		last_packet_arrival = packet->packet_arrives;
		//runAvg(&threadData->stats->avg_inter_arival_t, time_diff(threadData->start, packet->packet_arrives), &p);
		//If packet requires more tokens then Bucket can hold, dropp the packet
		if(packet->tokReq > threadData->params->B)
		{
			printf(", dropped\n");
			d++;
			free(packet);
			pthread_mutex_unlock(threadData->m);
			continue;			
		}
		else
			printf("\n");
		//add packet to Q1 and print
		My402ListAppend(threadData->Q1, packet);
		gettimeofday(&(packet->enters_Q1), NULL);
		printf("%012.3lfms: ", time_diff(threadData->start, packet->enters_Q1) / 1000);
		printf("p%d enters Q1\n", packet->num);

		//if packet can go to Q2, send it and broadcast to servers.
		if (My402ListLength(threadData->Q1) == 1 && threadData->TokenBucket >= required_tokens)
		{
			//Decrement Token Bucket
			threadData->TokenBucket -= required_tokens;

			//Remove Packet from Q1 and print
			My402ListUnlink(threadData->Q1, My402ListFirst(threadData->Q1));
			gettimeofday(&(packet->leaves_Q1), NULL);
			printf("%012.3lfms: ", time_diff(threadData->start, packet->leaves_Q1) / 1000);
			printf("p%d leaves Q1, time in Q1 = %.3lfms, token bucket now has %d tokens\n",
				   packet->num, time_diff(packet->enters_Q1, packet->leaves_Q1) / 1000, threadData->TokenBucket);

			//Add Packet to Q2 and print
			My402ListAppend(threadData->Q2, packet);
			gettimeofday(&(packet->enters_Q2), NULL);
			printf("%012.3lfms: ", time_diff(threadData->start, packet->enters_Q2) / 1000);
			printf("p%d enters Q2\n", packet->num);

			//Broadcast to Servers
			pthread_cond_broadcast(threadData->cv);
		}

		pthread_mutex_unlock(threadData->m);
		//Unlock
	}

	threadData->allPacketsCreated = 1;
	threadData->stats->p_drop_prob = (double)d/threadData->params->num;
	if(threadData->params->t != NULL)
		fclose(threadData->fp);
	return NULL;
}

/**
 * @brief tokenRouine is the token thread that handles the tokens arriving to the traffic controller.
 * 
 * @param arg 
 * @return void* 
 */
void *tokenRoutine(void *arg)
{
	SharedData *threadData = (SharedData *)(arg);
	int t = 0, d = 0;
	struct timeval t_arrives;

	while (!(threadData->allPacketsCreated && My402ListEmpty(threadData->Q1)))
	{
		usleep(1.0 / threadData->params->r * 1000000);
		t++;

		//Lock
		pthread_mutex_lock(threadData->m);
		//Set and Print token arrival time
		gettimeofday(&t_arrives, NULL);
		printf("%012.3lfms: ", time_diff(threadData->start, t_arrives) / 1000);
		printf("token t%d arrives, ", t);

		//if token bucket isn't full, add token to bucket
		if (threadData->TokenBucket < threadData->params->B)
		{
			threadData->TokenBucket++;
			if (threadData->TokenBucket == 1)
				printf("token bucket now has 1 token\n");
			else
				printf("token bucket now has %d tokens\n", threadData->TokenBucket);
		}
		else
		{
			printf("dropped\n");
			d++;
		}

		//if Q1 is not empty,check if there are enough tokens for the first paket.
		if (!My402ListEmpty(threadData->Q1))
		{
			My402ListElem *elem = My402ListFirst(threadData->Q1);
			Packet *packet = elem->obj;
			//if there are enough tokens in the bucket
			if (packet->tokReq <= threadData->TokenBucket)
			{
				//Decrement Token Bucket
				threadData->TokenBucket -= packet->tokReq;

				//Remove Packet from Q1 and print
				My402ListUnlink(threadData->Q1, elem);
				gettimeofday(&(packet->leaves_Q1), NULL);
				printf("%012.3lfms: ", time_diff(threadData->start, packet->leaves_Q1) / 1000);
				printf("p%d leaves Q1, time in Q1 = %.3lfms, token bucket now has %d tokens\n",
					   packet->num, time_diff(packet->enters_Q1, packet->leaves_Q1) / 1000, threadData->TokenBucket);

				//Add Packet to Q2 and print
				My402ListAppend(threadData->Q2, packet);
				gettimeofday(&(packet->enters_Q2), NULL);
				printf("%012.3lfms: ", time_diff(threadData->start, packet->enters_Q2) / 1000);
				printf("p%d enters Q2\n", packet->num);

				//Broadcast to Servers
				pthread_cond_broadcast(threadData->cv);
			}
		}

		pthread_mutex_unlock(threadData->m);
		//Unlock
	}

	threadData->allPacketsTokened = 1;
	pthread_cond_broadcast(threadData->cv);
	threadData->stats->t_drop_prob = (double)d/t;
	pthread_exit(NULL);
	return NULL;
}

/**
 * @brief serverRoutine handles the two server threads servicing packets.
 * 
 * @param arg 
 * @return void* 
 */
void *serverRoutine(void *arg)
{
	SharedData *threadData = (SharedData *)(arg);
	int s;
	//int p = 0;
	pthread_mutex_lock(threadData->m);
	if(threadData->sflag == 0)
	{
		s = 1; 
		threadData->sflag = 1;
	}
	else
		s = 2;
	pthread_mutex_unlock(threadData->m);
	
	My402ListElem *elem;
	Packet *packet;

	while (!(threadData->allPacketsTokened && My402ListEmpty(threadData->Q2)))
	{
		pthread_mutex_lock(threadData->m);
		while (!threadData->allPacketsTokened && My402ListEmpty(threadData->Q2))
		{
			pthread_cond_wait(threadData->cv, threadData->m);
		}

		if(My402ListEmpty(threadData->Q2))
		{
			pthread_mutex_unlock(threadData->m);
			continue;
		}

		//Select Packet
		elem = My402ListFirst(threadData->Q2);
		packet = (Packet *)elem->obj;
		//Remove Packet from Q2 and print
		My402ListUnlink(threadData->Q2, elem);
		gettimeofday(&(packet->leaves_Q2), NULL);
		printf("%012.3lfms: ", time_diff(threadData->start, packet->leaves_Q2) / 1000);
		printf("p%d leaves Q2, time in Q2 = %.3lfms\n",
			   packet->num, time_diff(packet->enters_Q2, packet->leaves_Q2) / 1000);

		//Begin Servicing Paket
		gettimeofday(&(packet->enters_S), NULL);
		printf("%012.3lfms: ", time_diff(threadData->start, packet->enters_S) / 1000);
		printf("p%d begins service at S%d, requesting %dms of service\n",
			   packet->num, s, packet->service_time);
		
		pthread_mutex_unlock(threadData->m);
		//Service the packet
		usleep(packet->service_time*1000);
		
		//Print that the packet was serviced
		pthread_mutex_lock(threadData->m);
		gettimeofday(&(packet->leaves_S), NULL);
		double packetTime_ms = time_diff(packet->packet_arrives, packet->leaves_S) / 1000;
		double serviceTime_ms = time_diff(packet->enters_S, packet->leaves_S) / 1000;

		printf("%012.3lfms: ", time_diff(threadData->start, packet->leaves_S) / 1000);
		printf("p%d departs from S%d, service time = %.3lfms, time in system = %.3lfms\n",
			   packet->num, s, serviceTime_ms, packetTime_ms);
		
		threadData->stats->avg_pkt_service_t = (threadData->ps * threadData->stats->avg_pkt_service_t + serviceTime_ms*1000) / (threadData->ps+1);
		threadData->stats->avg_system_t = (threadData->ps * threadData->stats->avg_system_t + (packetTime_ms/1000)) / (threadData->ps+1);
		threadData->stats->avg_system_t2 = (threadData->ps * threadData->stats->avg_system_t2 + pow(packetTime_ms/1000,2)) / (threadData->ps+1);

		threadData->stats->avg_num_pkts_Q1 += time_diff(packet->enters_Q1,packet->leaves_Q1);
		threadData->stats->avg_num_pkts_Q2 += time_diff(packet->enters_Q2,packet->leaves_Q2);
		if(s== 1)
			threadData->stats->avg_num_pkts_S1 += serviceTime_ms*1000;
		else
			threadData->stats->avg_num_pkts_S2 += serviceTime_ms*1000;
		threadData->ps++;
		pthread_mutex_unlock(threadData->m);
	}

	//pthread_exit(NULL);
	return NULL;
}

int main(int argc, char *argv[])
{
	//Initialize Emulation Parameters
	FILE *fp;
	InitParams *params = malloc(sizeof(InitParams));
	SharedData *threadData = malloc(sizeof(SharedData));
	defaultParams(params);
	parseArgs(argc, argv, params, &fp);
	initializeSharedData(threadData, fp, params);

	//Create & Run Emulation Threads
	pthread_t packet_thread, token_thread, serv1_thread, serv2_thread;
	pthread_create(&packet_thread, 0, packetRoutine, threadData);
	pthread_create(&token_thread, 0, tokenRoutine, threadData);
	pthread_create(&serv1_thread, 0, serverRoutine, threadData);
	pthread_create(&serv2_thread, 0, serverRoutine, threadData);
	pthread_join(packet_thread, NULL);
	printf("Packet Thread Closed\n");
	pthread_join(token_thread, NULL);
	printf("Token Thread Closed\n");
	pthread_join(serv1_thread, NULL);
	printf("First Server Closed\n");
	pthread_join(serv2_thread, NULL);
	printf("Second Server Closed\n");

	//Print End of Emulation
	gettimeofday(&(threadData->end), NULL);
	printf("%012.3lfms: emulation ends\n", time_diff(threadData->start, threadData->end) / 1000);
	printStatistics(threadData);
	pthread_exit(0);
	return (0);
}