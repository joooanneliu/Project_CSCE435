#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <caliper/cali.h>
#include <adiak.hpp>

bool isSorted(const std::vector<int>& vec) {
    CALI_CXX_MARK_FUNCTION;
    for (size_t i = 1; i < vec.size(); ++i) {
        if (vec[i] < vec[i - 1]) {
            return false;
        }
    }
    return true;
}

int main(int argc, char** argv) {
    CALI_CXX_MARK_FUNCTION;
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    std::vector<int> data;         // Holds the original data at root process
    std::vector<int> local_data;   // Holds each process's local chunk of data
    int exponent = atoi(argv[1]);
    int total_data_size = static_cast<int>(pow(2, exponent));     // Total size of data
    int local_size = total_data_size / size; // Size of data each process will handle
    
    if (rank == 0){
        std::cout << "SIZE: " << total_data_size << std::endl;
    }
    
    // Data Initialization
    CALI_MARK_BEGIN("data_init_runtime");
    if (rank == 0) {
        data.resize(total_data_size);
        std::srand(std::time(nullptr));
        for (int i = 0; i < total_data_size; ++i) {
            // data[i] = i; // sorted
            data[i] = std::rand() % 1000; // Random integers between 0 and 999
            // data[i] = total_data_size - i; // reverse sorted
        }
        // int swaps = std::max(1, n / 100); // 1% perturbed
        // for (int i = 0; i < swaps; ++i) {
        //     int idx1 = std::rand() % n;
        //     int idx2 = std::rand() % n;
        //     while (idx1 == idx2) {
        //         idx2 = std::rand() % n;
        //     }
        //     std::swap(data[idx1], data[idx2]);
        // }
    }
    CALI_MARK_END("data_init_runtime");

    // Resize local_data to accommodate the portion of data each process will work on
    local_data.resize(local_size);

    // Scatter data to all processes
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Scatter(data.data(), local_size, MPI_INT, local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    // Step 1: Each process locally sorts its data
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    std::sort(local_data.begin(), local_data.end());
    CALI_MARK_END("comp_large");
    

    // Step 2: Select `P - 1` evenly spaced samples from each sorted local data
    CALI_MARK_BEGIN("comp_small");
    int s = size - 1;
    std::vector<int> local_samples(s);
    for (int i = 0; i < s; ++i) {
        local_samples[i] = local_data[i * local_size / s];
    }
    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

    // Step 3: Gather all local samples at the root process
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");
    std::vector<int> gathered_samples;
    if (rank == 0) {
        gathered_samples.resize(size * s);
    }
    MPI_Gather(local_samples.data(), s, MPI_INT, gathered_samples.data(), s, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    // Step 4: Root process selects pivots and broadcasts them
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    std::vector<int> pivots(s);
    if (rank == 0) {
        std::sort(gathered_samples.begin(), gathered_samples.end());

        // Select P - 1 pivots from the gathered samples
        for (int i = 0; i < s; ++i) {
            pivots[i] = gathered_samples[(i + 1) * gathered_samples.size() / size];
        }
    }
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");
    MPI_Bcast(pivots.data(), s, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    // Step 5: Partition local data according to the pivots
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    std::vector<std::vector<int>> partitions(size);
    for (int i = 0; i < local_size; ++i) {
        int partition = 0;
        while (partition < s && local_data[i] > pivots[partition]) {
            ++partition;
        }
        partitions[partition].push_back(local_data[i]);
    }
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    // Step 6: All-to-All exchange of partitions
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    std::vector<int> send_counts(size), recv_counts(size);
    for (int i = 0; i < size; ++i) {
        send_counts[i] = partitions[i].size();
    }

    MPI_Alltoall(send_counts.data(), 1, MPI_INT, recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

    std::vector<int> send_buffer, recv_buffer;
    for (int i = 0; i < size; ++i) {
        send_buffer.insert(send_buffer.end(), partitions[i].begin(), partitions[i].end());
    }

    int total_recv = 0;
    std::vector<int> recv_displs(size), send_displs(size);
    for (int i = 0; i < size; ++i) {
        recv_displs[i] = total_recv;
        total_recv += recv_counts[i];
    }

    send_displs[0] = 0;
    for (int i = 1; i < size; ++i) {
        send_displs[i] = send_displs[i - 1] + send_counts[i - 1];
    }

    recv_buffer.resize(total_recv);
    MPI_Alltoallv(send_buffer.data(), send_counts.data(), send_displs.data(), MPI_INT,
                  recv_buffer.data(), recv_counts.data(), recv_displs.data(), MPI_INT, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    // Step 7: Local sort of the received data
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    std::sort(recv_buffer.begin(), recv_buffer.end());
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    // Step 8: Gather sorted data at root
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    std::vector<int> final_data;
    std::vector<int> final_recv_counts(size);
    std::vector<int> final_recv_displs(size);

    MPI_Gather(&total_recv, 1, MPI_INT, final_recv_counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int total_data_received = 0;
        for (int i = 0; i < size; ++i) {
            final_recv_displs[i] = total_data_received;
            total_data_received += final_recv_counts[i];
        }
        final_data.resize(total_data_received);
    }

    MPI_Gatherv(recv_buffer.data(), total_recv, MPI_INT, final_data.data(), final_recv_counts.data(),
                final_recv_displs.data(), MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    // Step 9: Correctness check
    
    if (rank == 0) {
        CALI_MARK_BEGIN("correctness_check");
        bool valid = isSorted(final_data);
        CALI_MARK_END("correctness_check");

        if (valid) {
            std::cout << "The data is sorted correctly!" << std::endl;
        } else {
            std::cout << "The data is NOT sorted correctly." << std::endl;
        }
    }
    
    
    adiak::init(NULL);
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();


    std::string algorithm = "sample_sort";
    std::string programming_model = "mpi";
    std::string data_type = "int";
    int size_of_data_type = sizeof(int);
    int input_size = total_data_size;
    std::string input_type = "random";  
    int num_procs = size;
    std::string scalability = "strong";  
    int group_number = 4;  
    std::string implementation_source = "handwritten, online"; 

    adiak::value("algorithm", algorithm);
    adiak::value("programming_model", programming_model);
    adiak::value("data_type", data_type);
    adiak::value("size_of_data_type", size_of_data_type);
    adiak::value("input_size", input_size);
    adiak::value("input_type", input_type);
    adiak::value("num_procs", num_procs);
    adiak::value("scalability", scalability);
    adiak::value("group_num", group_number);
    adiak::value("implementation_source", implementation_source);

    MPI_Finalize();
    return 0;
}
