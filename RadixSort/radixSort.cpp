/******************************************************************************
* FILE: radixSort.cpp
* DESCRIPTION: 
   * CSCE435 Group Project 
   * Radix Sort
   * In this code, the master task distributes an array
   * of integers to numtasks - 1 worker tasks to sort using radix sort.
* AUTHOR: Joanne Liu
* DATE: 10/14/2024
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */


void countSort(int arr[], int sz, int digit) {
   int exp = 1;

   for(int i = 0; i < digit - 1; i++) {
      exp *= 10;
   }

   int output[sz];
   int count[10] = {0};

   for(int i = 0; i < sz; i++) {
      count[(arr[i] / exp) % 10]++;
   }

   for(int i = 1; i < 10; i++) {
      count[i] += count[i - 1];
   }

   for(int i = sz-1; i >= 0; i--) {
      output[count[(arr[i] / exp) % 10] - 1] = arr[i];
      count[arr[i] / exp % 10]--;
   }

   for(int i = 0; i < sz; i++) {
      arr[i] = output[i];
   }
}

void radixsort(int arr[], int sz) {
   int maxNum = arr[0];
   for(int i = 0; i < sz; i++) {
      if(arr[i] > maxNum) {
         maxNum = arr[i];
      }
   }
   int maxDigits = 0;
   while(maxNum > 0) {
      maxDigits++;
      maxNum /= 10;
   }
   for(int i = 1; i <= maxDigits; i++) {
      countSort(arr, sz, i);
   }
}

void mergeTwo(int final[], int left[], int right[], int leftSize, int rightSize) {
   int i = 0;
   int j = 0;
   int curr = 0;
   while(i < leftSize && j < rightSize) {
      if(left[i] < right[j]) {
         final[curr++] = left[i++];
      } else {
         final[curr++] = right[j++];
      }
   }
   
   if(i < leftSize) {
      int origLeft = i;
      for(int h = 0; h < leftSize - origLeft; h++) {
         final[curr++] = left[i++];
      }
   }
   if(j < rightSize) {
      int origRight = j;
      for(int h = 0; h < rightSize - origRight; h++) {
         final[curr++] = right[j++];
      }
   }
} 

void mergeAllSubarrays(int arr[], int numworkers, int subarray_sizes[], int offsets[]) {
   for (int i = 1; i < numworkers; i++) {
      int *temp = (int *)malloc((offsets[i] + subarray_sizes[i]) * sizeof(int));
      mergeTwo(temp, arr, &arr[offsets[i]], offsets[i], subarray_sizes[i]);
      for (int j = 0; j < offsets[i] + subarray_sizes[i]; j++) {
         arr[j] = temp[j];
      }
      free(temp);
   }
}

bool checkValid(int arr[], int sz) {
   for(int i = 1; i < sz - 1; i++) {
      if(arr[i-1] > arr[i]) {
         return false;
      }
   }
   return true;
}

int main (int argc, char *argv[])
{
CALI_CXX_MARK_FUNCTION;
    
int sizeOfArray;
if (argc == 2)
{
   sizeOfArray = atoi(argv[1]);
}
else
{
   printf("\n Please provide the size of the matrix");
   return 0;
}
int	numtasks,              /* number of tasks in partition */
	taskid,                /* a task identifier */
	numworkers,            /* number of worker tasks */
	source,                /* task id of message source */
	dest,                  /* task id of message destination */
	mtype,                 /* message type */
	length,               /* total numbers already sent */
   avelen, extra, offset, /* used to determine length of subarray sent to each worker */
	i, j, k, rc;           /* misc */
// double	a[sizeOfMatrix][sizeOfMatrix],           /* matrix A to be multiplied */
// 	b[sizeOfMatrix][sizeOfMatrix],           /* matrix B to be multiplied */
// 	c[sizeOfMatrix][sizeOfMatrix];           /* result matrix C */
int* arr = (int *)malloc(sizeOfArray * sizeof(int)); 

MPI_Status status;
double worker_receive_time,       /* Buffer for worker recieve times */
   worker_calculation_time,      /* Buffer for worker calculation times */
   worker_send_time = 0;         /* Buffer for worker send times */
double whole_computation_time,    /* Buffer for whole computation time */
   master_initialization_time,   /* Buffer for master initialization time */
   master_send_receive_time = 0; /* Buffer for master send and receive time */

/* Define Caliper region names */
const char* whole_computation = "whole_computation";
const char* master_initialization = "master_initialization";
const char* master_send_recieve = "master_send_recieve";
const char* worker_recieve = "worker_recieve";
const char* worker_calculation = "worker_calculation";
const char* worker_send = "worker_send";
const char* data_init_runtime = "data_init_runtime";
const char* correctness_check = "correctness_check";
const char* mpi_init = "mpi_init";
const char* mpi_send = "mpi_send";
const char* mpi_recv = "mpi_recv";
const char* comp_large = "comp_large";

CALI_MARK_BEGIN(mpi_init);
MPI_Init(&argc,&argv);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
CALI_MARK_END(mpi_init);
if (numtasks < 2 ) {
   printf("Need at least two MPI tasks. Quitting...\n");
   MPI_Abort(MPI_COMM_WORLD, rc);
   exit(1);
}
numworkers = numtasks-1;

// New communicator excluding the master
MPI_Comm workers_comm;
MPI_Group world_group, workers_group;
int ranks[numworkers];
for(int i = 0; i <=numworkers; i++) {
   ranks[i-1] = i;
}
MPI_Comm_group(MPI_COMM_WORLD, &world_group);
MPI_Group_incl(world_group, numworkers, ranks, &workers_group);
MPI_Comm_create(MPI_COMM_WORLD, workers_group, &workers_comm);

// WHOLE PROGRAM COMPUTATION PART STARTS HERE
double whole_computation_start = MPI_Wtime();
CALI_MARK_BEGIN(whole_computation);

// Create caliper ConfigManager object
cali::ConfigManager mgr;
mgr.start();

/**************************** master task ************************************/
   if (taskid == MASTER)
   {
   
      // INITIALIZATION PART FOR THE MASTER PROCESS STARTS HERE

      printf("radixSort has started with %d tasks.\n",numtasks);
      printf("Initializing array...\n");

      double master_init_start = MPI_Wtime();
      CALI_MARK_BEGIN(master_initialization);


      srand(time(NULL));
      // printf("orig: ");
      for (int i = 0; i < sizeOfArray; i++) {
         arr[i] = rand() % 100;
         // printf(" %d ", arr[i]);
      }   
      // printf("\n");
            
      //INITIALIZATION PART FOR THE MASTER PROCESS ENDS HERE
      CALI_MARK_END(master_initialization);
      master_initialization_time = MPI_Wtime() - master_init_start;


      //SEND-RECEIVE PART FOR THE MASTER PROCESS STARTS HERE
      CALI_MARK_BEGIN(master_send_recieve);
      double master_send_receive_start = MPI_Wtime();

      /* Send matrix data to the worker tasks */
      
      avelen = sizeOfArray/numworkers;
      extra = sizeOfArray%numworkers;

      offset = 0;
      mtype = FROM_MASTER;
      int subarray_sizes[numworkers];
      int offsets[numworkers];
      CALI_MARK_BEGIN(mpi_send);
      for (dest=1; dest<=numworkers; dest++)
      {
         length = (dest <= extra) ? avelen+1 : avelen;   
         subarray_sizes[dest - 1] = length;
         offsets[dest - 1] = offset;
         	
         printf("Sending %d numbers to task %d offset=%d\n",length,dest,offset);
         MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&length, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&arr[offset], length, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         offset = offset + length;
      }
      CALI_MARK_END(mpi_send);
      /* Receive results from worker tasks */
      mtype = FROM_WORKER;
      CALI_MARK_BEGIN(mpi_recv);
      for (i=1; i<=numworkers; i++)
      {
         source = i;
         MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&length, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&arr[offset], length, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         printf("Received results from task %d\n",source);
      }
      CALI_MARK_END(mpi_recv);
      //SEND-RECEIVE PART FOR THE MASTER PROCESS ENDS HERE
      master_send_receive_time = MPI_Wtime() - master_send_receive_start;
      CALI_MARK_END(master_send_recieve);

      CALI_MARK_BEGIN(comp_large);
      mergeAllSubarrays(arr, numworkers, subarray_sizes, offsets);
      CALI_MARK_END(comp_large);

      // print sorted array
      // printf("new: ");
      // for(int i = 0; i < sizeOfArray; i++) {
      //   printf("%d ", arr[i]);
      // }
      // printf("\n");

      CALI_MARK_BEGIN(correctness_check);
      bool valid = checkValid(arr, sizeOfArray);
      CALI_MARK_END(correctness_check);

      if(valid) {
         printf("ALL SORTED\n");
      } else {
         printf("NOT SORTED\n"); 
      }  
      
   }


/**************************** worker task ************************************/
   if (taskid > MASTER)
   {
      //RECEIVING PART FOR WORKER PROCESS STARTS HERE
      double worker_start_time = MPI_Wtime(); 
      CALI_MARK_BEGIN(worker_recieve);

      mtype = FROM_MASTER;
      MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&length, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      int *subArray = (int *) malloc(length * sizeof(int));
      MPI_Recv(subArray, length, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      
      //RECEIVING PART FOR WORKER PROCESS ENDS HERE
      worker_receive_time = MPI_Wtime() - worker_start_time;
      CALI_MARK_END(worker_recieve);

      //CALCULATION PART FOR WORKER PROCESS STARTS HERE
      double worker_calculation_start = MPI_Wtime();
      CALI_MARK_BEGIN(worker_calculation);

      radixsort(subArray, length); 

      //CALCULATION PART FOR WORKER PROCESS ENDS HERE
      worker_calculation_time = MPI_Wtime() - worker_calculation_start;
      CALI_MARK_END(worker_calculation);

      //SENDING PART FOR WORKER PROCESS STARTS HERE
      double worker_send_start = MPI_Wtime();
      CALI_MARK_BEGIN(worker_send);

      mtype = FROM_WORKER;
      MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&length, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(subArray, length, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      
      free(subArray);
      //SENDING PART FOR WORKER PROCESS ENDS HERE
      worker_send_time = MPI_Wtime() - worker_send_start;
      CALI_MARK_END(worker_send);
   }

   // WHOLE PROGRAM COMPUTATION PART ENDS HERE
   whole_computation_time = MPI_Wtime() - whole_computation_start;
   CALI_MARK_END(whole_computation);

   adiak::init(NULL);
   adiak::init(NULL);
   adiak::launchdate();    // launch date of the job
   adiak::libraries();     // Libraries used
   adiak::cmdline();       // Command line used to launch the job
   adiak::clustername();   // Name of the cluster
   adiak::value("algorithm", "Radix Sort"); // The name of the algorithm you are using (e.g., "merge", "bitonic")
   adiak::value("programming_model", "mpi"); // e.g. "mpi"
   adiak::value("data_type", "int array"); // The datatype of input elements (e.g., double, int, float)
   adiak::value("size_of_data_type", sizeof(int)); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
   adiak::value("input_size", sizeOfArray); // The number of elements in input dataset (1000)
   adiak::value("input_type", "Random"); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
   adiak::value("num_procs", numworkers); // The number of processors (MPI ranks)
   adiak::value("scalability", "strong"); // The scalability of your algorithm. 
   // To scale it, just need to change the limit for the size of each number in array.
   adiak::value("group_num", "4"); // The number of your group (integer, e.g., 1, 10)
   adiak::value("implementation_source", "Modified from lab 2 by hand"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").
   
   double worker_receive_time_max,
      worker_receive_time_min,
      worker_receive_time_sum,
      worker_recieve_time_average,
      worker_calculation_time_max,
      worker_calculation_time_min,
      worker_calculation_time_sum,
      worker_calculation_time_average,
      worker_send_time_max,
      worker_send_time_min,
      worker_send_time_sum,
      worker_send_time_average = 0; // Worker statistic values.

   /* USE MPI_Reduce here to calculate the minimum, maximum and the average times for the worker processes.
   MPI_Reduce (&sendbuf,&recvbuf,count,datatype,op,root,comm). https://hpc-tutorials.llnl.gov/mpi/collective_communication_routines/ */
   if(taskid > MASTER) {
      MPI_Reduce(&worker_receive_time, &worker_receive_time_min, 1, MPI_DOUBLE, MPI_MIN, 0, workers_comm);
      MPI_Reduce(&worker_receive_time, &worker_receive_time_max, 1, MPI_DOUBLE, MPI_MAX, 0, workers_comm);
      MPI_Reduce(&worker_receive_time, &worker_receive_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, workers_comm);

      MPI_Reduce(&worker_calculation_time, &worker_calculation_time_min, 1, MPI_DOUBLE, MPI_MIN, 0, workers_comm);
      MPI_Reduce(&worker_calculation_time, &worker_calculation_time_max, 1, MPI_DOUBLE, MPI_MAX, 0, workers_comm);
      MPI_Reduce(&worker_calculation_time, &worker_calculation_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, workers_comm);

      MPI_Reduce(&worker_send_time, &worker_send_time_min, 1, MPI_DOUBLE, MPI_MIN, 0, workers_comm);
      MPI_Reduce(&worker_send_time, &worker_send_time_max, 1, MPI_DOUBLE, MPI_MAX, 0, workers_comm);
      MPI_Reduce(&worker_send_time, &worker_send_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, workers_comm);
   }

   if (taskid == 0)
   {
      // Master Times
      printf("******************************************************\n");
      printf("Master Times:\n");
      printf("Whole Computation Time: %f \n", whole_computation_time);
      printf("Master Initialization Time: %f \n", master_initialization_time);
      printf("Master Send and Receive Time: %f \n", master_send_receive_time);
      printf("\n******************************************************\n");

      // Add values to Adiak
      adiak::value("MPI_Reduce-whole_computation_time", whole_computation_time);
      adiak::value("MPI_Reduce-master_initialization_time", master_initialization_time);
      adiak::value("MPI_Reduce-master_send_receive_time", master_send_receive_time);

      // Must move values to master for adiak
      mtype = FROM_WORKER;
      MPI_Recv(&worker_receive_time_max, 1, MPI_DOUBLE, 1, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&worker_receive_time_min, 1, MPI_DOUBLE, 1, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&worker_recieve_time_average, 1, MPI_DOUBLE, 1, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&worker_calculation_time_max, 1, MPI_DOUBLE, 1, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&worker_calculation_time_min, 1, MPI_DOUBLE, 1, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&worker_calculation_time_average, 1, MPI_DOUBLE, 1, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&worker_send_time_max, 1, MPI_DOUBLE, 1, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&worker_send_time_min, 1, MPI_DOUBLE, 1, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&worker_send_time_average, 1, MPI_DOUBLE, 1, mtype, MPI_COMM_WORLD, &status);

      adiak::value("MPI_Reduce-worker_receive_time_max", worker_receive_time_max);
      adiak::value("MPI_Reduce-worker_receive_time_min", worker_receive_time_min);
      adiak::value("MPI_Reduce-worker_recieve_time_average", worker_recieve_time_average);
      adiak::value("MPI_Reduce-worker_calculation_time_max", worker_calculation_time_max);
      adiak::value("MPI_Reduce-worker_calculation_time_min", worker_calculation_time_min);
      adiak::value("MPI_Reduce-worker_calculation_time_average", worker_calculation_time_average);
      adiak::value("MPI_Reduce-worker_send_time_max", worker_send_time_max);
      adiak::value("MPI_Reduce-worker_send_time_min", worker_send_time_min);
      adiak::value("MPI_Reduce-worker_send_time_average", worker_send_time_average);
   }
   else if (taskid == 1)
   { // Print only from the first worker.
      // Print out worker time results.
      
      // Compute averages after MPI_Reduce
      worker_recieve_time_average = worker_receive_time_sum / (double)numworkers;
      worker_calculation_time_average = worker_calculation_time_sum / (double)numworkers;
      worker_send_time_average = worker_send_time_sum / (double)numworkers;

      printf("******************************************************\n");
      printf("Worker Times:\n");
      printf("Worker Receive Time Max: %f \n", worker_receive_time_max);
      printf("Worker Receive Time Min: %f \n", worker_receive_time_min);
      printf("Worker Receive Time Average: %f \n", worker_recieve_time_average);
      printf("Worker Calculation Time Max: %f \n", worker_calculation_time_max);
      printf("Worker Calculation Time Min: %f \n", worker_calculation_time_min);
      printf("Worker Calculation Time Average: %f \n", worker_calculation_time_average);
      printf("Worker Send Time Max: %f \n", worker_send_time_max);
      printf("Worker Send Time Min: %f \n", worker_send_time_min);
      printf("Worker Send Time Average: %f \n", worker_send_time_average);
      printf("\n******************************************************\n");

      mtype = FROM_WORKER;
      MPI_Send(&worker_receive_time_max, 1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&worker_receive_time_min, 1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&worker_recieve_time_average, 1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&worker_calculation_time_max, 1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&worker_calculation_time_min, 1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&worker_calculation_time_average, 1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&worker_send_time_max, 1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&worker_send_time_min, 1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&worker_send_time_average, 1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
   }

   // Flush Caliper output before finalizing MPI
   mgr.stop();
   mgr.flush();

   MPI_Finalize();
}