# CSCE 435 Group project

## 0. Group number: 4

## 1. Group members:
Joanne Liu,
Mrinal Yadev,
Simon Varadaraj,
Nathan Tran

## 2. Project topic: Parallel sorting algorithms
We will communicate through texting in our group chat. Files will be shared across Github and Google drive. 

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)
The algorithms that we will be evaluating are bitonic sort, sample sort, merge sort, and radix sort. Performance will be evaluated on the TAMU Grace cluster, which runs on Linux’s CentOS 7. The file systems on Grace are Lustre and IBM’s General Parallel File System (GPFS). The regular and GPU nodes are all 48-core and are organized in a two-level fat tree topology, with 5 core switches and 11 leaf switches. 

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes'

#### Bitonic Sort Psuedocode:
```
MPI_INIT()
MPI_COMM_SIZE(comm, num_procs) 
MPI_COMM_RANK(comm, rank)

Starttime = MPI_WTIME()

Bitonicsort(arr, lo, size, dir):
	If size > 1:
		Bitonicsort(arr, lo, size / 2, “up”)
Bitonicsort(arr, lo + size, size / 2, “down”)
Bitonicmerge(arr, lo, size, dir)

Bitonicmerge(arr, lo, size, dir):
	If size > 1:
		M = size / 2
		For i from low to low + size - 1:
			If dir == “up” and arr[i] > arr[i+M] or dir == “down” and arr[i] < arr[i+M]:
				swap(arr[i], arr[i+M])
		If rank < num_procs / 2:
        MPI_SEND(arr[low + m], m, MPI_INT, rank + num_procs / 2, tag, comm)
        MPI_RECV(arr[low + m], m, MPI_INT, rank + num_procs / 2, tag, comm, status)
    else: 
        MPI_RECV(arr[low], m, MPI_INT, rank - num_procs / 2, tag, comm, status) 
        MPI_SEND(arr[low], m, MPI_INT, rank - num_procs / 2, tag, comm)

		Bitonicmerge(arr, lo, M, dir)
		Bitonicmerge(arr, lo + M, M, dir)

Duration =  MPI_WTIME() - Starttime 
MPI_FINALIZE()
```

#### Sample Sort Pseudocode:
##### 1. Initialize MPI environment
```
MPI_Init()
rank ← MPI_Comm_rank(MPI_COMM_WORLD)
P ← MPI_Comm_size(MPI_COMM_WORLD)
```
##### 2. Distribute data among P processes
```
IF rank == 0 THEN
   Divide data into P chunks
   Send each chunk to corresponding process
END IF
MPI_Scatterv(data, chunk)
```
##### 3. Perform local sort on each process
```
Local_Sort(chunk)
```
##### 4. Select local samples
```
s ← P - 1
local_samples ← Select s evenly spaced elements from chunk
```
##### 5. Gather local samples to root process
```
MPI_Gather(local_samples, root)
```
##### 6. Root process selects global pivots
```
IF rank == 0 THEN
   merged_samples ← Merge(local_samples from all processes)
   pivots ← Select P-1 evenly spaced elements from merged_samples
END IF
MPI_Bcast(pivots)
```
##### 7. Partition local data based on pivots
```
partitions ← Partition(chunk, pivots)
```
##### 8. Perform all-to-all data exchange
```
MPI_Alltoall(partitions, received_partitions)
```
##### 9. Locally sort the received data
```
sorted_chunk ← Local_Sort(received_partitions)
```
##### 10. Gather sorted data to root process
```
MPI_Gatherv(sorted_chunk, root)
```
##### 11. Finalize MPI environment
```
MPI_Finalize()
```

Merge Sort Psuedocode
```
MPI_INIT()
MPI_COMM_SIZE(comm, num_procs) 
MPI_COMM_RANK(comm, rank)

Starttime = MPI_WTIME()

Merge(left, right):
	result = empty array
	i = 0
	j = 0
	
	while i < len(left) and j < len(right):
		if left[i] < right[j]:
			append left[i] to result
			i += 1
		else: 
			append right[j] to result
			j += 1
		append left[i to length] to result
		append right[j to length] to result
	return result

MergeSort(arr):
	if len(arr) <= 1:
		return arr
	middle = len(arr) / 2
	left = arr[0 to middle]
	right = arr[middle to end]
	sortLeft = MergeSort(left)
	sortRight = MergeSort(right)

	return merge(sortLeft, sortRight)

MPI_Mergesort(arr):
	Comm = MPI.COMM_WORLD
	rank = comm.Get_rank()
	size = comm.Get_size()
	split = []
	
	if rank == 0:
		split = list of arrays from splitting array into chunks with each sublist starting forms successive indices and containing every element in position (initial + multiples of size)
	
	Chunk = comm.scatter(split, root = 0)

	sorted = Mergesort(chunk)

	gather = comm.gather(sorted, root = 0)

	If rank == 0:
		while len(gather) > 1:
			gather[0] = merge(gather[0], gather.pop())
		
		complete = gather[0]
		Print(complete)

main():
	MPI.Mergesort(some arr)

```

Radix Sort Pseudocode
```
countSort(arr, digit):
	// digit = ones (1), tens (2), hundreds(3)....
	output = int[arr.size()]
	count = int[10]
	for int i = 0 to arr.size() - 1:
		count[ digit of arr[i] ]++;
	
	for int i = 1 to 9:
		count[i] = count[i-1] + count[i]
	
	For int i = 0 to int[arr.size]:
		output[count[digit of arr[i]] - 1] = arr[i]
		count[digit of arr[i]]--;
	return output;
radixSort(arr):
	maxNum = arr[0]	
	for i = 1 to arr.size() - 1:
		update maxNum if larger
	
	maxDigits = number of digits in maxNum
	for i = 1 to maxDigits:
		arr = countSort(arr, i)
	return arr
Main: 
	mpi_init()
	rank = Get the rank of the current process
	num_procs = total number of processes 
	startTime = mpi_Wtime()
	
	if rank = 0 // master process
		n = length of data
		split data into subarrays for each process and send with mpi_send
	else: // worker processes
	    	receive subarray from the master process with mpi_recv
		subarray = radix_sort(subarray)
		mpi_send the subarray back to master process
	Duration = mpi_Wtime() - startTime
	mpi_finialize()

```
### 2c. Evaluation plan - what and how will you measure and compare
We will keep a constant problem size while increasing the number of processors/nodes from [2, 4, 8, 16, 32, 64, 128] and then compare the MPI_Wtimes and Caliper times using Thicket.

We will also test by keeping the number of processors/nodes constant while increasing the problem size from [128, 16384, 1048576] with randomized values and then compare the MPI_Wtimes and Caliper times using Thicket.

### 3. Algorithm Description / Questions:
#### Simon: Bitonic Sort
The code described in bitonic.cpp in the bitonic child directory is an implementation of Bitonic Sort using MPI, Adiak, and Caliper. The code is heavily modeled off of the psuedocode shown in the above section that there are the follow functions: bitonicsort and bitonicmerge. Aside from that there are also the correctnessCheck function, iterating over the final array to see if it sorting was successful, and the mpiBitonicMerge function which manages communications. Since there are not multiple stages for communcation and computation, only comm_large and comp_large are present in the comm and comp sections respectively. On runtime, arrays are filled with randomly generated integers using the random library, ranging from 1-100 inclusive. 

#### Joanne: Radix Sort
The radixSort.cpp in the RadixSort folder is modified from the lab 2 matrix multiplcation file. The calculations parts are replaced by the radix sort and the helper functions, countSort and radixSort, were coded based on the pseudocode. There are also mergeTwo, which merges two arrays, mergeAllSubarrays, which is called after receiving all the subarrays from the worker processes. Another helper function was also added called checkValid, which acts as the correctness check function to check whether the array is properly sorted in the end. There are only comp_large since despite split into subarrays, the entire array is sorted at that block annotated. Communication annotations are split into master_send_receive and worker_send_receive, and both would fall under "comm_large" since the whole array is included in communications. The array size is decided when running the file, and filled with randomly generated integers that range from 1 to 100 inclusively. 

#### Nathan: Merge Sort
The files in the MergeMort directory is an implementation of merge sort that is based off of the psuedocode for mergesort above and utilizes MPI, adiak, and Caliper in order to achieve parallel sorting. Aside from the main, merge, and mergesort functions, there is also a correctsort function which validates whether the array was sorted correctly by checking whether the array is in increasing order. Since operations on done on the array itself, there is only sections for comp_large and comm_large in the implementation. The array size for this implementation is based on the input and the values in the array are randomized.

#### Mrinal: Sample Sort
<img width="462" alt="image" src="https://github.com/user-attachments/assets/2096f291-a78b-4885-815d-4df6944fe380">

I wrote the implementation of sample sort in sample sort.cpp, which is inside the samplesort folder. It uses MPI, Caliper, and Adiak and is based on the pseudocode written in the precious section. It starts off by creating an array of random values(0-999) and then performing the sample sort algorithm on the array; it finally checks if the array is correctly sorted at the end. The algorithm has many different stages of communication as it has to broadcast and receive the pivot points from each process. It also performs many computation functions by sorting the array in the processes using the pivots. I used vectors as they were easier to use compared to arrays and had the built-in sort function that I could use when sorting the chunks. 

### 4. Performance Evaluation
#### Simon: Bitonic Sort
Due to difficulties with the Grace queue, I was unable to gain all 280 cali files. I was able to get data for all of the input types and processors for the smallest size (2^16) as well as a wide variety of other size, processor, input combinations. Before the presentation I will be able gain the remaining cali files, so that I may present my strong scaling, speedup, and weak scaling plots. 

#### Joanne: Radix Sort
I was not able to complete all the runs for the array sizes. I was only able to get all of the processes for 2^26 and 2^24, so I can confirm that the code works. For the other way array sizes, the jobs were all stuck in the queue, so I will finish them before the presentation. Currently, I have maximum, minimum, average and variance number of processes v. overall run time (main) graphs, as well as average run times for comm and comp time, for the two finished array sizes. 

#### Nathan: Merge Sort
I was not able to complete all of the runs for the higher process counts. I was able to get the combinations of array sizes and types up to 256 processes but the runs for 512 processes and 1024 processes are stuck in the queue so these will have to be done before the presentation. The implementation is confirmed to work however and there are times for minimum, maximum, and average however I am having trouble getting the graphs to show up properly.
