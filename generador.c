#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_NODOS 2000
#define MAX_ARISTAS 5000
#define MAX_COORD 1000

typedef struct {
    int id;
    int x;
    int y;
} Nodo;

typedef struct {
    int u;
    int v;
    double peso;
} Arista;

double calcular_distancia(Nodo n1, Nodo n2);

int main(int argc, char *argv[]) {
    // --- 1. CONFIGURACION Y PARSEO ---
    int num_nodos = 50;
    int num_hubs = 5;
    int num_paquetes = 30;
    int capacidad_camion = 8;
    int deposito_id = 0; 
    unsigned int seed = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--nodos") == 0) num_nodos = atoi(argv[++i]);
        else if (strcmp(argv[i], "--hubs") == 0) num_hubs = atoi(argv[++i]);
        else if (strcmp(argv[i], "--paquetes") == 0) num_paquetes = atoi(argv[++i]);
        else if (strcmp(argv[i], "--seed") == 0) seed = atoi(argv[++i]);
    }

    if (num_hubs >= num_nodos) {
        fprintf(stderr, "Error: El número de hubs debe ser menor que el número total de nodos.\n");
        return 1;
    }
    if ((num_nodos - num_hubs - 1) <= 0) {
        fprintf(stderr, "Error: No hay nodos disponibles para entregas (num_nodos - num_hubs - 1 <= 0).\n");
        return 1;
    }

    if (seed == 0) seed = time(NULL);
    srand(seed);
    fprintf(stderr, "Generador iniciado con semilla: %u\n", seed);
    
    // --- 2. LOGICA DE GENERACION ---
    int **conectados = malloc(num_nodos * sizeof(int *));
    if (conectados == NULL) { fprintf(stderr, "Error de memoria.\n"); return 1; }
    for (int i = 0; i < num_nodos; i++) {
        conectados[i] = calloc(num_nodos, sizeof(int));
        if (conectados[i] == NULL) { fprintf(stderr, "Error de memoria.\n"); return 1; }
    }

    Nodo nodos[MAX_NODOS];
    Arista aristas[MAX_ARISTAS];
    int num_aristas = 0;

    for (int i = 0; i < num_nodos; i++) {
        nodos[i].id = i;
        nodos[i].x = rand() % (MAX_COORD + 1);
        nodos[i].y = rand() % (MAX_COORD + 1);
    }

    for (int i = 0; i < num_nodos - 1; i++) {
        aristas[num_aristas].u = i;
        aristas[num_aristas].v = i + 1;
        aristas[num_aristas].peso = calcular_distancia(nodos[i], nodos[i + 1]);
        conectados[i][i+1] = 1;
        conectados[i+1][i] = 1;
        num_aristas++;
    }

    int aristas_adicionales = num_nodos / 2;
    for (int i = 0; i < aristas_adicionales; i++) {
        int u = rand() % num_nodos;
        int v = rand() % num_nodos;
        if (u != v && !conectados[u][v]) {
            aristas[num_aristas].u = u;
            aristas[num_aristas].v = v;
            aristas[num_aristas].peso = calcular_distancia(nodos[u], nodos[v]);
            conectados[u][v] = 1;
            conectados[v][u] = 1;
            num_aristas++;
        }
    }

    // --- 3. ESCRITURA DEL ARCHIVO DE SALIDA ---
    FILE *fp = stdout; 

    fprintf(fp, "// --- CONFIGURACION ---\n");
    fprintf(fp, "NODOS %d\nHUBS %d\nPAQUETES %d\nCAPACIDAD_CAMION %d\nDEPOSITO_ID %d\n\n", num_nodos, num_hubs, num_paquetes, capacidad_camion, deposito_id);

    fprintf(fp, "// --- NODOS (ID X Y) ---\n");
    for (int i = 0; i < num_nodos; i++) {
        fprintf(fp, "%d %d %d", nodos[i].id, nodos[i].x, nodos[i].y);
        if (i == deposito_id) fprintf(fp, " // Deposito\n");
        else if (i <= num_hubs && i > 0) fprintf(fp, " // Hub\n");
        else fprintf(fp, " // Entrega\n");
    }

    fprintf(fp, "\n// --- HUBS (ID COSTO_ACTIVACION) ---\n");
    for (int i = 1; i <= num_hubs; i++) {
        fprintf(fp, "%d %.2f\n", i, 100 + (double)(rand() % 401));
    }

    fprintf(fp, "\n// --- PAQUETES (ID NODO_ORIGEN NODO_DESTINO) ---\n");
    for (int i = 0; i < num_paquetes; i++) {
        int nodo_entrega = (num_hubs + 1) + (rand() % (num_nodos - num_hubs - 1));
        fprintf(fp, "%d %d %d\n", i, deposito_id, nodo_entrega);
    }

    fprintf(fp, "\n// --- ARISTAS (NODO1 NODO2 PESO) ---\n");
    for (int i = 0; i < num_aristas; i++) {
        fprintf(fp, "%d %d %.2f\n", aristas[i].u, aristas[i].v, aristas[i].peso);
    }

    for (int i = 0; i < num_nodos; i++) {
        free(conectados[i]);
    }
    free(conectados);

    return 0;
}

double calcular_distancia(Nodo n1, Nodo n2) {
    return sqrt(pow(n1.x - n2.x, 2) + pow(n1.y - n2.y, 2));
}