#include "utils.h"



int main(int argc, char *argv[]) {
    int numtasks, rank, workers_no, my_leader;
    int N = atoi(argv[1]), communication_error = atoi(argv[2]);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    FILE *in = read_cluster(rank);
    
    int *workers, topology[CLUSTERS_NO][numtasks];
    for (int i = 0; i < CLUSTERS_NO; i++) {
        for (int j = 0; j < numtasks; j++) {
            topology[i][j] = 0;
        }
    }

    // Build the initial topology matrix for the current leader
    if (rank <= 3) {
        fscanf(in, "%d\n", &workers_no);
        workers = (int *)malloc(workers_no * sizeof(int));
        for (int i = 0; i < workers_no; i++) {
            fscanf(in, "%d\n", &workers[i]);
            topology[rank][workers[i]] = 1;
        }

        // Send leader rank to workers
        for (int i = 0; i < workers_no; i++) {
            MPI_Send(&rank, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, workers[i]);
        }
    }

    // Topology establishment
    if (communication_error == 0) {
        if (rank == 0) {
            send_topology(1, 0, 1);
            recv_topology(3, 0, 4);

            send_topology(1, 0, 4);
        } else if (rank == 1) {
            recv_topology(0, 0, 1);
            send_topology(2, 0, 2);

            recv_topology(0, 0, 4);
            send_topology(2, 0, 4);
        } else if (rank == 2) {
            recv_topology(1, 0, 2);
            send_topology(3, 0, 3);

            recv_topology(1, 0, 4);
            send_topology(3, 0, 4);
        } else if (rank == 3) {
            recv_topology(2, 0, 3);
            send_topology(0, 0, 4);

            recv_topology(2, 0, 4);
        }
        send_topology_to_workers;
    } else if (communication_error == 1) {
        if (rank == 1) {
            send_topology(2, 1, 1);

            recv_topology(2, 0, 4);
        } else if (rank == 2) {
            recv_topology(1, 1, 1);
            send_topology(3, 1, 2);

            recv_topology(3, 0, 4);
            send_topology(1, 0, 4);
        } else if (rank == 3) {
            recv_topology(2, 1, 2);
            send_topology(0, 1, 3);

            recv_topology(0, 0, 4);
            send_topology(2, 0, 4);
        } else if (rank == 0) {
            recv_topology(3, 1, 3);
            send_topology(3, 0, 4);
        }
        send_topology_to_workers;
    }

    // Workers receive the topology from their leaders
    if (communication_error == 0 || communication_error == 1) {
        if (rank > 3) {
            // Find the leader of the current worker
            MPI_Recv(&my_leader, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            recv_topology(my_leader, 0, 4);
        }
    }

    print_topology;

    // Vector calculations
    if (communication_error == 0) {
        if (rank == 0) {
            generate_and_pass_vector(1, 3);
        } else if (rank == 1) {
            recv_and_send_vector(0, 2);
        } else if (rank == 2) {
            recv_and_send_vector(1, 3);
        } else if (rank == 3) {
            recv_and_send_vector(2, 0);
        }
    } else if (communication_error == 1) {
        if (rank == 0) {
            generate_and_pass_vector(3, 3);
        } else if (rank == 3) {
            int received_V[N], start, end;
            MPI_Recv(received_V, N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < workers_no; i++) {
                MPI_Send(received_V, N, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, workers[i]);

                MPI_Recv(&start, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&end, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                MPI_Recv(received_V + start, end - start, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            MPI_Send(received_V, N, MPI_INT, 2, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 2);

            MPI_Recv(received_V, N, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(received_V, N, MPI_INT, 0, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 0);
        } else if (rank == 2) {
            int received_V[N], start, end;
            MPI_Recv(received_V, N, MPI_INT, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < workers_no; i++) {
                MPI_Send(received_V, N, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, workers[i]);

                MPI_Recv(&start, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&end, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                MPI_Recv(received_V + start, end - start, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            MPI_Send(received_V, N, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 1);

            MPI_Recv(received_V, N, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(received_V, N, MPI_INT, 3, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 3);
        } else if (rank == 1) {
            int received_V[N], start, end;
            MPI_Recv(received_V, N, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 0; i < workers_no; i++) {
                MPI_Send(received_V, N, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, workers[i]);

                MPI_Recv(&start, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&end, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                MPI_Recv(received_V + start, end - start, MPI_INT, workers[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            MPI_Send(received_V, N, MPI_INT, 2, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 2);
        }
    }
    
    if (rank > 3 && (communication_error == 0 || communication_error == 1)) {
        execute_vector_calculations;
    }
    if (rank <= 3) {
        free(workers);
        fclose(in);
    }
    MPI_Finalize();
}