#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *connection_handler( void * );

int main(int argc, char *argv[])
{
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
  server.sin_port = htons( 8888 );

  if( bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    puts("bind failed");
  }
  puts("bind done");
  
  listen(socket_desc, 3);

  puts("Waiting for incoming connections...");
  c = sizeof(struct sockaddr_in);
  while ( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c ) ) )
  {
    puts("Connection accepted");
    message = "Hello Client, I have received your connection. But I have to go now, bye\n";
    write(new_socket, message, strlen(message));

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
  char *message, client_message[2000];

  message = "Greetings! I am your connection handler\n";
  write(sock, message, strlen(message));

  message = "Now type something and I shall repeat what you type \n";
  write(sock, message, strlen(message));

  read_size = recv(sock, client_message, 2000, 0);

  puts(client_message);

  /*while( (read_size = recv(sock, client_message, 2000, 0)) > 0 )
  {
    write(sock, client_message, strlen(client_message));
  }

  if (read_size == 0)
  {
    puts("Client disconnected");
    fflush(stdout);
  }
  else if (read_size == -1)
  {
    perror("recv failed");
  }*/

  close(sock);

  free(socket_desc);
  return 0;
}
