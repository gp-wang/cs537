#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server.o request.o cs537.o client.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall

LIBS = -lpthread 

.SUFFIXES: .c .o 

all: server client output.cgi output_small.cgi  output_medium.cgi  output_large.cgi  output_very_large.cgi

server: server.o request.o cs537.o
	$(CC) $(CFLAGS) -o server server.o request.o cs537.o $(LIBS)

client: client.o cs537.o
	$(CC) $(CFLAGS) -o client client.o cs537.o $(LIBS)

output.cgi: output.c
	$(CC) $(CFLAGS) -o output.cgi output.c

output_small.cgi: output_small.c
	$(CC) $(CFLAGS) -o output_small.cgi output_small.c

output_medium.cgi: output_medium.c 
	$(CC) $(CFLAGS) -o output_medium.cgi output_medium.c

output_large.cgi: output_large.c 
	$(CC) $(CFLAGS) -o output_large.cgi output_large.c

output_very_large.cgi: output_very_large.c
	$(CC) $(CFLAGS) -o output_very_large.cgi output_very_large.c


.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) server client output.cgi
