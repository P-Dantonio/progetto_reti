#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "libclient.h"

void esegui_client(const char *server_ip, int server_port) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[MAX_LINE] = {0};
    ssize_t bytes_read;

    /* Creazione del socket TCP (AF_INET = IPv4, SOCK_STREAM = TCP) */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Errore creazione socket");
        return;
    }

    /* Inizializza la struttura dell’indirizzo del server */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    /* Converte l’indirizzo IP da stringa in formato binario */
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("IP non valido");
        close(sock);
        return;
    }

    /* Richiede la connessione TCP al server */
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connessione fallita");
        close(sock);
        return;
    }

    printf("Connesso al server %s:%d\n", server_ip, server_port);

    /*
     * Ciclo principale di comunicazione:
     * Il client legge dal server (read) finché non riceve un prompt
     * che indica che deve rispondere (come "Scelta:" o "Risposta:").
     */
    while (1) {
        int prompt_trovato = 0;
        do {
            bytes_read = read(sock, buffer, sizeof(buffer) - 1);
            if (bytes_read <= 0) {
                printf("\nConnessione chiusa dal server.\n");
                close(sock);
                return;
            }
            buffer[bytes_read] = '\0';
            printf("%s", buffer);

            if (strstr(buffer, "Scelta:") != NULL ||
                strstr(buffer, "Risposta:") != NULL ||
                strstr(buffer, "nickname") != NULL ||
                strstr(buffer, "Inserisci") != NULL ||
                strstr(buffer, "> ") != NULL) {
                prompt_trovato = 1;
            }
        } while (!prompt_trovato);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
}