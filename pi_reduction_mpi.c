/*
   Calculating the value of pi using reduction : Parallel Implementation
Author : Omkar Damle.
Date : August 2016.
 */

#include<stdio.h>
#include<math.h>
#include<mpi.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>


//  Using the MONOTONIC clock 
#define CLK CLOCK_MONOTONIC

/* Function to compute the difference between two points in time */
struct timespec diff(struct timespec start, struct timespec end);

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


int main(int argc, char* argv[])
{
	struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
	/* Should start before anything else */
	clock_gettime(CLK, &start_e2e);

	/* Check if enough command-line arguments are taken in. */
	if(argc < 3){
		printf( "Usage: %s n p \n", argv[0] );
		return -1;
	}

	int n=atoi(argv[1]);	/* size of input array */
	int p=atoi(argv[2]);	/* number of processors*/
	char *problem_name = "pi_using_series";
	char *approach_name = "reduction";
//	char buffer[10];
//	FILE* inputFile;
	FILE* outputFile;
	//	inputFile = fopen(argv[3],"r");

	char outputFileName[50];		
	sprintf(outputFileName,"output/%s_%s_%s_%s_output.txt",problem_name,approach_name,argv[1],argv[2]);	

	clock_gettime(CLK, &start_alg);	/* Start the algo timer */

	/*----------------------Core algorithm starts here----------------------------------------------*/
		int i;
	int ierr,num_procs,my_id,size;
	double step,partial_sum;
	double x,pi,sum=0.0;
	step=1.0/n;
	ierr=MPI_Init(&argc,&argv);
	ierr=MPI_Comm_rank(MPI_COMM_WORLD,&my_id);
	ierr=MPI_Comm_size(MPI_COMM_WORLD,&num_procs);
	double a[num_procs];
	for (i=(n/num_procs)*my_id;i<(n/num_procs)*(my_id+1);i++)
	{
		x=(i+0.5)*step;//calculating sum using formula
		sum=sum + 4/(1+x*x);												}
	partial_sum=step*sum;
	double total_sum=0,data;
	if(my_id!=0)
	{
		ierr=MPI_Send(&partial_sum,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
		//printf("%f\n of process %d\n",partial_sum,my_id);
	}
	else
	{
		for(i=1;i<num_procs;i++)
		{
			ierr=MPI_Recv(&data,1,MPI_DOUBLE,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			total_sum=total_sum+data;
			//printf("%f from process %d\n",data,i);
		}
		total_sum=total_sum+partial_sum;
	}
		ierr=MPI_Finalize();	


	/*----------------------Core algorithm finished--------------------------------------------------*/

	clock_gettime(CLK, &end_alg);	/* End the algo timer */
	/* Ensure that only the algorithm is present between these two
	   timers. Further, the whole algorithm should be present. */


	/* Should end before anything else (printing comes later) */
	clock_gettime(CLK, &end_e2e);
	e2e = diff(start_e2e, end_e2e);
	alg = diff(start_alg, end_alg);

	outputFile = fopen(outputFileName,"w");
	fprintf(outputFile,"%.8f\n",total_sum);		

	/* problem_name,approach_name,n,p,e2e_sec,e2e_nsec,alg_sec,alg_nsec
	   Change problem_name to whatever problem you've been assigned
	   Change approach_name to whatever approach has been assigned
	   p should be 0 for serial codes!! 
	 */
	printf("%s,%s,%d,%d,%d,%ld,%d,%ld\n", problem_name, approach_name, n, p, e2e.tv_sec, e2e.tv_nsec, alg.tv_sec, alg.tv_nsec);

	return 0;

}

