#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

//TODO: Check if this is necesary
int opt = 0;
struct opts {
  long int SERVER_PORT;
};


//TODO: Refactor multithreading
void *connection_handler( void * );

int main(int argc, char *argv[])
{
  
  //TODO: maybe delete this
  char *port, *endptr;

  //TODO: Refactor OPTS
  struct opts OPTS;
  OPTS.SERVER_PORT = 8888;

  
  //TODO: Reading Arguments
  while((opt = getopt(argc, argv, "p:")) != -1)
  {
    switch (opt)
    {
      case 'p':
      OPTS.SERVER_PORT  = strtol(optarg, &endptr, 10);
      break;

      case '?':
      if (optopt == 'p')
      {
        puts("Missing mandatory PORT option");
      } else 
      {
        puts("Invalid option received");
      }
      break;
    }
  }

  int socket_desc, new_socket, c, *new_sock;
  struct sockaddr_in server, client;
  char *message;

  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_desc == -1)
  {
    printf("Could not create the socket");
  } 

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(OPTS.SERVER_PORT);

  if( bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    puts("bind failed");
    return 1;
  }
  printf("Initializing The Faker Backend Server\r\n");
  
  listen(socket_desc, 5);

  printf("The Faker Backend is listening on http://localhost:%ld...\r\n", OPTS.SERVER_PORT);
  printf("Waiting for incoming connections...\r\n");
  c = sizeof(struct sockaddr_in);
  while ( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c ) ) )
  {
    //puts("Connection accepted");

    pthread_t sniffer_thread;
    new_sock = malloc(1);
    *new_sock = new_socket;

    if (pthread_create( &sniffer_thread, NULL, connection_handler, (void *) new_sock ) < 0 )
    {
      perror("Could not create thread");
      return 1;
    }

    puts("Handler assigned");
  }

  if (new_socket<0)
  {
    perror("accept failed");
    return 1;
  }

  return 0;
}

void *connection_handler(void *socket_desc)
{
  int sock = *(int *)socket_desc;
  int read_size;
  char *message, client_message[2000], *pch;

  read_size = recv(sock, client_message, 2000, 0);
  //puts(client_message);


  //TODO: Parser HTTP Headers
  pch = strtok(client_message, "\r\n");
  while (pch != NULL)
  {
    printf ("%s\n",pch);
    pch = strtok (NULL, "\r\n");
  }

  //TODO: Refactor The Response Headers
  //message = "HTTP/1.1 404 Not Found\r\n";
  message = "HTTP/1.1 201 Ok\r\n";
  send(sock, message, strlen(message), 0);

  message = "Server: The Fake Server\r\n";
  send(sock, message, strlen(message), 0);

  message = "Connection: close\r\n";
  send(sock, message, strlen(message), 0);


  message = "\r\n";
  send(sock, message, strlen(message), 0);

  message = "Body";
  send(sock, message, strlen(message), 0);

  close(sock);

  free(socket_desc);
  return 0;
}
