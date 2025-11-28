#ifndef PROTOCOLLO_H
#define PROTOCOLLO_H

#define MAX_CLIENT 10
#define MAX_NICKNAME 50
#define MAX_DOMANDA 256
#define MAX_RISPOSTA 128
#define MAX_LINE 512
#define NUM_DOMANDE 5
#define N_TEMI 4

#define PORTA_DEFAULT 12345


#define REGISTRA 1
#define ACCEDI 2
#define INIZIA_QUIZ 3
#define MOSTRA_PUNTEGGI 4
#define TERMINA_QUIZ 5
#define ESCI 6

// Struttura per memorizzare lo stato di un giocatore
typedef struct {
    char nickname[MAX_NICKNAME];
    int punteggi[N_TEMI]; // Un punteggio per ogni tema
    int temi_completati[N_TEMI]; // Flag per ogni tema
} Giocatore;


struct temi {
    char nome[MAX_NICKNAME];
    int svolto;
};

typedef struct {
    char domanda[MAX_DOMANDA];
    char risposta[MAX_RISPOSTA];
} Domanda;

#endif