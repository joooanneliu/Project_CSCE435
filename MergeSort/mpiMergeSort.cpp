#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

std::vector<int> Merge(const std::vector<int>& left, const std::vector<int>& right)
{
  std::vector<int> result;
	int i = 0;
    int j = 0;
	
	while (i < left.size() && j < right.size())
    {
		if (left[i] <= right[j])
        {
			result.push_back(left[i]);
			i++;
        }
		else
        {
			result.push_back(right[j]);
			j++;
        }
    }
    result.insert(result.end(), left.begin() + i, left.end());
    result.insert(result.end(), right.begin() + j, right.end());

	return result;
}

std::vector<int> MergeSort(const std::vector<int>& arr)
{
    if (arr.size() <= 1)
    {
        return arr;
    }

    int middle = arr.size() / 2;
    std::vector<int> left(arr.begin(), arr.begin() + middle);
    std::vector<int> right(arr.begin() + middle, arr.end());
    return Merge(MergeSort(left), MergeSort(right));
}

bool correctSort(const std::vector<int>& arr)
{
	for (int i = 0; i < arr.size() - 1; i++)
	{
		if (arr[i] > arr[i+1])
		{
			return false;
		}
	}
	return true;
}

int main(int argc, char** argv)
{
	int n;
	if (argc == 2)
	{
		n = atoi(argv[1]);
	}
	else
	{
		printf("\n Please provide the size of the matrix");
		return 0;
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
	if (rank == 0)
	{
		toSort.resize(n);
		for (int i = 0; i < n; i++)
		{
			toSort[i] = rand() % 100;
		}
	}
   CALI_MARK_END("data_init_runtime");

	int chunkSize = n / size;
	std::vector<int> local(chunkSize);
 
  CALI_MARK_BEGIN("comm");
  CALI_MARK_BEGIN("comm_large");

	MPI_Scatter(toSort.data(), chunkSize, MPI_INT, local.data(), chunkSize, MPI_INT, 0, MPI_COMM_WORLD);
 
  CALI_MARK_END("comm_large");
  CALI_MARK_END("comm");
  
  CALI_MARK_BEGIN("comp");
  CALI_MARK_BEGIN("comp_large");
  
	std::vector<int> localSort = MergeSort(local);

  CALI_MARK_END("comp_large");
  CALI_MARK_END("comp");
  
	std::vector<int> sorted;
  if (rank == 0)
  {
    sorted.resize(n);
  }

  CALI_MARK_BEGIN("comm");
  CALI_MARK_BEGIN("comm_large");
  
	MPI_Gather(localSort.data(), chunkSize, MPI_INT, sorted.data(), chunkSize, MPI_INT, 0, MPI_COMM_WORLD);
 
  CALI_MARK_END("comm_large");
  CALI_MARK_END("comm");

	if (rank == 0)
	{
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    
		std::vector<int> result = sorted;
		for (int i = 1; i < size; i++)
		{
			std::vector<int> left(result.begin(), result.begin() + i * chunkSize);
      std::vector<int> right(result.begin() + i * chunkSize, result.begin() + (i+1) * chunkSize);
      result = Merge(left, right);
		}
   
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
   
    CALI_MARK_BEGIN("correctness_check");
    if (correctSort(result))
	  {
		  std::cout << "Correct Result" << std::endl;
	  }
	  else
	  {
		  std::cout << "Failed" << std::endl;
	  }
    CALI_MARK_END("correctness_check");
     
    for (int i = 0; i < result.size(); i++)
	  {
	    std::cout << result[i] << " ";
	  }
    std::cout << std::endl;
	}

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
  adiak::value("input_type", "Random"); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
  adiak::value("num_procs", 16); // The number of processors (MPI ranks)
  adiak::value("scalability", "strong"); // The scalability of your algorithm. choices: ("strong", "weak")
  adiak::value("group_num", 4); // The number of your group (integer, e.g., 1, 10)
  adiak::value("implementation_source", "ai"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").

	mgr.stop();
   	mgr.flush();

   	MPI_Finalize();
}