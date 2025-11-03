#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "grafo.h"
#include "logistica.h" 
#include "utilidades.h" 

// La variable global SOLUCION_GLOBAL está declarada externamente en logistica.h
extern SolucionOptima SOLUCION_GLOBAL;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_de_entrada.txt>\n", argv[0]);
        return 1;
    }

    // --- 1. INICIALIZACIÓN DE TIEMPO ---
    clock_t start_time = clock();
    
    // --- 2. LECTURA Y CARGA DE DATOS ---
    Grafo *mapa = NULL;
    Paquete *paquetes = NULL;
    int num_paquetes = 0;
    int capacidad_camion = 0;
    int num_hubs = 0;
    
    mapa = leer_archivo_entrada(argv[1], &paquetes, &num_paquetes, &capacidad_camion, &num_hubs);
    
    if (!mapa) {
        fprintf(stderr, "Error: Falló la carga del archivo de entrada.\n");
        return 1;
    }

    // --- 3. PRE-CÁLCULO DE DISTANCIAS ---
    calcular_distancias_minimas(mapa);
    
    // --- 4. BÚSQUEDA DE SOLUCIÓN ÓPTIMA ---
    inicializar_solucion_global(); 
    
    EstadoRuta *estado_inicial = crear_estado_inicial(capacidad_camion, num_paquetes, num_hubs);
    
    iniciar_backtracking(mapa, paquetes, estado_inicial);
    
    // --- 5. MÉTRICAS Y SALIDA ---
    clock_t end_time = clock();
    double tiempo_ejecucion = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    if (SOLUCION_GLOBAL.costo_total_minimo != INFINITY_COST) {
        
        // **********************************************
        // LÓGICA CLAVE PARA NOMBRAR EL ARCHIVO DE SALIDA
        // **********************************************
        char filename_output[100];
        const char *input_name = argv[1]; 

        // Buscar la posición de "caso" en el nombre de entrada.
        const char *suffix_pos = strstr(input_name, "caso"); 
        
        if (suffix_pos != NULL) {
            // Construir el nuevo nombre: "solucion" + resto del nombre después de "caso"
            // Por ejemplo: "caso_pequeno.txt" -> "solucion_pequeno.txt"
            // suffix_pos + 4 avanza 4 caracteres desde la 'c' (c-a-s-o) para llegar al '_'
            snprintf(filename_output, sizeof(filename_output), "solucion%s", suffix_pos + 4);
        } else {
            // Fallback si no se encuentra "caso"
            snprintf(filename_output, sizeof(filename_output), "solucion_%s", input_name);
        }
        
        // Llamada a generar_archivo_salida con el nombre dinámico
        generar_archivo_salida(filename_output, &SOLUCION_GLOBAL, tiempo_ejecucion);

        printf("Solución óptima encontrada en %.6f segundos. Costo Total: %.2f. Guardada en %s\n", 
               tiempo_ejecucion, SOLUCION_GLOBAL.costo_total_minimo, filename_output);
    } else {
        printf("No se encontró una solución válida.\n");
    }
    
    // --- 6. LIBERACIÓN DE MEMORIA ---
    free(paquetes);
    liberar_grafo(mapa);
    
    return 0;
}