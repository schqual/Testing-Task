#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define HOST "127.0.0.1"
#define PORT 5584
#define DELAY_SEC 1

int main(int argc, char *argv[]){

    int socket_desc;
    struct sockaddr_in server;
    char message[64], *flag;

    struct timespec delay;
    delay.tv_sec = DELAY_SEC;

    //Creating socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0){
        perror("[!]Socket");
        exit(1);
    };

    //Remote server params
    server.sin_addr.s_addr = inet_addr(HOST);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    //Connecting to remote server
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("[!]Connect");
        close(socket_desc);
        exit(2);
    }

    printf("Connected to %s:%i successfully\n", HOST, PORT);

    //Sending message
    srand(time(NULL));
    while(signal(SIGPIPE, SIG_IGN) != SIG_ERR){
        sprintf(message, "%d", rand()%100);
        if (write(socket_desc, message, strlen(message)) < 0){
            perror("[!]Send");
            close(socket_desc);
            exit(3);
        }
        
        printf("Trying to send: %s\n", message);

        /*  Using nanosleep() because:
            POSIX.1-2001 declares usleep() obsolete and POSIX.1-2008
                removes the specification of usleep();
            On Linux, sleep() is implemented via nanosleep().
        */ 
        nanosleep(&delay, &delay);
    }

    //Closing socket
    close(socket_desc);

    return 0;
}