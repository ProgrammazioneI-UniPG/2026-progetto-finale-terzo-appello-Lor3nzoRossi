#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gamelib.h"

// variabili globali
struct Giocatore *giocatori[MAX_GIOCATORI];
struct Giocatore *giocatori_vincitori[3];
struct Giocatore *giocatori_morti[MAX_GIOCATORI];
struct Zona_mondoreale *prima_zona_mondoreale;
struct Zona_soprasotto *prima_zona_soprasotto;
int numero_giocatori = 0;
int partite_giocate = 0;
int giocatore_vincitore = 0;
int mappa_chiusa = 0;
int gioco_impostato = 0;

static void stampa_separatore()
{
    printf("\n=======================\n");
}

void stampa_regole()
{
    printf("=== REGOLE DEL GIOCO ===\n\n");

    printf(
        "Inizialmente il game master impostera' i giocatori, la mappa di gioco, e scegliera' quando iniziare la partita.\n\n"
        "I giocatori giocano a turno. Durante il proprio turno e' possibile eseguire diverse azioni tra:\n\n"
        "1) Avanza: sposta il giocatore nella zona successiva del mondo in cui si trova, a patto che riesca a sconfiggere il nemico nella zona, se presente.\n"
        "2) Indietreggia: sposta il giocatore nella zona precedente, a patto che riesca a sconfiggere il nemico nella zona, se presente.\n"
        "3) Cambia mondo: permette di passare dal Mondo Reale al Soprasotto o viceversa. Nel caso in cui dal Mondoreale si voglia passare al Soprasotto, prima sara' necessario sconfiggere il nemico nella zona, se presente.\n"
        "4) Combatti: permette di combattere il nemico nella zona, se presente. Se il nemico viene sconfitto, c'e' una possibilita' del 50%% che scompaia dalla zona.\n"
        "5) Stampa giocatore: visualizza le statistiche del tuo personaggio.\n"
        "6) Stampa zona: visualizza informazioni sulla zona corrente.\n"
        "7) Raccogli oggetto: permette di raccogliere l'oggetto nella zona, se prima si e' sconfitto il nemico nella stessa. Non puoi raccogliere oggetti se lo zaino e' pieno.\n"
        "8) Utilizza oggetto: puoi usare un oggetto presente nello zaino. E' possibile utilizzare un oggetto durante il turno e prima di un combattimento, in ogni caso alla fine del turno corrente gli eventuali attributi ottenuti, verranno ripristinati\n"
        "9) Passa turno: termina il turno e lo passa al prossimo giocatore.\n\n"
        "Nel caso delle azioni eseguibili solo dopo aver sconfitto il nemico della zona, non sara' strettamente necessario eseguire l'opzione 4 manualmente, ma il combattimento verra' eseguito automaticamente e in caso di successo, eseguita l'azione desiderata.\n\n"
        "Gli effetti associati a ciascun oggetto sono:\n"
        "2) BICICLETTA: permette di avanzare nuovamente, nel caso lo si abbia gia' fatto nel turno.\n"
        "3) MAGLIETTA_FUOCOINFERNO: aumenta la difesa psichica per la durata del combattimento. Puo' essere usata prima o durante il combattimento; se usata prima e poi non si combatte nessun nemico, viene comunque considerata usata.\n"
        "4) BUSSOLA: permette di stampare la mappa del mondo in cui ci si trova.\n"
        "5) SCHITARRATA_METALLICA: aumenta l'attacco psichico con le stesse condizioni della difesa (durante il combattimento o se usata prima).\n"
        "6) CROCE: permette di rianimare un giocatore se nello stesso ciclo di turni si raggiunge la zona in cui e' morto.\n\n"
        "FINE DELLA PARTITA\n"
        "Una partita finisce se:\n"
        "- un giocatore vince, sconfiggendo per primo il DEMOTORZONE\n"
        "- tutti i giocatori muoiono.\n\n"
        "Alla fine del gioco, e' possibile reimpostare il gioco per una nuova partita.\n");
}

static const char *nome_zona(enum Tipo_zona zona)
{
    switch (zona)
    {
    case BOSCO:
        return "BOSCO";
    case SCUOLA:
        return "SCUOLA";
    case LABORATORIO:
        return "LABORATORIO";
    case CAVERNA:
        return "CAVERNA";
    case STRADA:
        return "STRADA";
    case GIARDINO:
        return "GIARDINO";
    case SUPERMERCATO:
        return "SUPERMERCATO";
    case CENTRALE_ELETTRICA:
        return "CENTRALE ELETTRICA";
    case DEPOSITO_ABBANDONATO:
        return "DEPOSITO ABBANDONATO";
    case STAZIONE_POLIZIA:
        return "STAZIONE POLIZIA";
    default:
        return "NON RICONOSCIUTO";
    }
}

static const char *nome_nemico(enum Tipo_nemico nemico)
{
    switch (nemico)
    {
    case NESSUN_NEMICO:
        return "NESSUN NEMICO";
    case BILLI:
        return "BILLI";
    case DEMOCANE:
        return "DEMOCANE";
    case DEMOTORZONE:
        return "DEMOTORZONE";
    default:
        return "NON RICONOSCIUTO";
    }
}

static const char *nome_oggetto(enum Tipo_oggetto oggetto)
{
    switch (oggetto)
    {
    case NESSUN_OGGETTO:
        return "NESSUN OGGETTO";
    case BICICLETTA:
        return "BICICLETTA";
    case MAGLIETTA_FUOCOINFERNO:
        return "MAGLIETTA FUOCOINFERNO";
    case BUSSOLA:
        return "BUSSOLA";
    case SCHITARRATA_METALLICA:
        return "SCHITARRATA METALLICA";
    case CROCE:
        return "CROCE";
    default:
        return "NON RICONOSCIUTO";
    }
}

/**
 * Controlla se uno zaino contiene oggetti
 * Ritorna:
 * - 1: se contiene oggetti
 * - 0: se è vuoto
 */
static int controlla_zaino(enum Tipo_oggetto *zaino)
{
    for (int i = 0; i < DIMENSIONE_ZAINO; i++)
    {
        if (zaino[i] != NESSUN_OGGETTO)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * Controlla se nella zona in cui è presente il giocatore ci sono giocatori morti
 * Ritorna il numero di giocatori morti nella zona
 */
static int conta_giocatori_morti(struct Giocatore *giocatore, struct Giocatore **giocatori_morti_zona)
{
    int count = 0;
    struct Giocatore **zona_morti = NULL;

    if (giocatore->mondo == MONDO_REALE && giocatore->pos_mondoreale)
    {
        zona_morti = giocatore->pos_mondoreale->giocatori_morti;
    }
    else if (giocatore->mondo == SOPRA_SOTTO && giocatore->pos_soprasotto)
    {
        zona_morti = giocatore->pos_soprasotto->giocatori_morti;
    }

    if (zona_morti == NULL)
        return 0;

    for (int i = 0; i < MAX_GIOCATORI; i++)
    {
        if (zona_morti[i] != NULL)
        {
            if (giocatori_morti_zona != NULL)
                giocatori_morti_zona[count] = zona_morti[i];
            count++;
        }
    }

    return count;
}

/**
 * Stampa gli oggetti presenti in uno zaino
 */
static void stampa_zaino(enum Tipo_oggetto zaino[])
{
    if (!controlla_zaino(zaino))
    {
        printf("Zaino vuoto\n");
        return;
    }

    printf("Oggetti nello zaino:\n");
    for (int i = 0; i < DIMENSIONE_ZAINO; i++)
    {
        if (zaino[i] != NESSUN_OGGETTO)
        {
            printf("%d) %s\n", zaino[i], nome_oggetto(zaino[i]));
        }
    }
}

/**
 * Stampa i dati del giocatore
 */
static void stampa_giocatore(struct Giocatore *giocatore)
{

    if (!giocatore)
    {
        printf("Giocatore non passato.\n");
        return;
    }

    printf("\n== Attributi del giocatore: %s ==\n", giocatore->nome);
    printf("Mondo attuale: %s\n", giocatore->mondo == MONDO_REALE ? "Mondo Reale" : "Soprasotto");

    printf("Punti vita: %d\n", giocatore->vita);
    printf("Attacco pischico: %d\n", giocatore->attacco_pischico);
    printf("Difesa pischica: %d\n", giocatore->difesa_pischica);
    printf("Fortuna: %d\n", giocatore->fortuna);

    stampa_zaino(giocatore->zaino);

    printf("\n");
}

/**
 * Conto le zone della mappa del mondo reale (il numero di quelle del soprasotto è lo stesso)
 */
static int conta_zone_mappa()
{
    int count = 0;
    struct Zona_mondoreale *current = prima_zona_mondoreale;

    while (current)
    {
        count++;
        current = current->avanti;
    }

    return count;
}

/**
 * Verifica che ci sia un solo un demotorzone nella mappa del soprasotto
 * Ritorna:
 * - 1: se c'è un solo demotorzone nella mappa del soprasotto
 * - 2: se non è presente o ci sono più di un demotorzone nella mappa del soprasotto
 */
static int verifica_demotorzone()
{

    int count = 0;
    struct Zona_soprasotto *current = prima_zona_soprasotto;

    while (current)
    {
        if (current->nemico == DEMOTORZONE)
            count++;

        if (count > 1)
            return 0;

        current = current->avanti;
    }

    return count == 1;
}

void libera_giocatori()
{

    // rimuovi tutti i giocatori
    for (int i = 0; i < numero_giocatori; i++)
    {
        if (giocatori[i] != NULL)
        {
            free(giocatori[i]);
            giocatori[i] = NULL;
        }

        if (giocatori_morti[i] != NULL)
        {
            free(giocatori_morti[i]);
            giocatori_morti[i] = NULL;
        }
    }
}

/**
 * Libera la memoria della mappe del mondoreale e del soprasotto
 */
static void libera_mappe()
{
    // Libero la mappa del Mondo Reale
    struct Zona_mondoreale *current_mondo_reale = prima_zona_mondoreale;
    while (current_mondo_reale != NULL)
    {
        struct Zona_mondoreale *next = current_mondo_reale->avanti;
        free(current_mondo_reale);
        current_mondo_reale = next;
    }
    prima_zona_mondoreale = NULL;

    // Libero la mappa del Soprasotto
    struct Zona_soprasotto *current_sopra_sotto = prima_zona_soprasotto;
    while (current_sopra_sotto != NULL)
    {
        struct Zona_soprasotto *next = current_sopra_sotto->avanti;
        free(current_sopra_sotto);
        current_sopra_sotto = next;
    }
    prima_zona_soprasotto = NULL;
}

static void genera_mappa()
{
    printf("Inizio generazione mappa...\n");

    // libera memoria di eventuali mappe precedenti
    libera_mappe();

    struct Zona_mondoreale *indietro_mondo_reale = NULL;
    struct Zona_soprasotto *indietro_sopra_sotto = NULL;
    int demotorzone_presente = 0;
    int zone_da_creare = 15;

    for (int i = 0; i < zone_da_creare; i++)
    {
        // creazione mappa del mondo reale
        struct Zona_mondoreale *zona_mondoreale = malloc(sizeof(struct Zona_mondoreale));
        zona_mondoreale->tipo = rand() % NUM_TIPI_ZONA;

        int r = rand() % 100;
        if (r < 20)
            zona_mondoreale->nemico = NESSUN_NEMICO;
        else if (r < 70)
            zona_mondoreale->nemico = DEMOCANE;
        else
            zona_mondoreale->nemico = BILLI;

        r = rand() % 100;
        if (r < 30)
            zona_mondoreale->oggetto = NESSUN_OGGETTO;
        else if (r < 50)
            zona_mondoreale->oggetto = BICICLETTA;
        else if (r < 70)
            zona_mondoreale->oggetto = MAGLIETTA_FUOCOINFERNO;
        else if (r < 80)
            zona_mondoreale->oggetto = BUSSOLA;
        else if (r < 90)
            zona_mondoreale->oggetto = SCHITARRATA_METALLICA;
        else
            zona_mondoreale->oggetto = CROCE;

        zona_mondoreale->avanti = NULL;
        zona_mondoreale->indietro = indietro_mondo_reale;
        zona_mondoreale->link_soprasotto = NULL;
        // In genera_mappa, dopo malloc di zona_mondoreale:
        for (int j = 0; j < MAX_GIOCATORI; j++)
        {
            zona_mondoreale->giocatori_morti[j] = NULL;
        }

        if (indietro_mondo_reale)
            indietro_mondo_reale->avanti = zona_mondoreale;
        else
            prima_zona_mondoreale = zona_mondoreale;
        indietro_mondo_reale = zona_mondoreale;

        // creazione mappa del soprasotto
        struct Zona_soprasotto *zona_soprasotto = malloc(sizeof(struct Zona_soprasotto));
        zona_soprasotto->tipo = zona_mondoreale->tipo;
        zona_soprasotto->avanti = NULL;
        zona_soprasotto->indietro = indietro_sopra_sotto;
        // E per zona_soprasotto:
        for (int j = 0; j < MAX_GIOCATORI; j++)
        {
            zona_soprasotto->giocatori_morti[j] = NULL;
        }

        int r2 = rand() % 100;

        if (demotorzone_presente)
        {
            zona_soprasotto->nemico = r2 > 50 ? DEMOCANE : NESSUN_NEMICO;
        }
        else
        {
            if (r2 < 45)
                zona_soprasotto->nemico = NESSUN_NEMICO;
            else if (r2 < 90)
                zona_soprasotto->nemico = DEMOCANE;
            else
            {
                zona_soprasotto->nemico = DEMOTORZONE;
                demotorzone_presente = 1;
            }
        }

        if (i == zone_da_creare - 1 && !demotorzone_presente)
            zona_soprasotto->nemico = DEMOTORZONE;

        zona_soprasotto->link_mondoreale = zona_mondoreale;
        zona_mondoreale->link_soprasotto = zona_soprasotto;

        if (indietro_sopra_sotto)
            indietro_sopra_sotto->avanti = zona_soprasotto;
        else
            prima_zona_soprasotto = zona_soprasotto;
        indietro_sopra_sotto = zona_soprasotto;
    }

    // imposto la posizione di tutti i giocatori alla prima zona del mondo reale
    for (int i = 0; i < numero_giocatori; i++)
    {
        giocatori[i]->pos_mondoreale = prima_zona_mondoreale;
    }

    printf("=== Generazione della mappa completata. ===\n");
}

static void stampa_zona_giocatore(struct Giocatore *giocatore)
{
    if (!giocatore)
    {
        printf("Giocatore non valido.\n");
        return;
    }

    if (giocatore->mondo == MONDO_REALE)
    { // Mondo Reale
        if (!giocatore->pos_mondoreale)
        {
            printf("Il giocatore non si trova in una zona del Mondo Reale.\n");
            return;
        }
        printf("Zona attuale del mondo reale:\n");
        printf("Tipo: %s\n", nome_zona(giocatore->pos_mondoreale->tipo));
        printf("Nemico: %s\n", nome_nemico(giocatore->pos_mondoreale->nemico));
        printf("Oggetto: %s\n", nome_oggetto(giocatore->pos_mondoreale->oggetto));

        printf("\n");

        if (giocatore->pos_mondoreale->link_soprasotto)
        {
            printf("Collegamento con il soprasotto:\n");
            printf("Tipo: %s\n", nome_zona(giocatore->pos_mondoreale->link_soprasotto->tipo));
            printf("Nemico: %s\n", nome_nemico(giocatore->pos_mondoreale->link_soprasotto->nemico));
        }
    }
    else if (giocatore->mondo == SOPRA_SOTTO)
    { // Soprasotto
        if (!giocatore->pos_soprasotto)
        {
            printf("Il giocatore non si trova in una zona del Soprasotto.\n");
            return;
        }
        printf("Zona attuale del soprasotto:\n");
        printf("Tipo: %s\n", nome_zona(giocatore->pos_soprasotto->tipo));
        printf("Nemico: %s\n", nome_nemico(giocatore->pos_soprasotto->nemico));

        printf("\n");

        if (giocatore->pos_soprasotto->link_mondoreale)
        {
            printf("Collegamento con il mondo reale:\n");
            printf("Tipo: %s\n", nome_zona(giocatore->pos_soprasotto->link_mondoreale->tipo));
            printf("Nemico: %s\n", nome_nemico(giocatore->pos_soprasotto->link_mondoreale->nemico));
            printf("Oggetto: %s\n", nome_oggetto(giocatore->pos_soprasotto->link_mondoreale->oggetto));
        }
    }
    else
    {
        printf("Mondo non valido.\n");
    }

    stampa_separatore();
}

static void stampa_zona()
{
    int nZone = conta_zone_mappa();
    int posizione;

    if (nZone == 0)
    {
        printf("Impossibile stampare una zona: nessuna zona creata.\n");
        return;
    }

    do
    {
        printf("Inserire la posizione della zona del mondoreale da stampare (1-%d): ", nZone);

        if (scanf("%d", &posizione) != 1)
        {
            printf("Input non valido.\n");
            while (getchar() != '\n')
                ;
            posizione = -1;
            continue;
        }

        while (getchar() != '\n')
            ;

        if (posizione < 1 || posizione > nZone)
            printf("Posizione non valida.\n");

    } while (posizione < 1 || posizione > nZone);

    int i = 1;
    struct Zona_mondoreale *current_zona_mondoreale = prima_zona_mondoreale;

    while (current_zona_mondoreale && i < posizione)
    {
        current_zona_mondoreale = current_zona_mondoreale->avanti;
        i++;
    }

    if (!current_zona_mondoreale)
    {
        printf("Zona non trovata.\n");
        return;
    }

    printf("=== Zona %d ===\n", posizione);
    printf("Mondo Reale:\n");
    printf("  Tipo zona: %s\n", nome_zona(current_zona_mondoreale->tipo));
    printf("  Nemico: %s\n", nome_nemico(current_zona_mondoreale->nemico));
    printf("  Oggetto: %s\n", nome_oggetto(current_zona_mondoreale->oggetto));

    printf("-----------------------------\n");

    if (current_zona_mondoreale->link_soprasotto)
    {
        printf("Soprasotto:\n");
        printf("  Tipo zona: %s\n", nome_zona(current_zona_mondoreale->link_soprasotto->tipo));
        printf("  Nemico: %s\n", nome_nemico(current_zona_mondoreale->link_soprasotto->nemico));
    }
    else
    {
        printf("Link al soprasotto non presente.\n");
    }

    stampa_separatore();
}

/**
 * Stampa l'intera mappa del mondo reale
 */
static void stampa_mondoreale()
{
    if (prima_zona_mondoreale == NULL)
    {
        printf("Nessuna zona presente.\n");
        return;
    }

    printf("\n=== Mappa del Mondo Reale ===\n");
    struct Zona_mondoreale *zona_corrente = prima_zona_mondoreale;
    int count_zone = 1;

    while (zona_corrente != NULL)
    {
        printf("n.%d\n", count_zone);
        printf("  Tipo zona: %s\n", nome_zona(zona_corrente->tipo));
        printf("  Nemico: %s\n", nome_nemico(zona_corrente->nemico));
        printf("  Oggetto: %s\n", nome_oggetto(zona_corrente->oggetto));
        printf("-----------------------------\n");

        zona_corrente = zona_corrente->avanti;
        count_zone++;
    }
}

/**
 * Stampa l'intera mappa del soprasotto
 */
static void stampa_soprasotto()
{
    if (prima_zona_soprasotto == NULL)
    {
        printf("Nessuna zona presente.\n");
        return;
    }

    printf("\n=== Mappa del Soprasotto ===\n");
    struct Zona_soprasotto *zona_corrente = prima_zona_soprasotto;
    int count_zone = 1;

    while (zona_corrente != NULL)
    {
        printf("n.%d\n", count_zone);
        printf("  Tipo zona: %s\n", nome_zona(zona_corrente->tipo));
        printf("  Nemico: %s\n", nome_nemico(zona_corrente->nemico));
        printf("-----------------------------\n");

        zona_corrente = zona_corrente->avanti;
        count_zone++;
    }
}

static void stampa_mappa()
{
    char scelta[12];
    printf("Stampare la mappa del mondo reale o del Soprasotto? (reale/sotto): ");
    fgets(scelta, sizeof(scelta), stdin);
    scelta[strcspn(scelta, "\n")] = '\0';

    if (strcmp(scelta, "reale") == 0)
    {
        stampa_mondoreale();
    }
    else if (strcmp(scelta, "sotto") == 0)
    {
        stampa_soprasotto();
    }
    else
    {
        printf("Scelta non valida.\n");
    }
}

static void inserisci_zona()
{
    int nZone = conta_zone_mappa();
    int posizione;

    if (nZone > 0)
    {
        do
        {
            printf("Inserire la posizione in cui inserire la nuova zona (1-%d): ", nZone + 1);

            if (scanf("%d", &posizione) != 1)
            {
                printf("Input non valido.\n");
                while (getchar() != '\n')
                    ;
                posizione = -1;
                continue;
            }

            while (getchar() != '\n')
                ;

            if (posizione < 1 || posizione > nZone + 1)
                printf("Posizione non valida.\n");

        } while (posizione < 1 || posizione > nZone + 1);
    }
    else
    {
        printf("Inserimento della prima zona\n");
        posizione = 1;
    }

    printf("Creazione della zona nel mondo reale....\n");
    struct Zona_mondoreale *nuova_zona_mondo_reale = malloc(sizeof(struct Zona_mondoreale));
    nuova_zona_mondo_reale->tipo = rand() % NUM_TIPI_ZONA;
    int demotorzone_presente = verifica_demotorzone();
    int oggetto;
    int nemico;
    do
    {
        printf("Scegli il nemico per la zona del Mondo Reale (0=NESSUN_NEMICO, 1=BILLI, 2=DEMOCANE): ");
        if (scanf("%d", &nemico) != 1)
        {
            while (getchar() != '\n')
                ;
            nemico = -1;
            printf("Valore non valido.\n");
            continue;
        }
        while (getchar() != '\n')
            ;
        if (nemico != NESSUN_NEMICO && nemico != BILLI && nemico != DEMOCANE)
            printf("Nemico non valido.\n");

    } while (nemico != NESSUN_NEMICO && nemico != BILLI && nemico != DEMOCANE);

    nuova_zona_mondo_reale->nemico = nemico;

    do
    {
        printf("Scegli l'oggetto (0=NESSUN_OGGETTO, 1=BICICLETTA, 2=MAGLIETTA_FUOCOINFERNO, 3=BUSSOLA, 4=SCHITARRATA_METALLICA, 5=CROCE): ");
        if (scanf("%d", &oggetto) != 1)
        {
            while (getchar() != '\n')
                ;
            oggetto = -1;
            printf("Valore non valido.\n");
            continue;
        }
        while (getchar() != '\n')
            ;
        if (oggetto < NESSUN_OGGETTO || oggetto > CROCE)
            printf("Oggetto non valido.\n");

    } while (oggetto < NESSUN_OGGETTO || oggetto > CROCE);

    nuova_zona_mondo_reale->oggetto = oggetto;

    nuova_zona_mondo_reale->avanti = NULL;
    nuova_zona_mondo_reale->indietro = NULL;
    nuova_zona_mondo_reale->link_soprasotto = NULL;

    // inserimento nella mappa del mondoreale
    if (posizione == 1)
    {
        nuova_zona_mondo_reale->avanti = prima_zona_mondoreale;
        if (prima_zona_mondoreale)
            prima_zona_mondoreale->indietro = nuova_zona_mondo_reale;
        prima_zona_mondoreale = nuova_zona_mondo_reale;

        // reimposto i giocatori nella prima zona
        for (int i = 0; i < numero_giocatori; i++)
        {
            giocatori[i]->pos_mondoreale = nuova_zona_mondo_reale;
        }
    }
    else
    {
        struct Zona_mondoreale *current = prima_zona_mondoreale;
        int i = 1;

        while (current->avanti != NULL && i < posizione - 1)
        {
            current = current->avanti;
            i++;
        }

        nuova_zona_mondo_reale->avanti = current->avanti;
        nuova_zona_mondo_reale->indietro = current;

        if (current->avanti)
            current->avanti->indietro = nuova_zona_mondo_reale;

        current->avanti = nuova_zona_mondo_reale;
    }

    printf("Creazione della zona nel Soprasotto...\n");

    struct Zona_soprasotto *nuova_zona_sopra_sotto = malloc(sizeof(struct Zona_soprasotto));
    nuova_zona_sopra_sotto->tipo = nuova_zona_mondo_reale->tipo;
    nuova_zona_sopra_sotto->avanti = NULL;
    nuova_zona_sopra_sotto->indietro = NULL;
    nuova_zona_sopra_sotto->link_mondoreale = nuova_zona_mondo_reale;
    nuova_zona_mondo_reale->link_soprasotto = nuova_zona_sopra_sotto;

    if (demotorzone_presente)
    {
        do
        {
            printf("Scegli il nemico per il Soprasotto (0=NESSUN_NEMICO, 2=DEMOCANE): ");
            if (scanf("%d", &nemico) != 1)
            {
                while (getchar() != '\n')
                    ;
                nemico = -1;
                printf("Valore non valido.\n");
                continue;
            }
            while (getchar() != '\n')
                ;
            if (nemico != NESSUN_NEMICO && nemico != DEMOCANE)
                printf("Nemico non valido.\n");
        } while (nemico != NESSUN_NEMICO && nemico != DEMOCANE);
    }
    else
    {
        do
        {
            printf("Scegli il nemico per il Soprasotto (0=NESSUN_NEMICO, 2=DEMOCANE, 3=DEMOTORZONE): ");
            if (scanf("%d", &nemico) != 1)
            {
                while (getchar() != '\n')
                    ;
                nemico = -1;
                printf("Valore non valido.\n");
                continue;
            }
            while (getchar() != '\n')
                ;
            if (nemico != NESSUN_NEMICO && nemico != DEMOCANE && nemico != DEMOTORZONE)
                printf("Nemico non valido.\n");
        } while (nemico != NESSUN_NEMICO && nemico != DEMOCANE && nemico != DEMOTORZONE);
    }

    nuova_zona_sopra_sotto->nemico = nemico;

    // inserimento della zona nel sottosopra
    if (posizione == 1)
    {
        nuova_zona_sopra_sotto->avanti = prima_zona_soprasotto;
        if (prima_zona_soprasotto)
            prima_zona_soprasotto->indietro = nuova_zona_sopra_sotto;
        prima_zona_soprasotto = nuova_zona_sopra_sotto;
    }
    else
    {
        struct Zona_soprasotto *current = prima_zona_soprasotto;
        int i = 1;

        while (current->avanti != NULL && i < posizione - 1)
        {
            current = current->avanti;
            i++;
        }

        nuova_zona_sopra_sotto->avanti = current->avanti;
        nuova_zona_sopra_sotto->indietro = current;

        if (current->avanti)
            current->avanti->indietro = nuova_zona_sopra_sotto;

        current->avanti = nuova_zona_sopra_sotto;
    }

    printf("Zona inserita in posizione %d.\n", posizione);
}

static void utilizza_oggetto(struct Giocatore *giocatore)
{

    if (!controlla_zaino(giocatore->zaino))
    {
        printf("Zaino vuoto, nessun oggetto da poter utilizzare.\n");
        return;
    }

    int oggetto = 0;
    int trovato = 0;

    while (trovato == 0)
    {
        printf("Quale oggetto trasportato si vuole utilizzare? (digitare -1 per annullare)\n");
        stampa_zaino(giocatore->zaino);
        scanf("%d", &oggetto);

        if (oggetto == -1)
        {
            printf("Annullamento...\n");
            return;
        }

        for (int i = 0; i <= DIMENSIONE_ZAINO; i++)
        {
            if (giocatore->zaino[i] == oggetto)
                trovato = 1;
        }

        if (!trovato)
            printf("Impossibile utilizzare %s: oggetto non presente nello zaino del giocatore\n", nome_oggetto(oggetto));
    }

    switch (oggetto)
    {
    case NESSUN_OGGETTO:
        printf("Impossibile utilizzare l'oggetto\n");
        break;
    case BICICLETTA:
        printf("== BICICLETTA UTILIZZATA ==\n");
        giocatore->avanzato = 0;
        printf("Il giocatore puo' avanzare nuovamente questo turno.\n");
        break;
    case SCHITARRATA_METALLICA:
        printf("== SCHITARRATA METALLICA UTILIZZATA ==\n");
        giocatore->attacco_pischico += 2;
        printf("Attacco psichico aumentato di 2 punti: %d\n", giocatore->attacco_pischico);
        break;
    case MAGLIETTA_FUOCOINFERNO:
        printf("== MAGLIETTA FUOCOINFERNO UTILIZZATA ==\n");
        giocatore->difesa_pischica += 2;
        printf("Difesa psichica aumentata di 2 punti: %d\n", giocatore->difesa_pischica);
        break;
    case BUSSOLA:
        printf("== BUSSOLA UTILIZZATA ==\n");
        if (giocatore->mondo == MONDO_REALE)
            stampa_mondoreale();
        else
            stampa_soprasotto();
        break;
    case CROCE:
    {
        struct Giocatore *morti_in_zona[MAX_GIOCATORI] = {NULL};
        int num_morti = conta_giocatori_morti(giocatore, morti_in_zona);

        if (num_morti == 0)
        {
            printf("Non ci sono giocatori morti in questa zona.\n");
            break;
        }

        printf("\n=== Giocatori morti nella zona ===\n");
        for (int i = 0; i < num_morti; i++)
        {
            printf("%d) %s\n", i + 1, morti_in_zona[i]->nome);
        }

        int scelta = -1;
        do
        {
            printf("Quale giocatore vuoi rianimare? (0 per annullare): ");
            if (scanf("%d", &scelta) != 1)
            {
                while (getchar() != '\n')
                    ;
                scelta = -1;
                continue;
            }
            while (getchar() != '\n')
                ;

            if (scelta == 0)
            {
                printf("Operazione annullata.\n");
                return; // annullamento senza usare l'oggetto
            }

            if (scelta < 1 || scelta > num_morti)
            {
                printf("Scelta non valida.\n");
                scelta = -1;
            }
        } while (scelta < 0 || scelta > num_morti);

        if (giocatore->vita < 5)
        {
            printf("%s non possiede abbastanza punti vita.\n", giocatore->nome);
            return;
        }

        struct Giocatore *giocatore_morto = morti_in_zona[scelta - 1];

        char conferma;
        printf("Pagare 5 punti vita per rianimare %s? (s/n) ", giocatore_morto->nome);
        scanf(" %c", &conferma);
        while (getchar() != '\n')
            ;

        if (conferma != 's' && conferma != 'S')
        {
            printf("Operazione annullata.\n");
            return; // annullamento senza usare l'oggetto
        }

        giocatore->vita -= 5;

        // rimuovo il giocatore dai giocatori morti
        for (int i = 0; i < MAX_GIOCATORI; i++)
        {
            if (giocatori_morti[i] == giocatore_morto)
            {
                giocatori_morti[i] = NULL;
                break;
            }
        }

        // rimuovo il giocatore morto nella zona
        if (giocatore->mondo == MONDO_REALE && giocatore->pos_mondoreale)
        {
            for (int i = 0; i < MAX_GIOCATORI; i++)
            {
                if (giocatore->pos_mondoreale->giocatori_morti[i] == giocatore_morto)
                {
                    giocatore->pos_mondoreale->giocatori_morti[i] = NULL;
                    break;
                }
            }
        }
        else if (giocatore->mondo == SOPRA_SOTTO && giocatore->pos_soprasotto)
        {
            for (int i = 0; i < MAX_GIOCATORI; i++)
            {
                if (giocatore->pos_soprasotto->giocatori_morti[i] == giocatore_morto)
                {
                    giocatore->pos_soprasotto->giocatori_morti[i] = NULL;
                    break;
                }
            }
        }

        // reinserisco il giocatore nei giocatori vivi
        for (int i = 0; i < MAX_GIOCATORI; i++)
        {
            if (giocatori[i] == NULL)
            {
                giocatori[i] = giocatore_morto;
                numero_giocatori++;
                break;
            }
        }

        giocatore_morto->vita = 15;

        printf("Il giocatore %s e' stato riportato in vita da %s con 15 punti vita\n", giocatore_morto->nome, giocatore->nome);
        printf("Vita di %s: %d", giocatore->nome, giocatore->vita);

        break;
    }

    default:
        printf("Oggetto %d non riconosciuto\n", oggetto);
        break;
    }

    // rimozione dell'oggetto dallo zaino dopo averlo utilizzato
    for (int i = 0; i < DIMENSIONE_ZAINO; i++)
    {
        if (giocatore->zaino[i] == oggetto)
            giocatore->zaino[i] = NESSUN_OGGETTO;
    }
}

static void cancella_zona()
{
    int nZone = conta_zone_mappa();
    int posizione;

    if (nZone == 0)
    {
        printf("Impossibile cancellare una zona: nessuna zona creata.\n");
        return;
    }

    do
    {
        printf("Inserire la posizione della zona da rimuovere (1-%d): ", nZone);

        if (scanf("%d", &posizione) != 1)
        {
            printf("Input non valido.\n");
            while (getchar() != '\n')
                ;
            posizione = -1;
            continue;
        }

        while (getchar() != '\n')
            ;

        if (posizione < 1 || posizione > nZone)
            printf("Posizione non valida.\n");

    } while (posizione < 1 || posizione > nZone);

    int i = 1;

    struct Zona_mondoreale *current_zona_mondoreale = prima_zona_mondoreale;
    struct Zona_soprasotto *current_zona_soprasotto = prima_zona_soprasotto;

    while (current_zona_mondoreale && current_zona_soprasotto && i < posizione)
    {
        current_zona_mondoreale = current_zona_mondoreale->avanti;
        current_zona_soprasotto = current_zona_soprasotto->avanti;
        i++;
    }

    if (!current_zona_mondoreale || !current_zona_soprasotto)
        return;

    // rimuovo zona dal mondo reale
    if (current_zona_mondoreale->indietro)
        current_zona_mondoreale->indietro->avanti = current_zona_mondoreale->avanti;
    else
        prima_zona_mondoreale = current_zona_mondoreale->avanti;

    if (current_zona_mondoreale->avanti)
        current_zona_mondoreale->avanti->indietro = current_zona_mondoreale->indietro;

    // rimuovo zona dal sopra sotto
    if (current_zona_soprasotto->indietro)
        current_zona_soprasotto->indietro->avanti = current_zona_soprasotto->avanti;
    else
        prima_zona_soprasotto = current_zona_soprasotto->avanti;

    if (current_zona_soprasotto->avanti)
        current_zona_soprasotto->avanti->indietro = current_zona_soprasotto->indietro;

    free(current_zona_mondoreale);
    free(current_zona_soprasotto);

    printf("Zona rimossa con successo.\n");
    return;
}

/**
 * Permette di chiudere la mappa
 * Ritorna:
 * - 1: se la mappa è stata chiusa
 * - 0: se non è stato possibile chiudere la mappa
 */
static int chiudi_mappa()
{
    if (conta_zone_mappa() < 15)
    {
        printf("Non e' possibile chiudere la mappa: meno di 15 zone presenti.\n");
        return 0;
    }
    if (!verifica_demotorzone())
    {
        printf("Non e' possibile chiudere la mappa: demotorzone mancante o multiplo.\n");
        return 0;
    }
    mappa_chiusa = 1;
    printf("Mappa chiusa con successo.\n");
    return 1;
}

void imposta_gioco()
{

    mappa_chiusa = 0;
    giocatore_vincitore = 0;
    int undici_virgola_cinque_usato = 0;

    srand(time(NULL));

    printf("=== Fase di creazione dei giocatori ===\n");

    do
    {
        printf("Inserire il numero di giocatori (1-4)? ");
        if (scanf("%d", &numero_giocatori) != 1)
        {
            while (getchar() != '\n')
                ;
            numero_giocatori = 0;
        }
    } while (numero_giocatori < 1 || numero_giocatori > MAX_GIOCATORI);

    for (int i = 0; i < MAX_GIOCATORI; i++)
    {
        if (i >= numero_giocatori)
        {
            if (giocatori[i] != NULL)
            {
                free(giocatori[i]);
                giocatori[i] = NULL;
            }
            continue;
        }

        giocatori_morti[i] = NULL;
        giocatori[i] = malloc(sizeof(struct Giocatore));
        giocatori[i]->mondo = MONDO_REALE;
        giocatori[i]->avanzato = 0;
        giocatori[i]->vita = 20;
        giocatori[i]->pos_mondoreale = NULL;
        giocatori[i]->pos_soprasotto = NULL;

        int nome_valido;

        do
        {
            nome_valido = 1;

            printf("Giocatore %d, inserisci il tuo nome (max 12 caratteri): ", i + 1);
            scanf("%12s", giocatori[i]->nome);
            while (getchar() != '\n')
                ;

            for (int j = 0; j < i; j++)
            {
                if (strcmp(giocatori[j]->nome, giocatori[i]->nome) == 0)
                {
                    printf("Nome gia' utilizzato, sceglierne uno differente.\n");
                    nome_valido = 0;
                    break;
                }
            }

        } while (!nome_valido);

        nome_valido = 0;

        // lancio dei dadi
        printf("\nLancio dei dadi...\n");

        giocatori[i]->attacco_pischico = rand() % 20 + 1;
        printf("Giocatore %d - Lancio dado attacco psichico: %d\n", i + 1, giocatori[i]->attacco_pischico);

        giocatori[i]->difesa_pischica = rand() % 20 + 1;
        printf("Giocatore %d - Lancio dado difesa psichica: %d\n", i + 1, giocatori[i]->difesa_pischica);

        giocatori[i]->fortuna = rand() % 20 + 1;
        printf("Giocatore %d - Lancio dado fortuna: %d\n\n", i + 1, giocatori[i]->fortuna);

        for (int j = 0; j < 3; j++)
            giocatori[i]->zaino[j] = NESSUN_OGGETTO; // imposto lo zaino vuoto per ogni giocatore

        int scelta = 0;
        do
        {
            printf("Giocatore %s, scegliere le statistiche desiderate:\n", giocatori[i]->nome);
            printf("1) +3 attacco psichico, -3 difesa psichica\n");
            printf("2) -3 attacco psichico, +3 difesa psichica\n");

            if (!undici_virgola_cinque_usato)
                printf("3) Diventare UndiciVirgolaCinque (+4 attacco e difesa, -7 fortuna)\n");

            printf("Scelta: ");
            if (scanf("%d", &scelta) != 1)
            {
                printf("Input non valido.\n");
                while (getchar() != '\n')
                    ;
                scelta = 0;
                continue;
            }
            if((scelta == 3 && undici_virgola_cinque_usato))
                printf("UndiciVirgolaCinque non disponibile.\n");
            if(scelta < 1 || scelta > 3)
                printf("Scelta non valida.\n");
        } while (scelta < 1 || scelta > 3 || (scelta == 3 && undici_virgola_cinque_usato));

        if (scelta == 1)
        {

            giocatori[i]->attacco_pischico += 3;
            giocatori[i]->difesa_pischica -= 3;
        }
        else if (scelta == 2)
        {

            giocatori[i]->difesa_pischica += 3;
            giocatori[i]->attacco_pischico -= 3;
        }
        else if (scelta == 3)
        {

            giocatori[i]->attacco_pischico += 4;
            giocatori[i]->difesa_pischica += 4;
            giocatori[i]->fortuna -= 7;
            strcpy(giocatori[i]->nome, "UndiciVirgolaCinque");
            undici_virgola_cinque_usato = 1;
        }
    }

    // Stampa riassuntiva dei giocatori
    stampa_separatore();
    printf("Giocatori in partita:\n\n");

    for (int i = 0; i < MAX_GIOCATORI; i++)
    {
        if (giocatori[i] != NULL)
        {
            printf("Giocatore %d:\n", i + 1);
            printf("  Nome: %s\n", giocatori[i]->nome);
            printf("  Attacco psichico: %d\n", giocatori[i]->attacco_pischico);
            printf("  Difesa psichica:  %d\n", giocatori[i]->difesa_pischica);
            printf("  Fortuna:          %d\n", giocatori[i]->fortuna);
            printf("-------------------------------------\n");
        }
    }

    // definizione della mappa di gioco

    printf("Ora il game master deve definire la mappa di gioco.\n");

    int scelta = 0;

    do
    {
        printf("\n===== MENU CREAZIONE MAPPA =====\n");
        printf("1) Genera mappa automatica\n");
        printf("2) Inserisci zona manualmente\n");
        printf("3) Cancella zona\n");
        printf("4) Stampa mappa completa\n");
        printf("5) Stampa zona specifica\n");
        printf("6) Chiudi creazione mappa\n");
        printf("Scelta: ");

        if (scanf("%d", &scelta) != 1)
        {
            printf("Input non valido.\n");
            scelta = 0; 
        }

        while (getchar() != '\n')
            ;

        switch (scelta)
        {
        case 1:
            genera_mappa();
            break;
        case 2:
            inserisci_zona();
            break;
        case 3:
            cancella_zona();
            break;
        case 4:
            stampa_mappa();
            break;
        case 5:
            stampa_zona();
            break;
        case 6:
            chiudi_mappa();
            break;
        default:
            printf("Scelta non valida. Riprova.\n");
            break;
        }

    } while (!mappa_chiusa);

    gioco_impostato = 1;
}

/**
 * Permette di sorteggiare il turno in un combattimento tra un nemico e un giocatore
 * Ritorna:
 * - 1: se il giocatore vince il sorteggio
 * - 0: se è il nemico a vincere il sorteggio
 */
static int sorteggio_turno(struct Giocatore *giocatore, enum Tipo_nemico nemico)
{
    int tiro_giocatore = rand() % 20 + 1 + giocatore->fortuna;
    int tiro_nemico = rand() % 20 + 1;

    printf("Sorteggio turno:\n");
    printf("%s tira: %d\n", giocatore->nome, tiro_giocatore);
    printf("%s tira: %d\n", nome_nemico(nemico), tiro_nemico);

    if (tiro_giocatore >= tiro_nemico)
    {
        printf("%s inizia il combattimento.\n", giocatore->nome);
        return 1;
    }
    else
    {
        printf("%s inizia il combattimento.\n", nome_nemico(nemico));
        return 0;
    }
}

/**
 * Performa il combattimento tra il giocatore e il nemico presente nella zona in cui si trova
 * Ritorna:
 * - 1: se il giocatore vince
 * - 0: se il giocatore muore
 */
static int combatti(struct Giocatore *giocatore)
{
    enum Tipo_nemico nemico;

    /* Recupero nemico dalla posizione */
    if (giocatore->mondo == MONDO_REALE && giocatore->pos_mondoreale)
        nemico = giocatore->pos_mondoreale->nemico;
    else if (giocatore->mondo == SOPRA_SOTTO && giocatore->pos_soprasotto)
        nemico = giocatore->pos_soprasotto->nemico;
    else
    {
        printf("Posizione del giocatore non valida.\n");
        return 0;
    }

    if (nemico == NESSUN_NEMICO)
    {
        printf("Nessun nemico nella zona.\n");
        return 1;
    }

    int attacco_nemico = 0;
    int difesa_nemico = 0;
    int vita_nemico = 20;

    /* Impostazione statistiche nemico */
    switch (nemico)
    {
    case BILLI:
        attacco_nemico = 5;
        difesa_nemico = 10;
        break;
    case DEMOCANE:
        attacco_nemico = 10;
        difesa_nemico = 15;
        break;
    case DEMOTORZONE:
        attacco_nemico = 15;
        difesa_nemico = 20;
        vita_nemico = 25;
        break;
    default:
        break;
    }

    printf("== Inizio combattimento contro %s == \n- vita: %d\n- attacco: %d\n- difesa: %d \n", nome_nemico(nemico), vita_nemico, attacco_nemico, difesa_nemico);
    stampa_separatore();

    /* Uso oggetto prima del combattimento */
    if (controlla_zaino(giocatore->zaino))
    {
        char scelta;
        do
        {
            printf("Vuoi usare un oggetto prima del combattimento? (s/n): ");
            scanf(" %c", &scelta);

            if (scelta == 's')
                utilizza_oggetto(giocatore);
            else if (scelta != 'n')
                printf("Scelta non valida.\n");

        } while (scelta != 's' && scelta != 'n');
    }

    int turno_giocatore = sorteggio_turno(giocatore, nemico);

    printf("\nInizia %s!\n",
           turno_giocatore ? giocatore->nome : nome_nemico(nemico));

    while (giocatore->vita > 0 && vita_nemico > 0)
    {
        if (turno_giocatore)
        {
            /* TURNO GIOCATORE */
            int tiro_attacco = rand() % 20 + 1;
            int tiro_difesa_nemico = rand() % 20 + 1;

            int attacco_tot = tiro_attacco + giocatore->attacco_pischico;
            int difesa_tot = tiro_difesa_nemico + difesa_nemico;

            printf("\n--- Turno di %s ---\n", giocatore->nome);
            printf("Lancio del dado di attacco: %d\n", tiro_attacco);

            printf("Lancio del dado di difesa del nemico: %d\n", tiro_difesa_nemico);

            printf("Attacco totale: %d\n", attacco_tot);
            printf("Difesa totale del nemico: %d\n", difesa_tot);

            if (attacco_tot >= difesa_tot)
            {
                int danno = attacco_tot - difesa_tot;
                vita_nemico -= danno;

                printf("Colpo riuscito! Danni inflitti: %d\n", danno);
                printf("Vita rimanente del nemico: %d\n", vita_nemico);
            }
            else
            {
                printf("Il nemico riesce a difendersi dall'attacco.\n");
            }
        }
        else
        {
            /* TURNO NEMICO */
            int tiro_attacco = rand() % 20 + 1;
            int tiro_difesa_giocatore = rand() % 20 + 1;

            int attacco_tot = tiro_attacco + attacco_nemico;
            int difesa_tot = tiro_difesa_giocatore + giocatore->difesa_pischica;

            printf("\n--- Turno di %s ---\n", nome_nemico(nemico));
            printf("Lancio del dado di attacco del nemico: %d\n", tiro_attacco);

            printf("Lancio del dado di difesa di %s: %d\n", giocatore->nome, tiro_difesa_giocatore);

            printf("Attacco totale del nemico: %d\n", attacco_tot);
            printf("Difesa totale del giocatore: %d\n", difesa_tot);

            if (attacco_tot >= difesa_tot)
            {
                int danno = attacco_tot - difesa_tot;
                giocatore->vita -= danno;

                printf("Il nemico colpisce! Danni subiti: %d\n", danno);
                printf("Vita rimanente di %s: %d\n", giocatore->nome, giocatore->vita);
            }
            else
            {
                printf("%s riesce a parare l'attacco del nemico.\n", giocatore->nome);
            }
        }

        turno_giocatore = !turno_giocatore;
    }

    if (giocatore->vita <= 0)
    {
        // Aggiungi il giocatore morto alla zona
        if (giocatore->mondo == MONDO_REALE && giocatore->pos_mondoreale)
        {
            for (int i = 0; i < MAX_GIOCATORI; i++)
            {
                if (giocatore->pos_mondoreale->giocatori_morti[i] == NULL)
                {
                    giocatore->pos_mondoreale->giocatori_morti[i] = giocatore;
                    break;
                }
            }
        }
        else if (giocatore->mondo == SOPRA_SOTTO && giocatore->pos_soprasotto)
        {
            for (int i = 0; i < MAX_GIOCATORI; i++)
            {
                if (giocatore->pos_soprasotto->giocatori_morti[i] == NULL)
                {
                    giocatore->pos_soprasotto->giocatori_morti[i] = giocatore;
                    break;
                }
            }
        }
        return 0;
    }

    if (vita_nemico <= 0)
    {
        printf("\n%s e' stato sconfitto!\n", nome_nemico(nemico));
        // se il nemico sconfitto era il demotorzone, imposta il giocatore come vincitore
        if (nemico == DEMOTORZONE)
            giocatore_vincitore = 1;
    }

    // possibilità del 50% che il nemico scompaia
    if (rand() % 100 < 50)
    {
        if (giocatore->mondo == MONDO_REALE)
            giocatore->pos_mondoreale->nemico = NESSUN_NEMICO;
        else
            giocatore->pos_soprasotto->nemico = NESSUN_NEMICO;

        printf("%s e' scomparso dalla zona.\n", nome_nemico(nemico));
    }
    else
    {
        printf("%s non e' scomparso dalla zona.\n", nome_nemico(nemico));
    }

    return 1;
}

/**
 * Permette al giocatore di indietreggiare
 * Ritorna:
 * - 1: se il giocatore riesce ad indietreggiare
 * - 0: se non riesce
 */
static int indietreggia(struct Giocatore *giocatore)
{

    if (!combatti(giocatore))
    {
        printf("Impossibile indietreggiare.\n");
        return 0;
    }

    if (giocatore->mondo == MONDO_REALE)
    {
        if (giocatore->pos_mondoreale && giocatore->pos_mondoreale->indietro)
        {

            giocatore->pos_mondoreale = giocatore->pos_mondoreale->indietro;
            return 1;
        }
        else
        {
            printf("Non puoi indietreggiare oltre nel Mondo Reale.\n");
            return 0;
        }
    }
    else if (giocatore->mondo == SOPRA_SOTTO)
    {
        if (giocatore->pos_soprasotto && giocatore->pos_soprasotto->indietro)
        {

            giocatore->pos_soprasotto = giocatore->pos_soprasotto->indietro;
            return 1;
        }
        else
        {
            printf("Non puoi indietreggiare oltre nel Soprasotto.\n");
            return 0;
        }
    }

    printf("Mondo del giocatore non valido.\n");
    return 0;
}

static void cambia_mondo(struct Giocatore *giocatore)
{
    if (giocatore->mondo == MONDO_REALE)
    {
        if (combatti(giocatore))
        {
            printf("Trasferimento nel soprasotto...\n");
            giocatore->pos_soprasotto = giocatore->pos_mondoreale->link_soprasotto;
            giocatore->pos_mondoreale = NULL;
            giocatore->mondo = SOPRA_SOTTO;
            printf("Trasferimento nel soprasotto completato.\n");
        }
    }
    else if (giocatore->mondo == SOPRA_SOTTO)
    {
        printf("Fortuna del giocatore: %d\n", giocatore->fortuna);
        printf("Lancio del dado per scappare da %s...\n", nome_nemico(giocatore->pos_soprasotto->nemico));
        int dado = rand() % 20;
        printf("Risultato: %d\n", dado);
        if (dado < giocatore->fortuna)
        {
            printf("%s e' riuscito a scappare da %s\n", giocatore->nome, nome_nemico(giocatore->pos_soprasotto->nemico));
            printf("Trasferimento nel mondo reale...\n");
            giocatore->pos_mondoreale = giocatore->pos_soprasotto->link_mondoreale;
            giocatore->pos_soprasotto = NULL;
            giocatore->mondo = MONDO_REALE;
            printf("Trasferimento nel mondo reale completato.\n");
        }
        else
        {
            printf("Risulato del dado maggiore o uguale della fortuna di %s, impossibile scappare da %s.\n", giocatore->nome, nome_nemico(giocatore->pos_soprasotto->nemico));

            if (combatti(giocatore))
            {
                printf("Trasferimento nel mondoreale...\n");
                giocatore->pos_mondoreale = giocatore->pos_soprasotto->link_mondoreale;
                giocatore->pos_soprasotto = NULL;
                giocatore->mondo = MONDO_REALE;
                printf("Trasferimento nel mondoreale completato.\n");
            }
        }
    }
    else
    {
        printf("Mondo del giocatore non valido.\n");
        return;
    }
}

/**
 * Permette di raccogliere l'oggetto in una zona
 * Ritorna
 * - L'oggetto raccolto
 * - 0 se il giocatore non riesce a raccogliere l'oggetto oppure questo non è presente nella zona
 */
static enum Tipo_oggetto raccogli_oggetto(struct Giocatore *giocatore)
{
    int oggetti_trasportati = 0;
    int oggetto = 0;

    if (giocatore->pos_mondoreale)
        oggetto = giocatore->pos_mondoreale->oggetto;

    if (!oggetto)
    {
        printf("Nessun oggetto nella zona.\n");
        return 0;
    }

    for (int i = 0; i < 3; i++)
    {
        if (giocatore->zaino[i] != NESSUN_OGGETTO)
            oggetti_trasportati++;
    }

    if (oggetti_trasportati >= 3)
    {
        printf("Zaino pieno. Impossibile raccogliere l'oggetto nella zona\n");
        return 0;
    }

    if (combatti(giocatore))
    {

        for (int i = 0; i < DIMENSIONE_ZAINO; i++)
        {
            if (giocatore->zaino[i] == NESSUN_OGGETTO)
            {
                giocatore->zaino[i] = oggetto;
                giocatore->pos_mondoreale->oggetto = NESSUN_OGGETTO;
                return oggetto;
            }
        }
    }
    return 0;
}

static void passa(int *azione, struct Giocatore *giocatore)
{
    *azione = 0;
    if (giocatore)
        giocatore->avanzato = 0;
}

/**
 * Permette al giocatore di passare alla zona successiva del mondo in cui si trova
 * Ritorna:
 * - 1: se l'avanzamento è andato a buon fine
 * - 0: se il giocatore non è riuscito ad avanzare o è morto
 */
static int avanza(struct Giocatore *giocatore)
{
    if (!combatti(giocatore))
    {
        printf("Impossibile avanzare.\n");
        return 0;
    }

    if (giocatore->mondo == MONDO_REALE) // mondo reale
    {
        if (giocatore->pos_mondoreale && giocatore->pos_mondoreale->avanti)
        {
            giocatore->pos_mondoreale = giocatore->pos_mondoreale->avanti;
            return 1;
        }
        else
        {
            printf("Non puoi avanzare oltre.\n");
            return 0;
        }
    }
    else if (giocatore->mondo == SOPRA_SOTTO) // soprasotto
    {
        if (giocatore->pos_soprasotto && giocatore->pos_soprasotto->avanti)
        {
            giocatore->pos_soprasotto = giocatore->pos_soprasotto->avanti;
            return 1;
        }
        else
        {
            printf("Non puoi avanzare oltre.\n");
            return 0;
        }
    }

    printf("Mondo non valido.\n");
    return 0;
}

void gioca()
{

    if (!gioco_impostato)
    {
        printf("Gioco non impostato.\n");
        return;
    }

    int count = 0;

    do
    {
        count++;
        printf("\n=== %d turno ===\n", count);
        printf("Sorteggio dei turni...\n");

        struct Giocatore *turni[numero_giocatori];
        int num_turni = 0;

        for (int i = 0; i < MAX_GIOCATORI; i++)
        {
            if (giocatori[i] != NULL)
            {
                turni[num_turni] = giocatori[i];
                num_turni++;
            }
        }

        // sorteggio dei turni
        for (int i = num_turni - 1; i > 0; i--)
        {
            int j = rand() % (i + 1);
            struct Giocatore *temp = turni[i];
            turni[i] = turni[j];
            turni[j] = temp;
        }

        printf("Ordine dei turni:\n");
        for (int i = 0; i < num_turni; i++)
        {
            printf("%d) %s\n", i + 1, turni[i]->nome);
        }

        for (int i = 0; i < num_turni; i++)
        {
            struct Giocatore *giocatore = turni[i];

            // salta se il giocatore è morto durante questo turno
            if (giocatore == NULL)
                continue;

            int attacco_psichico = giocatore->attacco_pischico;
            int difesa_psichica = giocatore->difesa_pischica;

            int azione = -1;

            do
            {
                printf("\nTurno di %s\n", giocatore->nome);

                if (giocatore->mondo == MONDO_REALE && giocatore->pos_mondoreale && giocatore->pos_mondoreale->oggetto != NESSUN_OGGETTO)
                    printf("e' presente un oggetto nella zona!\n");

                struct Giocatore *morti_in_zona[MAX_GIOCATORI] = {NULL};
                int num_morti = conta_giocatori_morti(giocatore, morti_in_zona);

                if (num_morti > 0)
                {
                    printf("Ci sono %d giocatori morti nella zona:\n", num_morti);
                    for (int i = 0; i < num_morti; i++)
                    {
                        printf("%d) %s\n", i + 1, morti_in_zona[i]->nome);
                    }
                }

                printf(
                    "Scegli un'azione:\n"
                    "1) Avanza\n"
                    "2) Indietreggia\n"
                    "3) Cambia mondo\n"
                    "4) Combatti\n"
                    "5) Stampa giocatore\n"
                    "6) Stampa zona\n"
                    "7) Raccogli oggetto\n"
                    "8) Utilizza oggetto\n"
                    "9) Passa turno\n");

                printf("Scelta: ");

                if (scanf("%d", &azione) != 1)
                {
                    while (getchar() != '\n')
                        ;
                    azione = -1;
                    printf("Input non valido.\n");
                }
                while (getchar() != '\n')
                    ;
                stampa_separatore();

                switch (azione)
                {
                case 1:
                    if (!giocatore->avanzato)
                    {
                        if (avanza(giocatore))
                        {
                            printf("Il Giocatore e' avanzato con successo.\n\nNuova posizione di: %s\n ", giocatore->nome);
                            stampa_zona_giocatore(giocatore);
                        }
                    }
                    else
                        printf("Avanzamento gia' effettuato in questo turno.\n");

                    giocatore->avanzato = 1;
                    break;

                case 2:
                    if (indietreggia(giocatore))
                    {
                        printf("Il giocatore e' indietreggiato con successo.\nNuova posizione di %s\n: ", giocatore->nome);
                        stampa_zona_giocatore(giocatore);
                    }
                    break;

                case 3:
                    if (!giocatore->avanzato)
                    {
                        cambia_mondo(giocatore);
                    }
                    else
                        printf("Avanzamento gia' effettuato in questo turno.");

                    giocatore->avanzato = 1;
                    break;

                case 4:
                    combatti(giocatore);
                    break;
                case 5:
                    stampa_giocatore(giocatore);
                    break;

                case 6:
                    stampa_zona_giocatore(giocatore);
                    break;

                case 7:
                {
                    int oggetto = raccogli_oggetto(giocatore);
                    if (oggetto)
                        printf("Oggetto %s raccolto con successo.\n", nome_oggetto(oggetto));

                    stampa_zaino(giocatore->zaino);
                }
                break;

                case 8:
                    utilizza_oggetto(giocatore);
                    break;

                case 9:
                    passa(&azione, giocatore);
                    printf("Turno passato.\n");
                    break;

                default:
                    printf("Azione non valida.\n");
                    azione = -1;
                    break;
                }

                // Controlla se il giocatore è morto
                if (giocatore->vita <= 0)
                {
                    printf("%s e' morto.\n", giocatore->nome);

                    // aggiungo il giocatore morto all'array di giocatori morti
                    for (int j = 0; j < MAX_GIOCATORI; j++)
                    {
                        if (giocatori_morti[j] == NULL)
                        {
                            giocatori_morti[j] = giocatore;
                            break;
                        }
                    }

                    for (int k = 0; k < MAX_GIOCATORI; k++)
                    {
                        if (giocatori[k] != NULL && strcmp(giocatori[k]->nome, giocatore->nome) == 0)
                        {
                            giocatori[k] = NULL;
                            numero_giocatori--;
                            break;
                        }
                    }

                    turni[i] = NULL;
                    break;
                }
                else if (giocatore_vincitore) // se il giocatore non è morto, controlla se è vincitore
                {
                    printf("\n== Fine della partita ==\n");
                    printf("Congratulazioni %s! Hai salvato il mondoreale dalla minaccia del temibile %s!\n", giocatore->nome, nome_nemico(DEMOTORZONE));

                    // aggiorna classifica vincitori
                    for (int j = 2; j > 0; j--)
                    {
                        giocatori_vincitori[j] = giocatori_vincitori[j - 1];
                    }
                    giocatori_vincitori[0] = giocatore;

                    // rimuove il giocatore vincitore dall'array di giocatori prima di liberarlo
                    for (int k = 0; k < numero_giocatori; k++)
                    {
                        if (giocatori[k] == giocatore)
                        {
                            for (int j = k; j < numero_giocatori - 1; j++)
                            {
                                giocatori[j] = giocatori[j + 1];
                            }
                            giocatori[numero_giocatori - 1] = NULL;
                            numero_giocatori--;
                            break;
                        }
                    }

                    libera_mappe();
                    libera_giocatori();
                    partite_giocate++;
                    return;
                }

            } while (azione != 0);

            if (giocatore && giocatore->vita > 0)
            {
                // reset di eventuali buff di attacco e difesa dati da oggetti
                giocatore->attacco_pischico = attacco_psichico;
                giocatore->difesa_pischica = difesa_psichica;
                printf("Statistiche ripristinate.\n");
            }
        }

        // liberazione dei giocatori morti
        for (int i = 0; i < MAX_GIOCATORI; i++)
        {
            if (giocatori_morti[i] != NULL)
            {
                // Rimuovi il riferimento dalle zone
                if (giocatori_morti[i]->pos_mondoreale)
                {
                    for (int j = 0; j < MAX_GIOCATORI; j++)
                    {
                        if (giocatori_morti[i]->pos_mondoreale->giocatori_morti[j] == giocatori_morti[i])
                        {
                            giocatori_morti[i]->pos_mondoreale->giocatori_morti[j] = NULL;
                            break;
                        }
                    }
                }
                else if (giocatori_morti[i]->pos_soprasotto)
                {
                    for (int j = 0; j < MAX_GIOCATORI; j++)
                    {
                        if (giocatori_morti[i]->pos_soprasotto->giocatori_morti[j] == giocatori_morti[i])
                        {
                            giocatori_morti[i]->pos_soprasotto->giocatori_morti[j] = NULL;
                            break;
                        }
                    }
                }

                free(giocatori_morti[i]);
                giocatori_morti[i] = NULL; // IMPORTANTE: azzera il puntatore
            }
        }

    } while (numero_giocatori > 0);

    printf("\n== Fine della partita ==\n");
    printf("Tutti i giocatori sono morti.\n");

    // libero la memoria e incremento le partite giocate se tutti i giocatori muoiono
    libera_mappe();
    libera_giocatori();
    partite_giocate++;
}

void termina_gioco()
{

    libera_giocatori();
    libera_mappe();

    printf("        _____ _____ ____   _____ ____    _______ ______ _____  __  __ _____ _   _       _______ ____         \n");
    printf("      / ____|_   _/ __ \\ / ____/ __ \\  |__   __|  ____|  __ \\|  \\/  |_   _| \\ | |   /\\|__   __/ __ \\        \n");
    printf("     | |  __  | || |  | | |   | |  | |    | |  | |__  | |__) | \\  / | | | |  \\| |  /  \\  | | | |  | |       \n");
    printf("     | | |_ | | || |  | | |   | |  | |    | |  |  __| |  _  /| |\\/| | | | | . ` | / /\\ \\ | | | |  | |       \n");
    printf("     | |__| |_| || |__| | |___| |__| |    | |  | |____| | \\ \\| |  | |_| |_| |\\  |/ ____ \\| | | |__| |       \n");
    printf("   ___\\_____|_____\\____/ \\_____\\____/     |_|  |______|_|  \\_\\_|  |_|_____|_| \\_/_/    \\_\\_|  \\____/      _ \n");
    printf("  / ____|             (_)                                                     (_)               | |      | |\n");
    printf(" | |  __ _ __ __ _ _____  _   _ __   ___ _ __    __ ___   _____ _ __    __ _ _  ___   ___ __ _| |_ ___ | |\n");
    printf(" | | |_ | '__/ _` |_  / |/ _ \\ | '_ \\ / _ \\ '__|  / _` \\ \\ / / _ \\ '__|  / _` | |/ _ \\ / __/ _` | __/ _ \\| |\n");
    printf(" | |__| | | | (_| |/ /| |  __/ | |_) |  __/ |    | (_| |\\ V /  __/ |    | (_| | | (_) | (_| (_| | || (_) |_|\n");
    printf("  \\_____|_|  \\__,_/___|_|\\___| | .__/ \\___|_|     \\__,_| \\_/ \\___|_|     \\__, |_|\\___/ \\___\\__,_|\\__\\___/(_)\n");
    printf("                               | |                                        __/ |                             \n");
    printf("                               |_|                                       |___/                              \n");
}

void crediti()
{
    printf("=== CREDITI ===\n");
    printf("Creatore del gioco: Lorenzo Rossi\n");

    int partite = partite_giocate > 3 ? 3 : partite_giocate;

    if (partite > 0)
    {
        printf("Ultimi vincitori:\n");

        int contatore = 1;
        for (int i = partite_giocate - partite; i < partite_giocate; i++)
        {
            if (giocatori_vincitori[i] != NULL)
                printf("%d) %s\n", contatore, giocatori_vincitori[i]->nome);
            else
                printf("%d) Nessun vincitore\n", contatore);
            contatore++;
        }

        printf("Partite totali giocate: %d\n", partite_giocate);
    }
    else
    {
        printf("Nessuna partita giocata.\n");
    }
}
