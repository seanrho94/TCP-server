# TCP-server
First of all, open the configuration file by using fopen() and read its contents by fread() to obtain IP address and port number in byte order. Then create sockaddr_in structure to handle the IP address and port number that was obtained from the configuration file.

Then create socket with a socket() call. After creating the socket, assign the IP and port number specified in the sockaddr_in structure using bind() call. Use listen() call to put the socket in a passive mode to ensure that it waits for a connect() request from clients. After the call to listen(), the socket becomes a fully functional listening socket. 
To establish a connection between server and client, we use accept() call to get the  first connection request from the queue and return a file descriptor that represents the client socket. At this stage, server and client can communicate each other. For an instance, client can send an echo request to the server. 

Then the server can receive the request from the client socket using read() function and send response using write() function. Note that accept() is running in an infinite loop to make sure that server is always live to accept connection request from clients.

Fork() is used in order to handle multiple connections. The parent process begins by calling accept() to establish a connection between client and create a child process to pass the client socket file descriptor. Then the child process can receive requests from the client or send responds to the client using the client socket file descriptor that was passed from the parent process. 

For an instance, let’s say there are client 1, client 2 and a server. When the client 1 and 2 are connected to the server, the parent process will create two child processes. Then each child process can handle the request from the corresponding client.
