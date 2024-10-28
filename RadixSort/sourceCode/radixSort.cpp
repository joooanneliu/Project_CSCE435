/******************************************************************************
* FILE: radixSort.cpp
* DESCRIPTION: 
   * CSCE435 Group Project 
   * Radix Sort
   * In this code, the master task distributes an array
   * of integers to numtasks - 1 worker tasks to sort using radix sort.
   * Source code: https://www.geeksforgeeks.org/radix-sort/ and Lab 2
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
const char* array_type = "";

if (argc == 3)
{
   sizeOfArray = atoi(argv[1]);
   array_type = argv[2];
}
else 
{
   printf("Invalid Args: %d\n", argc);
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
MPI_Status status;
int* arr = (int *)malloc(sizeOfArray * sizeof(int)); 

/* Define Caliper region names */
const char* main = "main";
const char* data_init_runtime = "data_init_runtime";
const char* correctness_check = "correctness_check";
const char* mpi_init = "mpi_init";
const char* mpi_send = "mpi_send";
const char* mpi_recv = "mpi_recv";
const char* comp_large = "comp_large";
const char* comp = "comp";
const char* comm_large = "comm_large";
const char* comm = "comm";


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
// MPI_Comm workers_comm;
// MPI_Group world_group, workers_group;
// int ranks[numworkers];
// for(int i = 0; i <=numworkers; i++) {
//    ranks[i-1] = i;
// }
// MPI_Comm_group(MPI_COMM_WORLD, &world_group);
// MPI_Group_incl(world_group, numworkers, ranks, &workers_group);
// MPI_Comm_create(MPI_COMM_WORLD, workers_group, &workers_comm);

// WHOLE PROGRAM COMPUTATION PART STARTS HERE
// Create caliper ConfigManager object
cali::ConfigManager mgr;
mgr.start();

CALI_MARK_BEGIN(main);

/**************************** master task ************************************/
   if (taskid == MASTER)
   {
   
      // INITIALIZATION PART FOR THE MASTER PROCESS STARTS HERE

      printf("radixSort has started with %d tasks.\n",numtasks);
      printf("Initializing array...\n");

      CALI_MARK_BEGIN(data_init_runtime);


      srand(time(NULL));
      if(array_type == "Random") {
         for (int i = 0; i < sizeOfArray; i++) {
            arr[i] = rand() % sizeOfArray + 1;
         }   
      } else if(array_type == "Reverse_sorted") {
         for (int i = 0; i < sizeOfArray; i++) {
            arr[i] = sizeOfArray - i;
         }   
      } else {
         for (int i = 0; i < sizeOfArray; i++) {
            arr[i] = i + 1;
         } 
         if(array_type == "1_perturbed")
         for(int i = 0; i < sizeOfArray * 0.1; i++) {
            int index1 = rand() % sizeOfArray;
            int index2 = rand() % sizeOfArray;

            int temp = arr[index1];
            arr[index1] = arr[index2];
            arr[index2] = temp;
         }  
      }
            
      //INITIALIZATION PART FOR THE MASTER PROCESS ENDS HERE
      CALI_MARK_END(data_init_runtime);

      //SEND-RECEIVE PART FOR THE MASTER PROCESS STARTS HERE
      CALI_MARK_BEGIN(comm_large);

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
      CALI_MARK_END(comm_large);
      
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
      CALI_MARK_BEGIN(comm_large);

      mtype = FROM_MASTER;
      MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&length, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      int *subArray = (int *) malloc(length * sizeof(int));
      MPI_Recv(subArray, length, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      
      //RECEIVING PART FOR WORKER PROCESS ENDS HERE
      CALI_MARK_END(comm_large);

      //CALCULATION PART FOR WORKER PROCESS STARTS HERE
      CALI_MARK_BEGIN(comp_large);

      radixsort(subArray, length); 

      //CALCULATION PART FOR WORKER PROCESS ENDS HERE
      CALI_MARK_END(comp_large);

      //SENDING PART FOR WORKER PROCESS STARTS HERE
      CALI_MARK_BEGIN(comm_large);

      mtype = FROM_WORKER;
      MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&length, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(subArray, length, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      
      free(subArray);
      //SENDING PART FOR WORKER PROCESS ENDS HERE
      CALI_MARK_END(comm_large);
   }

   // WHOLE PROGRAM COMPUTATION PART ENDS HERE
   CALI_MARK_END(main);

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
   adiak::value("input_type", array_type); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
   adiak::value("num_procs", numworkers); // The number of processors (MPI ranks)
   adiak::value("scalability", "strong"); // The scalability of your algorithm. 
   // To scale it, just need to change the limit for the size of each number in array.
   adiak::value("group_num", "4"); // The number of your group (integer, e.g., 1, 10)
   adiak::value("implementation_source", "Modified from lab 2 linked source code in header by hand and ai"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").

   // Flush Caliper output before finalizing MPI
   mgr.stop();
   mgr.flush();

   MPI_Finalize();
}