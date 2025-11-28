
# Compilatore
CC = gcc

# Opzioni di compilazione: mostra warning ed ottimizza
CFLAGS = -Wall -Wextra -O2

# File oggetto
OBJS_SERVER = server.o libserver.o
OBJS_CLIENT = client.o libclient.o

# Eseguibili finali
SERVER = server
CLIENT = client

# ===== Regole principali =====

all: $(SERVER) $(CLIENT)

# Compilazione server
$(SERVER): $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $(SERVER) $(OBJS_SERVER)

# Compilazione client
$(CLIENT): $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $(CLIENT) $(OBJS_CLIENT)

# File oggetto (regole implicite)
server.o: server.c libserver.h
	$(CC) $(CFLAGS) -c server.c

libserver.o: libserver.c libserver.h
	$(CC) $(CFLAGS) -c libserver.c

client.o: client.c libclient.h
	$(CC) $(CFLAGS) -c client.c

libclient.o: libclient.c libclient.h
	$(CC) $(CFLAGS) -c libclient.c

# pulizia
clean:
	rm -f *.o $(SERVER) $(CLIENT)

# esecuzioni rapide
run-server: $(SERVER)
	./$(SERVER)

run-client: $(CLIENT)
	./$(CLIENT) 12345

.PHONY: all clean run-server run-client

