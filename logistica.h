#ifndef LOGISTICA_H
#define LOGISTICA_H

#include "grafo.h"
#include <stdbool.h> // Necesario para 'bool'

// Declaración forward para las estructuras definidas en otro lugar
typedef struct Paquete Paquete;
typedef struct SolucionOptima SolucionOptima;
typedef struct EstadoRuta EstadoRuta;

// --- ESTRUCTURAS (Declaradas en utilidades.h) ---

// --- FIRMAS DE FUNCIONES ---

void inicializar_solucion_global();

// Nivel 1: Exploración de combinaciones de Hubs
void explorar_hubs(Grafo *g, Paquete *paquetes, EstadoRuta *estado_base, int hub_index);

// Nivel 2: Exploración de la Ruta (para un conjunto fijo de hubs activos)
void buscar_ruta_optima(Grafo *g, Paquete *paquetes, EstadoRuta *estado_actual);

// Función de entrada principal para el Backtracking
void iniciar_backtracking(Grafo *g, Paquete *paquetes, EstadoRuta *estado_inicial);

// Funciones de Estado
EstadoRuta *crear_estado_inicial(int capacidad, int paquetes_totales, int num_hubs);
void liberar_estado(EstadoRuta *estado);
EstadoRuta *copiar_estado(const EstadoRuta *original);

#endif // LOGISTICA_H