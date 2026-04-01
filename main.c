#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9090
#define BUFFER_SIZE 1024

void sendHTML(int *sock, const char *file) {
  FILE *html = fopen(file, "r");
  if (!html) {
    perror("could not open HTML file");
    return;
  }
  fclose(html);
}

int main() {
  int serverSock;
  if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("could not get our serversocket fd");
    return -1;
  }

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(PORT);

  if (bind(serverSock, (struct sockaddr *)&serverAddr, sizeof serverAddr) < 0) {
    perror("could not bind to server socket and address");
    return -1;
  }

  if (listen(serverSock, 5) < 0) {

    perror("could not listen on server socket and ip");
  }

  printf("listening on port %d\n", PORT);

  while (1) {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int *clientSocket = malloc(sizeof(int));

    if ((*clientSocket = accept(serverSock, (struct sockaddr *)&clientAddr,
                                &clientLen)) < 0) {
      perror("could not accept client");
      continue;
    }
    printf("client connected");
    char *msg = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello World!\n";
    send(*clientSocket, msg, strlen(msg), 0);
    close(*clientSocket);
    printf("client disconnected");
    free(clientSocket);
  }
  close(serverSock);

  return 0;
}
