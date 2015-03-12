#ifndef __REQUEST_H__

/* gw */
typedef struct stat stat_t;

typedef struct __request_t{
  int fd;
  int is_static;

  /* ?request line */
  /* ?request headers */
  //  struct stat *ptrsbuf;
  stat_t *ptrsbuf;
  char buf[MAXLINE];
  char method[MAXLINE];
  /* char urt[MAXLINE]; */
  char uri[MAXLINE];
  char version[MAXLINE];
  char filename[MAXLINE];
  char cgiargs[MAXLINE];


  /* for scheduler */
  int sequence;			/* FIFO */
  int nameLength;		/* SFNF */
  int fileSize;			/* SFF */

  int error;

  /* index in buffer, -1 if not in buffer */
  int ibuf;
}request_t;

/* v0 */
//void requestHandle(int fd);
/* gw */
void requestPreHandle(request_t* r);
void requestHandle(request_t* r);

#endif
