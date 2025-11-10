#include "utilidades.h"
#include <string.h>
#include <limits.h>

extern SolucionOptima SOLUCION_GLOBAL;

static void consumir_linea(FILE *fp) {
    int c;
    while ((c = fgetc(fp)) != '\n' && c != EOF);
}

static void sync_file_pointer(FILE *fp) {
    long current_pos = ftell(fp);
    char buffer[128]; 
    
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "Generador iniciado con semilla:") != NULL) {
            return;
        }
    }
    
    fseek(fp, current_pos, SEEK_SET);
}

static bool buscar_seccion(FILE *fp, const char *encabezado) {
    char buffer[256]; 
    while(fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strlen(buffer) <= 1 && buffer[0] == '\n') continue;
        
        if (strstr(buffer, encabezado) != NULL) {
            return true;
        }
    }
    return false;
}


Grafo *leer_archivo_entrada(const char *filename, Paquete **paquetes_out, int *num_paquetes_out, int *capacidad_out, int *num_hubs_out) {
    
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error al abrir el archivo de entrada");
        return NULL;
    }

    //  1. SINCRONIZACION INICIAL 
    sync_file_pointer(fp); 
    
    Grafo *g = NULL;
    *paquetes_out = NULL;
    
    char key[50];
    int num_nodos = 0;
    int deposito_id = 0;
    int capacidad_camion = 0;

    // --- 1. LECTURA DE CONFIGURACION ---
    if (!buscar_seccion(fp, "// --- CONFIGURACION ---")) { fclose(fp); return NULL; }
    
    if (fscanf(fp, "%s %d", key, &num_nodos) != 2 || strcmp(key, "NODOS") != 0) { fclose(fp); return NULL; }
    consumir_linea(fp); 
    
    if (fscanf(fp, "%s %d", key, num_hubs_out) != 2 || strcmp(key, "HUBS") != 0) { fclose(fp); return NULL; }
    consumir_linea(fp); 

    if (fscanf(fp, "%s %d", key, num_paquetes_out) != 2 || strcmp(key, "PAQUETES") != 0) { fclose(fp); return NULL; }
    consumir_linea(fp); 

    if (fscanf(fp, "%s %d", key, &capacidad_camion) != 2 || strcmp(key, "CAPACIDAD_CAMION") != 0) { fclose(fp); return NULL; }
    consumir_linea(fp); 

    if (fscanf(fp, "%s %d", key, &deposito_id) != 2 || strcmp(key, "DEPOSITO_ID") != 0) { fclose(fp); return NULL; }
    consumir_linea(fp); 
    
    *capacidad_out = capacidad_camion;
    
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
        
        consumir_linea(fp); 
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
        consumir_linea(fp); 
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
        consumir_linea(fp); 
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
            g->matriz_adyacencia[v][u] = peso; 
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
    
    // --- RUTA OPTIMA ---
    fprintf(f, "//RUTA OPTIMA\n");
    for (int i = 0; i < solucion->largo_ruta_optima; i++) {
        fprintf(f, "%d", solucion->ruta_optima[i]);
        if (i < solucion->largo_ruta_optima - 1) {
            fprintf(f, " -> ");
        }
    }
    fprintf(f, "\n");

    // --- METRICAS ---
    fprintf(f, "//METRICAS\n");
    fprintf(f, "COSTO_TOTAL: %.2f\n", solucion->costo_total_minimo);
    fprintf(f, "DISTANCIA_RECORRIDA: %.2f\n", solucion->distancia_minima);
    fprintf(f, "COSTO_HUBS: %.2f\n", solucion->costo_hubs_optimo);
    fprintf(f, "TIEMPO EJECUCION: %.6f segundos\n", tiempo_ejecucion); 

    fclose(f);
}