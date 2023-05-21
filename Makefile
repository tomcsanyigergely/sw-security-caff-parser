CC = g++
WFLAGS = -Wall -Wextra -Wpedantic -Wformat=2 -Wnull-dereference -Wstack-protector -Wstrict-overflow=3 -Wtrampolines -Warray-bounds=2 -Wcast-qual -Wstringop-overflow=4 -Wconversion -Wsign-conversion -Warith-conversion -Wformat-security
CFLAGS = -O2 -fstack-protector-strong -fstack-clash-protection -fPIE -fcf-protection=full -ftrapv -D_FORTIFY_SOURCE=2

parser: main.o parser.o jpge.o
	$(CC) $(CFLAGS) $(WFLAGS) -o parser main.o parser.o jpge.o
 
main.o: main.c parser.h jpge.h
	$(CC) $(CFLAGS) $(WFLAGS) -c main.c

parser.o: parser.c parser.h
	$(CC) $(CFALGS) $(WFLAGS) -c parser.c

jpge.o: jpge.c jpge.h
	$(CC) $(CFLAGS) -c jpge.c