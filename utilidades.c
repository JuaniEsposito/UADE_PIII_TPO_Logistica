#include "utilidades.h"
#include <string.h>
#include <limits.h>

// La definición de SOLUCION_GLOBAL está en logistica.c
extern SolucionOptima SOLUCION_GLOBAL;

// Función auxiliar para consumir caracteres hasta el final de la línea o EOF
static void consumir_linea(FILE *fp) {
    int c;
    while ((c = fgetc(fp)) != '\n' && c != EOF);
}

// NUEVA FUNCIÓN: Se asegura de que el puntero del archivo salte el mensaje de status del generador
static void sync_file_pointer(FILE *fp) {
    long current_pos = ftell(fp);
    char buffer[128]; // <-- DECLARADA AQUÍ
    
    // Intentar leer la primera línea
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Si la línea contiene el mensaje de semilla, significa que el archivo está corrupto.
        if (strstr(buffer, "Generador iniciado con semilla:") != NULL) {
            // El puntero ya avanzó. La próxima lectura será la CONFIGURACION.
            return;
        }
    }
    
    // Si la lectura falló o no encontramos el mensaje, rebobinamos.
    fseek(fp, current_pos, SEEK_SET);
}


// Función auxiliar para buscar el inicio de una sección específica
static bool buscar_seccion(FILE *fp, const char *encabezado) {
    char buffer[256]; // <-- DECLARADA AQUÍ
    while(fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Ignorar líneas vacías
        if (strlen(buffer) <= 1 && buffer[0] == '\n') continue;
        
        // Buscar el encabezado
        if (strstr(buffer, encabezado) != NULL) {
            return true;
        }
    }
    return false;
}


// Implementación de leer_archivo_entrada para el formato 'caso.txt'
Grafo *leer_archivo_entrada(const char *filename, Paquete **paquetes_out, int *num_paquetes_out, int *capacidad_out, int *num_hubs_out) {
    
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error al abrir el archivo de entrada");
        return NULL;
    }

    // ************* 1. SINCRONIZACIÓN INICIAL *************
    sync_file_pointer(fp); 
    
    // Inicialización de punteros y variables (seguridad)
    Grafo *g = NULL;
    *paquetes_out = NULL;
    
    // Las variables buffer ya NO se declaran aquí, sino dentro de sync_file_pointer/buscar_seccion
    char key[50];
    int num_nodos = 0;
    int deposito_id = 0;
    int capacidad_camion = 0;

    // --- 1. LECTURA DE CONFIGURACIÓN ---
    if (!buscar_seccion(fp, "// --- CONFIGURACION ---")) { fclose(fp); return NULL; }
    
    // Leemos los 5 parámetros de configuración, consumiendo el \n después de cada uno.
    if (fscanf(fp, "%s %d", key, &num_nodos) != 2 || strcmp(key, "NODOS") != 0) { fclose(fp); return NULL; }
    consumir_linea(fp); 
    
    if (fscanf(fp, "%s %d", key, num_hubs_out) != 2 || strcmp(key, "HUBS") != 0) { fclose(fp); return NULL; }
    consumir_linea(fp); 

    if (fscanf(fp, "%s %d", key, num_paquetes_out) != 2 || strcmp(key, "PAQUETES") != 0) { fclose(fp); return NULL; }
    consumir_linea(fp); 

    if (fscanf(fp, "%s %d", key, &capacidad_camion) != 2 || strcmp(key, "CAPACIDAD_CAMION") != 0) { fclose(fp); return NULL; }
    consumir_linea(fp); 

    if (fscanf(fp, "%s %d", key, &deposito_id) != 2 || strcmp(key, "DEPOSITO_ID") != 0) { fclose(fp); return NULL; }
    consumir_linea(fp); // Consumir el \n después de DEPOSITO_ID
    
    *capacidad_out = capacidad_camion;
    
    // Inicializar Grafo y Paquetes después de leer num_nodos/num_paquetes
    g = crear_grafo(num_nodos, *num_hubs_out);
    *paquetes_out = (Paquete *)malloc(*num_paquetes_out * sizeof(Paquete));

    if (!g || !*paquetes_out) {
        fprintf(stderr, "Error: Fallo de asignacion de memoria.\n");
        liberar_grafo(g); free(*paquetes_out); fclose(fp); return NULL;
    }
    
    // --- 2. NODOS ---
    if (!buscar_seccion(fp, "// --- NODOS (ID X Y) ---")) { fclose(fp); return NULL; }

    for (int i = 0; i < num_nodos; i++) {
        int id, x, y;
        if (fscanf(fp, "%d %d %d", &id, &x, &y) != 3) {
            fprintf(stderr, "Error leyendo NODOS en línea %d.\n", i);
            liberar_grafo(g); free(*paquetes_out); fclose(fp); return NULL;
        }
        
        g->nodos[id].id = id;
        if (id == deposito_id) g->nodos[id].tipo = TIPO_DEPOSITO;
        else if (id <= *num_hubs_out && id > 0) g->nodos[id].tipo = TIPO_HUB_POTENCIAL;
        else g->nodos[id].tipo = TIPO_DESTINO_PAQUETE;
        
        consumir_linea(fp); // Consumir el resto de la línea (comentarios)
    }

    // --- 3. HUBS ---
    if (!buscar_seccion(fp, "// --- HUBS (ID COSTO_ACTIVACION) ---")) { fclose(fp); return NULL; }
    
    for (int i = 0; i < *num_hubs_out; i++) {
        int id_hub;
        double costo_activacion;
        if (fscanf(fp, "%d %lf", &id_hub, &costo_activacion) != 2) {
             fprintf(stderr, "Error leyendo HUBS en línea %d.\n", i);
             liberar_grafo(g); free(*paquetes_out); fclose(fp); return NULL;
        }
        if (id_hub < g->num_nodos) {
             g->nodos[id_hub].costo_activacion = costo_activacion;
        }
        consumir_linea(fp); // Consumir el resto de la línea
    }

    // --- 4. PAQUETES ---
    if (!buscar_seccion(fp, "// --- PAQUETES (ID NODO_ORIGEN NODO_DESTINO) ---")) { 
        fprintf(stderr, "Error: No se encontró el encabezado de PAQUETES.\n");
        liberar_grafo(g); free(*paquetes_out); fclose(fp); return NULL;
    }
    
    for (int i = 0; i < *num_paquetes_out; i++) {
        int id, origen, destino;
        if (fscanf(fp, "%d %d %d", &id, &origen, &destino) != 3) {
             fprintf(stderr, "ERROR FATAL: Lectura incompleta de PAQUETE %d\n", i);
             liberar_grafo(g); free(*paquetes_out); fclose(fp); return NULL;
        }
        (*paquetes_out)[i].id_paquete = id;
        (*paquetes_out)[i].id_destino = destino;
        consumir_linea(fp); // Consumir el resto de la línea
    }
    
    // --- 5. ARISTAS ---
    if (!buscar_seccion(fp, "// --- ARISTAS (NODO1 NODO2 PESO) ---")) {
         fprintf(stderr, "Error: No se encontró el encabezado de ARISTAS.\n");
         liberar_grafo(g); free(*paquetes_out); fclose(fp); return NULL;
    }
    
    int u = 0, v = 0; 
    double peso;
    while(fscanf(fp, "%d %d %lf", &u, &v, &peso) == 3) {
        if (u < g->num_nodos && v < g->num_nodos) {
            g->matriz_adyacencia[u][v] = peso;
            g->matriz_adyacencia[v][u] = peso; // Grafo no dirigido
        }
    }

    fclose(fp);
    return g;
}


void generar_archivo_salida(const char *filename, SolucionOptima *solucion, double tiempo_ejecucion) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Error al crear el archivo de salida");
        return;
    }

    // --- HUBS ACTIVADOS ---
    fprintf(f, "//\n");
    fprintf(f, "HUBS ACTIVADOS\n");
    for (int i = 0; i < solucion->num_hubs_activos; i++) {
        fprintf(f, "%d\n", solucion->hubs_activados[i]); 
    }
    
    // --- RUTA ÓPTIMA ---
    fprintf(f, "//RUTA OPTIMA\n");
    for (int i = 0; i < solucion->largo_ruta_optima; i++) {
        fprintf(f, "%d", solucion->ruta_optima[i]);
        if (i < solucion->largo_ruta_optima - 1) {
            fprintf(f, " -> ");
        }
    }
    fprintf(f, "\n");

    // --- MÉTRICAS ---
    fprintf(f, "//METRICAS\n");
    fprintf(f, "COSTO_TOTAL: %.2f\n", solucion->costo_total_minimo);
    fprintf(f, "DISTANCIA_RECORRIDA: %.2f\n", solucion->distancia_minima);
    fprintf(f, "COSTO_HUBS: %.2f\n", solucion->costo_hubs_optimo);
    fprintf(f, "TIEMPO EJECUCION: %.6f segundos\n", tiempo_ejecucion); 

    fclose(f);
}