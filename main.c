#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 8080
#define LISTEN_BACKLOG 50
#define BUFFER_SIZE 1024

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

void reap_children(int sig) {
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

void send_error(char *msg) {}
long get_file_size_ftell(FILE *fp) {
  long size = -1; // Initialize size to an error value

  if (fseek(fp, 0L, SEEK_END) == 0) { // Move pointer to end
    size = ftell(fp);                 // Get current position (size)
    if (size == -1L) {
      perror("Failed to get file position");
    }
  } else {
    perror("Failed to seek to end of file");
  }

  rewind(fp);
  return size;
}
void handle_get(int cfd, char *path) {
  char complete_path[200];
  strcpy(complete_path, "public");
  if (!strcmp("/", path)) {
    strcat(complete_path, "/index.html");
  } else {
    strcat(complete_path, path);
  }
  path = realpath(complete_path, NULL);

  if (strncmp(path, "public", 6)) {
    send_error("ILLEGAL PATH");
    return;
  }
  FILE *fr = fopen(path, "rb");
  if (fr == NULL) {
    send_error("PATH NOT FOUND");
    return;
  }

  long file_size = get_file_size_ftell(fr);
  char buffer[BUFFER_SIZE];

  size_t size = fread(buffer, sizeof(char), BUFFER_SIZE, fr);
}
void handle_delete(int cfd, char *path) {}

void handle_request_type(int cfd, char *request_type, char *path) {
  if (!strcmp(request_type, "GET")) {
    handle_get(cfd, path);
  } else if (!strcmp(request_type, "DELETE")) {
    handle_delete(cfd, path);
  } else {
  }
}

void handle_request(int cfd) {
  char buffer[BUFFER_SIZE];
  ssize_t bytes_read = read(cfd, buffer, BUFFER_SIZE - 1);
  buffer[bytes_read] = 0;
  printf("It works!!!!\n");
  printf("%s\n", buffer);

  // GET, POST, ...
  char request_type[10];
  int i = 0, j = 0;
  while (buffer[i] != ' ') {
    request_type[i] = buffer[i];
    i++;
  }

  request_type[i] = 0;
  char path[100];
  i++;
  while (buffer[i] != ' ') {

    path[j] = buffer[i];
    j++;
    i++;
  }

  path[j] = 0;
  handle_request_type(cfd, request_type, path);
  close(cfd);
  exit(0);
}

int main(int argc, char *argv[]) {
  int sfd, cfd;
  socklen_t peer_addr_size;

  struct sigaction sa;
  sa.sa_handler = reap_children;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sigaction(SIGCHLD, &sa, NULL);

  struct sockaddr_in my_addr, peer_addr;
  peer_addr_size = sizeof(my_addr);

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(PORT);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;

  if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  if (bind(sfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
    handle_error("bind");

  if (listen(sfd, LISTEN_BACKLOG) == -1)
    handle_error("listen");

  printf("\nServer is running...\n");

  while (1) {
    cfd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_size);
    pid_t pid;

    if (cfd == -1) {
      handle_error("accept");
    }
    pid = fork();

    if (pid == 0) {
      handle_request(cfd);
    }
    close(cfd);
  }

  return 0;
}
