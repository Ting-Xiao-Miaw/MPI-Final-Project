#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>
#include "socket.h"
int main(int argc, char ** argv){
        int rank, size;
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        int N = 7, K = 4;
        int R = N - K;
        int *H;
        int *Num_Matrix;
        int *Data;
        FILE *fptr;
        char *filename = "rev_data.txt";
        if(rank == 0){
                /* File Pointer */
                fptr = fopen(filename, "w");
                if( fptr == NULL ){
                    printf("Can't open file.");
                    exit(1);
                }
                // Input a H matrix
                printf("H matrix:\n");
                H = malloc(sizeof(int)*(R*N));
                for(int i=0; i<N; i++){
                        for(int j=0; j<R; j++){
                                scanf("%d", &H[i*R+j]);
                        }
                }
                // Socket Recv
                int sockfd = bindSocket();
                Receive(Num_Matrix, sockfd, 1);
                Data = malloc(sizeof(int)*(N*K)*(*Num_Matrix));
                for(int t = 0; t < *Num_Matrix; t++){
                    for(int i=0; i<K; i++){
                            for(int j=0; j<N; j++){
                                Receive(&Data[t*28+i*N+j], sockfd, 1);
                            }
                    }
                }
        }
        if(rank!=0){
            H = malloc(sizeof(int) * (R*N));
        }
        // Broadcast the H matrix
        MPI_Bcast(H, R*N, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(Num_Matrix, 1, MPI_INT, 0, MPI_COMM_WORLD);
        for(int t = 0; t < *Num_Matrix; t++){
            int *row;
            int *dec_row;
            row = malloc(sizeof(int) * N);
            MPI_Scatter(&Data[t*28], N, MPI_INT, row, N, MPI_INT, 0, MPI_COMM_WORLD);
            // Each worker decodes the row to the new one
            dec_row = malloc(sizeof(int) * R);
            for(int i = 0; i < R; i++){
                dec_row[i] = 0;
                for(int j = 0; j < N; j++)
                    dec_row[i] += row[j] * H[j*R+i];
                dec_row[i] = dec_row[i]%2;
            }
            // Compare Syndrome, do Error Correction
            for(int i = 0; i < N; i++){
                int cmp = memcmp(dec_row, &H[i*R], sizeof(int)*R);
                if(cmp == 0){
                    row[i] = (row[i] + 1)%2;
                }
            }
            // Gather decoded rows to the manager, get an matrix DD
            int *DD;
            DD = malloc(sizeof(int)*(N*K));
            MPI_Gather(row, N*(K/size), MPI_INT, DD, N*(K/size), MPI_INT, 0, MPI_COMM_WORLD);
            if(rank == 0){
                // Extract the original data D
                int *D;
                D = malloc(sizeof(int)*(K*K));
                for(int i=0; i<K; i++){
                    for(int j=R; j<N; j++){
                        D[i*K+j-R] = DD[i*N+j];
                    }
                }
                /* For loop for read 8-bit, write it to the txt file in char mode */
                for( int i = 0; i < 2; i++ ){
                    /* Binary to ASCII int */
                    int r = 0;
                    for( int j = 0; j < 8; j++ ){
                        /* p, caculate pow(2,7-j) */
                        int p = 1;
                        for(int k = 0; k < 7-j; k++ ){
                            p = p*2;
                        }
                        r += D[j+i*8] * p;
                    }
                    fprintf( fptr, "%c", r );
                } 
            }
        }
        if(rank == 0){
            fclose(fptr);
            printf("File '%s' created.\n", filename);
        }
        MPI_Finalize();
}
