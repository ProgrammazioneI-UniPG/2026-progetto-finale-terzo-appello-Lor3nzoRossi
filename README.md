[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/CD0c0pIj)

# Progetto-finale-2025-Cosestrane
Progetto finale di Programmazione Procedurale  
Università degli Studi di Perugia – Informatica

## Studente
- **Nome:** Lorenzo  
- **Cognome:** Rossi  
- **Matricola:** 395926  

## Commenti / Modifiche al progetto

### Logica di combattimento implementata
- In base al nemico incontrato vengono impostati diversi valori di **attacco**, **difesa** e **punti vita**
- Il giocatore può utilizzare un **oggetto**, se presente nello zaino
- L’ordine di inizio del combattimento viene sorteggiato tramite un **dado a 20 facce**  
  (al tiro del giocatore viene sommato il valore di **fortuna**)
- Ad ogni turno viene effettuato un **lancio del dado di attacco**
- Il difensore effettua un **lancio del dado di difesa** per ridurre o annullare il danno
- Se il valore di attacco supera quello di difesa, il difensore perde punti vita
- Il combattimento continua finché uno dei due termina i punti vita
- Se il giocatore perde, i suoi punti vita scendono a zero e viene considerato **morto**
- Se il giocatore vince:
  - Le azioni che richiedevano la sconfitta del nemico diventano eseguibili
  - C’è una probabilità del **50%** che il nemico scompaia dalla zona
- Se il nemico sconfitto è il **DEMOTORZONE**, il giocatore **vince immediatamente la partita**

### Effetti legati agli oggetti
- **BICICLETTA**: permette di avanzare nuovamente anche se si è già avanzato nel turno
- **MAGLIETTA_FUOCOINFERNO**: aumenta la difesa psichica per la durata del combattimento
- **BUSSOLA**: permette di stampare la mappa del mondo corrente
- **SCHITARRATA_METALLICA**: aumenta l’attacco psichico per la durata del combattimento
- **CROCE**: permette di rianimare un giocatore morto in una determinata stanza, se raggiunta
  nello stesso ciclo di turni in cui è stato sconfitto

### Funzionalità e ausili creati
- Oggetto `CROCE`: spiegato nella sezione  [Effetti legati agli oggetti](#effetti-legati-agli-oggetti)
- `Conta_giocatori_morti()`: conta il numero di giocatori morti presenti in una zona
- Costante `MAX_GIOCATORI`: indica il numero massimo di giocatori che possono essere presenti in una partita
- Costante `DIMENSIONE_ZAINO`: indica il numero massimo di oggetti trasportabili nello zaino di ciascun giocatore
- Campo `NUM_TIPI_ZONA`: aggiunto alla fine nell'enum **TIPO_ZONA** per sapere dinamicamente il numero delle zone
- `enum TipoMondo`:  
  - `0` = Mondoreale  
  - `1` = Soprasotto
- `stampa_regole()`: stampa le regole del gioco
- `stampa_zaino()`: stampa gli oggetti presenti nello zaino
- `controlla_zaino()`: verifica se lo zaino è vuoto (usata in `combatti()`)
- `conta_zone_mappa()`: conta il numero di zone del mondoreale
- `verifica_demotorzone()`: controlla la presenza di almeno un DEMOTORZONE nel soprasotto
- `libera_mappe()`: libera la memoria delle mappe
- `stampa_mondoreale()` e `stampa_soprasotto()`: stampano le mappe dei mondi
- `stampa_separatore()`: stampa un separatore grafico di lunghezza costante
- `sorteggio_turno()`: determina chi inizia il combattimento
- `nome_nemico()`, `nome_oggetto()`, `nome_zona()`:
  restituiscono il nome della chiave corrispondente al valore associato, nell'enum di riferimento

### Note
- Poiché nella traccia erano presenti due funzioni chiamate `stampa_zona()`  
  (una nel menù di creazione della mappa e una nel menù di gioco),
  la funzione del menù di gioco è stata rinominata in  
  **`stampa_zona_giocatore()`**
