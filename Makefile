CC = g++
CFLAGS = -Wall
 
parser: main.o parser.o jpge.o
	$(CC) $(CFLAGS) -o parser main.o parser.o jpge.o
 
main.o: main.c parser.h jpge.h
	$(CC) $(CFLAGS) -c main.c

parser.o: parser.c parser.h
	$(CC) $(CFALGS) -c parser.c

jpge.o: jpge.c jpge.h
	$(CC) $(CFLAGS) -c jpge.c