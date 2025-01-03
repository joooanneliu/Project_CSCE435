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
![image](https://github.com/user-attachments/assets/142b3ed1-bf4e-4d04-aeee-741ae3287742)

The code described in bitonic.cpp in the bitonic child directory is an implementation of Bitonic Sort using MPI, Adiak, and Caliper. The code is heavily modeled off of the psuedocode shown in the above section that there are the follow functions: bitonicsort and bitonicmerge. Aside from that there are also the correctnessCheck function, iterating over the final array to see if it sorting was successful, and the mpiBitonicMerge function which manages communications. Since there are not multiple stages for communcation and computation, only comm_large and comp_large are present in the comm and comp sections respectively. On runtime, arrays are filled with randomly generated integers using the random library, ranging from 1-100 inclusive. 

#### Joanne: Radix Sort
![image](https://github.com/user-attachments/assets/b4cd0eec-0561-4482-a323-5accc2cb6b92)

The radixSort.cpp in the RadixSort folder is modified from the lab 2 matrix multiplcation file. The calculations parts are replaced by the radix sort and the helper functions, countSort and radixSort, were coded based on the pseudocode. There are also mergeTwo, which merges two arrays, mergeAllSubarrays, which is called after receiving all the subarrays from the worker processes. Another helper function was also added called checkValid, which acts as the correctness check function to check whether the array is properly sorted in the end. There are only comp_large since despite split into subarrays, the entire array is sorted at that block annotated. Communication annotations are split into master_send_receive and worker_send_receive, and both would fall under "comm_large" since the whole array is included in communications. The array size is decided when running the file, and filled with randomly generated integers that range from 1 to 100 inclusively. 

#### Nathan: Merge Sort
![image](https://github.com/user-attachments/assets/24d60cd0-7f02-47c9-a233-302ee16a75d2)
The files in the MergeMort directory is an implementation of merge sort that is based off of the psuedocode for mergesort above and utilizes MPI, adiak, and Caliper in order to achieve parallel sorting. Aside from the main, merge, and mergesort functions, there is also a correctsort function which validates whether the array was sorted correctly by checking whether the array is in increasing order. Since operations on done on the array itself, there is only sections for comp_large and comm_large in the implementation. The array size for this implementation is based on the input and the values in the array are randomized.

#### Mrinal: Sample Sort
<img width="462" alt="image" src="https://github.com/user-attachments/assets/2096f291-a78b-4885-815d-4df6944fe380">

I wrote the implementation of sample sort in sample sort.cpp, which is inside the samplesort folder. It uses MPI, Caliper, and Adiak and is based on the pseudocode written in the precious section. It starts off by creating an array of random values(0-999) and then performing the sample sort algorithm on the array; it finally checks if the array is correctly sorted at the end. The algorithm has many different stages of communication as it has to broadcast and receive the pivot points from each process. It also performs many computation functions by sorting the array in the processes using the pivots. I used vectors as they were easier to use compared to arrays and had the built-in sort function that I could use when sorting the chunks. 

### 4. Performance Evaluation
Each of the performance evaluation reports and respective plots are separated by sort. The links to them are listed below. 
#### Simon: Bitonic Sort
![image](https://github.com/user-attachments/assets/764a1385-c57b-4c73-a27e-faab460dcda4)

Comp takes a large majority of the time, which could be due to the poor implementation of the swapping mechanic necessary for bitonic sort. This further explains why in future graphs, the speed of computation remains constant over the amount of processes, as implementation bottlenecks the algorithm's predicted performance.

##### Times for 2^16 Array Size
![image](https://github.com/user-attachments/assets/979a63b7-1877-4f2e-b805-66b7d9042164)
![image](https://github.com/user-attachments/assets/97d45753-283c-42d3-b375-b171b330fdd4)
![image](https://github.com/user-attachments/assets/3b69a315-dadf-4a2e-b3e4-f0548d3303d7)
##### Times for 2^18 Array Size
![image](https://github.com/user-attachments/assets/22302a86-c402-4af7-8807-c93826901d50)
![image](https://github.com/user-attachments/assets/5cf3b369-b2b6-44c2-aea6-a86d27cc970a)
![image](https://github.com/user-attachments/assets/262bea8f-6ade-439f-ad29-e8111ace7607)
##### Times for 2^20 Array Size
![image](https://github.com/user-attachments/assets/4083dd3c-4b6d-4cea-8c31-52de08430abd)
![image](https://github.com/user-attachments/assets/9924ec6e-0ddf-4ebd-92df-b158f3fb9fd5)
![image](https://github.com/user-attachments/assets/72c2e18d-dcc4-4a74-9ff2-164b4c2a4c0d)
##### Times for 2^22 Array Size
![image](https://github.com/user-attachments/assets/b278cf7d-7710-4466-8c17-d0dbe012f17f)
![image](https://github.com/user-attachments/assets/634c0135-2845-40b8-9d45-3d93f667c493)
![image](https://github.com/user-attachments/assets/f743fa67-415a-4a23-a410-95f3a50c360a)
##### Times for 2^24 Array Size
![image](https://github.com/user-attachments/assets/b92bbfc6-2e1e-4680-80d3-a1993bdfa681)
![image](https://github.com/user-attachments/assets/f27bc6b5-1629-4100-80a4-88ff6cb18ecd)
![image](https://github.com/user-attachments/assets/90a6910b-57c5-44d3-b701-77cbbb0810bf)
##### Times for 2^26 Array Size
![image](https://github.com/user-attachments/assets/c1aa4836-043e-4617-ad08-2d0c7a46702e)
![image](https://github.com/user-attachments/assets/2fec244b-313c-48ae-8320-6e74191f1770)
![image](https://github.com/user-attachments/assets/1867d670-b7d5-41f1-9ec5-5960c0fd6b6c)
##### Times for 2^28 Array Size
![image](https://github.com/user-attachments/assets/8371239b-cd81-4f42-9897-c2b24d6a609a)
![image](https://github.com/user-attachments/assets/499ab3a3-8c92-4109-a9c0-8818a78a8fe4)
![image](https://github.com/user-attachments/assets/d6b685c0-8419-4f58-93f9-4c53fb046df4)

In many of the cases shown above, one can see that the main time is increasing as both computation and communication times grow with the number of processors. This growth in computation time, instead of the expected decrease or stability, is most likely due to bottlenecks from ab inefficient implementation. Poorly optimized code may introduce redundant calculations or excessive synchronization points, which may negate the benefits of increasing processors. As expected we can see that the time for communication increases, due to the communication overhead. Together, the inefficient computation and rising communication costs lead to a significant increase in the main execution time, indicating that the scalability of the implementation could be improved.

##### Weak Scaling Main
![image](https://github.com/user-attachments/assets/5141aa54-9e13-403b-bd03-89a8471a0cf7)
![image](https://github.com/user-attachments/assets/f4ce349a-3c06-48f5-be31-fffb3ab4ff97)
![image](https://github.com/user-attachments/assets/2c9014fa-0fd1-46de-9151-e98b5148c522)
![image](https://github.com/user-attachments/assets/76c2d9c2-12b0-41ae-bce6-530e13f89faa)
##### Weak Scaling Comm
![image](https://github.com/user-attachments/assets/d30bcbb3-586b-4164-9cb6-e4d273a5def3)
![image](https://github.com/user-attachments/assets/852c2b14-481c-4b0a-b607-bf251e067265)
![image](https://github.com/user-attachments/assets/76d2a8c2-2d66-45a6-bd07-c6a7ee80809c)
![image](https://github.com/user-attachments/assets/8415d6d4-e533-4c67-879c-5cd112e9ef35)
##### Weak Scaling Comp
![image](https://github.com/user-attachments/assets/9ddd8a19-82d9-4c7d-a7b1-9c8822d9274f)
![image](https://github.com/user-attachments/assets/49499574-8597-4ef4-af24-7b965d94b302)
![image](https://github.com/user-attachments/assets/cfd153b2-1684-42db-ae02-af7925a8c74f)
![image](https://github.com/user-attachments/assets/07075a5d-5eb8-497a-a32a-ea5f832160ac)

For weak scaling, it's expected that the output would be constant, however this implementation shows that greates exponential increase out of all the other sorts. This could be due to various bottlenecks and overheads in the algorithm. As the number of processes grows, so does the communication overhead, as each process may need to frequently exchange data with other. A faulty implementation could lead to the repeating of steps in the algorithm, negating the effects of paralellization in it's entirety.

##### Strong Scaling
![image](https://github.com/user-attachments/assets/e7f60a55-217d-4c5c-a8d7-cb9373fb8d7b)
![image](https://github.com/user-attachments/assets/f7a265b1-fd24-4ec2-bed4-b85d30ba598e)
![image](https://github.com/user-attachments/assets/50b9f68d-1ff1-4eda-ba97-806838b3bedc)
![image](https://github.com/user-attachments/assets/8eef2efe-0d21-4727-bd6c-0e81576a355b)

These graphs shows that generally, for smaller arrays liek 2^16, one can see a rapid decrease in execution time as the number of processes increases, showing that a relatively low process count creates optimal speedup. However, for larger arrays, the speedup is less pronounced, with execution times initially decreasing but then plateauing or even increasing slightly as more processes are added. This is not the expected behavior, as it would be assumed that increase in processors would lead to an increase in speedup for larger array sizes. (as computation time is dramatically shortened). These graphs however show that while parallelization improves performance up to a point, larger arrays face diminishing returns or increased overhead at higher process counts, likely due to poor implementation.

##### Cache Misses Main
![image](https://github.com/user-attachments/assets/9b653e97-069f-40b8-bf76-36cc12e3a417)
![image](https://github.com/user-attachments/assets/cce645dd-eeba-4a1b-9826-06d33ef0f1a3)
![image](https://github.com/user-attachments/assets/7148a6f8-9867-41de-a170-bb9e08019625)

##### Cache Misses Comm
![image](https://github.com/user-attachments/assets/cb4057d1-4058-42a9-84f7-e790fbfa3a3f)
![image](https://github.com/user-attachments/assets/1eea9602-84d0-4ba6-b863-6de806f27e68)
![image](https://github.com/user-attachments/assets/5902c66d-159f-403a-8b1d-d61e20486466)

##### Cache Misses Comp
![image](https://github.com/user-attachments/assets/280d0d00-1d3b-468a-a354-a911ca5dbb5b)
![image](https://github.com/user-attachments/assets/b2ab7b29-984f-4af4-9fa5-2204b1fe862c)
![image](https://github.com/user-attachments/assets/bce99de0-c542-425d-813e-089bdb88b8da)

The graphs show a clear trend for the L2 and L1 cache misses as the number of processes increases for sorted input data. The amount of Total L2 cache misses rises incredibly steeply with the increase in the number of processes at a magnitude far greater than the other lines, showing that the overall cache demand intensifies as more processes are used. The other lines shows that the cache behavior per individual rank doesn't vary very much as more ranks are added. This pattern implies that the parallelization strategy is effective in distributing cache accesses evenly across ranks, but the collective cache demand scales with the number of processes, potentially leading to increased cache contention at higher process counts. This may not be indicative of properly created algorithms, as it is a given that this implementation is flawed in various ways. Such as the Comp.

#### Joanne: Radix Sort
<img width="678" alt="Screenshot 2024-11-04 at 6 48 07 PM" src="https://github.com/user-attachments/assets/1c97d951-2537-43f4-8484-c8b7e1984e09">
For the Radix Sort algorithm, the comm_large and comp_large act the same as comm and comp, since they include all the computation and communication sections. 

Below are the strong scaling graphs for the main section. Due to the implementation, the algorithm’s total run time increases linearly for the two smaller array sizes as the algorithm parallelizes. For the larger array sizes, as the number of processes increases, the total main time increases exponentially. As the array sizes increase, the different types of user inputs also perform more and more similarly. 

![image](https://github.com/user-attachments/assets/8f42d367-5501-47e9-8961-acc472ef0067)
![image](https://github.com/user-attachments/assets/8f42d367-5501-47e9-8961-acc472ef0067)
![image](https://github.com/user-attachments/assets/e770f3e2-c13f-4b9c-aca5-7b0ae124dd7a)
![image](https://github.com/user-attachments/assets/30d6dcee-16af-48cb-b8a4-18b6977ae642)
![image](https://github.com/user-attachments/assets/4a6b7c6b-83d2-42a9-9fbb-0e0dc3e8337c)
![image](https://github.com/user-attachments/assets/89f786a7-23b5-4a50-be87-daba12cc2289)
![image](https://github.com/user-attachments/assets/7652ebe8-21c0-4286-beac-124a5fe12d2b)
![image](https://github.com/user-attachments/assets/ccadc353-f3b0-4407-b9f6-b61a0e9ba5d4)

The graphs below are the strong scaling graphs for communication time. For the 2^16 and 2^18 array sizes, the communication time stays generally constant until 256 processes and increases significantly for 512 and 1024 processes. The random array performed the best for those sizes. 

![image](https://github.com/user-attachments/assets/5dfc8b4f-afd6-4162-a4ba-808c20dcb9ea)
![image](https://github.com/user-attachments/assets/270b296f-6df2-4c17-95ed-5de88b12badf)

For the other larger array sizes, the time decreases from 2 processes until around 256 processes and increases significantly, with the exception of 2^20 size. The communication time graph is similar for the 2^20 array size but instead starts increasing from 64 processes, and peaks at 512 processes and drops to its lowest time for 1024 processes, which shows that the algorithm parallelized the best for that size. 

![image](https://github.com/user-attachments/assets/8ef6968b-bbc1-4081-9219-4a58f5fcb56c)
![image](https://github.com/user-attachments/assets/f78d02f5-629f-447e-aec1-5e3d45709e2d)
![image](https://github.com/user-attachments/assets/ae3ec9f2-0a63-4a11-bf2c-9e3aee18b2f2)
![image](https://github.com/user-attachments/assets/6f0748cd-f08c-4bcc-a2a4-6695ed06f3a5)
![image](https://github.com/user-attachments/assets/c787c89f-bf0b-4ed6-bcfb-ad4d5b77b0ad)

In the strong scaling computation time graphs below, all the computation time decreased exponentially as the number of processes decreased. All input types performed similarly across the different array sizes.
![image](https://github.com/user-attachments/assets/b0ece0aa-9df5-4dd2-b7ee-0d56848cf273)
![image](https://github.com/user-attachments/assets/c08140ef-349e-415d-bc28-451e60b618be)
![image](https://github.com/user-attachments/assets/335170d4-5dc3-433c-b00b-d26bc208a221)
![image](https://github.com/user-attachments/assets/901b25af-03a3-46c8-9720-66ddd046a618)
![image](https://github.com/user-attachments/assets/6446bbd4-6ee3-4385-9234-38cfb7a793f5)
![image](https://github.com/user-attachments/assets/d9ebbabe-7bc1-46d1-8d29-387e941d5ffa)
![image](https://github.com/user-attachments/assets/960d5993-c3cb-4545-a61c-68e4f3bc501a)

Below are the total main time weak scaling graphs for each input type, where the number of processes and the input sizes increased at a constant rate. For all input types, the total main time increased at a faster rate than the number of processes and problem size. Ideally, the weak scaling graph should be a horizontal line, but for the main time especially, the total time is only somewhat linear up until 16 processes, which then starts growing exponentially. The graphs look similar for communication and computation times, showing that the implementation does not allow for the algorithm to scale well as the number of processes and problem size increases. 

![image](https://github.com/user-attachments/assets/a5451339-f554-435d-ba96-6bcf73165b67)
![image](https://github.com/user-attachments/assets/91bb4489-60c3-4a6b-aba5-f76970de742b)
![image](https://github.com/user-attachments/assets/b06117fd-d440-4600-9eda-5e96c1cb2a46)
![image](https://github.com/user-attachments/assets/758da8f2-0c3d-42a8-9ac4-22453a7c41b9)
![image](https://github.com/user-attachments/assets/f90e678d-83be-4a4a-b397-d7e2a2df018c)
![image](https://github.com/user-attachments/assets/7d922177-564a-4b75-bc51-1f53c01dae00)
![image](https://github.com/user-attachments/assets/c2b699a1-82fd-40c9-ba7b-7ee7bf591102)
![image](https://github.com/user-attachments/assets/eb64b43e-8fba-4f6b-ab0f-881594d5aa94)
![image](https://github.com/user-attachments/assets/951f79a1-1d5c-42c8-9078-adffd8259794)
![image](https://github.com/user-attachments/assets/549382e9-5bed-49d7-a415-a3783e3c3614)
![image](https://github.com/user-attachments/assets/c90279ce-c99c-4748-844f-db4e63bce057)
![image](https://github.com/user-attachments/assets/560a013c-1920-49aa-a2e0-d6af0e7fb754)

For all total main time strong scaling speedup graphs, most array sizes had the highest speedup at 8 processors, and then decreased eventually below 2, which shows that the total main time gets longer as the number of processes increases. 
![image](https://github.com/user-attachments/assets/eced5360-36f0-4585-b1e0-f2d6ea3d9ee7)
![image](https://github.com/user-attachments/assets/28cd8c35-11a1-4660-8b29-54e762384fd7)
![image](https://github.com/user-attachments/assets/2f8094f5-4e2a-4bcd-b1ba-6affc39eecb6)
![image](https://github.com/user-attachments/assets/f8f21226-6eb8-4b7a-ba8e-2ccc232c9f11)

For the strong scaling communication speedup, for most of the array sizes, the speed up peaked at 64 processors, which makes sense since that is when the number of nodes requested starts to increase. The large decrease in speedup for communication time for the larger number of processes also suggests that there is a large communication overhead in the implementation that prevents the algorithm from benefiting from the parallelization. 

![image](https://github.com/user-attachments/assets/76ff6ca1-c948-457a-9f94-b167f9a71451)
![image](https://github.com/user-attachments/assets/4125b8b6-59f2-4de8-9397-7854e49658b8)
![image](https://github.com/user-attachments/assets/d242528d-8485-4ae5-b01a-26d0251abcc0)
![image](https://github.com/user-attachments/assets/f77cd742-91b5-402c-bf4d-4d1e71501443)

For the strong scaling computation speedup graph for all the input types, they all increased as the number of processes increased. The growth rate tapered off for the larger number of processes. 
![image](https://github.com/user-attachments/assets/a823b12f-6169-4e4a-bf8f-01e57dc61376)
![image](https://github.com/user-attachments/assets/e07820a3-08d1-4b2a-99d9-5857dddc1d57)
![image](https://github.com/user-attachments/assets/887d445d-ab68-4e9f-a68f-01a8afb5f7cd)
![image](https://github.com/user-attachments/assets/36749a71-ac31-4662-85c7-d8e20795b25c)

Below are the L1 and L2 cache misses per rank for each input type. Shown in the cache misses graphs, the large array sizes have more cache misses. For all of the total main time and comp_large graphs, the total number of cache misses also increases as the number of processes increases from 16 to 32. For the comm_large graphs, the cache misses would decrease as the number of processes increases. 
![image](https://github.com/user-attachments/assets/48a72020-5e96-4905-83ff-24956f14ff64)
![image](https://github.com/user-attachments/assets/86e37874-3ebd-484f-9090-3895b0aec3c5)
![image](https://github.com/user-attachments/assets/46b766de-cfec-4380-b4f4-63681ad89acf)
![image](https://github.com/user-attachments/assets/228b9814-71e3-4466-96df-64ca3bcead49)
![image](https://github.com/user-attachments/assets/5446db65-59e5-4a1c-91d2-8cb9a08fe451)
![image](https://github.com/user-attachments/assets/8ba37a8b-968e-4769-af2d-da14ff3b1c77)
![image](https://github.com/user-attachments/assets/3d6917ea-91d0-438a-b7d8-03c709620134)
![image](https://github.com/user-attachments/assets/b79e7b1f-b707-4b87-b6b6-0f0d63129aa4)
![image](https://github.com/user-attachments/assets/353f7d56-2e11-42fc-922d-869a77228f4c)
![image](https://github.com/user-attachments/assets/d1738a53-4cb9-4737-a8c2-adeb218ad060)
![image](https://github.com/user-attachments/assets/bf8da415-2876-4f0c-ae91-00105d63d0ef)
![image](https://github.com/user-attachments/assets/a4e0d522-6639-4478-a64d-c1b8a0062ad3)
![image](https://github.com/user-attachments/assets/fc4dc388-6c80-4f63-a241-dabbe693a9bc)
![image](https://github.com/user-attachments/assets/f8b985a3-015b-4c33-a3f2-90933e905f65)
![image](https://github.com/user-attachments/assets/ab0e8ecd-fc06-4d17-be1a-27b364e290ab)
![image](https://github.com/user-attachments/assets/20318b21-b395-4801-ad51-aeaf538b3102)
![image](https://github.com/user-attachments/assets/5dba6f0d-fc21-430f-aa64-5c4dbe675853)
![image](https://github.com/user-attachments/assets/03853d40-8cf7-421c-b59f-fff8efae4d08)

#### Nathan: Merge Sort
![image](https://github.com/user-attachments/assets/5c78a834-bea9-49c8-a2eb-a5a3df717f6b)  
For merge sort, the comp_large and comm_large sections are the same as comp and comm respectively since all the communication and computation involved fall under comp_large and comm_large  

![image](https://github.com/user-attachments/assets/d6e99286-be3b-4fac-b053-8ad20f0ffff2)
![image](https://github.com/user-attachments/assets/ad738112-7205-4b62-b10c-e0e27b85f6de)  
For the 2^16 array size, you can see that though the comp time decreases as number of processes increases, the comm time also increases causing the overall main time to trend toward increasing as number of processes increases as well due to overhead and all times are similar regardless of sorting type  
![image](https://github.com/user-attachments/assets/68a9f817-ea13-4344-8d3e-ee3422997155)
![image](https://github.com/user-attachments/assets/bfadfe69-9b2f-4ec9-85a5-4442f79a5ad9)  
For the 2^18 array size, you can see that though the comp time decreases as number of processes increases, the comm time also increases causing the overall main time to trend toward increasing as number of processes increases as well due to overhead and all times are similar regardless of sort type  
![image](https://github.com/user-attachments/assets/0b5fdd89-37bc-49b6-a323-35d5cb5d7747)
![image](https://github.com/user-attachments/assets/c0968bff-d9de-4039-8cc4-0053b3a152ca)  
For the 2^20 array size, you can see that the comp time decreases as the number of processes increases and the comm time remains somewhat constant until an increase near the larger numbers of processors which still causes the overall main time to generally trend toward increasing as the number of processes increases as well due to overhead. For this array size, the comm times for random and 1_perc_perturbed sorting types were greater  
![image](https://github.com/user-attachments/assets/24ef18d3-903d-4bef-be17-9b6fa68e3232)
![image](https://github.com/user-attachments/assets/f514f037-3e46-419f-800e-8062e39908f0)
For the 2^22 array size, you can see that as the number of processes increases, the comp time increases and the comm time remains somewhat constant and since the decrease in comp time is more significant than the overhead, the total main time trends towards decreasing as the number of processes increases. For this array size, the comm times for random and 1_perc_perturbed sort types are greater.  
![image](https://github.com/user-attachments/assets/96738e4d-cff5-47dd-b456-2f0847dbcd21)
![image](https://github.com/user-attachments/assets/0a3a9233-e062-4315-ba72-3975b74426a3)  
For the 2^24 array size, you can see that as the number of processes increases, the comp time increases and the comm time remains somewhat constant and since the decrease in comp time is more significant than the overhead, the total main time trends towards decreasing as the number of processes increases. For this array size, the comm times for random and 1_perc_perturbed sort types are greater.  
![image](https://github.com/user-attachments/assets/61d64855-80b2-48f5-9f01-040d9a9eaaa7)
![image](https://github.com/user-attachments/assets/8aaf6ae7-0edb-4ad2-abe5-02f5b753d1a5)  
For the 2^26 array size, you can see that as the number of processes increases, the comp time increases and the comm time remains somewhat constant and since the decrease in comp time is more significant than the overhead, the total main time trends towards decreasing as the number of processes increases. For this array size, the comm times for random and 1_perc_perturbed sort types are greater.  
![image](https://github.com/user-attachments/assets/0be2b578-7597-40d8-a125-f27ce2796d7b)
![image](https://github.com/user-attachments/assets/acd70b16-cf3a-4269-aa95-96b4a3e03fd8)  
For the 2^28 array size, you can see that as the number of processes increases, the comp time increases and the comm time remains somewhat constant and since the decrease in comp time is more significant than the overhead, the total main time trends towards decreasing as the number of processes increases. For this array size, the comm times for random and 1_perc_perturbed sort types are greater.  
![image](https://github.com/user-attachments/assets/964271fa-3ab2-4a01-bcad-c5280c3663d5)
![image](https://github.com/user-attachments/assets/d143bcb6-f38e-4899-9af7-3ed72e1089c6)
![image](https://github.com/user-attachments/assets/0204d7f1-63a9-4f3a-9d73-e66402cb3009)  
 For the weak scaling plot for comp, the graph follows an increasing concave up trend for all sorting types though the times for random and 1_perc_perturbed are a bit higher. This trend is not ideal as the weak scaling should be about linear in nature and the difference of the plots from the ideal linear trend is probably due to issues with overhead in implementation  
![image](https://github.com/user-attachments/assets/133900ea-edb2-412d-b826-22f24c256805)
![image](https://github.com/user-attachments/assets/2c5b6a1a-ed2e-4cdd-9df2-cdf7bc334b18)
![image](https://github.com/user-attachments/assets/83897dd2-75f7-4590-926a-752a36036523)  
 For the weak scaling plot for comm, the graph follows an increasing concave up trend for all sorting types though the times for random and 1_perc_perturbed are noticeably higher especially at the highest numbers of processes. This trend is not ideal as the weak scaling should be about linear in nature and the difference of the plots from the ideal linear trend is probably due to issues with overhead in implementation  
![image](https://github.com/user-attachments/assets/cea2002d-7d86-4f37-800f-2434a7aa45a4)
![image](https://github.com/user-attachments/assets/2acbbb42-6612-465d-b958-14e7039391af)
![image](https://github.com/user-attachments/assets/cb6a6858-4ecc-429b-ac52-f0d768cbab31)  
 For the weak scaling plot for main, the graph follows an increasing concave up trend for all sorting types though the times for random and 1_perc_perturbed are noticeably higher especially at the highest numbers of processes. This trend is not ideal as the weak scaling should be about linear in nature and the difference of the plots from the ideal linear trend is probably due to issues with overhead in implementation  
![image](https://github.com/user-attachments/assets/eb5e196e-2d46-46d2-8e98-3c0adc85e812)
![image](https://github.com/user-attachments/assets/b469f709-ce8b-4d16-868d-6ab9191658af)
![image](https://github.com/user-attachments/assets/2b97ef79-43c6-4dcb-93c3-07709882bf1a)  
For the strong scaling speedup plots for comp, for all of the array sizes, the comp times trend towards increasing in a concave up manner as number of processes increases with similar values regardless of input type  
![image](https://github.com/user-attachments/assets/af4626c5-d7c8-4cae-b2c9-56d4bff01157)
![image](https://github.com/user-attachments/assets/a5493c4d-28bb-4019-a8e8-64470e51ba88)
![image](https://github.com/user-attachments/assets/bd4c8cbd-7f64-4d70-af5a-5a8a44120106)  
For the strong scaling plots for comm, for all of the array sizes, there is a trend of an initial decrease which then evens out and becomes about constant though the smaller array sizes tend to have a greater fluctuation in this constant as compared to the larger array sizes. The plots are about similar across input types  
![image](https://github.com/user-attachments/assets/bf77ecb4-5450-4ff9-94c2-98dc618d71f7)
![image](https://github.com/user-attachments/assets/57d2b535-6088-43ca-a7c5-3ac0026ae02a)
![image](https://github.com/user-attachments/assets/53068552-caff-44fe-81f5-2f64accb8b25)  
For the strong scaling speedup plots for main, the smaller array sizes remain about constant while the larger array sizes have an initial steep increase that then becomes less steep and eventually plateaus as the number of processes increases. This is due to the overhead becoming greater than the benefits in computation from adding processes. The times are higher for reverse sorted and sorted input types  
![image](https://github.com/user-attachments/assets/0baa44d6-c92e-4f40-a791-064e430aacec)
![image](https://github.com/user-attachments/assets/7e0fe89c-5468-4892-8bff-e52a55d0d491)  
For comp large it can be seen that for the L1 cache misses, there is a slight change in cache misses as the processes increases but mainly, there is a notable increase in cache misses as the array size increases  
![image](https://github.com/user-attachments/assets/58c73fec-f0ee-4614-839a-084a383d9eae)
![image](https://github.com/user-attachments/assets/735c62fd-810e-4fd4-bd22-d5c721cf93a3)  
For comm large it can be seen that for the L1 cache misses, there is a slight change in cache misses as the processes increases but mainly, there is a notable increase in cache misses as the array size increases  
![image](https://github.com/user-attachments/assets/0b785823-6d2c-4258-8f4a-7966c1e7b570)
![image](https://github.com/user-attachments/assets/f09e26d6-d233-42e5-830c-f9f7de5387eb)  
For main it can be seen that for the L1 cache misses, there is a slight change in cache misses as the processes increases but mainly, there is a notable increase in cache misses as the array size increases  
![image](https://github.com/user-attachments/assets/bb83eade-b96c-46ff-902e-7cd5109b4d9c)
![image](https://github.com/user-attachments/assets/69955bc2-00bf-4350-b5dc-298a03ddb1e5)  
For comp large it can be seen that for the L2 cache misses, there is a slight change in cache misses as the processes increases but mainly, there is a notable increase in cache misses as the array size increases  
![image](https://github.com/user-attachments/assets/3aa9a7c0-e8b4-4922-b981-5d7f1c17a9f7)
![image](https://github.com/user-attachments/assets/e27bfb2b-3251-4fee-8920-4ffdad05447f)  
For comm large it can be seen that for the L2 cache misses, there is a slight change in cache misses as the processes increases aside from the case for sorted input for 2^28 array size but mainly, there is a notable increase in cache misses as the array size increases  
![image](https://github.com/user-attachments/assets/c79d8c86-1c8c-4e67-a2b8-24b8a494876a)
![image](https://github.com/user-attachments/assets/0a33adb9-0d52-4e50-9931-919926831335)  
For comm large it can be seen that for the L2 cache misses, there is a slight change in cache misses as the processes increases aside from the cases for sorted input but mainly, there is a notable increase in cache misses as the array size increases  

#### Mrinal : Sample Sort
<img width="804" alt="image" src="https://github.com/user-attachments/assets/65ca37ea-6042-4da5-9d8e-d3f8579693af">

For the sample sort algorithm, the comp takes the most time, specifically the comp large portion, as each bucket has to sort and calculate its pivots.

![image](https://github.com/user-attachments/assets/5789b104-bf1a-43fb-8e99-bd4fe9f6e4fa)
![image](https://github.com/user-attachments/assets/2093ff31-4cfe-4810-8854-92a371171d7e)
![image](https://github.com/user-attachments/assets/5528d02e-669d-4988-b4d1-c7d2791cf7ad)
![image](https://github.com/user-attachments/assets/ab8ec459-51e4-4ce6-ab00-5c248028cfc6)
![image](https://github.com/user-attachments/assets/55a3b6bb-5734-4f9a-8cf4-2a43779b0c60)
![image](https://github.com/user-attachments/assets/4f86a6be-8461-452c-8d79-a7dae593f2f0)
![image](https://github.com/user-attachments/assets/8e5ddea1-0d8d-42df-9b91-ae5015d40c13)

Here are the strong scaling graphs for the main section. As you can see, the total run time increases with more processes until around an array size of 2^24. After 2^24, the total run time decreases exponentially the more processors the algorithm has. There does not seem to be a difference in run time based on the input type; however, random and 1% perturbed seem to have higher spikes than sorted and reverse sorted. This might be because sorted and reverse sorted do not need much time picking pivots and sorting buckets.

![image](https://github.com/user-attachments/assets/bad3e4a8-b2cb-451f-99b0-71cc2dea95fc)
![image](https://github.com/user-attachments/assets/33894977-b6ae-40d3-b2f2-56c82f6e34f6)
![image](https://github.com/user-attachments/assets/3dbc4623-28e9-4953-984c-c7b1958f51b4)
![image](https://github.com/user-attachments/assets/4af27219-99b4-43bb-8db3-9942ab4c3c89)

Here are the strong scaling speedup graphs for the main section. For these graphs, a similar trend seems to be in all of them where the speedup remains constant until an input size of 2^26, where the speedup increases with the number of processors. All the graphs also seem to have the speedup decrease when there are 1024 processors. These trends might be because of communication and synchronization costs at higher processor counts.

![image](https://github.com/user-attachments/assets/7c155144-92e0-408e-9bd1-b84d1fd65d87)
![image](https://github.com/user-attachments/assets/42708652-7c3a-486b-ab93-987d7ae3a4d2)
![image](https://github.com/user-attachments/assets/8ba677c8-c9d3-4cce-8ad3-7bbcd6af11da)
![image](https://github.com/user-attachments/assets/287d2b9a-91c6-463a-a1ca-65f28f4ce6d1)

Here are the weak scaling graphs for the main section. The same general trend can be seen in all these graphs, increasing exponentially until 64 processors and then spiking at 128 processors. This trend might be explained by increased communication at higher processor counts, increasing the time taken.

![image](https://github.com/user-attachments/assets/96e0abfc-c050-433c-bb3c-120e0674a20e)
![image](https://github.com/user-attachments/assets/d3cce0c9-2234-4a30-a66c-befda7bbbd72)
![image](https://github.com/user-attachments/assets/ae0e1208-26af-4275-88b3-7ba9e4f8a5ff)
![image](https://github.com/user-attachments/assets/2c3224a1-4611-4068-8ee3-cfe8c2bebda4)
![image](https://github.com/user-attachments/assets/73a44684-175c-4fce-877a-422040485b56)
![image](https://github.com/user-attachments/assets/6445eb9a-64f9-4466-b169-8a30cc9641c2)
![image](https://github.com/user-attachments/assets/fc720e0e-cc69-49c7-9eae-e10286190097)

Here are the strong scaling graphs for the comp large section. For the input sizes of 2^16 and 2^18, it can be seen that the time spikes at 512 processors and higher. In the 2^18 graph, it can also be seen that 2 and 4 processors also take a lot of time to compute. After these two input sizes the rest of the graphs follow the same trend of exponentially decreasing the more processors the algorithm has. This is because it takes more time to perform the computations to pick pivots the fewer processors you have unless you have a low input size.

![image](https://github.com/user-attachments/assets/bff816b0-c123-4aa5-baa8-b3afc44456e7)
![image](https://github.com/user-attachments/assets/0520d2c5-ff77-4e29-8629-4bbc8459eec1)
![image](https://github.com/user-attachments/assets/8366fdd6-1dfc-4b8b-8dc1-ac3f3c7b3e18)
![image](https://github.com/user-attachments/assets/aa7233cc-1674-46bf-8fa3-6aba0320391a)

Here are the strong scaling speedup graphs for the comp large section. All these graphs follow the ideal speedup trend where the speedup increases with more processors added. However, the speedup does flatten for an input size of 2^16 and 2^18. For the random input type, the speedup for each input size is more spread out than all the other input types. This might be because the random input types need more computations to select pivots.

![image](https://github.com/user-attachments/assets/c6a36c52-d793-4067-a7e2-577183d935d4)
![image](https://github.com/user-attachments/assets/cfb425f3-8b98-4d84-bfa3-982f1f041535)
![image](https://github.com/user-attachments/assets/b0c099eb-3956-4736-af98-28819986721d)
![image](https://github.com/user-attachments/assets/60c7e17f-2f28-4669-8061-6996433dc555)

Here are the weak scaling graphs for the comp large section. All the graphs follow the same trend of exponentially increasing with processor size regardless of input type. This trend might be explained by the increasing number of computations done the higher the processor count.

![image](https://github.com/user-attachments/assets/7be37df1-1a30-4494-ac18-35f844d4ed38)
![image](https://github.com/user-attachments/assets/6a95dfe5-e7d9-4a99-bbac-2878bd8e992e)
![image](https://github.com/user-attachments/assets/d3ef91be-01c3-4d89-824e-001f41fb4b0c)
![image](https://github.com/user-attachments/assets/71570eca-56c2-4e38-af00-b818de276ce2)
![image](https://github.com/user-attachments/assets/ef676f56-38a1-4c6b-9c2a-f68bb8e1923e)
![image](https://github.com/user-attachments/assets/a06b422d-1908-484a-8c0d-bf68b1e23700)
![image](https://github.com/user-attachments/assets/5fcfea47-54e0-4530-8566-8590d984d066)

Here are the strong scaling graphs for the comm section. For the input size of 2^16, you can see that the time generally increases the more processors there are, with a massive spike at 256 processors for the reverse sorted input type. This is most likely because it is a bad run, as it does not follow the trends set by the other runs. The 2^18 input size follows the same trend with a spike at 64 processors for random and sorted input types. This also might be due to a bad run, as no other spikes follow. The 2^20 input size has a quadratic trend as the lower the number of processors, the higher the time, and the higher the number of processors, the higher the time. There is also a spike at 128 processors for the sorted input type, probably resulting from a bad run. The rest of the input sizes all follow the same general trend of the time exponentially decreasing with the number of processors, with 2^20 having a spike at 128 processors for the random input type. This is most likely because it takes less to communicate the pivots and data with more processors, the higher the input size. 

![image](https://github.com/user-attachments/assets/d223d77b-e0de-4bbf-90da-93584307897a)
![image](https://github.com/user-attachments/assets/f8eaa3a8-eb30-4349-b5cf-226e501d1b5e)
![image](https://github.com/user-attachments/assets/4f041a7a-2ef0-43f0-9ba5-fac66dc4dd33)
![image](https://github.com/user-attachments/assets/cdb60d67-17db-4a64-8e9a-35843ab23bdb)

Here are the strong scaling speedup graphs for the comm section. The speedup increases for the random input type until around 64 processors and then decreases/remains constant. This is most likely because random runs might not scale properly with more processors. The speedup decreases the more processors there are for the sorted input type, likely because the more processors there are, the more time it takes to communicate between each processor, even though there isn't a reason to communicate since it is already sorted. The speedup for the 1% perturbed seems to follow the same trend as the speedup for the main. The reverse sorted input type speedup follows a positive quadratic curve for lower input sizes and a negative quadratic curve for higher input sizes. There isn't really an explanation for why this is, and it might be due to how much communication needs to be done between processors based on the input size. 

![image](https://github.com/user-attachments/assets/9f0e2409-fae4-4903-82ed-e7136a002608)
![image](https://github.com/user-attachments/assets/b84c4660-3682-4ba4-9a42-28979f7114fe)
![image](https://github.com/user-attachments/assets/582efc2a-6740-4777-80cf-869361e74a98)
![image](https://github.com/user-attachments/assets/cb911897-1abe-4ce8-8a30-f5809223fc4a)

Here are the weak scaling graphs for the comm section. They all seem to follow the same trends as the main and comp large sections. This might be due to how much overhead there is when processors need to communicate with each other. That would add extra time the more processors there are, which might not scale linearly.

![image](https://github.com/user-attachments/assets/9d37903e-1936-42f4-8169-1c3dcb737924)
![image](https://github.com/user-attachments/assets/9cbef80c-8b84-41f2-9326-fbeb2a5fcd7d)
![image](https://github.com/user-attachments/assets/9fc3a7f0-42ed-409f-919d-ff8ba429d106)
![image](https://github.com/user-attachments/assets/27c2ebfc-82c3-426d-90b1-2e2f625ba595)
![image](https://github.com/user-attachments/assets/df042b6c-9d03-404b-8c40-17fb16abd491)
![image](https://github.com/user-attachments/assets/a4a09659-032d-4d49-a080-d3de6c32547c)

Here are the L1 and L2 cache miss graphs for the main section. For both the L1 and L2 cache, the number of processors seems to decrease the amount of cache misses regardless of input type. There seems to be a huge spike in the number of cache misses for the L2 cache in a 2^27 array with 16 processors with a sorted input. However, this is probably the result of a bad run, as the trend does not repeat with other graphs. 

![image](https://github.com/user-attachments/assets/bc90c74c-597f-43b2-86a7-838e580f8b6b)
![image](https://github.com/user-attachments/assets/d9c428c1-faba-47fd-be21-dbce1172f42e)
![image](https://github.com/user-attachments/assets/d1455d9f-46b7-492a-81f7-8549881a89ce)
![image](https://github.com/user-attachments/assets/12dac682-7497-4b4e-a1f7-ef9b45cb445b)
![image](https://github.com/user-attachments/assets/fb09da09-3d77-4fdb-8429-ed007f970b85)
![image](https://github.com/user-attachments/assets/3428ad16-cbd1-43e6-a56f-f981d106713b)

Here are the L1 and L2 cache miss graphs for the comp_large section. These graphs follow the same trend as the graphs in the main section, with the number of misses decreasing with the number of processors and the number of misses in total increasing with higher input size. There is also the same spike as before, probably the result of a bad run. 

![image](https://github.com/user-attachments/assets/72e5b032-4879-4383-a547-d94bb6fbc5da)
![image](https://github.com/user-attachments/assets/389bdfa1-b35b-4023-8590-3d8a7b8dc345)
![image](https://github.com/user-attachments/assets/6039398f-1147-4c0c-9260-813162790426)
![image](https://github.com/user-attachments/assets/2857ab5a-e900-46e4-b9c9-96f8ba1c7a02)
![image](https://github.com/user-attachments/assets/5dbd90c6-d92e-462c-99ed-a9acc671da46)
![image](https://github.com/user-attachments/assets/d033fa8e-ad32-41d0-8c3b-afdba9ec33e0)

Here are the L1 and L2 cache miss graphs for the comm section. These graphs differ from the previous two sections as the graphs show a slight increase in the number of misses the more processors there are. This might be because there are more communication events the more processors there are, which increases the chances of there being a cache miss. There seems to be roughly the same amount of misses, whether L1 or L2.
