#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/wait.h>
#include <sys/un.h>


#define PORT 8080
#define LISTEN_PACKLOG 50

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct test{
  int a;
  long b;
};


int main(int argc, char *argv[])
{ 
  int sfd, cfd;
  
  struct test hello;

  
  struct sockaddr_in my_addr;

  my_addr.sin_family = AF_INET;
  
  return 0;
}
