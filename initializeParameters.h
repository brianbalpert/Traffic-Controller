#include "defs.h"
#include "my402list.h"
#include "cs402.h"

typedef struct InitParams
{
	int num, B, P;
	double lambda, mu, r;
	char *t;
} InitParams;


/* 
average packet inter-arrival time = <real-value>
average packet service time = <real-value>
  
average number of packets in Q1 = <real-value>
average number of packets in Q2 = <real-value>
average number of packets at S1 = <real-value>
average number of packets at S2 = <real-value>
    
average time a packet spent in system = <real-value>
standard deviation for time spent in system = <real-value>

token drop probability = <real-value>
packet drop probability = <real-value> 
*/
typedef struct Stats
{
	double avg_inter_arival_t, avg_pkt_service_t;
	double avg_num_pkts_Q1, avg_num_pkts_Q2, avg_num_pkts_S1, avg_num_pkts_S2;
	double avg_system_t, avg_system_t2;
	double t_drop_prob, p_drop_prob;
} Stats;

typedef struct SharedData
{
	pthread_mutex_t *m;
	pthread_cond_t *cv;
	My402List *Q1, *Q2;
	int TokenBucket, allPacketsCreated, allPacketsTokened, sflag, ps;
	InitParams *params;
	Stats *stats;
	struct timeval start, end;
	FILE *fp;
} SharedData;

void defaultParams(InitParams *params);
int checkParameterBounds(InitParams *params);
void parseArgs(int argc, char *argv[], InitParams *params, FILE **fp);
void initializeSharedData(SharedData *threadData, FILE *fp, InitParams *params);