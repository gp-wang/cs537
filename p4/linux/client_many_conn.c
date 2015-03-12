/*
 * client.c: A very, very primitive HTTP client.
 * 
 * To run, try: 
 *      client www.cs.wisc.edu 80 /
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * CS537: For testing your server, you will want to modify this client.  
 * For example:
 * 
 * You may want to make this multi-threaded so that you can 
 * send many requests simultaneously to the server.
 *
 * You may also want to be able to request different URIs; 
 * you may want to get more URIs from the command line 
 * or read the list from a file. 
 *
 * When we test your server, we will be using modifications to this client.
 *
 */

#include "cs537.h"

#include <time.h>

void waitFor (unsigned int secs) {
  unsigned retTime = time(0) + secs;     // Get finishing time.
    while (time(0) < retTime);    // Loop until it arrives.
}
int numClients=3;
pthread_mutex_t client_buffer_lock;
pthread_mutex_t client_count_lock;
pthread_cond_t* client_buffer_fill;
//pthread_cond_t client_buffer_empty;



volatile static int *client_count;
//volatile static int *connection_fill;
/*
 * Send an HTTP request for the specified file 
 */
/* volatile static int fd[numClients]; */
/* volatile static  int seq[numClients]; */
typedef struct __node_t {

  int fd;
  int index;
} node_t;
 
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;

  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  /* n = Rio_readlineb(&rio, buf, MAXBUF); */
  /* while (strcmp(buf, "\r\n") && (n > 0)) { */
  /*   printf("Header: %s", buf); */
  /*   n = Rio_readlineb(&rio, buf, MAXBUF); */

  /*   /\* If you want to look for certain HTTP tags... *\/ */
  /*   if (sscanf(buf, "Content-Length: %d ", &length) == 1) { */
  /*     printf("Length = %d\n", length); */
  /*   } */
  /* } */

  /* Read and display the HTTP Body */
  /* n = Rio_readlineb(&rio, buf, MAXBUF); */
  /* while (n > 0) { */
  /*   printf("%s", buf); */
  /*   n = Rio_readlineb(&rio, buf, MAXBUF); */
  /* } */
}

//void* worker_client(void* ptr_clientfd)  {
void* worker_client(void* ptr_conn_node)  {
  //  while(1){
    /* ********************lock******************** */
    pthread_mutex_lock(&client_buffer_lock);

    int index=((node_t*)ptr_conn_node)->index;
    int fd=((node_t*)ptr_conn_node)->fd;
    /* consumer */
    while(client_count==0)
      pthread_cond_wait(&client_buffer_fill[index], &client_buffer_lock);
    /* gw */
    printf("\nClient being received # %d:\n",index);

    client_count--;
    //    clientPrint(*(int*)ptr_clientfd);
    clientPrint(fd);
    pthread_mutex_unlock(&client_buffer_lock);
    //    Close(*(int*)ptr_clientfd);
    Close(fd);
    /* ********************unlock******************** */
    //  }
    return NULL;
}
void spin(int num) {
  int i=0;
  while(i<num)
    i++;
}

int main(int argc, char *argv[])
{
  int i;
  char *host, *filename;
  int port;
  /* int clientfd; */

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
    exit(1);
  }

  host = argv[1];
  port = atoi(argv[2]);
  filename = argv[3];

  /* Open a single connection to the specified host and port */

  client_buffer_fill=(pthread_cond_t*) malloc(numClients*sizeof(pthread_cond_t));
  pthread_mutex_init(&client_buffer_lock,NULL);


  for(i=0;i<numClients;i++)
    pthread_cond_init(&client_buffer_fill[i],NULL);


  client_count=0;

  /* pawn worker threads */
  pthread_t* threads=(pthread_t*)malloc(numClients*sizeof(pthread_t));

  node_t conn_nodes[numClients];
  for(i=0;i<numClients;i++){
    waitFor(1);
    conn_nodes[i].fd= Open_clientfd(host, port);
    conn_nodes[i].index=i;
  }


  for(i=0;i<numClients;i++){
    pthread_create(&threads[i], NULL, worker_client,(void*) &conn_nodes[i]);
  }

  /* create a few clients in sequence and send request to server */
  for(i=0;i<numClients;i++) {
    waitFor(1);
    pthread_mutex_lock(&client_buffer_lock);
    /* producer */
    clientSend(conn_nodes[i].fd, filename);
    printf("\nRequest being sent out:  %d, fd: %d", conn_nodes[i].index, conn_nodes[i].fd);
    client_count++;
    pthread_cond_signal(&client_buffer_fill[i]);
    pthread_mutex_unlock(&client_buffer_lock);
  }


  for(i=0;i<numClients;i++) {
    pthread_join(threads[i],NULL);
  }




  exit(0);
}
