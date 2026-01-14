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

void handle_request(int cfd) {

  char buffer[BUFFER_SIZE];
  ssize_t bytes_read = read(cfd, buffer, BUFFER_SIZE - 1);
  buffer[bytes_read] = 0;

  printf("It works!!!!\n");
  printf("%s\n", buffer);

  char msg[] = "Hello world!";
  int msg_size = sizeof(msg);

  int len =
      snprintf(buffer, 1024,
               "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
               "%d\r\n\r\n%s",
               msg_size, msg);

  write(cfd, buffer, len);
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
