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

	const char* whole_computation = "whole_computation";
	const char* scatter = "scatter";
	const char* calculation = "calculation";
	const char* gather = "gather";

	MPI_Init(&argc, &argv);
	int rank;
	int size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	cali::ConfigManager mgr;
	mgr.start();

	CALI_MARK_BEGIN(whole_computation);

	CALI_MARK_BEGIN(scatter);

	std::vector<int> toSort;
	

	if (rank == 0)
	{
		toSort.resize(n);
		for (int i = 0; i < n; i++)
		{
			toSort[i] = rand() % 100;
		}
   
    for (int i = 0; i < toSort.size(); i++)
	  {
	    std::cout << toSort[i] << " ";
	  }
    std::cout << std::endl;
	}

	int chunkSize = n / size;
	std::vector<int> local(chunkSize);

	MPI_Scatter(toSort.data(), chunkSize, MPI_INT, local.data(), chunkSize, MPI_INT, 0, MPI_COMM_WORLD);

	CALI_MARK_END(scatter);

	CALI_MARK_BEGIN(calculation);

	std::vector<int> localSort = MergeSort(local);

	CALI_MARK_END(calculation);

	CALI_MARK_BEGIN(gather);

	std::vector<int> sorted;
  if (rank == 0)
  {
    sorted.resize(n);
  }

	MPI_Gather(localSort.data(), chunkSize, MPI_INT, sorted.data(), chunkSize, MPI_INT, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		std::vector<int> result = sorted;
		for (int i = 1; i < size; i++)
		{
			std::vector<int> left(result.begin(), result.begin() + i * chunkSize);
      std::vector<int> right(result.begin() + i * chunkSize, result.begin() + (i+1) * chunkSize);
      result = Merge(left, right);
		}
   
    if (correctSort(result))
	  {
		  std::cout << "Correct Result" << std::endl;
	  }
	  else
	  {
		  std::cout << "Failed" << std::endl;
	  }
     
    for (int i = 0; i < result.size(); i++)
	  {
	    std::cout << result[i] << " ";
	  }
    std::cout << std::endl;
	}

	CALI_MARK_END(gather);


	CALI_MARK_END(whole_computation);

	mgr.stop();
   	mgr.flush();

   	MPI_Finalize();
}