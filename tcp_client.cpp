#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "tcp_client.h"
int connectToServer() {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    portno = atoi("224");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1;
    }
    //server = gethostbyname("localhost");
    server = gethostbyname("192.168.0.9");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        return -1;
    return sockfd;
}

void closeSocket(int sock_id) {
    close(sock_id);
}

int readMessage(int sock_id, char* buffer) {
    bzero(buffer, 256);
    int n = 0;
    int size_message = 0;
    recv(sock_id, &size_message, sizeof(size_message), 0);
    n = recv(sock_id, buffer, size_message, 0);
    if (n < 0) {
        return -1;
    }
    return n;
}

int writeMessage(int sock_id, char* buffer) {
    int n = 0;
    int size_message = strlen(buffer);
    send(sock_id, &size_message, sizeof(size_message), 0);
    n = send(sock_id, buffer, size_message, 0);
    if (n < 0) {
        return -1;
    }
    return n;
}

int sendImage(int sock_id, char *file_name) {
    FILE *picture;
    picture = fopen(file_name, "r");
    int size;
    fseek(picture, 0, SEEK_END);
    size = ftell(picture);
    fseek(picture, 0, SEEK_SET);
    // send picture size
    int n = send(sock_id, &size, sizeof(size), 0);
    if (n < 0) {
        return -1;
    }

    // send picture as byte array
    char send_buffer[size];
    //while(!feof(picture)) {
        fread(send_buffer, 1, sizeof(send_buffer), picture);
        n = send(sock_id, send_buffer, sizeof(send_buffer), 0);
        if (n < 0) {
            return -1;
        }
        bzero(send_buffer, sizeof(send_buffer));
    //}
    //fflush(stdout);         // make sure everything makes it to the output
    return n;
}

void get_image(int sock_id, char* file_name) {
    bzero(file_name, sizeof(file_name));
    strcpy(file_name, "predictions.png");
    int size;
    int n = recv(sock_id, &size, sizeof(int), 0);
    if (n < 0) {
        return;
    }
    printf("Size image: %d\nReading image...\n", size);
    char image_bytes[size];
    char *point = image_bytes;
    n = 0;
    while (size > 0) {
        n = recv(sock_id, point, size, 0);
        point += n;
        if (n < 0) {
            return;
        }
        size -= n;
    }
    printf("n = %d\n", n);
    printf("Image received\n");
    // convert bytes to file
    FILE *image;
    image = fopen(file_name, "w");
    fwrite(image_bytes, 1, sizeof(image_bytes), image);
    fclose(image);
    //fflush(stdout);         // make sure everything makes it to the output
}
