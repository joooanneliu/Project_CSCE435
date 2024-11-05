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
Due to difficulties with the Grace queue, I was unable to gain all 280 cali files. I was able to get data for all of the input types and processors for the smallest size (2^16) as well as a wide variety of other size, processor, input combinations. Before the presentation I will be able gain the remaining cali files, so that I may present my strong scaling, speedup, and weak scaling plots. 

#### Joanne: Radix Sort
<img width="678" alt="Screenshot 2024-11-04 at 6 48 07 PM" src="https://github.com/user-attachments/assets/1c97d951-2537-43f4-8484-c8b7e1984e09">
For the Radix Sort algorithm, the comm_large and comp_large act the same as comm and comp, since they include all the computation and communication sections. 

Below are the strong scaling graphs for the main section. Due to the implementation, the algorithm’s total run time increases linearly for the two smaller array sizes as the algorithm parallelizes. For the larger array sizes, as the number of processes increases, the total main time increases exponentially. As the array sizes increase, the different types of user inputs also perform more and more similarly. 

<img width="892" alt="Screenshot 2024-11-04 at 6 47 24 PM" src="https://github.com/user-attachments/assets/8f42d367-5501-47e9-8961-acc472ef0067">
<img width="883" alt="Screenshot 2024-11-04 at 6 47 20 PM" src="https://github.com/user-attachments/assets/e770f3e2-c13f-4b9c-aca5-7b0ae124dd7a">
<img width="858" alt="Screenshot 2024-11-04 at 6 47 15 PM" src="https://github.com/user-attachments/assets/30d6dcee-16af-48cb-b8a4-18b6977ae642">
<img width="888" alt="Screenshot 2024-11-04 at 6 47 10 PM" src="https://github.com/user-attachments/assets/4a6b7c6b-83d2-42a9-9fbb-0e0dc3e8337c">
<img width="901" alt="Screenshot 2024-11-04 at 6 47 06 PM" src="https://github.com/user-attachments/assets/89f786a7-23b5-4a50-be87-daba12cc2289">
<img width="890" alt="Screenshot 2024-11-04 at 6 47 01 PM" src="https://github.com/user-attachments/assets/7652ebe8-21c0-4286-beac-124a5fe12d2b">
<img width="866" alt="Screenshot 2024-11-04 at 6 46 57 PM" src="https://github.com/user-attachments/assets/ccadc353-f3b0-4407-b9f6-b61a0e9ba5d4">

The graphs below are the strong scaling graphs for communication time. For the 2^16 and 2^18 array sizes, the communication time stays generally constant until 256 processes and increases significantly for 512 and 1024 processes. The random array performed the best for those sizes. 
<img width="904" alt="Screenshot 2024-11-04 at 6 50 18 PM" src="https://github.com/user-attachments/assets/5dfc8b4f-afd6-4162-a4ba-808c20dcb9ea">
<img width="894" alt="Screenshot 2024-11-04 at 6 50 13 PM" src="https://github.com/user-attachments/assets/270b296f-6df2-4c17-95ed-5de88b12badf">

For the other larger array sizes, the time decreases from 2 processes until around 256 processes and increases significantly, with the exception of 2^20 size. The communication time graph is similar for the 2^20 array size but instead starts increasing from 64 processes, and peaks at 512 processes and drops to its lowest time for 1024 processes, which shows that the algorithm parallelized the best for that size. 

<img width="933" alt="Screenshot 2024-11-04 at 6 51 09 PM" src="https://github.com/user-attachments/assets/8ef6968b-bbc1-4081-9219-4a58f5fcb56c">
<img width="901" alt="Screenshot 2024-11-04 at 6 51 03 PM" src="https://github.com/user-attachments/assets/f78d02f5-629f-447e-aec1-5e3d45709e2d">
<img width="859" alt="Screenshot 2024-11-04 at 6 50 57 PM" src="https://github.com/user-attachments/assets/ae3ec9f2-0a63-4a11-bf2c-9e3aee18b2f2">
<img width="896" alt="Screenshot 2024-11-04 at 6 50 52 PM" src="https://github.com/user-attachments/assets/6f0748cd-f08c-4bcc-a2a4-6695ed06f3a5">
<img width="881" alt="Screenshot 2024-11-04 at 6 50 47 PM" src="https://github.com/user-attachments/assets/c787c89f-bf0b-4ed6-bcfb-ad4d5b77b0ad">

In the strong scaling computation time graphs below, all the computation time decreased exponentially as the number of processes decreased. All input types performed similarly across the different array sizes.
<img width="865" alt="Screenshot 2024-11-04 at 6 52 40 PM" src="https://github.com/user-attachments/assets/b0ece0aa-9df5-4dd2-b7ee-0d56848cf273">
<img width="868" alt="Screenshot 2024-11-04 at 6 52 34 PM" src="https://github.com/user-attachments/assets/c08140ef-349e-415d-bc28-451e60b618be">
<img width="884" alt="Screenshot 2024-11-04 at 6 52 27 PM" src="https://github.com/user-attachments/assets/335170d4-5dc3-433c-b00b-d26bc208a221">
<img width="892" alt="Screenshot 2024-11-04 at 6 52 22 PM" src="https://github.com/user-attachments/assets/901b25af-03a3-46c8-9720-66ddd046a618">
<img width="862" alt="Screenshot 2024-11-04 at 6 52 18 PM" src="https://github.com/user-attachments/assets/6446bbd4-6ee3-4385-9234-38cfb7a793f5">
<img width="893" alt="Screenshot 2024-11-04 at 6 52 01 PM" src="https://github.com/user-attachments/assets/d9ebbabe-7bc1-46d1-8d29-387e941d5ffa">
<img width="873" alt="Screenshot 2024-11-04 at 6 51 56 PM" src="https://github.com/user-attachments/assets/960d5993-c3cb-4545-a61c-68e4f3bc501a">

Below are the total main time weak scaling graphs for each input type, where the number of processes and the input sizes increased at a constant rate. For all input types, the total main time increased at a faster rate than the number of processes and problem size. Ideally, the weak scaling graph should be a horizontal line, but for the main time especially, the total time is only somewhat linear up until 16 processes, which then starts growing exponentially. The graphs look similar for communication and computation times, showing that the implementation does not allow for the algorithm to scale well as the number of processes and problem size increases. 

<img width="885" alt="Screenshot 2024-11-04 at 6 53 32 PM" src="https://github.com/user-attachments/assets/a5451339-f554-435d-ba96-6bcf73165b67">
<img width="868" alt="Screenshot 2024-11-04 at 6 53 27 PM" src="https://github.com/user-attachments/assets/91bb4489-60c3-4a6b-aba5-f76970de742b">
<img width="861" alt="Screenshot 2024-11-04 at 6 53 23 PM" src="https://github.com/user-attachments/assets/b06117fd-d440-4600-9eda-5e96c1cb2a46">
<img width="867" alt="Screenshot 2024-11-04 at 6 53 10 PM" src="https://github.com/user-attachments/assets/758da8f2-0c3d-42a8-9ac4-22453a7c41b9">
<img width="876" alt="Screenshot 2024-11-04 at 6 54 51 PM" src="https://github.com/user-attachments/assets/f90e678d-83be-4a4a-b397-d7e2a2df018c">
<img width="867" alt="Screenshot 2024-11-04 at 6 54 46 PM" src="https://github.com/user-attachments/assets/7d922177-564a-4b75-bc51-1f53c01dae00">
<img width="871" alt="Screenshot 2024-11-04 at 6 54 40 PM" src="https://github.com/user-attachments/assets/c2b699a1-82fd-40c9-ba7b-7ee7bf591102">
<img width="859" alt="Screenshot 2024-11-04 at 6 54 36 PM" src="https://github.com/user-attachments/assets/eb64b43e-8fba-4f6b-ab0f-881594d5aa94">
<img width="896" alt="Screenshot 2024-11-04 at 6 54 30 PM" src="https://github.com/user-attachments/assets/951f79a1-1d5c-42c8-9078-adffd8259794">
<img width="891" alt="Screenshot 2024-11-04 at 6 54 26 PM" src="https://github.com/user-attachments/assets/549382e9-5bed-49d7-a415-a3783e3c3614">
<img width="880" alt="Screenshot 2024-11-04 at 6 54 20 PM" src="https://github.com/user-attachments/assets/c90279ce-c99c-4748-844f-db4e63bce057">
<img width="874" alt="Screenshot 2024-11-04 at 6 54 15 PM" src="https://github.com/user-attachments/assets/560a013c-1920-49aa-a2e0-d6af0e7fb754">

For all total main time strong scaling speedup graphs, most array sizes had the highest speedup at 8 processors, and then decreased eventually below 2, which shows that the total main time gets longer as the number of processes increases. 
<img width="876" alt="Screenshot 2024-11-04 at 6 56 05 PM" src="https://github.com/user-attachments/assets/eced5360-36f0-4585-b1e0-f2d6ea3d9ee7">
<img width="864" alt="Screenshot 2024-11-04 at 6 56 11 PM" src="https://github.com/user-attachments/assets/28cd8c35-11a1-4660-8b29-54e762384fd7">
<img width="876" alt="Screenshot 2024-11-04 at 6 56 39 PM" src="https://github.com/user-attachments/assets/2f8094f5-4e2a-4bcd-b1ba-6affc39eecb6">
<img width="874" alt="Screenshot 2024-11-04 at 6 56 35 PM" src="https://github.com/user-attachments/assets/f8f21226-6eb8-4b7a-ba8e-2ccc232c9f11">

For the strong scaling communication speedup, for most of the array sizes, the speed up peaked at 64 processors, which makes sense since that is when the number of nodes requested starts to increase. The large decrease in speedup for communication time for the larger number of processes also suggests that there is a large communication overhead in the implementation that prevents the algorithm from benefiting from the parallelization. 

<img width="872" alt="Screenshot 2024-11-04 at 6 57 03 PM" src="https://github.com/user-attachments/assets/76ff6ca1-c948-457a-9f94-b167f9a71451">
<img width="880" alt="Screenshot 2024-11-04 at 6 57 13 PM" src="https://github.com/user-attachments/assets/4125b8b6-59f2-4de8-9397-7854e49658b8">
<img width="860" alt="Screenshot 2024-11-04 at 6 57 26 PM" src="https://github.com/user-attachments/assets/d242528d-8485-4ae5-b01a-26d0251abcc0">
<img width="883" alt="Screenshot 2024-11-04 at 6 57 37 PM" src="https://github.com/user-attachments/assets/f77cd742-91b5-402c-bf4d-4d1e71501443">

For the strong scaling computation speedup graph for all the input types, they all increased as the number of processes increased. The growth rate tapered off for the larger number of processes. 
<img width="868" alt="Screenshot 2024-11-04 at 6 57 55 PM" src="https://github.com/user-attachments/assets/a823b12f-6169-4e4a-bf8f-01e57dc61376">
<img width="857" alt="Screenshot 2024-11-04 at 6 58 03 PM" src="https://github.com/user-attachments/assets/e07820a3-08d1-4b2a-99d9-5857dddc1d57">
<img width="872" alt="Screenshot 2024-11-04 at 6 58 19 PM" src="https://github.com/user-attachments/assets/887d445d-ab68-4e9f-a68f-01a8afb5f7cd">
<img width="840" alt="Screenshot 2024-11-04 at 6 58 31 PM" src="https://github.com/user-attachments/assets/36749a71-ac31-4662-85c7-d8e20795b25c">

Below are the L1 and L2 cache misses per rank for each input type. Shown in the cache misses graphs, the large array sizes have more cache misses. For all of the total main time and comp_large graphs, the total number of cache misses also increases as the number of processes increases from 16 to 32. For the comm_large graphs, the cache misses would decrease as the number of processes increases. 
<img width="886" alt="Screenshot 2024-11-04 at 6 59 04 PM" src="https://github.com/user-attachments/assets/48a72020-5e96-4905-83ff-24956f14ff64">
<img width="867" alt="Screenshot 2024-11-04 at 6 59 13 PM" src="https://github.com/user-attachments/assets/86e37874-3ebd-484f-9090-3895b0aec3c5">
<img width="878" alt="Screenshot 2024-11-04 at 6 59 23 PM" src="https://github.com/user-attachments/assets/46b766de-cfec-4380-b4f4-63681ad89acf">
<img width="869" alt="Screenshot 2024-11-04 at 6 59 31 PM" src="https://github.com/user-attachments/assets/228b9814-71e3-4466-96df-64ca3bcead49">
<img width="880" alt="Screenshot 2024-11-04 at 6 59 39 PM" src="https://github.com/user-attachments/assets/5446db65-59e5-4a1c-91d2-8cb9a08fe451">
<img width="881" alt="Screenshot 2024-11-04 at 6 59 48 PM" src="https://github.com/user-attachments/assets/8ba37a8b-968e-4769-af2d-da14ff3b1c77">
<img width="871" alt="Screenshot 2024-11-04 at 6 59 57 PM" src="https://github.com/user-attachments/assets/3d6917ea-91d0-438a-b7d8-03c709620134">
<img width="872" alt="Screenshot 2024-11-04 at 7 00 07 PM" src="https://github.com/user-attachments/assets/b79e7b1f-b707-4b87-b6b6-0f0d63129aa4">
<img width="884" alt="Screenshot 2024-11-04 at 7 00 16 PM" src="https://github.com/user-attachments/assets/353f7d56-2e11-42fc-922d-869a77228f4c">
<img width="881" alt="Screenshot 2024-11-04 at 7 00 24 PM" src="https://github.com/user-attachments/assets/d1738a53-4cb9-4737-a8c2-adeb218ad060">
<img width="881" alt="Screenshot 2024-11-04 at 7 00 31 PM" src="https://github.com/user-attachments/assets/bf8da415-2876-4f0c-ae91-00105d63d0ef">
<img width="853" alt="Screenshot 2024-11-04 at 7 00 38 PM" src="https://github.com/user-attachments/assets/a4e0d522-6639-4478-a64d-c1b8a0062ad3">
<img width="870" alt="Screenshot 2024-11-04 at 7 00 46 PM" src="https://github.com/user-attachments/assets/fc4dc388-6c80-4f63-a241-dabbe693a9bc">
<img width="855" alt="Screenshot 2024-11-04 at 7 00 55 PM" src="https://github.com/user-attachments/assets/f8b985a3-015b-4c33-a3f2-90933e905f65">
<img width="878" alt="Screenshot 2024-11-04 at 7 01 04 PM" src="https://github.com/user-attachments/assets/ab0e8ecd-fc06-4d17-be1a-27b364e290ab">
<img width="868" alt="Screenshot 2024-11-04 at 7 01 14 PM" src="https://github.com/user-attachments/assets/20318b21-b395-4801-ad51-aeaf538b3102">
<img width="866" alt="Screenshot 2024-11-04 at 7 01 22 PM" src="https://github.com/user-attachments/assets/5dba6f0d-fc21-430f-aa64-5c4dbe675853">
<img width="921" alt="Screenshot 2024-11-04 at 7 01 34 PM" src="https://github.com/user-attachments/assets/03853d40-8cf7-421c-b59f-fff8efae4d08">

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
Due to issues with Grace and the queue, I could not get all 280 Cali files. I was able to get data for all the random sorted 2^16 2^18 2^20 and 2^22. For the other types of data and sizes, the jobs were stuck in the queue or had network issues, so I will fix the issues and finish them before we present. 
