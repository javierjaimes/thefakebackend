#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "headers/header.h"
#include "headers/querystring.h"

#define DELIMITERS "\r\n"

//Set up
#define LEN 150

//TODO: Check if this is necesary
int opt = 0;

//TODO: Change the name by conf
struct Opts {
  long int SERVER_PORT;
};

struct Request {
  char METHOD[10];
  char URL[100];
  char PROTOCOL[20];
  struct Header HEADERS[20];
  struct QueryString QUERYPARAMS[20];
};


//TODO: Refactor multithreading
void *connection_handler( void * );

int main(int argc, char *argv[])
{
  
  //TODO: maybe delete this
  char *endptr;

  //TODO: Refactor OPTS
  struct Opts OPTS;
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
  //char *message;

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
  struct Request Req;

  int sock = *(int *)socket_desc;
  char *message, client_message[10000], *strtorequest;

  recv(sock, client_message, 10000, 0);
  //puts (client_message);

  //int n = sscanf(client_message, "%s %s %s %s", Req.METHOD, Req.URL, Req.PROTOCOL, crlf );
  //printf("Strings read:%d %s %s\n", n, Req.METHOD, crlf);

  //TODO: Parser HTTP Headers

  //int i = 0;
  //strcpy(headers, client_message);

  strtorequest = strtok(client_message, "\r\n");
  sscanf(strtorequest, "%s %s %s", Req.METHOD, Req.URL, Req.PROTOCOL);

  while ((strtorequest = strtok(NULL, DELIMITERS)) != NULL)
  {
    //printf("Header is: %s\r\n", strtorequest);

    struct Header H;
    sscanf (strtorequest, "%s[:], %s", H.NAME, H.VALUE);
    int i = 0;
    Req.HEADERS[i] =  H;
    i += 1;
    //
    printf("%s %s\r\n", H.NAME, H.VALUE);
  }
  
  //printf("Strings read:%d %s %s\n", n, Req.METHOD, crlf);

  /*Req.METHOD = strtok(pch, " ");
  Req.URL  = strtok(NULL, " ");
  Req.PROTOCOL  = strtok(NULL, " ");*/

  /*char *header = strtok(headers, "\r\n");
  header = strtok(NULL, "\r\n");
  while (header != NULL)
  {
    printf("%s\n", header);
    header = strtok(NULL, "\r\n");
  }*/


  //printf("%s\n", Req.PROTOCOL);
  /*while (pch != NULL)
  {
    //printf ("%s\n",pch);
    if (i==0)
    {

    } else
    {

    }
    pch = strtok (NULL, "\r\n");
  }*/

  //TODO: Refactor The Response Headers
  //message = "HTTP/1.1 404 Not Found\r\n";
  message = "HTTP/1.1 200 Ok\r\n";
  send(sock, message, strlen(message), 0);

  message = "Server: The Fake Backend Server\r\n";
  send(sock, message, strlen(message), 0);

  //Set the current time
  char buf[LEN];
  time_t rawtime;
  struct tm *loc_time;
  
  rawtime = time(NULL);
  loc_time = localtime(&rawtime);
  strftime(buf, LEN, "%a, %d %b %Y %X GMT%z\r\n", loc_time);

  char date[] = "Date: ";
  strcat(date, buf);
  send(sock, date, strlen(date), 0);

  message = "Content-Type: application/json\r\n";
  send(sock, message, strlen(message), 0);

  char  body[] = "{\"format\":\"JSON\"}";
  int body_lenght = strlen(body);
  char body_size[20];
  sprintf(body_size, "%d \r\n", body_lenght);

  char content [] = "Content-Length: ";
  strcat(content, body_size);
  send(sock, content, strlen(content), 0);

  message = "Connection: close\r\n";
  send(sock, message, strlen(message), 0);


  message = "\r\n";
  send(sock, message, strlen(message), 0);

  //TODO: send the response
  send(sock, body, strlen(body), 0);

  close(sock);

  free(socket_desc);
  return 0;
}
