#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <inttypes.h>
#include <signal.h>
#define SERVER_PORT 3005
#define LOCAL_IP "10.0.2.4"

// #define DEBUG

#ifdef DEBUG
#define LOG printf
#endif

#ifndef DEBUG
#define LOG(fmt,...) 0
#endif

// Function to calculate the factorial of a number (up to 20)
uint64_t fact(uint64_t n) {
    uint64_t result = 1;
    for (uint64_t i = 1; i <= n && i <= 20; i++) {
        result *= i;
    }
    return result;
}

int handle_client(int client_socket) {
    uint64_t n;
    ssize_t bytes_received;
    uint64_t factorial;

    // Read the payload (64-bit unsigned integer) from the client
    bytes_received = recv(client_socket, &n, sizeof(n), 0);
    LOG("recived %ld\n",n);
    if (bytes_received < 0) {
        if(errno == ECONNRESET){
            return 0;
        }
        char ebuf[256];
        if(strerror_r(errno,ebuf,sizeof(ebuf))== ERANGE){
            printf("Too large error msg, errno %d\n",errno);
        }else{
            printf("recv from client failed %s\n",ebuf);
        }
        return 0;
    }
    if(bytes_received == 0){
        //connection closed by the client
        return 0;
    }

    // Calculate the factorial (up to 20) of the received number
    factorial = fact(n);

    // Send the factorial result back to the client
    if (send(client_socket, &factorial, sizeof(factorial), 0) < 0) {
        if(errno == ECONNRESET){
            return 0;
        }
       char ebuf[256];
        if(strerror_r(errno,ebuf,sizeof(ebuf)) == ERANGE ){
            printf("Too large error msg, errno %d\n",errno);
        }else{
            printf("could not send to client %s, errno %d\n",ebuf,errno);
        }
        return 0;
    }
    LOG("sent %ld\n",factorial);
    return 1;
}

int main() {
   int server_socket_fd = -1;

   struct sockaddr_in server_addr;
   server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

   if(server_socket_fd < 0){
        char ebuf[256];
        if(strerror_r(errno,ebuf,sizeof(ebuf) == ERANGE)){
            printf("Too large error msg, errno %d\n",errno);
        }else{
            printf("Socket creation failed %s\n",ebuf);
        }
        exit(EXIT_FAILURE);
   }
   printf("Socket created\n");
   memset(&server_addr,0,sizeof(server_addr));
   
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(SERVER_PORT);
   server_addr.sin_addr.s_addr = inet_addr(LOCAL_IP);

    if(bind(server_socket_fd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
        char ebuf[256];
        if(strerror_r(errno,ebuf,sizeof(ebuf)) == ERANGE ){
            printf("Too large error msg, errno %d \n",errno);
        }else{
            printf("Socket bind failed %s \n",ebuf);
        }
        exit(EXIT_FAILURE);
    }
    
    printf("Socket Binded to port %d\n",SERVER_PORT);


    if(listen(server_socket_fd,4000) < 0){
        char ebuf[256];
        if(strerror_r(errno,ebuf,sizeof(ebuf)) == ERANGE){
            printf("Too large error msg, errno %d \n",errno);
        }else{
            printf("Listen failed %s \n",ebuf);
        }
        exit(EXIT_FAILURE);
    }
    printf("Server listenting on port %d\n",SERVER_PORT);


    int client_socket_fd = -1;
    while(1){
        struct sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);

        client_socket_fd = accept(server_socket_fd,(struct sockaddr *)&client_addr,&addr_size);

        if(client_socket_fd < 0){
            char ebuf[256];
            if(strerror_r(errno,ebuf,sizeof(ebuf)) == ERANGE){
                printf("Too large error msg, errno %d \n",errno);
            }else{
                printf("Failed to accept a new client %s \n",ebuf);
            }
            exit(EXIT_FAILURE);
        }

        LOG("New connectection from %s:%d\n",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        int client_handler_pid = fork();
        if(client_handler_pid == 0){
            close(server_socket_fd);
            while(handle_client(client_socket_fd) == 1);
            close(client_socket_fd);
            exit(EXIT_SUCCESS);
        }

    }

}
