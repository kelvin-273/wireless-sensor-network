#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>
#include "array_of_keys.h"

#ifndef BASE_STATION
#define BASE_STATION 60
#endif

#ifndef NULL_KEY
#define NULL_KEY (key) {-1,-1,-1}
#endif

void init_rand(int rank, int numtasks) {
    // root takes the time as the seed
    // and generates random seeds for the other processes
    if (rank == BASE_STATION) {
        srand(time(NULL));
        for (int dest = 0; dest < BASE_STATION; dest++) {
            int seed_to_send = rand();
            MPI_Send(&seed_to_send, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }
    } else {
        int seed;
        MPI_Recv(&seed, 1, MPI_INT, BASE_STATION, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        srand(seed);
    }
}

void pass(int dest, int *send_list, int send_len, MPI_Comm comm, int rank) {
    if (dest != -1) {
        MPI_Request req;
        MPI_Isend(send_list, send_len, MPI_INT, dest, 0, comm, &req);
        MPI_Wait(&req, MPI_STATUS_IGNORE);
    }
}

void catch(int source, LN *array, MPI_Comm comm, int rank) {
    if (source != -1) {
        MPI_Status status;
        int *recv_buff, recv_len;
        MPI_Probe(source, 0, comm, &status);
        MPI_Get_count(&status, MPI_INT, &recv_len);
        recv_buff = malloc(recv_len * sizeof(int));
        MPI_Recv(recv_buff, recv_len, MPI_INT, source, 0, comm, MPI_STATUS_IGNORE);
        for (int i = 0; i < recv_len / 3; i++) {
            append(array, recv_buff + 3*i);
        }
        free(recv_buff);
    }
}

int main(int argc, char *argv[]) {
    int rank, numtasks, reading;
    int dim[2], period[2], reorder;
    key own_key;
    int coords[2];

    // Initialise the environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // Checks number of processes
    if (numtasks != 61) {
        if (rank == 0) {
            printf("Requires 61 processes\n");
        }
        MPI_Finalize();
        return 1;
    }

    // creates the cartesian grid with the first 60 nodes
    dim[0] = 4; dim[1] = 15;
    period[0] = 0; period[1] = 0;
    reorder = 0;

    MPI_Comm wsn;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &wsn);
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank != BASE_STATION){
        MPI_Cart_coords(wsn, rank, numtasks, coords);
        own_key[1] = coords[0];
        own_key[2] = coords[1];
    }

    // sets the seed for each process
    init_rand(rank, numtasks);
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank != BASE_STATION) {
        while (true) {
            own_key[0] = rand() % 2;
            // ++ printf("p-%d\tkey-%s\n", rank, showN(own_key));
            LN *array = init_array();
            append(array, own_key);

            int left, right, up, down;
            MPI_Cart_shift(wsn, 1, 1, &left, &right);
            MPI_Cart_shift(wsn, 0, 1, &up, &down);

            // first round of key sharing
            // write info from dynamic array to static send_list to be sent
            int *send_list, send_len;
            send_len = 3 * array->len;
            send_list = (int *) malloc(sizeof(int) * send_len);
            for (int i = 0; i < array->len; i++) {
                copyN(array->list[i], send_list + 3*i); // pointer arithmetic
            }
            // here the nodes pair off into groups of 2 sharing their list of
            // nodes with each other. first parings form vertically.
            if (coords[0] % 2 == 0) {
                pass(down, send_list, send_len, wsn, rank);
                catch(down, array, wsn, rank);
                pass(up, send_list, send_len, wsn, rank);
                catch(up, array, wsn, rank);
            } else {
                catch(up, array, wsn, rank);
                pass(up, send_list, send_len, wsn, rank);
                catch(down, array, wsn, rank);
                pass(down, send_list, send_len, wsn, rank);
            }
            // and then horizontally
            if (coords[1] % 2 == 0) {
                pass(right, send_list, send_len, wsn, rank);
                catch(right, array, wsn, rank);
                pass(left, send_list, send_len, wsn, rank);
                catch(left, array, wsn, rank);
            } else {
                catch(left, array, wsn, rank);
                pass(left, send_list, send_len, wsn, rank);
                catch(right, array, wsn, rank);
                pass(right, send_list, send_len, wsn, rank);
            }

            // The send list is rebuilt for the next round of sending.
            // Each key has a list of neighbour keys with readings and coords.
            send_len = 3 * array->len;
            send_list = (int *) realloc(send_list, sizeof(int) * send_len);
            for (int i = 0; i < array->len; i++) {
                copyN(array->list[i], send_list + 3*i); // pointer arithmetic
            }

            // second round of key sharing
            // this info gets passed to neighbours => nodes have info of
            // neighbours and neighbours of neighbours
            if (coords[0] % 2 == 0) {
                pass(down, send_list, send_len, wsn, rank);
                catch(down, array, wsn, rank);
                pass(up, send_list, send_len, wsn, rank);
                catch(up, array, wsn, rank);
            } else {
                catch(up, array, wsn, rank);
                pass(up, send_list, send_len, wsn, rank);
                catch(down, array, wsn, rank);
                pass(down, send_list, send_len, wsn, rank);
            }
            if (coords[1] % 2 == 0) {
                pass(right, send_list, send_len, wsn, rank);
                catch(right, array, wsn, rank);
                pass(left, send_list, send_len, wsn, rank);
                catch(left, array, wsn, rank);
            } else {
                catch(left, array, wsn, rank);
                pass(left, send_list, send_len, wsn, rank);
                catch(right, array, wsn, rank);
                pass(right, send_list, send_len, wsn, rank);
            }

            // sort and uniq
            quicksort(own_key, array->list, array->len);
            uniq(array);
            // check if event has occured and send to BASE_STATION if so
            if (event_verify(array)) {
                MPI_Send(own_key, 3, MPI_INT, BASE_STATION, 0, MPI_COMM_WORLD);
                // ++ printLN(array);
            }
            free(array);
            free(send_list);
            MPI_Barrier(wsn);
            if (rank == 0) {
                MPI_Send(NULL_KEY, 3, MPI_INT, BASE_STATION, 0, MPI_COMM_WORLD);
            }
            MPI_Barrier(wsn);
            // ++ break;
        }
    } else {
        double start_time = MPI_Wtime();
        FILE *fp;
        fp = fopen("output.csv", "w");
        fprintf(fp, "elapsed_time,cycle_no,node_rank,node_key\n");
        fclose(fp);
        int counter = 0;
        while (true) {

            fp = fopen ("output.csv","a");
            key recv_buff;
            MPI_Recv(recv_buff, 3, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            double recv_time = MPI_Wtime();
            int recv_rank = 15 * recv_buff[1] + recv_buff[2];

            if (eqN(recv_buff, NULL_KEY)) {
                counter++;
            } else if (fp!=NULL) {
                fprintf(fp, "%f,%d,%d,\"%s\"\n", recv_time - start_time,
                            counter, recv_rank, showN(recv_buff));
            }
            fclose (fp);
            // ++ break;
        }
    }

    MPI_Finalize();
    return 0;
}
