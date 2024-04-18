CC = gcc
#CFLAGS = -Wall -pthread -I./include
CFLAGS = -Wall -pthread -I./include -Wno-unused-variable
PROGRAM = server client
SERVER_DIR = TCP_Server
CLIENT_DIR = TCP_Client
DEPS = tcp_socket.h session.h utility.h resolver.h
OBJ = include/tcp_socket.o include/session.o include/utility.o include/resolver.o

#_______________________________________________________________________________
all: clean $(PROGRAM)
tcp: tcpserver tcpclient

#_______________________________________________________________________________
server: $(SERVER_DIR)/server.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

client: $(CLIENT_DIR)/client.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

test: tcp_test.c
	gcc -pthread tcp_test.c -o test

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

#_______________________________________________________________________________
clean:
	rm -f *.o $(SERVER_DIR)/*.o $(CLIENT_DIR)/*.o include/*.o $(PROGRAM)
clean-all:
	rm -f *.o *.exe ex? $(SERVER_DIR)/*.o $(CLIENT_DIR)/*.o include/*.o $(PROGRAM)

#
#_______________________________________________________________________________
tcpserver: TCP_Echo_Server.c
	$(CC) -o $@ $^ $(CFLAGS)

tcpclient: TCP_Echo_Client.c
	$(CC) -o $@ $^ $(CFLAGS)

#_______________________________________________________________________________
ex1: Example_1.c
	$(CC) -o $@ $^ $(CFLAGS)
ex2: Example_2.c
	$(CC) -o $@ $^ $(CFLAGS)
ex3: Example_3.c
	$(CC) -o $@ $^ $(CFLAGS)
ex4: Example_4.c
	$(CC) -o $@ $^ $(CFLAGS)
ex5: Example_5.c utility.c
	$(CC) -o $@ $^ $(CFLAGS)
ex6: Example_6.c utility.c
	$(CC) -o $@ $^ $(CFLAGS)