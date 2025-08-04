#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define CLUSTERS_NO 4

// Only used by leaders
#define send_topology_to_workers \
    for (int i = 0; i < workers_no; i++) { \
        MPI_Send(topology, 4 * numtasks, MPI_INT, workers[i], 0, MPI_COMM_WORLD); \
        printf("M(%d,%d)\n", rank, workers[i]); \
    } \

#define print_topology \
    printf("%d -> ", rank); \
    for (int i = 0; i < CLUSTERS_NO; i++) { \
        printf("%d:", i); \
        int list_start = 1; \
        for (int j = 0; j < numtasks; j++) { \
            if (topology[i][j]) { \
                if (list_start) { \
                    printf("%d", j); \
                    list_start = 0; \
                } else { \
                    printf(",%d", j); \
                } \
            } \
        } \
        if (i < CLUSTERS_NO - 1) { \
            printf(" "); \
        } \
    } \
    printf("\n"); \

#define execute_vector_calculations \
    /* Only workers do the calculations, so we exclude the leaders 
    when computing start and end */ \
    int start = (rank - CLUSTERS_NO) * (double)N / (numtasks - CLUSTERS_NO); \
    int end = min(((rank - CLUSTERS_NO) + 1) * (double)N / (numtasks - CLUSTERS_NO), N); \
    int received_V[N]; \
    \
    /* Receive vector from leader */ \
    MPI_Recv(received_V, N, MPI_INT, my_leader, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); \
    for (int i = start; i < end; i++) { \
        received_V[i] *= 5; \
    } \
    \
    /* We also send start and end so that the leader knows which chunk of
    its vector to overwrite */ \
    MPI_Send(&start, 1, MPI_INT, my_leader, 0, MPI_COMM_WORLD); \
    printf("M(%d,%d)\n", rank, my_leader); \
    MPI_Send(&end, 1, MPI_INT, my_leader, 0, MPI_COMM_WORLD); \
    printf("M(%d,%d)\n", rank, my_leader); \
    MPI_Send(received_V + start, end - start, MPI_INT, my_leader, 0, MPI_COMM_WORLD); \
    printf("M(%d,%d)\n", rank, my_leader); \

// node = source/destination
// offset = which line of the topology matrix to start the send/recv with
// k = how many lines to send/recv
#define send_topology(node, offset, k) \
    MPI_Send(topology[offset], k * numtasks, MPI_INT, node, 0, MPI_COMM_WORLD); \
    printf("M(%d,%d)\n", rank, node); \

#define recv_topology(node, offset, k) \
    MPI_Recv(topology[offset], k * numtasks, MPI_INT, node, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); \


// Receives the vector from previous leader. Sends it to
// the cluster and receive it modified by the workers. 
// Finally, pass it to the next leader.
#define recv_and_send_vector(source, destination) \
    int received_V[N], start, end; \
    MPI_Recv(received_V, N, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); \
    \
    for (int i = 0; i < workers_no; i++) { \
        MPI_Send(received_V, N, MPI_INT, workers[i], 0, MPI_COMM_WORLD); \
        printf("M(%d,%d)\n", rank, workers[i]); \
        \
        MPI_Recv(&start, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); \
        MPI_Recv(&end, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); \
        \
        MPI_Recv(received_V + start, end - start, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); \
    } \
    MPI_Send(received_V, N, MPI_INT, destination, 0, MPI_COMM_WORLD); \
    printf("M(%d,%d)\n", rank, destination); \

// Used by leader with rank 0. Generates the initial
// vector and passes it to next leader. Finally,
// receives it entirely modified and prints it.
#define generate_and_pass_vector(next_leader, neighbor) \
    int V[N], start, end; \
    for (int K = 0; K < N; K++) { \
        V[K] = N - K - 1; \
    } \
    \
    /* Sends vector to the cluster and receives it modified
       by the workers */ \
    for (int i = 0; i < workers_no; i++) { \
        MPI_Send(V, N, MPI_INT, workers[i], 0, MPI_COMM_WORLD); \
        printf("M(%d,%d)\n", rank, workers[i]); \
        \
        MPI_Recv(&start, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); \
        MPI_Recv(&end, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); \
        \
        MPI_Recv(V + start, end - start, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); \
    } \
    \
    /* Passes the vector to the next leader */ \
    MPI_Send(V, N, MPI_INT, next_leader, 0, MPI_COMM_WORLD); \
    printf("M(%d,%d)\n", rank, next_leader); \
    \
    /* Receives the final vector and prints it */ \
    MPI_Recv(V, N, MPI_INT, neighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); \
    printf("Rezultat: "); \
    for (int i = 0; i < N; i++) { \
        if (i < N - 1) { \
            printf("%d ", V[i]); \
        } else { \
            printf("%d\n", V[i]); \
        } \
    } \

FILE *read_cluster(int rank) {
    if (rank == 0) {
        return fopen("cluster0.txt", "r");
    } else if (rank == 1) {
        return fopen("cluster1.txt", "r");
    } else if (rank == 2) {
        return fopen("cluster2.txt", "r");
    } else if (rank == 3) {
        return fopen("cluster3.txt", "r");
    }
    return NULL;
}

int min(int a, int b) {
    return a < b ? a : b;
}

#endif