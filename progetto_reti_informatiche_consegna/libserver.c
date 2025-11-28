#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>  
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>

#include "libserver.h"

/* file utilizzati */
#define CLASSIFICA_FILE "classifica.txt"
#define CLIENTI_FILE    "clienti_correnti.txt"

#define DOMANDE_MITI_FILE        "domande/domande_miti.txt"
#define DOMANDE_DOCTORWHO_FILE   "domande/domande_doctor_who.txt"
#define DOMANDE_CULTURA_FILE     "domande/domande_cultura.txt"
#define DOMANDE_ROCK_FILE        "domande/domande_rock.txt"
#define DOMANDE_TECNO_FILE       "domande/domande_tecnologia.txt"


static void trim_nl(char *s) {
    if (!s) return;
    s[strcspn(s, "\r\n")] = '\0';
}

/* Caricamento domande */
static void carica_domande(Domanda *arr, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Errore apertura file domande");
        exit(EXIT_FAILURE);
    }
/* Ogni domanda è seguita dalla risposta in una riga successiva */
    char line[MAX_LINE];
    int i = 0;
    while (i < NUM_DOMANDE) {
        if (!fgets(line, sizeof(line), fp)) break;
        trim_nl(line);
        snprintf(arr[i].domanda, MAX_DOMANDA, "%.*s", (int)(MAX_DOMANDA - 1), line);

        if (!fgets(line, sizeof(line), fp)) break;
        trim_nl(line);
        snprintf(arr[i].risposta, MAX_RISPOSTA, "%.*s", (int)(MAX_RISPOSTA - 1), line);
        i++;
    }
    fclose(fp);
}


/* Carica la classifica da file (nickname, punteggi, temi completati) */
static void carica_giocatori(Giocatore giocatori[], int *num_giocatori) {
    FILE *fp = fopen(CLASSIFICA_FILE, "r");
    if (!fp) { *num_giocatori = 0; return; }

    int i = 0;
    while (i < MAX_CLIENT) {
        if (fscanf(fp, "%49s", giocatori[i].nickname) != 1) break;

        for (int j = 0; j < N_TEMI; j++) {
            if (fscanf(fp, "%d", &giocatori[i].punteggi[j]) != 1)
                giocatori[i].punteggi[j] = 0;
        }
        for (int j = 0; j < N_TEMI; j++) {
            if (fscanf(fp, "%d", &giocatori[i].temi_completati[j]) != 1)
                giocatori[i].temi_completati[j] = 0;
        }
        i++;
    }
    *num_giocatori = i;
    fclose(fp);
}

/* Salva i dati aggiornati dei giocatori su file */
static void salva_giocatori(const Giocatore giocatori[], int num_giocatori) {
    FILE *fp = fopen(CLASSIFICA_FILE, "w");
    if (!fp) { perror("Errore salvataggio classifica"); return; }
    for (int i = 0; i < num_giocatori; i++) {
        fprintf(fp, "%s", giocatori[i].nickname);
        for (int j = 0; j < N_TEMI; j++) fprintf(fp, " %d", giocatori[i].punteggi[j]);
        for (int j = 0; j < N_TEMI; j++) fprintf(fp, " %d", giocatori[i].temi_completati[j]);
        fprintf(fp, "\n");
    }
    fclose(fp);
}

/* Trova un giocatore già registrato */
static Giocatore* trova_giocatore(const char *nickname, Giocatore giocatori[], int num_giocatori) {
    for (int i = 0; i < num_giocatori; i++) {
        if (strcmp(giocatori[i].nickname, nickname) == 0) return &giocatori[i];
    }
    return NULL;
}

/* Aggiunge un nuovo giocatore se non esiste */
static int aggiungi_giocatore(const char *nickname, Giocatore giocatori[], int *num_giocatori) {
    if (*num_giocatori >= MAX_CLIENT) return -1;
    Giocatore nuovo;
    snprintf(nuovo.nickname, MAX_NICKNAME, "%s", nickname);
    for (int i = 0; i < N_TEMI; i++) {
        nuovo.punteggi[i] = 0;
        nuovo.temi_completati[i] = 0;
    }
    giocatori[*num_giocatori] = nuovo;
    (*num_giocatori)++;
    salva_giocatori(giocatori, *num_giocatori);
    return 0;
}

/* Aggiorna il punteggio di un giocatore dopo un quiz */
static void aggiorna_punteggio(const char *nickname, int tema, int punteggio,
                               Giocatore giocatori[], int num_giocatori) {
    Giocatore *g = trova_giocatore(nickname, giocatori, num_giocatori);
    if (g) {
        g->punteggi[tema] += punteggio;
        g->temi_completati[tema] = 1;
        salva_giocatori(giocatori, num_giocatori);
    }
}


/* Funzione di confronto per ordinare la classifica */
static int punteggio_totale(const Giocatore *g) {
    int tot = 0;
    for (int i = 0; i < N_TEMI; i++) tot += g->punteggi[i];
    return tot;
}

static int cmp_classifica(const void *a, const void *b) {
    const Giocatore *g1 = (const Giocatore *)a;
    const Giocatore *g2 = (const Giocatore *)b;
    return punteggio_totale(g2) - punteggio_totale(g1); 
}

/* connessione dei client*/

static void clienti_aggiungi(const char *nickname) {
    /* evita duplicati */
    FILE *r = fopen(CLIENTI_FILE, "r");
    char name[MAX_NICKNAME];
    if (r) {
        while (fscanf(r, "%49s", name) == 1) {
            if (strcmp(name, nickname) == 0) { fclose(r); return; }
        }
        fclose(r);
    }
    FILE *a = fopen(CLIENTI_FILE, "a");
    if (!a) return;
    fprintf(a, "%s\n", nickname);
    fclose(a);
}

static void clienti_rimuovi(const char *nickname) {
    FILE *r = fopen(CLIENTI_FILE, "r");
    if (!r) return;
    char tutti[MAX_CLIENT][MAX_NICKNAME];
    int n = 0;
    while (n < MAX_CLIENT && fscanf(r, "%49s", tutti[n]) == 1) n++;
    fclose(r);

    FILE *w = fopen(CLIENTI_FILE, "w");
    if (!w) return;
    for (int i = 0; i < n; i++) {
        if (strcmp(tutti[i], nickname) != 0) fprintf(w, "%s\n", tutti[i]);
    }
    fclose(w);
}

/* stampa lato server */
static void stampa_server_status(void) {
    /* client connessi */
    FILE *fp = fopen(CLIENTI_FILE, "r");
    char nomi[MAX_CLIENT][MAX_NICKNAME];
    int n = 0;
    if (fp) {
        while (n < MAX_CLIENT && fscanf(fp, "%49s", nomi[n]) == 1) n++;
        fclose(fp);
    }

    printf("\n[Server] Client connessi (%d): ", n);
    for (int i = 0; i < n; i++) printf("%s%s", nomi[i], (i < n - 1) ? ", " : "");
    if (n == 0) printf("-");
    printf("\n[Server] Temi disponibili: Mitologia, Doctor Who, Cultura Generale, Rock, Tecnologia\n");

    /* Classifica */
    Giocatore arr[MAX_CLIENT];
    int ng = 0;
    carica_giocatori(arr, &ng);
    Giocatore copia[MAX_CLIENT];
    memcpy(copia, arr, sizeof(Giocatore) * ng);
    qsort(copia, ng, sizeof(Giocatore), cmp_classifica);

    printf("[Server] Classifica attuale:\n");
    for (int i = 0; i < ng; i++) {
        printf("%d) %.*s | Totale:%d | Mitologia:%d  DoctorWho:%d  Cultura:%d  Rock:%d  Tecnologia:%d\n",
               i + 1, MAX_NICKNAME - 1, copia[i].nickname,
               punteggio_totale(&copia[i]),
               copia[i].punteggi[0], copia[i].punteggi[1],
               copia[i].punteggi[2], copia[i].punteggi[3], copia[i].punteggi[4]);
    }
    if (ng == 0) printf("(nessun giocatore registrato)\n");
    printf("----------------------------------------------------\n");
}

/* invio classifica */
static void invia_classifica_al_client(int sock) {
    Giocatore arr[MAX_CLIENT];
    int n = 0;
    carica_giocatori(arr, &n);

    Giocatore copia[MAX_CLIENT];
    memcpy(copia, arr, sizeof(Giocatore) * n);
    qsort(copia, n, sizeof(Giocatore), cmp_classifica);

    char msg[8192] = "\n--- CLASSIFICA ---\n";
    char temp[512];
    for (int i = 0; i < n; i++) {
        snprintf(temp, sizeof(temp),
                 "%d) %.*s | Totale:%d | Mitologia:%d  DoctorWho:%d  Cultura:%d  Rock:%d  Tecnologia:%d\n",
                 i + 1, MAX_NICKNAME - 1, copia[i].nickname,
                 punteggio_totale(&copia[i]),
                 copia[i].punteggi[0], copia[i].punteggi[1],
                 copia[i].punteggi[2], copia[i].punteggi[3], copia[i].punteggi[4]);
        strncat(msg, temp, sizeof(msg) - strlen(msg) - 1);
    }
    send(sock, msg, strlen(msg), 0);
}


static void invia_menu_principale(int sock) {
    const char *menu =
        "\nMenu:\n"
        "1) Avvia quiz\n"
        "2) Classifica\n"
        "3) Esci\n"
        "Scelta: ";
    send(sock, menu, strlen(menu), 0);
}

static void invia_menu_temi(int sock) {
    const char *menu_temi =
        "Temi:\n"
        "1) Mitologia\n"
        "2) Doctor Who\n"
        "3) Cultura Generale\n"
        "4) Rock\n"
        "5) Tecnologia\n"
        "Scelta: ";
    send(sock, menu_temi, strlen(menu_temi), 0);
}


static void gestisci_connessione(int new_socket,
                                 Domanda domande_miti[],
                                 Domanda domande_doctorwho[],
                                 Domanda domande_cultura[],
                                 Domanda domande_rock[],
                                 Domanda domande_tecnologia[]) {

    char buffer[MAX_LINE];
    char nickname[MAX_NICKNAME];

    /* Registrazione/ri-accesso in base allo stato del nickname */
    {
        const char *ask = "Inserisci nickname: ";
        send(new_socket, ask, strlen(ask), 0);
    }
    int len = recv(new_socket, nickname, sizeof(nickname) - 1, 0);
    if (len <= 0) { close(new_socket); return; }
    nickname[len] = '\0';
    trim_nl(nickname);

    /* Carico classifica e controllo stato nickname */
    Giocatore arr[MAX_CLIENT];
    int n = 0;
    carica_giocatori(arr, &n);

    Giocatore *g_esistente = trova_giocatore(nickname, arr, n);
    if (g_esistente) {
        /* Verifica se ha già completato tutti i temi */
        int tutti_fatti = 1;
        for (int i = 0; i < N_TEMI; i++) {
            if (!g_esistente->temi_completati[i]) {
                tutti_fatti = 0;
                break;
            }
        }
        if (tutti_fatti) {
            const char *msg = "Hai già completato tutti i quiz. Connessione chiusa.\n";
            send(new_socket, msg, strlen(msg), 0);
            close(new_socket);
            return;
        } else {
            
            clienti_aggiungi(nickname);
            stampa_server_status();
            const char *ok = "Bentornato! Riprendi da dove avevi lasciato.\n";
            send(new_socket, ok, strlen(ok), 0);
        }
    } else {
        /* Nuova registrazione */
        if (aggiungi_giocatore(nickname, arr, &n) < 0) {
            const char *msg = "Server pieno. Connessione chiusa.\n";
            send(new_socket, msg, strlen(msg), 0);
            close(new_socket);
            return;
        }
        clienti_aggiungi(nickname);
        stampa_server_status();
        const char *ok = "Registrazione completata!\n";
        send(new_socket, ok, strlen(ok), 0);
    }

    /* Menu principale */
    int continua = 1;
    while (continua) {
        invia_menu_principale(new_socket);
        len = recv(new_socket, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) break;
        buffer[len] = '\0';
        trim_nl(buffer);

        int choice = atoi(buffer);
        if (choice == 1) {
            invia_menu_temi(new_socket);
            len = recv(new_socket, buffer, sizeof(buffer) - 1, 0);
            if (len <= 0) {
                clienti_rimuovi(nickname);
                close(new_socket);
                stampa_server_status();
                return;
            }

            buffer[len] = '\0';
            trim_nl(buffer);
            int tema = atoi(buffer);
            if (tema < 1 || tema > 5) {
                const char *errt = "Scelta tema non valida.\n";
                send(new_socket, errt, strlen(errt), 0);
                invia_menu_temi(new_socket);
                continue;
            }

            /* ricarico giocatore aggiornato */
            carica_giocatori(arr, &n);
            Giocatore *g = trova_giocatore(nickname, arr, n);
            if (!g) break;

            if (g->temi_completati[tema - 1]) {
                const char *done = "Tema già completato.\n";
                send(new_socket, done, strlen(done), 0);
                continue;
            }

            Domanda *domande_tema = NULL;
            if (tema == 1) domande_tema = domande_miti;
            if (tema == 2) domande_tema = domande_doctorwho;
            if (tema == 3) domande_tema = domande_cultura;
            if (tema == 4) domande_tema = domande_rock;
            if (tema == 5) domande_tema = domande_tecnologia;

            int punti = 0;
            int interrotto = 0;

            for (int i = 0; i < NUM_DOMANDE; i++) {
                char qbuf[MAX_LINE + MAX_DOMANDA];
                snprintf(qbuf, sizeof(qbuf),
                         "\nDomanda %d: %s\n(Rispondi, oppure 'endquiz' per uscire, 'show score' per classifica)\nRisposta: ",
                         i + 1, domande_tema[i].domanda);
                send(new_socket, qbuf, strlen(qbuf), 0);

                len = recv(new_socket, buffer, sizeof(buffer) - 1, 0);
                if (len <= 0) { interrotto = 1; break; }
                buffer[len] = '\0';
                trim_nl(buffer);

                if (strcasecmp(buffer, "endquiz") == 0) {
                    const char *quitmsg = "Quiz interrotto. Torno al menu.\n";
                    send(new_socket, quitmsg, strlen(quitmsg), 0);
                    interrotto = 1;
                    break;
                }
                if (strcasecmp(buffer, "show score") == 0) {
                    invia_classifica_al_client(new_socket);
                    i--; /* ripeti la stessa domanda */
                    continue;
                }

                if (strcasecmp(buffer, domande_tema[i].risposta) == 0) {
                    const char *ok = "Corretto!\n";
                    punti++;
                    send(new_socket, ok, strlen(ok), 0);
                } else {
                   
                    char sb[512];
                    const char *prefix = "Sbagliato! Risposta: ";
                    size_t maxcopy = sizeof(sb) - strlen(prefix) - 2; // '\n' + '\0'
                    if (maxcopy > (size_t)(MAX_RISPOSTA - 1)) {
                        maxcopy = (size_t)(MAX_RISPOSTA - 1);
                    }
                    int w = snprintf(sb, sizeof(sb), "%s%.*s\n",
                                     prefix, (int)maxcopy, domande_tema[i].risposta);
                    if (w < 0) sb[0] = '\0';
                    send(new_socket, sb, strlen(sb), 0);
                }
            }

            if (!interrotto) {
                /* aggiorno punteggio e segno il tema completo */
                carica_giocatori(arr, &n);
                aggiorna_punteggio(nickname, tema - 1, punti, arr, n);
                char pmsg[64];
                snprintf(pmsg, sizeof(pmsg), "\nPunteggio: %d/%d\n", punti, NUM_DOMANDE);
                send(new_socket, pmsg, strlen(pmsg), 0);
                stampa_server_status();
            }
        }
        else if (choice == 2) {
            invia_classifica_al_client(new_socket);
        }
        else if (choice == 3) {
            continua = 0;
        }
        else {
            const char *err = "Scelta non valida.\n";
            send(new_socket, err, strlen(err), 0);
            invia_menu_principale(new_socket);   // ristampa subito il menu
            continue;
        }
    }

    clienti_rimuovi(nickname);
    close(new_socket);
    stampa_server_status();
}

/* ====== SERVER MAIN  ====== */
void esegui_server(void) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int opt = 1;

    Domanda domande_miti[NUM_DOMANDE], domande_doctorwho[NUM_DOMANDE],
            domande_cultura[NUM_DOMANDE], domande_rock[NUM_DOMANDE],
            domande_tecnologia[NUM_DOMANDE];

    carica_domande(domande_miti,       DOMANDE_MITI_FILE);
    carica_domande(domande_doctorwho,  DOMANDE_DOCTORWHO_FILE);
    carica_domande(domande_cultura,    DOMANDE_CULTURA_FILE);
    carica_domande(domande_rock,       DOMANDE_ROCK_FILE);
    carica_domande(domande_tecnologia, DOMANDE_TECNO_FILE);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORTA_DEFAULT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("[Server] In ascolto sulla porta %d...\n", PORTA_DEFAULT);
    stampa_server_status();

    /* evita zombie */
    signal(SIGCHLD, SIG_IGN);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }
        pid_t pid = fork();
        if (pid == 0) {
            /* FIGLIO */
            close(server_fd);
            gestisci_connessione(new_socket,
                                 domande_miti, domande_doctorwho,
                                 domande_cultura, domande_rock,
                                 domande_tecnologia);
            _exit(0);
        } else if (pid > 0) {
            /* PADRE */
            close(new_socket);
        } else {
            perror("fork");
            close(new_socket);
        }
    }
}

