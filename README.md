# Introduction

## Features & Components

This homework creates a multithreaded client-server file transfer
system. The homework has the following components:

-   A server that remains open despite the client disconnecting. The
    server will continue to accept connections and transfer requested
    files.

-   The server will be able to clients simultaneously, but this process
    is not multithreaded.

-   A client that connects to the server, requests a file, and then
    disconnects from the server upon succesful or unsuccessful file
    transfer.

-   The filetransfer protocol is multithreaded and uses a mutex to
    ensure that only one thread is accessing the file at a time.

-   The filetransfer makes sure the empty characters are not being
    written and the new file created is the same size as the original
    file.

-   The codebase contains 4 files:

    -   `server.c`

    -   `client.c`

    -   `helpers.h`

    -   `makefile`

## Usage

To compile the code, run the following command in the terminal:

         make

To run the server, type:

        ./server <ip> <port>

To run the client, type:

        ./client <ip> <port> <requested filename> <save as filename>

For example, to run the server and client on the same local machine
(`127.0.0.1`), on port `8080`, and to request a file named `test.txt`
and save it as `testNew.txt`, type:

       ./server 127.0.0.1 8080 (Terminal 1)
       ./client 127.0.0.1 8080 test.txt testNew.txt (Terminal 2)

## Functions

In the `server.c` file, there are the following functions:

        int sendFile(int socket);
        * Sends file to client in a single thread
        * @param socket - socket to send file to
        * @return 1 if file send, 0 if file not sent

        void *threadedFileRead(void *threadarg);
        * Reads and Sends a certain chunk of a file
        * @param *threadarg - struct containing fp, socket, buffer, s, f, fSize
        * @return void*

        int threadedSendFile(int socket);
        * Sends file to client via multiple threads
        * @param socket - socket to send file to
        * @return 1 if file send, 0 if file not sent

In the `client.c` file, there are the following functions, and have the
following functionalities.

        int recieveFile(int socket, char *filename);
        * Recieves file from server in a single thread
        * @param socket - socket to recieve file from
        * @param filename - name of file to recieve
        * @return 1 if file recieved, 0 if file not recieved
        
        void *threadedFileWrite(void *threadarg);
        * Recieves and Writes a certain chunk to a file
        * @param *threadarg - struct containing fp, socket, buffer, s, f, fSize
        * @return void*
        
        int threadedRecieveFile(int socket, char *filename);
        * Recieves file from server via multiple threads
        * @param socket - socket to recieve file from
        * @param filename - name of file to recieve
        * @return 1 if file recieved, 0 if file not recieved

In the `helpers.h` file, there are the following functions:

        struct thread_data;
        * Defines a struct for sending to individual threads
        
        void error(char *msg);
        * Prints an emboldend red error message to terminal, e.g. [-]Error:<msg>
        
        void success(char *msg);
        * Prints an emboldend green success message to terminal, e.g. [+]Success:<msg>
        
        void wait(char *msg);
        * Prints an emboldend yellow waiting message to terminal, e.g. [*]Wait:<msg>...
        
        void reply(char *msg);
        * Prints server messages on client-side, e.g. [S]:<msg>
