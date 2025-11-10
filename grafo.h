#ifndef GRAFO_H
#define GRAFO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define INFINITY_COST 999999.0
#define DEPOSITO_ID 0 

// --- ESTRUCTURAS ---

typedef enum {
    TIPO_DEPOSITO,
    TIPO_HUB_POTENCIAL,
    TIPO_DESTINO_PAQUETE
} TipoUbicacion;

typedef struct {
    int id;                   
    TipoUbicacion tipo;       
    double costo_activacion;  
} Nodo;

typedef struct {
    int num_nodos;
    int num_hubs;
    Nodo *nodos;
    double **matriz_adyacencia;
    double **distancias_minimas;
} Grafo;

// --- FIRMAS DE FUNCIONES ---

Grafo *crear_grafo(int num_nodos, int num_hubs);
void liberar_grafo(Grafo *g);
void calcular_distancias_minimas(Grafo *g);
double obtener_distancia_minima(Grafo *g, int origen_id, int destino_id);

#endif