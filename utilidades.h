#ifndef UTILIDADES_H
#define UTILIDADES_H

#include "grafo.h"
#include <stdio.h>

#define FILENAME_OUTPUT "solucion.txt"

// --- ESTRUCTURAS CENTRALIZADAS ---

typedef struct Paquete {
    int id_paquete;
    int id_destino; 
} Paquete;

typedef struct SolucionOptima {
    double costo_total_minimo;
    double distancia_minima;
    double costo_hubs_optimo;
    int *hubs_activados;
    int num_hubs_activos;
    int *ruta_optima;
    int largo_ruta_optima;
} SolucionOptima;

typedef struct EstadoRuta {
    int num_hubs;
    int paquetes_totales;
    int capacidad_maxima; 
    
    // Estado de la busqueda
    bool *hubs_activos;        
    bool *paquetes_entregados;  
    bool *paquetes_en_camion;   
    int paquetes_cargados_count; 
    bool *paquetes_en_hub; 
    
    int ubicacion_actual_id;    
    
    // Metrica y Ruta
    double costo_total_parcial; 
    double costo_hubs_fijo;     
    double distancia_recorrida_parcial; 

    // Ruta
    int *ruta_secuencia;
    int largo_ruta;
} EstadoRuta;

// --- FIRMAS DE FUNCIONES ---

Grafo *leer_archivo_entrada(const char *filename, Paquete **paquetes_out, int *num_paquetes_out, int *capacidad_out, int *num_hubs_out);

void generar_archivo_salida(const char *filename, SolucionOptima *solucion, double tiempo_ejecucion);

#endif 