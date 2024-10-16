#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <random>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

using namespace std;

random_device seed;
mt19937 gen{seed()};
uniform_int_distribution<> dist{1, 100}; 

void bitonicSort(vector<int>& arr, int lo, int size, bool dir);
void bitonicMerge(vector<int>& arr, int lo, int size, bool dir);
void mpiBitonicMerge(vector<int>& arr, int lo, int size, bool dir, int rank, int num_procs, MPI_Comm comm);
bool correctnessCheck(const vector<int>& arr);

void bitonicSort(vector<int>& arr, int lo, int size, bool dir) {
    CALI_CXX_MARK_FUNCTION;
    CALI_MARK_BEGIN("comp");
    if (size > 1) {
        int m = size / 2;
        CALI_MARK_BEGIN("comp_large");
        bitonicSort(arr, lo, m, true);
        bitonicSort(arr, lo + m, m, false);
        CALI_MARK_END("comp_large");

        bitonicMerge(arr, lo, size, dir);
    }
    CALI_MARK_END("comp");
}

void bitonicMerge(vector<int>& arr, int lo, int size, bool dir) {
    CALI_CXX_MARK_FUNCTION;
    if (size > 1) {
        int m = size / 2;
        for (int i = lo; i < lo + m; i++) {
            if (dir == (arr[i] > arr[i + m])) {
                swap(arr[i], arr[i + m]);
            }
        }
        bitonicMerge(arr, lo, m, dir);
        bitonicMerge(arr, lo + m, m, dir);
    }
}

void mpiBitonicMerge(vector<int>& arr, int lo, int size, bool dir, int rank, int num_procs, MPI_Comm comm) {
    CALI_CXX_MARK_FUNCTION;
    CALI_MARK_BEGIN("comm");
    
    MPI_Status status;
    int tag = 0;  
    int m = size / 2;

    if (rank < num_procs / 2) {
        CALI_MARK_BEGIN("comm_large");
        MPI_Send(&arr[lo + m], m, MPI_INT, rank + num_procs / 2, tag, comm);
        MPI_Recv(&arr[lo + m], m, MPI_INT, rank + num_procs / 2, tag, comm, &status);
        CALI_MARK_END("comm_large");
    } else {
        CALI_MARK_BEGIN("comm_large");
        MPI_Recv(&arr[lo], m, MPI_INT, rank - num_procs / 2, tag, comm, &status);
        MPI_Send(&arr[lo], m, MPI_INT, rank - num_procs / 2, tag, comm);
        CALI_MARK_END("comm_large");
    }

    bitonicMerge(arr, lo, size, dir);
    CALI_MARK_END("comm");
}

bool correctnessCheck(const vector<int>& arr) {
    CALI_CXX_MARK_FUNCTION;
    for (size_t i = 1; i < arr.size(); i++) {
        if (arr[i - 1] > arr[i]) {
            return false;
        }
    }
    return true;
}

int main(int argc, char** argv) {
    CALI_CXX_MARK_FUNCTION;
    
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Comm_size(comm, &num_procs); 
    MPI_Comm_rank(comm, &rank);       

    const int N = 65536; 
    vector<int> arr(N);

    CALI_MARK_BEGIN("data_init_runtime");
    for (int i = 0; i < N; i++) {
        arr[i] = dist(gen);
    }
    CALI_MARK_END("data_init_runtime");

    double start_time = MPI_Wtime();  


    bitonicSort(arr, 0, N, true);
    mpiBitonicMerge(arr, 0, N, true, rank, num_procs, comm);

    double duration = MPI_Wtime() - start_time;  

    CALI_MARK_BEGIN("correctness_check");
    bool correct = correctnessCheck(arr);
    CALI_MARK_END("correctness_check");

    adiak::init(NULL);
    adiak::launchdate();    // launch date of the job
    adiak::libraries();     // Libraries used
    adiak::cmdline();       // Command line used to launch the job
    adiak::clustername();   // Name of the cluster
    adiak::value("algorithm", "bitonic"); // The name of the algorithm you are using (e.g., "merge", "bitonic")
    adiak::value("programming_model", "mpi"); // e.g. "mpi"
    adiak::value("data_type", "int"); // The datatype of input elements (e.g., double, int, float)
    adiak::value("size_of_data_type", sizeof(int)); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
    adiak::value("input_size", N); // The number of elements in input dataset (1000)
    adiak::value("input_type", "Random"); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
    adiak::value("num_procs", 16); // The number of processors (MPI ranks)
    adiak::value("scalability", "strong"); // The scalability of your algorithm. choices: ("strong", "weak")
    adiak::value("group_num", 4); // The number of your group (integer, e.g., 1, 10)
    adiak::value("implementation_source", "ai"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").

    if (rank == 0) {
        cout << "Sorted array: ";
        for (int i = 0; i < N; i++) {
            cout << arr[i] << " ";
        }
        cout << endl;
        cout << "Execution time: " << duration << " seconds" << endl;
        cout << "Correctness: " << (correct ? "Passed" : "Failed") << endl;
    }

    MPI_Finalize();
    return 0;
}