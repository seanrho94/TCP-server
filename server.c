#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdint.h>

// Read configuration file to get IP, port and path of the target directory.
void read_config_file (char* argv, uint32_t *ip, uint16_t *port){
    FILE *fptr;

    fptr = fopen(argv,"rb");

    if(fptr == NULL) {
        printf("Unable to open the file\n");
    }

    uint32_t return_ip;    
    uint16_t return_port;
  
    fread(&return_ip, sizeof(return_ip), 1, fptr);
    fread(&return_port, sizeof(return_port), 1, fptr);
  
    *ip = return_ip;
    *port = return_port;

    fclose(fptr); 
}

// Check if the request is equal to possible request types.
// return 1 if the request is a valid type, otherwise return 0.
int is_client_request_type (uint8_t *request_type) {
    // Possible requests that client can send.
    uint8_t list_client_requests[5] = {0x00, 0x02, 0x04, 0x06, 0x08};
    
    int check_client_list = 1;

    for (int i = 0; i < 5; i++) {
        if(*request_type != list_client_requests[i]) {
            check_client_list = 0;
        }else {
            check_client_list = 1;
            break;
        }
    }

    if (check_client_list == 1) {
        return 1;
    }

    return 0;
}

// Check if the request is equal to possible client respond types.
// return 1 if the request is equal to any client respond types, otherwise return 0.
int is_server_request_type (uint8_t *request_type) {
    // Possible responses that server can send.
    uint8_t list_server_requests[4] = {0x01, 0x03, 0x05, 0x07};

    int check_server_list = 1;

    for (int i = 0; i < 4; i++) {
        if(*request_type != list_server_requests[i]) {
            check_server_list = 0;
        }else {
            check_server_list = 1;
            break;
        }
    }

    if(check_server_list == 1) {
        return 1;
    }

    return 0;
}

int main(int argc, char** argv) {
    uint32_t ip;
    uint16_t port;
    
    // Read configuration file to get IP, port and path of the target directory.
    read_config_file(argv[1], &ip, &port); 
   
    int server_socket_fd = -1;
    int client_socket_fd = -1;

    struct sockaddr_in address;

    // Assign IP and pport number.
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = ip;
    address.sin_port = port;

    // initialise server socket
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_socket_fd < 0){
        printf("Fail to initialise the socket!\n");
        exit(1);
    }

    if(bind(server_socket_fd, (struct sockaddr*) &address, sizeof(struct sockaddr_in))) {
        printf("Fail to bind!\n");
        exit(1);
    }

    listen(server_socket_fd, 4);
    while(1) {
        uint32_t addrlen = sizeof(struct sockaddr_in);
        client_socket_fd = accept(server_socket_fd, (struct sockaddr*) &address, &addrlen);
        pid_t p = fork();

        if (p == 0) {
            while(1){
                uint8_t error_response[9] = { 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                uint8_t request_type;
                uint8_t payload_len_nb[8]; // Payload length in network byte order.
                uint64_t payload_len = 0;
                uint8_t payload;
          
                read(client_socket_fd, &request_type, 1);
                read(client_socket_fd, payload_len_nb, 8);

                // Convert the playload lenth from byte order to long by using bitwise operation.
                for (int i = 0; i < 8; i++) {
                    payload_len += (payload_len_nb[i] << (56 - (i * 8)));
                }

                // Two flags to check if the request is valid.
                int is_valid_client_req_t = is_client_request_type(&request_type);
                int is_valid_server_req_t = is_server_request_type(&request_type);
          
                // If the request  type is not valid OR it is equal to server respond types, send back error response.
                if (is_valid_client_req_t == 0 || is_valid_server_req_t == 1) {
                    write(client_socket_fd, error_response, 9);
                    // Close connection after responding.
                    break;
                }

                // If the request type is echo
                if (request_type == 0x00) {
                    // The message header that the server will respond.
                    uint8_t response_type = 0x10;

                    // Send 0x10 as message header (1byte).
                    write(client_socket_fd, &response_type, 1);

                    // Send payload lenth (8byte).
                    write(client_socket_fd, payload_len_nb, 8);
                    
                    // Send payload (variable byte).
                    for (int i = 0; i < payload_len; i++) {
                        read(client_socket_fd, &payload, 1);
                        write(client_socket_fd, &payload, 1);
                    }
                }
            }

            close(client_socket_fd);
            exit(0);
        }
        close(client_socket_fd);        
    }
    close(server_socket_fd);


    return 0;

}