/*

*/
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
unsigned int *nSeed;

void PRNG(int threadNo)
{
	unsigned int val = nSeed[512*threadNo];
	val = (8253729 * val + 2396403); 
	nSeed[threadNo*512]= (val  % 32768);
}

/* Some of the available clocks. 
   Use the MONOTONIC clock for your codes 
*/

#define CLK CLOCK_MONOTONIC
/* #define CLK CLOCK_REALTIME */
/* #define CLK CLOCK_PROCESS_CPUTIME_ID */
/* #define CLK CLOCK_THREAD_CPUTIME_ID */


/* Function to compute the difference between two points in time */
struct timespec diff(struct timespec start, struct timespec end);


int main(int argc, char* argv[]){
	struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
	/* Should start before anything else */
	clock_gettime(CLK, &start_e2e);

	/* Check if enough command-line arguments are taken in. */
	if(argc < 3){
		/* Compare to 4 in parallel code if file input is taken. */
		printf( "Usage: %s n p\np=0 for serial code.", argv[0] );
		return -1;
	}

	double PI = 0;
	double r = 0.5;
	int n = atoi(argv[1]);
	long count = 0;
	int p = 0;
	int threadNo = 0;
	srand(time(NULL));
	char *problem_name = "monte_carlo"; /* Change */
	char *approach_name = "own-prng"; /* Change */
	float f;
  
	/* Here you would have your pre-processing elements. */

	clock_gettime(CLK, &start_alg);   /* Start the algo timer */
	nSeed = (unsigned int *)malloc(sizeof(unsigned int)*512*1);
	long i=0;
	for (i = 0; i <= p; ++i)
	{
		nSeed[512*i]=rand()%32768;
	}
	
	/* Core algorithm follows */
	for (i = 0; i < n; ++i)
	{
		PRNG(threadNo);
		double x = nSeed[threadNo*512];
		PRNG(threadNo);
		double y = nSeed[threadNo*512];
		x/=32767;
		y/=32767;
		if((x-0.5)*(x-0.5) + (y-0.5)*(y-0.5) <= r*r){
			count++;
		}
	}
	/* Core algorithm finished */
	PI = (double)(count*4)/(n);
		clock_gettime(CLK, &end_alg); /* End the algo timer */
	/* Ensure that only the algorithm is present between these two
	 timers. Further, the whole algorithm should be present. */
	

	/* Various methods in which you can print times. 
	 Don't use these. Only for illustration. */


	/* Should end before anything else (printing comes later) */
	clock_gettime(CLK, &end_e2e);
	e2e = diff(start_e2e, end_e2e);
	alg = diff(start_alg, end_alg);

	/* problem_name,approach_name,n,p,e2e_sec,e2e_nsec,alg_sec,alg_nsec
	 Change problem_name to whatever problem you've been assigned
	 Change approach_name to whatever approach has been assigned
	 p should be 0 for serial codes!! 
	*/  
	printf("%s,%s,%d,%d,%ld,%ld,%ld,%ld\n", problem_name, approach_name, n, p, e2e.tv_sec, e2e.tv_nsec, alg.tv_sec, alg.tv_nsec);

	return 0;
}



/* 
Function to computes the difference between two time instances

Taken from - http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/ 

Further reading:
http://stackoverflow.com/questions/6749621/how-to-create-a-high-resolution-timer-in-linux-to-measure-program-performance
http://stackoverflow.com/questions/3523442/difference-between-clock-realtime-and-clock-monotonic
*/
struct timespec diff(struct timespec start, struct timespec end){
  struct timespec temp;
  if((end.tv_nsec-start.tv_nsec)<0){
	temp.tv_sec = end.tv_sec-start.tv_sec-1;
	temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  }
  else{
	temp.tv_sec = end.tv_sec-start.tv_sec;
	temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}

  /*
	The following can be used to print times. Don't use these for 
	the code you submit. Only for your reference purposes.
	
  printf("%20.9lf seconds\n", diff(t1, t2).tv_sec*1e0 + diff(t1, t2).tv_nsec/1e9);
  printf("%20.9lf milliseconds\n", diff(t1, t2).tv_sec*1e3 + diff(t1, t2).tv_nsec/1e6);
  printf("%20.9lf microseconds\n", diff(t1, t2).tv_sec*1e6 + diff(t1, t2).tv_nsec/1e3);
  printf("%20.9lf nanoseconds\n", diff(t1, t2).tv_sec*1e9 + diff(t1, t2).tv_nsec/1e0);
  */
