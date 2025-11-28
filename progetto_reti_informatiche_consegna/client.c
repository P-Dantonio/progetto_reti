#include <stdio.h>
#include <stdlib.h>
#include "libclient.h"

/* Uso richiesto: ./client <PORTA>  (si connette a 127.0.0.1) */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <PORTA>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int server_port = atoi(argv[1]);
    if (server_port <= 0) {
        fprintf(stderr, "Porta non valida.\n");
        return EXIT_FAILURE;
    }
    esegui_client("127.0.0.1", server_port);
    return 0;
}