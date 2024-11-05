#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>
#include <string>
#include <random>
#include <algorithm>
#include <ctime>

std::vector<int> Merge(const std::vector<int>& left, const std::vector<int>& right) {
    std::vector<int> result;
    int i = 0;
    int j = 0;

    while (i < left.size() && j < right.size()) {
        if (left[i] <= right[j]) {
            result.push_back(left[i]);
            i++;
        } else {
            result.push_back(right[j]);
            j++;
        }
    }
    result.insert(result.end(), left.begin() + i, left.end());
    result.insert(result.end(), right.begin() + j, right.end());

    return result;
}

std::vector<int> MergeSort(const std::vector<int>& arr) {
    if (arr.size() <= 1) {
        return arr;
    }

    int middle = arr.size() / 2;
    std::vector<int> left(arr.begin(), arr.begin() + middle);
    std::vector<int> right(arr.begin() + middle, arr.end());
    return Merge(MergeSort(left), MergeSort(right));
}

bool correctSort(const std::vector<int>& arr) {
    for (int i = 0; i < arr.size() - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return false;
        }
    }
    return true;
}

std::vector<int> generateData(int n, int type) {
    std::vector<int> data(n);
    
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }

    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<int> dist(0, n - 1); 

    switch (type) {
        case 0: 
            break;
        case 1: 
            std::reverse(data.begin(), data.end()); 
            break;
        case 2: 
            std::shuffle(data.begin(), data.end(), rng); 
            break;
        case 3: { 
            int pCount = n / 100;
            std::vector<int> temp = data; 
            std::shuffle(temp.begin(), temp.end(), rng); 
            data = temp; 
            
            
            for (int i = 0; i < pCount; i++) {
                int index = dist(rng);
                data[index] = dist(rng);
            }
            break;
        }
        default:
            std::cerr << "Invalid Input\n";
            exit(EXIT_FAILURE);
    }
    return data;
}

int main(int argc, char** argv) {
    CALI_CXX_MARK_FUNCTION;
    int n;
    int type;
    if (argc == 3) {
        n = atoi(argv[1]);
        type = atoi(argv[2]);
    } else {
        printf("\n Invalid Usage \n");
        return 0;
    }
    
    std::string inputType;
    if(type == 0)
    {
      inputType = "Sorted";
    }
    else if (type == 1)
    {
      inputType = "ReverseSorted";
    }
    else if (type == 2)
    {
      inputType = "Random";
    }
    else if (type == 3)
    {
      inputType = "1_perc_perturbed";
    }

    MPI_Init(&argc, &argv);
    int rank;
    int size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    cali::ConfigManager mgr;
	  mgr.start();


    std::vector<int> toSort;
    CALI_MARK_BEGIN("data_init_runtime");
    if (rank == 0) {
        toSort = generateData(n, type);
    }
    CALI_MARK_END("data_init_runtime");

    std::vector<int> sendCounts(size), disp(size);
    int rem = n % size;
    int offset = 0;
    for (int i = 0; i < size; ++i) {
        if (i < rem)
        { 
          sendCounts[i] = sendCounts[i] = n / size + 1;
        }
        else
        {
          sendCounts[i] = sendCounts[i] = n / size;
        }
        disp[i] = offset;
        offset += sendCounts[i];
    }

    std::vector<int> local(sendCounts[rank]);
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Scatterv(toSort.data(), sendCounts.data(), disp.data(), MPI_INT,
                 local.data(), sendCounts[rank], MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    std::vector<int> localSort = MergeSort(local);
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
    
    for (int step = 1; step < size; step *= 2) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                int rSize;
                CALI_MARK_BEGIN("comm");
                CALI_MARK_BEGIN("comm_large");
                MPI_Recv(&rSize, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                CALI_MARK_END("comm_large");
                CALI_MARK_END("comm");
                std::vector<int> rData(rSize);
                CALI_MARK_BEGIN("comm");
                CALI_MARK_BEGIN("comm_large");
                MPI_Recv(rData.data(), rSize, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                CALI_MARK_END("comm_large");
                CALI_MARK_END("comm");
                CALI_MARK_BEGIN("comp");
                CALI_MARK_BEGIN("comp_large");
                localSort = Merge(localSort, rData);
               CALI_MARK_END("comp_large");
                CALI_MARK_END("comp");
                
            }
        } else {
            int targ = rank - step;
            int sSize = localSort.size();
            CALI_MARK_BEGIN("comm");
            CALI_MARK_BEGIN("comm_large");
            MPI_Send(&sSize, 1, MPI_INT, targ, 0, MPI_COMM_WORLD);
            MPI_Send(localSort.data(), sSize, MPI_INT, targ, 0, MPI_COMM_WORLD);
            CALI_MARK_END("comm_large");
            CALI_MARK_END("comm");
            break;
        }
    }

    CALI_MARK_BEGIN("correctness_check");
    if (rank == 0) {
        if (correctSort(localSort)) std::cout << "Correct Result\n";
        else std::cout << "Failed\n";
        /**for (int i = 0; i < localSort.size(); i++)
        {
          std::cout << localSort[i] << " ";
        }
        std::cout << std::endl;*/
    }
    CALI_MARK_END("correctness_check");

    
    adiak::init(NULL);
    adiak::launchdate();    // launch date of the job
    adiak::libraries();     // Libraries used
    adiak::cmdline();       // Command line used to launch the job
    adiak::clustername();   // Name of the cluster
    adiak::value("algorithm", "merge"); // The name of the algorithm you are using (e.g., "merge", "bitonic")
    adiak::value("programming_model", "mpi"); // e.g. "mpi"
    adiak::value("data_type", "int"); // The datatype of input elements (e.g., double, int, float)
    adiak::value("size_of_data_type", sizeof(int)); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
    adiak::value("input_size", n); // The number of elements in input dataset (1000)
    adiak::value("input_type", inputType); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
    adiak::value("num_procs", size); // The number of processors (MPI ranks)
    adiak::value("scalability", "strong"); // The scalability of your algorithm. choices: ("strong", "weak")
    adiak::value("group_num", 4); // The number of your group (integer, e.g., 1, 10)
    adiak::value("implementation_source", "ai"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").
    mgr.stop();
   	mgr.flush();

    MPI_Finalize();
}
