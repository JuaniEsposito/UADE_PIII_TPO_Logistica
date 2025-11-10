#ifndef LOGISTICA_H
#define LOGISTICA_H

#include "grafo.h"
#include <stdbool.h> 

typedef struct Paquete Paquete;
typedef struct SolucionOptima SolucionOptima;
typedef struct EstadoRuta EstadoRuta;

// --- ESTRUCTURAS ---

// --- FIRMAS DE FUNCIONES ---

void inicializar_solucion_global();

// Nivel 1: Exploracion de combinaciones de Hubs
void explorar_hubs(Grafo *g, Paquete *paquetes, EstadoRuta *estado_base, int hub_index);

// Nivel 2: Exploracion de la Ruta 
void buscar_ruta_optima(Grafo *g, Paquete *paquetes, EstadoRuta *estado_actual);

// Funcion de entrada principal para el Backtracking
void iniciar_backtracking(Grafo *g, Paquete *paquetes, EstadoRuta *estado_inicial);

// Funciones de Estado
EstadoRuta *crear_estado_inicial(int capacidad, int paquetes_totales, int num_hubs);
void liberar_estado(EstadoRuta *estado);
EstadoRuta *copiar_estado(const EstadoRuta *original);

#endif 