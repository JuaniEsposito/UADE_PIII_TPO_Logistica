#include "grafo.h"
#include <string.h>

// --- GESTIÓN DE MEMORIA DEL GRAFO ---

Grafo *crear_grafo(int num_nodos, int num_hubs) {
    Grafo *g = (Grafo *)malloc(sizeof(Grafo));
    if (!g) return NULL;

    g->num_nodos = num_nodos;
    g->num_hubs = num_hubs;
    g->nodos = (Nodo *)malloc(num_nodos * sizeof(Nodo));

    g->matriz_adyacencia = (double **)malloc(num_nodos * sizeof(double *));
    g->distancias_minimas = (double **)malloc(num_nodos * sizeof(double *));

    for (int i = 0; i < num_nodos; i++) {
        g->matriz_adyacencia[i] = (double *)malloc(num_nodos * sizeof(double));
        g->distancias_minimas[i] = (double *)malloc(num_nodos * sizeof(double));
        
        for(int j = 0; j < num_nodos; j++) {
            g->matriz_adyacencia[i][j] = INFINITY_COST;
            g->distancias_minimas[i][j] = INFINITY_COST;
        }
        g->matriz_adyacencia[i][i] = 0.0;
        g->distancias_minimas[i][i] = 0.0;
    }

    return g;
}

void liberar_grafo(Grafo *g) {
    if (g) {
        for (int i = 0; i < g->num_nodos; i++) {
            free(g->matriz_adyacencia[i]);
            free(g->distancias_minimas[i]);
        }
        free(g->matriz_adyacencia);
        free(g->distancias_minimas);
        free(g->nodos);
        free(g);
    }
}

// --- ALGORITMO DE DIJKSTRA ---

void dijkstra(Grafo *g, int inicio, double *distancias) {
    int n = g->num_nodos;
    
    // Asignación de memoria: calloc inicializa todos los bytes a 0 (false)
    bool *visitado = (bool *)calloc(n, sizeof(bool));
    
    if (visitado == NULL) {
        // Falla la asignación de memoria
        return; 
    }
    
    // Inicialización de distancias
    distancias[inicio] = 0.0;
    
    for (int count = 0; count < n - 1; count++) {
        double min = INFINITY_COST;
        int u = -1;

        // Búsqueda del nodo no visitado con la menor distancia (extracción del mínimo)
        for (int v = 0; v < n; v++) {
            if (!visitado[v] && distancias[v] <= min) {
                min = distancias[v];
                u = v;
            }
        }

        if (u == -1) break; // Todos los nodos alcanzables han sido visitados

        visitado[u] = true;

        // Relajación de aristas
        for (int v = 0; v < n; v++) {
            if (!visitado[v] 
                && g->matriz_adyacencia[u][v] != INFINITY_COST
                && distancias[u] != INFINITY_COST
                && distancias[u] + g->matriz_adyacencia[u][v] < distancias[v]) {
                
                distancias[v] = distancias[u] + g->matriz_adyacencia[u][v];
            }
        }
    }

    // Liberación de memoria
    free(visitado); 
}

void calcular_distancias_minimas(Grafo *g) {
    int n = g->num_nodos;
    for (int i = 0; i < n; i++) {
        // Ejecutar Dijkstra desde cada nodo como origen
        dijkstra(g, i, g->distancias_minimas[i]);
    }
}

double obtener_distancia_minima(Grafo *g, int origen_id, int destino_id) {
    // Si los IDs no son válidos, devuelve un costo infinito
    if (origen_id < 0 || origen_id >= g->num_nodos || destino_id < 0 || destino_id >= g->num_nodos) {
        return INFINITY_COST;
    }
    return g->distancias_minimas[origen_id][destino_id];
}