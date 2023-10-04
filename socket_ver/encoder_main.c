#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include"socket.h"
#define SERVERADDR "10.98.1.230"
#define MATRIX_SIZE 4

void decimalToBinary(int num, int* onebyte) {
    if (num == 0) {
        for(int i = 0; i < 8; i++)
            onebyte[i] = 0;
        return;
    }
   // Stores binary representation of number.
   int binaryNum[8]; 
   int i=0;

   for ( ;num > 0; ){
      binaryNum[i++] = num % 2;
      num /= 2;
   }
   int b = 0;
   for (int z = 0;z < 8 - i; z++)
      onebyte[b++] = 0;
   for (int j = i-1; j >= 0; j--){
      onebyte[b++] = binaryNum[j];
   }
}

int main(int argc, char ** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int N = 7, K = 4;
    int *G;
    int *Data;
    int *Num_Matrix;
    Num_Matrix = malloc(sizeof(int));
    FILE* file;
    char *filename = "data.txt";
    char address[] = SERVERADDR;
    if(rank == 0){
        // Input a G matrix
        G = malloc(sizeof(int)*(N*K));
        for(int i=0; i<K; i++){
            for(int j=0; j<N; j++){
                scanf("%d", &G[i*N+j]);
            }
        }
        // Open binary file, then read into the variable "data"
        file = fopen(filename, "rb");
        if (file == NULL) {
            printf("Cannot open the file.\n");
            return 1;
        }
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);
        unsigned char* data = (unsigned char*)malloc(file_size);
        fread(data, 1, file_size, file);
        fclose(file);
        // Slice data into 16-bit matrix
        int num_matrices = (file_size + 1)/2;
        Num_Matrix = &num_matrices;
        Data = malloc((K*K)* num_matrices * sizeof(int*));
        int** matrices = (int**)malloc(num_matrices * sizeof(int*));
        for (int i = 0; i < num_matrices; i++) {
            unsigned char matrix[MATRIX_SIZE][MATRIX_SIZE];
            int byte_index = i * 2;
            for (int row = 0; row < MATRIX_SIZE; row++) {
                for (int col = 0; col < MATRIX_SIZE; col++) {
                    int byte_value = data[byte_index + col + (row * MATRIX_SIZE)];
                    matrix[row][col] = byte_value;
                }
            }
            // Implicit covert matrix from char to int
            int* int_matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
            for (int row = 0; row < MATRIX_SIZE; row++) {
                for (int col = 0; col < MATRIX_SIZE; col++) {
                    int_matrix[row * MATRIX_SIZE + col] = matrix[row][col];
                }
            }
            matrices[i] = int_matrix;
            decimalToBinary(int_matrix[0], &Data[i*16]);
            decimalToBinary(int_matrix[1], &Data[i*16+8]);
        }
    }//rank 0
    if(rank != 0){
        G = malloc(sizeof(int) * (N*K));
    }
    // Broadcast the G matrix
    MPI_Bcast(G, N*K, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(Num_Matrix, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank == 0)
        Send(Num_Matrix, address, 1);
    for(int i = 0; i < *Num_Matrix; i++){
        int *row;
        int *enc_row;
        int *E;
        row = malloc(sizeof(int) * (K*K/size));
        MPI_Scatter(&Data[i*16], K*K/size, MPI_INT, row, K*K/size, MPI_INT, 0, MPI_COMM_WORLD);
        // Each worker encodes the row to the new one
        enc_row = malloc(sizeof(int) * N);
        for(int j = 0; j < N; j++){
            enc_row[j] = 0;
            for(int k = 0; k < K; k++)
                enc_row[j] += row[k] * G[k*N+j];
            enc_row[j] = enc_row[j]%2;
        }
        // Gather encoded rows to the manager, get an encoded matrix E
        E = malloc(sizeof(int)*(N*K));
        MPI_Gather(enc_row, N*(K/size), MPI_INT, E, N*(K/size), MPI_INT, 0, MPI_COMM_WORLD);
        if(rank == 0){
            // Send out each 16 bits data
            for(int i=0; i<K; i++){
                 for(int j=0; j<N; j++){
                     Send(&E[i*N+j], address, 1);
                 }
            }
        }
    }
    MPI_Finalize();
    return 0;
}
