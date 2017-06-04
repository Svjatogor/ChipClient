#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
int connectToServer();
void closeSocket(int sock_id);
int readMessage(int sock_id, char* buffer);
int writeMessage(int sock_id, char* buffer);
int sendImage(int sock_id, char* file_name);
int getImage(int sock_id, char* file_name);
#endif // TCP_CLIENT_H
