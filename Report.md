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
### 2c. Evaluation plan - what and how will you measure and compare
We will keep a constant problem size while increasing the number of processors/nodes from [2, 4, 8, 16, 32, 64, 128] and then compare the MPI_Wtimes and Caliper times using Thicket.

We will also test by keeping the number of processors/nodes constant while increasing the problem size from [128, 16384, 1048576] with randomized values and then compare the MPI_Wtimes and Caliper times using Thicket.
