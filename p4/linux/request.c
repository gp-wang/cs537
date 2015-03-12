//
// request.c: Does the bulk of the work for the web server.
// 

#include "cs537.h"
#include "request.h"




// requestError(      fd,    filename,        "404",    "Not found", "CS537 Server could not find this file");
void requestError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) 
{
   char buf[MAXLINE], body[MAXBUF];

   // Create the body of the error message
   sprintf(body, "<html><title>CS537 Error</title>");
   sprintf(body, "%s<body bgcolor=""fffff"">\r\n", body);
   sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
   sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
   sprintf(body, "%s<hr>CS537 Web Server\r\n", body);

   // Write out the header information for this response
   sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Type: text/html\r\n");
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   // Write out the content
   Rio_writen(fd, body, strlen(body));
   printf("%s", body);

}


//
// Reads and discards everything up to an empty text line
//
void requestReadhdrs(rio_t *rp)
{
   char buf[MAXLINE];

   Rio_readlineb(rp, buf, MAXLINE);
   while (strcmp(buf, "\r\n")) {
      Rio_readlineb(rp, buf, MAXLINE);
   }
   return;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int requestParseURI(char *uri, char *filename, char *cgiargs) 
{
   char *ptr;

   if (!strstr(uri, "cgi")) {
      // static
      strcpy(cgiargs, "");
      sprintf(filename, ".%s", uri);
      if (uri[strlen(uri)-1] == '/') {
         strcat(filename, "home.html");
      }
      return 1;
   } else {
      // dynamic
      ptr = index(uri, '?');
      if (ptr) {
         strcpy(cgiargs, ptr+1);
         *ptr = '\0';
      } else {
         strcpy(cgiargs, "");
      }
      sprintf(filename, ".%s", uri);
      return 0;
   }
}

//
// Fills in the filetype given the filename
//
void requestGetFiletype(char *filename, char *filetype)
{
   if (strstr(filename, ".html")) 
      strcpy(filetype, "text/html");
   else if (strstr(filename, ".gif")) 
      strcpy(filetype, "image/gif");
   else if (strstr(filename, ".jpg")) 
      strcpy(filetype, "image/jpeg");
   else 
      strcpy(filetype, "text/plain");
}

void requestServeDynamic(int fd, char *filename, char *cgiargs)
{
   char buf[MAXLINE], *emptylist[] = {NULL};

   // The server does only a little bit of the header.  
   // The CGI script has to finish writing out the header.
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: CS537 Web Server\r\n", buf);

   Rio_writen(fd, buf, strlen(buf));

   if (Fork() == 0) {
      /* Child process */
      Setenv("QUERY_STRING", cgiargs, 1);
      /* When the CGI process writes to stdout, it will instead go to the socket */
      Dup2(fd, STDOUT_FILENO);
      Execve(filename, emptylist, environ);
   }
   Wait(NULL);
}


void requestServeStatic(int fd, char *filename, int filesize) 
{
   int srcfd;
   char *srcp, filetype[MAXLINE], buf[MAXBUF];

   requestGetFiletype(filename, filetype);

   srcfd = Open(filename, O_RDONLY, 0);

   // Rather than call read() to read the file into memory, 
   // which would require that we allocate a buffer, we memory-map the file
   srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
   Close(srcfd);

   // put together response
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: CS537 Web Server\r\n", buf);
   sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
   sprintf(buf, "%sContent-Type: %s\r\n\r\n", buf, filetype);

   Rio_writen(fd, buf, strlen(buf));

   //  Writes out to the client socket the memory-mapped file 
   Rio_writen(fd, srcp, filesize);
   Munmap(srcp, filesize);

}


void requestPreHandle(request_t* r)
{

  /* v0 */
   /* int is_static; */
   struct stat sbuf;
   /* char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; */
   /* char filename[MAXLINE], cgiargs[MAXLINE]; */



   rio_t rio;
   //   printf("\ncheckpoint_prehandle_1");
   /* v0 */
   //   Rio_readinitb(&rio, fd);
   /* gw */
   Rio_readinitb(&rio, r->fd);
   Rio_readlineb(&rio, r->buf, MAXLINE);
   sscanf(r->buf, "%s %s %s", r->method, r->uri, r->version);
   printf("%s %s %s\n", r->method, r->uri, r->version);


   //   printf("\ncheckpoint_prehandle_2");
   /* gw */
   if (strcasecmp(r->method, "GET")) {
      requestError(r->fd, r->method, "501", "Not Implemented", "CS537 Server does not implement this method");
      r->error=1;
      return;
   }
   requestReadhdrs(&rio);

   //   printf("\ncheckpoint_prehandle_3");
   /* parse this in serverc main */
   /* gw */
   r->is_static = requestParseURI(r->uri,r->filename, r->cgiargs); /* populates info into r->filename, cigargs */
   //   printf("\nAfter parse uri:");
   //   printf("\nuri=%s, filename=%s, cgiargs=%s", r->uri, r->filename, r->cgiargs);

   //   stat(r->filename, r->ptrsbuf);
   if (stat(r->filename, r->ptrsbuf) < 0) {
   /* /\* //    int fd = open(r->filename, O_RDONLY);//?? GW:O_RDONLY cannot be foundin any above header files=> found in man 2 open *\/ */
   /* /\* //    fstat(open(r->filename, O_RDONLY), r->ptrsbuf); *\/ */
   /* /\* //   if (stat(r->filename, &sbuf) < 0) { *\/ */
     //   printf("\ncheckpoint_prehandle_3.1");
      requestError(r->fd, r->filename, "404", "Not found", "CS537 Server could not find this file");
      r->error=1;
      return;
   }
   //   printf("\ncheckpoint_prehandle_4");
   //   *(r->ptrsbuf)=sbuf;

   /* gw */
   /* get nameLength and fileSize */
   int i;
   char* ic;
   r->nameLength=strlen(r->filename);
   /* for(i=0,ic=r->filename;ic!=NULL;i++,ic++) */
   /*   r->nameLength++; */

   r->fileSize=r->ptrsbuf->st_size;

   //   printf("\ncheckpoint_prehandle_5");
   /* gw */
   if (r->is_static) {
      if (!(S_ISREG(r->ptrsbuf->st_mode)) || !(S_IRUSR & r->ptrsbuf->st_mode)) {
         requestError(r->fd, r->filename, "403", "Forbidden", "CS537 Server could not read this file");
	 r->error=1;
         return;
      }
      //      requestServeStatic(r->fd, r->filename, r->sbuf.st_size);
   } else {
      if (!(S_ISREG(r->ptrsbuf->st_mode)) || !(S_IXUSR & r->ptrsbuf->st_mode)) {
         requestError(r->fd, r->filename, "403", "Forbidden", "CS537 Server could not run this CGI program");
	 r->error=1;
         return;
      }
      //      requestServeDynamic(r->fd, r->filename, r->cgiargs);
   }
}


// handle a request
/* v0 */
//void requestHandle(int fd)
/* gw */
void requestHandle(request_t* r)
{


   /* gw */
  //   if (r->is_static&&!r->error) {
  if (r->is_static) {
      requestServeStatic(r->fd, r->filename, r->ptrsbuf->st_size);
      //   } else if (!r->is_static&&!r->error) {
  } else if (!r->is_static) {
      requestServeDynamic(r->fd, r->filename, r->cgiargs);
   }
   else {
     perror("requestHandle");
     exit(-1);
   }
}


