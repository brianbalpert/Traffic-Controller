#include "initializeParameters.h"

void defaultParams(InitParams *params)
{
	params->B = 10;
	params->P = 3;
	params->num = 20;
	params->lambda = 1.0;
	params->mu = 0.35;
	params->r = 1.5;
	params->t = NULL; 
}

int checkParameterBounds(InitParams *params){
	int err = 0, real = 0;
	if(params->B <= 0){ fprintf(stderr, "B "); err = 1;}
	else if(params->P <= 0){ fprintf(stderr, "P "); err = 1;}
	else if(params->num <= 0){ fprintf(stderr, "num "); err = 1;}
	else if(params->lambda <= 0){ fprintf(stderr, "lambda "); err = 1; real = 1;}
	else if(params->mu <= 0){ fprintf(stderr, "mu "); err = 1; real = 1;}
	else if(params->r <= 0){ fprintf(stderr, "r "); err = 1; real = 1;}
	if(err){
		fprintf(stderr, "must be a positive ");
		if(real)
			fprintf(stderr, "real number\n");
		else
			fprintf(stderr, "integer.\n");
		return 0;
	}

	return 1;
}

void initializeSharedData(SharedData *threadData, FILE *fp, InitParams *params){
	//Create Emulation Components
	Stats *stats = malloc(sizeof(Stats));
	My402List *q1 = malloc(sizeof(My402List));
	My402List *q2 = malloc(sizeof(My402List));
	memset(stats, 0, sizeof(Stats));
	memset(q1, 0, sizeof(My402List));
	memset(q2, 0, sizeof(My402List));
	(void)My402ListInit(q1);
	(void)My402ListInit(q2);
	
	//Create Mutex
	pthread_mutex_t *m = malloc(sizeof(pthread_mutex_t)); 
	pthread_mutex_init(m, 0);
	pthread_cond_t *cv = malloc(sizeof(pthread_cond_t)); 
	pthread_cond_init(cv, 0);

	//Populate Data Structure to be shared among threads
	threadData->params = params;
	threadData->stats = stats;
	threadData->m = m;
	threadData->cv = cv;
	threadData->Q1 = q1;
	threadData->Q2 = q2;
	threadData->fp = fp;
}

void parseArgs(int argc, char *argv[], InitParams *params, FILE **fp)
{
	int opt;
	char *ptr;
	int opt_index = 0;
	static struct option long_options[] = {
		{"lambda", required_argument, NULL, 'l'},
		{"mu", required_argument, NULL, 'm'},
		{"B", required_argument, NULL, 'b'},
		{"P", required_argument, NULL, 'p'},
		{"r", required_argument, NULL, 'r'},
		{"n", required_argument, NULL, 'n'},
		{"t", required_argument, NULL, 't'},
		{0, 0, 0, 0}};
	while ((opt = getopt_long_only(argc, argv, "", long_options, &opt_index)) != -1)
	{
		switch (opt)
		{
		case 'l':
			params->lambda = strtod(optarg, &ptr); break;
		case 'm':
			params->mu = strtod(optarg, &ptr); break;
		case 'b':
			params->B = atoi(optarg); break;
		case 'p':
			params->P = atoi(optarg); break;
		case 'r':
			params->r = strtod(optarg, &ptr); break;
		case 'n':
			params->num = atoi(optarg); break;
		case 't':
			params->t = optarg; break;
 		case '?':
			fprintf(stderr, "Usage: trafficController [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile] \n");
			exit(EXIT_FAILURE);
		default:
			fprintf(stderr, "Usage: trafficController [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile] \n");
			exit(EXIT_FAILURE);
		}
	}

	char line[1025];
	if(params->t != NULL){
		*fp = fopen(params->t,"r");
		if(*fp == NULL){
			char errmesage[100]="";
			sprintf(errmesage,"cannot open file %s",params->t);
			perror(errmesage);
			return;
		}
		fgets(line,1024,*fp);
		params->num = atoi(line); 
	}

	if(!checkParameterBounds(params)) exit(EXIT_FAILURE);

	printf("Emulation Parameters:\n");
	printf("number to arrive = %d\n",params->num);
	if(params->t == NULL){
		printf("lambda = %.2lf\n",params->lambda);
		printf("mu = %.2lf\n",params->mu);
	}
	printf("r = %.2lf\n",params->r);
	printf("B = %d\n",params->B);
	if(params->t == NULL)
		printf("P = %d\n",params->P);
	if(params->t != NULL)
		printf("tsfile = %s\n",params->t);

}