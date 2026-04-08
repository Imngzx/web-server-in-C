#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9090
#define BUFFER_SIZE 2048

char *parseRoute(const char *route) {
  if (strcmp(route, "/") == 0 || strcmp(route, "/home") == 0) {
    return "static/index.html";
  } else if (strcmp(route, "/about") == 0) {
    return "static/about.html";
  } else if (strcmp(route, "/favicon.ico") == 0) {
    return "static/favicon.ico";
  }

  return "static/notFound.html";
}

void sendHTML(int *sock, const char *file) {
  FILE *html = fopen(file, "r");
  if (!html) {
    perror("could not open HTML file");
    char *errorMsg = "HTTP/1.1 404 Not Found\r\n\r\nFile Not Found on Server.";
    send(*sock, errorMsg, strlen(errorMsg), 0);
    return;
  }

  char buffer[BUFFER_SIZE] = {0};
  size_t read = 0;

  char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  send(*sock, header, strlen(header), 0);

  while ((read = fread(buffer, sizeof(char), BUFFER_SIZE, html)) > 0) {
    send(*sock, buffer, read, 0);
  }

  fclose(html);
}

int main() {
  int serverSock;
  if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("could not get our serversocket fd");
    return -1;
  }

  int opt = 1;
  setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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
      free(clientSocket);
      continue;
    }
    printf("client connected\n");

    char recBuf[BUFFER_SIZE] = {0};
    recv(*clientSocket, recBuf, BUFFER_SIZE - 1, 0);
    printf("%s\n", recBuf);

    char method[16] = {0};
    char path[256] = {0};

    if (sscanf(recBuf, "%15s %255s", method, path) == 2) {
      printf("Requested Path: %s\n", path);

      sendHTML(clientSocket, parseRoute(path));
    }

    close(*clientSocket);
    printf("client disconnected\n\n");
    free(clientSocket);
  }

  close(serverSock);
  return 0;
}
