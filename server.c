#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#define PORT 5584
#define CONN_NUM 10

int thread_cntr = 0;
pthread_t thread_id[CONN_NUM];

void *connection_handler();

int main(){
    int socket_desc, new_socket, c, *new_sock;
    struct sockaddr_in server, client;
    char message[64];

    //Creating socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0){
        perror("[!]Socket");
        exit(1);
    };

    //Server params
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    //Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0){
        perror("[!]Bind");
        close(socket_desc);
        exit(2);
    }

    printf("Binding complete\n");

    //Listen
    listen(socket_desc, CONN_NUM);

    //Accept connections
    printf("Waiting for connections...\n");

    c = sizeof(struct sockaddr_in);

    while(new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)){

        char *client_ip = inet_ntoa(client.sin_addr);
        int client_port = ntohs(client.sin_port);

        printf("Connection from %s:%i accepted\n", client_ip, client_port);

        /* Malloc check added and corrected size */
        if ((new_sock = malloc(sizeof(*new_sock))) == NULL){
            perror("[!]Malloc");
            close(socket_desc);
            close(new_socket);
            for ( int i = thread_cntr; i > 0; i--){
                pthread_exit(&thread_id[i - 1]);
            }
            exit(5);           
        }
        *new_sock = new_socket;

        if (thread_cntr == CONN_NUM){
            for ( int i = thread_cntr; i > 0; i--){
                pthread_join(thread_id[i - 1], NULL);
            }
        };

        if (pthread_create(&thread_id[thread_cntr], NULL, connection_handler, (void*) new_sock) < 0){
            perror("[!]Thread");
            close(socket_desc);
            close(new_socket);
            for ( int i = thread_cntr; i > 0; i--){
                pthread_exit(&thread_id[i - 1]);
            }
            exit(4);
        }
        
        printf("Handler ready.\n");
        thread_cntr++;
    }

    if (new_socket < 0){
        perror("[!]Accept failed");
        close(socket_desc);
        for ( int i = thread_cntr; i > 0; i--){
            pthread_exit(&thread_id[i - 1]);
        }
        exit(3);
    }

    close(socket_desc);
    close(new_socket);
    for ( int i = thread_cntr; i > 0; i--){
        pthread_exit(&thread_id[i - 1]);
    }

    return 0;
}

void *connection_handler(void *socket_desc){
    //Get socker descriptor
    int sock = *(int*)socket_desc;
    int read_size, c;
    char client_message[64];
    struct sockaddr_in client;

    //Who connected to us?
    c = sizeof(struct sockaddr_in);
	getpeername(sock, (struct sockaddr *)&client, (socklen_t*)&c);
    char *client_ip = inet_ntoa(client.sin_addr);
    int client_port = ntohs(client.sin_port);

	//Receive a message from client
	while((read_size = read(sock, client_message, 64)) > 0 ){
        printf("[Msg from %s:%i] %s\n", client_ip, client_port, client_message);
        memset(client_message, '\0', sizeof(client_message));
	}
	
	if(read_size == 0){
		printf("Client %s:%i disconnected\n", client_ip, client_port);
		fflush(stdout);
	}
	else if(read_size == -1){
		perror("[!]Recv");
	}
    
    //Cleaning up
    close(sock);
    free(socket_desc);
    thread_cntr--;
    pthread_exit(NULL);
}