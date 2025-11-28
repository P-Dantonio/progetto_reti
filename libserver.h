#ifndef LIBSERVER_H
#define LIBSERVER_H

#include <netinet/in.h>

#define PORTA_DEFAULT 12345

#define MAX_CLIENT     50
#define MAX_NICKNAME   50
#define MAX_LINE       1024
#define MAX_DOMANDA    256
#define MAX_RISPOSTA   256
#define NUM_DOMANDE    5
#define N_TEMI         5   /* 1) Mitologia 2) Doctor Who 3) Cultura 4) Rock 5) Tecnologia */

/* Strutture */
typedef struct {
    char domanda[MAX_DOMANDA];
    char risposta[MAX_RISPOSTA];
} Domanda;

typedef struct {
    char nickname[MAX_NICKNAME];
    int  punteggi[N_TEMI];
    int  temi_completati[N_TEMI];
} Giocatore;

/* Avvio server */
void esegui_server(void);

#endif
