#include <stdio.h>
#include "gamelib.h"

int main()
{

    int scelta;

    printf("    _____ ____   _____ ______    _____ _______ _____            _   _ ______ \n");
    printf("  / ____/ __ \\ / ____|  ____|  / ____|__   __|  __ \\     /\\   | \\ | |  ____|\n");
    printf(" | |   | |  | | (___ | |__    | (___    | |  | |__) |   /  \\  |  \\| | |__   \n");
    printf(" | |   | |  | |\\___ \\|  __|    \\___ \\   | |  |  _  /   / /\\ \\ | . ` |  __|  \n");
    printf(" | |___| |__| |____) | |____   ____) |  | |  | | \\ \\  / ____ \\| |\\  | |____ \n");
    printf("  \\_____\\____/|_____/|______|_|_____/___|_|  |_|  \\_\\/_/ ___\\_\\_| \\_|______|\n");
    printf("         |__   __| |  | |  ____|  / ____|   /\\   |  \\/  |  ____|            \n");
    printf("            | |  | |__| | |__    | |  __   /  \\  | \\  / | |__               \n");
    printf("            | |  |  __  |  __|   | | |_ | / /\\ \\ | |\\/| |  __|              \n");
    printf("            | |  | |  | | |____  | |__| |/ ____ \\| |  | | |____             \n");
    printf("            |_|  |_|  |_|______|  \\_____/_/    \\_\\_|  |_|______|            \n\n");

    stampa_regole();

    do
    {
        printf("\n===== MENU GIOCO =====\n");
        printf("0) Stampa regole\n");
        printf("1) Imposta gioco\n");
        printf("2) Gioca\n");
        printf("3) Termina gioco\n");
        printf("4) Visualizza crediti\n");
        printf("Inserisci la tua scelta: ");

        if (scanf("%d", &scelta) != 1)
        {
            printf("Input non valido. Inserire un numero (0-4).\n");

            while (getchar() != '\n')
                ;

            scelta = -1;
            continue;
        }

        switch (scelta)
        {
        case 0:
            stampa_regole();
            break;
        case 1:
            imposta_gioco();
            break;
        case 2:
            gioca();
            break;
        case 3:
            termina_gioco();
            break;
        case 4:
            crediti();
            break;
        }

        if (scelta < 0 || scelta > 4)
            printf("Scelta non valida, riprovare.\n");
    } while (scelta != 3);

    return 0;
}
