/* TODO:
   make worker routine (prelim)
   make thread (prelim)
   make cond var (prelim)
   change request.c requestHandle() to handle request_t (prelim)
   make scheduler (prelim)
   error codes
   */

#include "cs537.h"
#include "request.h"

/* gw */
/* already there in cs537.h */
/* #include <stdlib.h>	   /\* atoi *\/ */
// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

pthread_mutex_t buffer_lock;
pthread_mutex_t count_lock;
pthread_cond_t buffer_fill;
pthread_cond_t buffer_empty;

/* gw */
/* may need to add extern */
/* TBD */
static volatile int count=0;
static volatile int *used;
/* first available slot # in buf */
static volatile int first; 
static volatile int scheduler;
static volatile int numThreads,numBuffers;
/* Alternative:
   remove static
   add extern*/




int comparator_FIFO( void *p1,  void *p2){
  int aa,bb;
  aa=((request_t*)p1)->sequence;
  bb=((request_t*)p2)->sequence;
  if (aa<bb) return -1;
  else if(aa==bb) return 0;
  else return 1;

}

int comparator_SFNF( void *p1,  void *p2){
  int aa,bb;
  aa=((request_t*)p1)->nameLength;
  bb=((request_t*)p2)->nameLength;
  if (aa<bb) return -1;
  else if(aa==bb) return 0;
  else return 1;
}

int comparator_SFF( void *p1,  void *p2){
  int aa,bb;
  aa=((request_t*)p1)->fileSize;
  bb=((request_t*)p2)->fileSize;
  if (aa<bb) return -1;
  else if(aa==bb) return 0;
  else return 1;
}

void swap(void **v, int i, int j) { 
  void *temp; 
  temp = v[i]; 
  v[i] = v[j]; 
  v[j] = temp; 
}

void merge(void** m, void** a, size_t na, void** b, size_t nb, int (*comp)(void *, void *) ) {
  //merge two portions into one sorted whole

  int ia,ib,im;
  //the twist here is you have to keep an original ordered m sequence in order to fill the merged-sorted-list

  void** new=(void**)malloc((na+nb)*sizeof(void*));
  for(ia=0,ib=0,im=0;(ia<na)&&(ib<nb);) {
    if((*comp)(a[ia], b[ib]) < 0)  {
      new[im]=a[ia];
      ia++;
      im++;
    }
    else{
      new[im]=b[ib];
      ib++;
      im++;
    }
  }

  if(ia==na)
    //a finishes before b
    for(;ib<nb;im++,ib++)
      new[im]=b[ib];
  else if(ib==nb)
    for(;ia<na;im++,ia++)
      new[im]=a[ia];
  else {perror("merge");exit(-1);}

  int i;
  for(i=0;i<na+nb;i++)
    m[i]=new[i];
  free(new);

}



//api following c book qsort eg.
void msort(void **v, int left, int right, int (*comp)(void *, void *)) {
  //TODO: how to write below api: void qsort(void *base, size_t nmemb, size_t size,int (*compar)(const void *, const void *)){


  if(left>right)  {
    perror("msort"); 
    exit(-1);
  }

  //base case 1
  if(left==right) return;

  //base case 2
   if((left+1==right)&&(*comp)(v[left], v[right]) > 0)  {
    swap(v, left, right);
    return;
  }
  else if((left+1==right)&&(*comp)(v[left], v[right]) <= 0)  {
    return;
  }

  int middle=(left+right)/2;
  //recursion 1, left half
  msort(v,left,middle,comp);

  //recursion 2, right half
  msort(v,middle+1,right,comp);

  //merge sorted two lists
  merge(v+left,	v+left,	(size_t) (middle-left+1), v+(middle+1), (size_t) (right-middle), comp);
}


// CS537: Parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
}

/* gw */
/* except pbuf to be passed into worker, other var are global var */
//void* worker(void* pbuf[]) {
void* worker(void* pbuf) {

  while(1){

    /* ********************lock buffer********************  */
    pthread_mutex_lock(&buffer_lock);
    request_t*  rpick=(request_t*)malloc(sizeof(request_t));
    while(count==0) {
      //    pthread_mutex_unlock(&buffer_lock);
      pthread_cond_wait(&buffer_fill, &buffer_lock);
    }
    if(scheduler==0) msort((void**)pbuf,0,numBuffers-1,comparator_FIFO);
    else if(scheduler==1) msort((void**)pbuf,0,numBuffers-1,comparator_SFNF);
    else if(scheduler==2) msort((void**)pbuf,0,numBuffers-1,comparator_SFF);

    /* get(i) */
    /* TBD: maybe Ok to delete count lock */
    //    pthread_mutex_lock(&count_lock);

    /* v0.1 */
    //    request_t *rpick=(request_t*)(*((void**)pbuf+0)); /* pbuf[0] */

    /* v0.2 */
    //    request_t *rpick;
    int i;
    int ibuf;
    request_t *rptr;
    for(i=0;i<numBuffers;i++) {
      rptr=(request_t*)(*((void**)pbuf+i)); /* pbuf[0] */
      ibuf=rptr->ibuf;
      /* find the first filled slot in the queue */
      if(used[ibuf]==1) {
	used[ibuf]=0;
	break;
      }
    }
    *rpick=*rptr;
    /* copy non primitive vars */
    /*   
	stat_t *ptrsbuf;
  char buf[MAXLINE];
  char method[MAXLINE];
  char uri[MAXLINE];
  char version[MAXLINE];
  char filename[MAXLINE];
  char cgiargs[MAXLINE];
 */
    rpick->ptrsbuf=(stat_t*)malloc(sizeof(stat_t));
    *(rpick->ptrsbuf)=*(rptr->ptrsbuf);
    /* TODO:verify array assign is just base or all contents */
    /* rpick->buf=(char*)malloc(MAXLINE*sizeof(char)); */
    /* rpick->method=(char*)malloc(MAXLINE*sizeof(char)); */
    /* rpick->uri=(char*)malloc(MAXLINE*sizeof(char)); */
    /* rpick->version=(char*)malloc(MAXLINE*sizeof(char)); */
    /* rpick->filename=(char*)malloc(MAXLINE*sizeof(char)); */
    /* rpick->cgiargs=(char*)malloc(MAXLINE*sizeof(char)); */
    for(i=0;i<MAXLINE;i++) {
      rpick->buf[i]=rptr->buf[i];
      rpick->method[i]=rptr->method[i];
      rpick->uri[i]=rptr->uri[i];
      rpick->version[i]=rptr->version[i];
      rpick->filename[i]=rptr->filename[i];
      rpick->cgiargs[i]=rptr->cgiargs[i];
    }


    //    printf("\n********************Out request: index=%d, fileSize=%d",rpick->sequence, rpick->fileSize);

    count--;
    //    pthread_mutex_unlock(&count_lock);
    /* end get(i) */

    pthread_cond_signal(&buffer_empty);



    /* ********************unlock buffer******************** */
    pthread_mutex_unlock(&buffer_lock);

    requestHandle(rpick);
    /* rpick->sequence=65535;  */
    /* rpick->nameLength=65535; */
    /* rpick->fileSize=65535; */
    /* rpick->error=0; */
    /* rpick->ibuf=-1; */

    //    Close(rpick->fd);
    /* rpick->fd=0; */
    free(rpick->ptrsbuf);
    free(rpick);

  }
    /* TODO:verify array assign is just base or all contents */



  /* free(rpick->cgiargs); */
  /* free(rpick->filename); */
  /* free(rpick->version); */
  /* free(rpick->uri); */
  /* free(rpick->method); */
  /* free(rpick->buf); */
  /* free(rpick->ptrsbuf); */
  /* free(rpick); */
}


int main(int argc, char *argv[])
{
  /* shown OK */
  //    printf("\nserver.c main start");

  /* v0 */
    /* int listenfd, connfd, port, clientlen; */
    int listenfd,  port, clientlen;
    struct sockaddr_in clientaddr;

    /* v0 */
    /* getargs(&port, argc, argv); */

    /* gw */
    //    int numThreads,numBuffers; 
    //    char* scheduler;
    if (argc != 5) {
	fprintf(stderr, "Usage: %s <port> <# of worker threads> <# buffer size> <scheduling method: FIFO/SFNF/SFF>\n", argv[0]);
	exit(1);
    }

    port=atoi(argv[1]);
    numThreads=atoi(argv[2]);
    numBuffers=atoi(argv[3]);

    if(strcmp("FIFO", argv[4]) == 0){
      scheduler=0;
    }
    else if(strcmp("SFNF", argv[4]) == 0){
      scheduler=1;
    }
    else if(strcmp("SFF", argv[4]) == 0){
      scheduler=2;
    }
    else {
	fprintf(stderr, "Usage: %s <port> <# of worker threads> <# buffer size> <scheduling method: FIFO/SFNF/SFF>\n", argv[0]);
	exit(1);
    }

    /* parse end */

    int i;
    /* init buffer array */
    /* actual buffer array */
    request_t* rbuf=(request_t*)malloc(numBuffers*sizeof(request_t));
    for(i=0;i<numBuffers;i++){       
      rbuf[i].ptrsbuf=(stat_t*)malloc(sizeof(stat_t));
      rbuf[i].is_static=1; 	/* default to static */
      rbuf[i].sequence=65535;
      rbuf[i].nameLength=65535;
      rbuf[i].fileSize=65535;
      rbuf[i].error=0;
      rbuf[i].ibuf=-1;
    }
    /* pointer to bufs for sorting */
    request_t** pbuf=(request_t**)malloc(numBuffers*sizeof(request_t*));
    /* mark  buffer availability, if used, 1, unused, 0 */
    used=(int*)malloc(numBuffers*sizeof(int));

    for(i=0;i<numBuffers;i++) used[i]=0;

    /* init thread pool */
    pthread_t* threads=(pthread_t*)malloc(numThreads*sizeof(pthread_t));
    for(i=0;i<numThreads;i++)       
      pthread_create(&threads[i], NULL, worker, (void*)pbuf);
      //      pthread_create(&threads[i], NULL, worker, (void**)pbuf);




    /* init cond and mutex */

    pthread_mutex_init(&buffer_lock,NULL);
    pthread_mutex_init(&count_lock,NULL);
    pthread_cond_init(&buffer_empty,NULL);
    pthread_cond_init(&buffer_fill,NULL);

    /*  shownOK */
    //    printf("\nserver.c main init done");

    // 
    // CS537: Create some threads...
    //

    listenfd = Open_listenfd(port);
    while (1) {
      /* shown ok */
      /* printf("\nserver.c main entered loop"); */
      /* printf("\nBefore iteration"); */
      /* 	printf("\ncount= %d, numThreads = %d, numBuffers = %d",count,numThreads, numBuffers); */
      /* 	printf("\nused \n"); */
	/* for(i=0;i<numBuffers;i++) */
	/*   printf("%d ",used[i]); */

	clientlen = sizeof(clientaddr);

	/* gw */
	/* wrap request */
	request_t* rnew=(request_t*)malloc(sizeof(request_t));
	/* alloc space for struct stat *ptrsbuf */
	rnew->ptrsbuf=(stat_t*)malloc(sizeof(stat_t));

        rnew->fd=Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
	/* init rnew content */
	rnew->is_static=0; 
	/* method[MAXLINE]; */
	/* urt[MAXLINE]; */
	/* version[MAXLINE]; */
	rnew->ibuf=-1;
	rnew->sequence=65535; 	/* init to max number */
	rnew->nameLength=65535; 	/* init to max number */
	rnew->fileSize=65536;
	rnew->error=0;

	//	printf("\ncheckpoint1");

	//	printf("\ncheckpoint2");

	/* gw */
	/* PRODUCER: */
	/* ********************buffer lock******************** */
	pthread_mutex_lock(&buffer_lock);
	requestPreHandle(rnew);
	//	printf("\ncheckpoint3");
	while(count>=numBuffers) {
	  //	  pthread_mutex_unlock(&buffer_lock);
	  pthread_cond_wait(&buffer_empty, &buffer_lock);

	}
	//	printf("\ncheckpoint4");
	/* put(i): */
	/* lock count, used*/
	/* TBD: count_lock may be removed */
	//	pthread_mutex_lock(&count_lock);
	//	printf("\ncheckpoint5");
	for(i=0;i<numBuffers;i++) 
	  if(!used[i]){
	    first=i;
	    used[first]=1;
	    break;
	  }
	//	printf("\ncheckpoint6");
	rbuf[first]=*rnew;
	rbuf[first].ibuf=first;	/* actual index request in buffer */
	rbuf[first].sequence=count;
	//	printf("\n********************In request: index =%d, fileSize =%d",rbuf[first].sequence, rbuf[first].fileSize);
	count++;		/* total # of request in buffer */
	//	pthread_mutex_unlock(&count_lock);
	//	printf("\ncheckpoint7");
	/* end put(i) */

	/* silly updating method */
	for(i=0;i<numBuffers;i++) pbuf[i]=&rbuf[i];
	//	printf("\ncheckpoint8");
	pthread_cond_signal(&buffer_fill);
	
	/* ********************buffer unlock******************** */
	//	printf("\ncheckpoint9");
	pthread_mutex_unlock(&buffer_lock);

	free(rnew);

	/* debug */
	//	printf("\nAfter one ieration");
	//	printf("\nused \n");
	//	for(i=0;i<numBuffers;i++)
	//	  printf("%d ",used[i]);
        /* v0 */
	//	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

	// 
	// CS537: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. However, for SFF, you may have to do a little work
	// here (e.g., a stat() on the filename) ...
	//
	/* v0 */
	//	requestHandle(connfd);

	/* v0 */
	//	Close(connfd);

    }

    //	free(used);
	free(pbuf);
	free(rbuf);

}


    


 
