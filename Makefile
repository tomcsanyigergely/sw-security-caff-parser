CC = g++
WFLAGS = -Wall -Wextra -Werror -Wpedantic -Wformat=2 -Wnull-dereference -Wstack-protector -Wstrict-overflow=3 -Wtrampolines -Warray-bounds=2 -Wcast-qual -Wstringop-overflow=4 -Wconversion -Wsign-conversion -Warith-conversion -Wformat-security -Walloca -Wnull-dereference -Wvla -Wpointer-arith -Wimplicit-fallthrough 
CFLAGS = -O2 -fstack-protector-strong -fstack-clash-protection -fPIE -fcf-protection=full -ftrapv -D_FORTIFY_SOURCE=3 -fsanitize=bounds -fsanitize-undefined-trap-on-error -fno-sanitize-recover
LDFLAGS = -Wl,-z,now -Wl,-z,relro -Wl,-z,noexecstack -Wl,-z,separate-code
OBJS = main.o parser.o jpge.o

parser: $(OBJS)
	$(CC) $(CFLAGS) $(WFLAGS) $(OBJS) $(LDFLAGS) -o parser
 
main.o: main.c parser.h jpge.h
	$(CC) $(CFLAGS) $(WFLAGS) -c main.c

parser.o: parser.c parser.h
	$(CC) $(CFALGS) $(WFLAGS) -c parser.c

jpge.o: jpge.c jpge.h
	$(CC) $(CFLAGS) -c jpge.c
	
clean:
	rm -f *.o parser