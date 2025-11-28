#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "libserver.h"

void gestisci_sigint(int sig) {
    (void)sig; // evitiamo il warning "unused parameter"
    printf("\n[Server] Arresto in corso...\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int porta = PORTA_DEFAULT;

    // Se viene passata una porta da linea di comando, la uso
    // Esempio: ./server 12345
    if (argc == 2) {
        porta = atoi(argv[1]);
        if (porta <= 0) {
            fprintf(stderr, "Uso corretto: %s [porta]\n", argv[0]);
            return 1;
        }
    }

    signal(SIGINT, gestisci_sigint);
    printf("[Server] Avvio del server sulla porta %d...\n", porta);

    // Avvia la logica principale del server (definita in libserver.c)
    esegui_server();

    return 0;
}
