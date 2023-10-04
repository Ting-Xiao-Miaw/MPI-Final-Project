# MPI-Final-Project
Forward Error Correction in Practice
# For socket version
The port number for the transmission is defined in 'socket.h', and the receiver's address is defined in 'encoder_main.c'.
Note: You should run the Receiver/Decoder first to establish the socket tunnel.

1. Receiver/Decoder:
    >> mpirun -n 4 ./decoder_main.exe < H_matrix
    After receiving data from the sender, a new file named 'rev_data.txt' will be created in the same directory.

2. Sender/Encoder:
    >> mpirun -n 4 ./encoder_main.exe < G_matrix
    Encode the data from the file named 'data.txt' and then transmit it to the receiver.

---------------------------------------------------------------------------------------------------------------------
# For stdio version
This version is designed for testing error correction functionality. 
If you modify one bit in each row (7 bits), the Decoder will correct the data to the intended state.
1. Encoder:
    >> mpirun -n 4 ./encoder_main.exe < G_matrix
      It will output the encoded result.
    Or
    >> mpirun -n 4 ./encoder_main.exe < G_matrix > result
      Store the encoded result in the file 'data.txt'.

2. Decoder:
    >> mpirun -n 4 ./decoder_main.exe
      H matrix:
        <input H matrix>
      Encoded data:
        <input the output of encoder>
      File reverted(as 'rev_data.txt').
    Or
    >> cat H_matrix result | mpirun -n 4 ./decoder_main.exe 
