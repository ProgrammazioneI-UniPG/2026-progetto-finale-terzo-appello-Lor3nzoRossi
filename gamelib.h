#ifndef GAMELIB_H
#define GAMELIB_H
#define DIMENSIONE_ZAINO 3
#define MAX_GIOCATORI 4

// Tipi enumerativi
enum Tipo_zona
{
    BOSCO,
    SCUOLA,
    LABORATORIO,
    CAVERNA,
    STRADA,
    GIARDINO,
    SUPERMERCATO,
    CENTRALE_ELETTRICA,
    DEPOSITO_ABBANDONATO,
    STAZIONE_POLIZIA,
    NUM_TIPI_ZONA // aggiunto per sapere dinamicamente il numero delle zone
};

enum Tipo_nemico
{
    NESSUN_NEMICO,
    BILLI,
    DEMOCANE,
    DEMOTORZONE
};

enum Tipo_oggetto
{
    NESSUN_OGGETTO,
    BICICLETTA,             // permette di avanzare nuovamente, nel caso lo si abbia già fatto nel turno
    MAGLIETTA_FUOCOINFERNO, // aumenta difesa psichica per la durata del combattimento (può essere usata prima o durante il combattimento, se la si usa prima e poi non si combatte nessun nemico, viene comunque considerata usata)
    BUSSOLA,                // permete di stampare la mappa del mondo in cui ci si trova
    SCHITARRATA_METALLICA,   // aumenta attacco psichico con le stesse condizioni della difesa
    CROCE //permette di rianimare un giocatore se nello stesso ciclo di turni si raggiunge la zona in cui è morto
};

// aggiunta per comodità
enum Tipo_mondo
{
    MONDO_REALE,
    SOPRA_SOTTO
};

// struct

struct Zona_soprasotto;

struct Zona_mondoreale
{
    enum Tipo_zona tipo;
    enum Tipo_nemico nemico;
    enum Tipo_oggetto oggetto;

    struct Zona_mondoreale *avanti;
    struct Zona_mondoreale *indietro;

    struct Zona_soprasotto *link_soprasotto;
    struct Giocatore *giocatori_morti[MAX_GIOCATORI];
};

struct Zona_soprasotto
{
    enum Tipo_zona tipo;
    enum Tipo_nemico nemico;

    struct Zona_soprasotto *avanti;
    struct Zona_soprasotto *indietro;

    struct Zona_mondoreale *link_mondoreale;
    struct Giocatore *giocatori_morti[MAX_GIOCATORI];
};

struct Giocatore
{
    char nome[20];
    enum Tipo_mondo mondo;
    int avanzato; // aggiunto per tenere traccia dello stato di avanzamento nel turno
    struct Zona_mondoreale *pos_mondoreale;
    struct Zona_soprasotto *pos_soprasotto;

    int vita; // aggiunto per tenere traccia della vita dei giocatori
    int attacco_pischico;
    int difesa_pischica;
    int fortuna;

    enum Tipo_oggetto zaino[DIMENSIONE_ZAINO];
};

// dichiarazione delle funzioni
void imposta_gioco();
void gioca();
void termina_gioco();
void crediti();
void stampa_regole();

#endif
