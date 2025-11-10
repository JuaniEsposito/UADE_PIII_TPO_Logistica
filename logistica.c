#include "logistica.h"
#include "utilidades.h"
#include <string.h>
#include <limits.h>

// Definicion de la variable global para la poda
SolucionOptima SOLUCION_GLOBAL;

// FUNCION AUXILIAR DE PODA HEURISTICA

double calcular_costo_minimo_restante(Grafo *g, Paquete *paquetes, EstadoRuta *estado) {
    double costo_restante = 0.0;
    int u = estado->ubicacion_actual_id;

    // 1. Costo minimo para entregar todos los paquetes restantes.
    for (int p = 0; p < estado->paquetes_totales; p++) {
        if (!estado->paquetes_entregados[p]) {
            double dist = obtener_distancia_minima(g, u, paquetes[p].id_destino);
            costo_restante += dist; 
            u = paquetes[p].id_destino; 
        }
    }
    
    // 2. Costo minimo para regresar al deposito
    costo_restante += obtener_distancia_minima(g, u, DEPOSITO_ID);
    
    return costo_restante;
}

// GESTION DE ESTADO Y COPIA

void inicializar_solucion_global() {
    if (SOLUCION_GLOBAL.hubs_activados) free(SOLUCION_GLOBAL.hubs_activados);
    if (SOLUCION_GLOBAL.ruta_optima) free(SOLUCION_GLOBAL.ruta_optima);

    SOLUCION_GLOBAL.costo_total_minimo = INFINITY_COST;
    SOLUCION_GLOBAL.distancia_minima = 0.0;
    SOLUCION_GLOBAL.costo_hubs_optimo = 0.0;
    SOLUCION_GLOBAL.hubs_activados = NULL;
    SOLUCION_GLOBAL.ruta_optima = NULL;
    SOLUCION_GLOBAL.largo_ruta_optima = 0;
}

EstadoRuta *crear_estado_inicial(int capacidad, int paquetes_totales, int num_hubs) {
    EstadoRuta *estado = (EstadoRuta *)malloc(sizeof(EstadoRuta));
    
    estado->num_hubs = num_hubs;
    estado->paquetes_totales = paquetes_totales;
    estado->capacidad_maxima = capacidad;
    
    size_t size_paquetes = paquetes_totales * sizeof(bool);
    size_t size_inventario_hub = num_hubs * paquetes_totales * sizeof(bool);
    
    estado->hubs_activos = (bool *)calloc(num_hubs, sizeof(bool));
    estado->paquetes_entregados = (bool *)calloc(size_paquetes, 1);
    estado->paquetes_en_camion = (bool *)calloc(size_paquetes, 1);
    estado->paquetes_en_hub = (bool *)calloc(size_inventario_hub, 1); 
    estado->paquetes_cargados_count = 0;
    
    estado->ubicacion_actual_id = DEPOSITO_ID; 
    
    estado->costo_hubs_fijo = 0.0;
    estado->distancia_recorrida_parcial = 0.0;
    estado->costo_total_parcial = 0.0;
    
    // Ruta inicial-> se asigna espacio para una ruta larga
    estado->ruta_secuencia = (int *)malloc((2 * paquetes_totales + num_hubs) * sizeof(int));
    estado->ruta_secuencia[0] = DEPOSITO_ID;
    estado->largo_ruta = 1;
    
    return estado;
}

void liberar_estado(EstadoRuta *estado) {
    if (estado) {
        free(estado->hubs_activos);
        free(estado->paquetes_entregados);
        free(estado->paquetes_en_camion);
        free(estado->paquetes_en_hub);
        free(estado->ruta_secuencia);
        
        free(estado); 
    }
}

EstadoRuta *copiar_estado(const EstadoRuta *original) {
    EstadoRuta *copia = (EstadoRuta *)malloc(sizeof(EstadoRuta));
    memcpy(copia, original, sizeof(EstadoRuta)); 
    
    size_t size_paquetes = original->paquetes_totales * sizeof(bool);
    size_t size_inventario_hub = original->num_hubs * original->paquetes_totales * sizeof(bool);

    copia->hubs_activos = (bool *)malloc(original->num_hubs * sizeof(bool)); 
    memcpy(copia->hubs_activos, original->hubs_activos, original->num_hubs * sizeof(bool));

    copia->paquetes_entregados = (bool *)malloc(size_paquetes);
    memcpy(copia->paquetes_entregados, original->paquetes_entregados, size_paquetes);

    copia->paquetes_en_camion = (bool *)malloc(size_paquetes);
    memcpy(copia->paquetes_en_camion, original->paquetes_en_camion, size_paquetes);
    
    copia->paquetes_en_hub = (bool *)malloc(size_inventario_hub);
    memcpy(copia->paquetes_en_hub, original->paquetes_en_hub, size_inventario_hub);

    copia->ruta_secuencia = (int *)malloc((original->largo_ruta + 1) * sizeof(int));
    memcpy(copia->ruta_secuencia, original->ruta_secuencia, original->largo_ruta * sizeof(int));

    return copia;
}

void actualizar_solucion_global(EstadoRuta *estado, double costo_final, double distancia_final) {
    // 1. Liberar rutas antiguas
    if (SOLUCION_GLOBAL.ruta_optima) free(SOLUCION_GLOBAL.ruta_optima);
    if (SOLUCION_GLOBAL.hubs_activados) free(SOLUCION_GLOBAL.hubs_activados);

    // 2. Actualizar metricas
    SOLUCION_GLOBAL.costo_total_minimo = costo_final;
    SOLUCION_GLOBAL.distancia_minima = distancia_final;
    SOLUCION_GLOBAL.costo_hubs_optimo = estado->costo_hubs_fijo;

    // 3. Copiar la ruta
    SOLUCION_GLOBAL.largo_ruta_optima = estado->largo_ruta;
    SOLUCION_GLOBAL.ruta_optima = (int *)malloc(estado->largo_ruta * sizeof(int));
    memcpy(SOLUCION_GLOBAL.ruta_optima, estado->ruta_secuencia, estado->largo_ruta * sizeof(int));

    // 4. Copiar los hubs activados
    int count = 0;
    for (int i = 0; i < estado->num_hubs; i++) {
        if (estado->hubs_activos[i]) count++;
    }
    SOLUCION_GLOBAL.num_hubs_activos = count;
    SOLUCION_GLOBAL.hubs_activados = (int *)malloc(count * sizeof(int));
    
    int idx = 0;
    for (int i = 0; i < estado->num_hubs; i++) {
        if (estado->hubs_activos[i]) {
            SOLUCION_GLOBAL.hubs_activados[idx++] = i + 1; 
        }
    }
}


// LOGICA DE BACKTRACKING: IMPLEMENTACION

bool todos_entregados(EstadoRuta *estado) {
    for (int i = 0; i < estado->paquetes_totales; i++) {
        if (!estado->paquetes_entregados[i]) return false;
    }
    return true;
}

// Funcion envolvente
void iniciar_backtracking(Grafo *g, Paquete *paquetes, EstadoRuta *estado_inicial) {
    explorar_hubs(g, paquetes, estado_inicial, 0);
}


// Nivel 1: Explora las 2^H combinaciones de activacion de hubs
void explorar_hubs(Grafo *g, Paquete *paquetes, EstadoRuta *estado_base, int hub_index) {
    
    if (hub_index == estado_base->num_hubs) {
        // Caso Base: Hubs fijos. Calcular costo fijo e iniciar ruta.
        EstadoRuta *ruta_estado = copiar_estado(estado_base);
        
        double costo_fijo = 0.0;
        for (int i = 0; i < estado_base->num_hubs; i++) {
            if (estado_base->hubs_activos[i]) {
                costo_fijo += g->nodos[i + 1].costo_activacion;
            }
        }
        ruta_estado->costo_hubs_fijo = costo_fijo;
        ruta_estado->costo_total_parcial = costo_fijo;
        
        for (int p = 0; p < ruta_estado->paquetes_totales; p++) {
            if (ruta_estado->paquetes_cargados_count < ruta_estado->capacidad_maxima) {
                ruta_estado->paquetes_en_camion[p] = true;
                ruta_estado->paquetes_cargados_count++;
            }
        }
        
        // Si hay algo que entregar, iniciamos la busqueda
        if (ruta_estado->paquetes_cargados_count > 0) {
            buscar_ruta_optima(g, paquetes, ruta_estado);
        } else {
            liberar_estado(ruta_estado);
        }
        
        return;
    }

    // Opcion 1: No activar el hub en 'hub_index'
    EstadoRuta *estado_no_activado = copiar_estado(estado_base);
    estado_no_activado->hubs_activos[hub_index] = false;
    explorar_hubs(g, paquetes, estado_no_activado, hub_index + 1);
    
    // Opcion 2: Activar el hub en 'hub_index'
    EstadoRuta *estado_activado = copiar_estado(estado_base);
    estado_activado->hubs_activos[hub_index] = true;
    explorar_hubs(g, paquetes, estado_activado, hub_index + 1);

    // LIBERAR EL ESTADO BASE DE ESTE NIVEL
    liberar_estado(estado_base); 
}


// Nivel 2: Explora la ruta para un conjunto fijo de hubs
void buscar_ruta_optima(Grafo *g, Paquete *paquetes, EstadoRuta *estado_actual) {

    // 1. PODA
    double costo_estimado_total = estado_actual->costo_total_parcial;
    
    if (SOLUCION_GLOBAL.costo_total_minimo != INFINITY_COST) {
        costo_estimado_total += calcular_costo_minimo_restante(g, paquetes, estado_actual);
    }
    
    // Poda: Si el costo actual + la mejor estimacion restante supera el optimo global.
    if (costo_estimado_total >= SOLUCION_GLOBAL.costo_total_minimo) {
        liberar_estado(estado_actual);
        return;
    }

    // 2. CONDICION DE PARADA
    if (todos_entregados(estado_actual)) {
        int u = estado_actual->ubicacion_actual_id;
        double costo_vuelta = 0.0;
        
        // Regreso al deposito si es necesario
        if (u != DEPOSITO_ID) {
            costo_vuelta = obtener_distancia_minima(g, u, DEPOSITO_ID);
        }
        
        double costo_total_final = estado_actual->costo_total_parcial + costo_vuelta;

        if (costo_total_final < SOLUCION_GLOBAL.costo_total_minimo) {
            // Actualizar la solucion global
            EstadoRuta *estado_final = copiar_estado(estado_actual);
            if (u != DEPOSITO_ID) {
                estado_final->ruta_secuencia[estado_final->largo_ruta++] = DEPOSITO_ID;
            }
            actualizar_solucion_global(estado_final, costo_total_final, estado_final->distancia_recorrida_parcial + costo_vuelta);
            liberar_estado(estado_final); 
        }
        
        liberar_estado(estado_actual);
        return;
    }

    // 3. EXPLORACION DE TRANSICIONES
    int u = estado_actual->ubicacion_actual_id;

    // A. Transicion: **ENTREGA** 
    for (int p = 0; p < estado_actual->paquetes_totales; p++) {
        if (estado_actual->paquetes_en_camion[p] && !estado_actual->paquetes_entregados[p]) {
            EstadoRuta *siguiente = copiar_estado(estado_actual);
            int v = paquetes[p].id_destino;
            double costo_viaje = obtener_distancia_minima(g, u, v);
            
            if (costo_viaje >= INFINITY_COST) { liberar_estado(siguiente); continue; }

            siguiente->distancia_recorrida_parcial += costo_viaje;
            siguiente->costo_total_parcial = siguiente->costo_hubs_fijo + siguiente->distancia_recorrida_parcial;
            siguiente->ubicacion_actual_id = v;
            
            // Actualizacion de Paquetes
            siguiente->paquetes_entregados[p] = true;
            siguiente->paquetes_en_camion[p] = false;
            siguiente->paquetes_cargados_count--;

            // Añadir a la ruta
            siguiente->ruta_secuencia[siguiente->largo_ruta++] = v;
            
            buscar_ruta_optima(g, paquetes, siguiente);
        }
    }

    // B. Transicion: **INTERACCION CON DEPOSITO/HUBS** 
    
    // I. Recarga en Deposito (ID 0)
    if (u == DEPOSITO_ID) { 
        if (estado_actual->paquetes_cargados_count < estado_actual->capacidad_maxima) {
            
            EstadoRuta *temp_carga = copiar_estado(estado_actual);
            
            for (int p = 0; p < temp_carga->paquetes_totales; p++) {
                bool no_entregado = !temp_carga->paquetes_entregados[p];
                bool no_en_camion = !temp_carga->paquetes_en_camion[p];
                
                if (no_entregado && no_en_camion && temp_carga->paquetes_cargados_count < temp_carga->capacidad_maxima) {
                    temp_carga->paquetes_en_camion[p] = true;
                    temp_carga->paquetes_cargados_count++;
                }
            }

            if (temp_carga->paquetes_cargados_count > estado_actual->paquetes_cargados_count) {
                 buscar_ruta_optima(g, paquetes, temp_carga);
            } else {
                 liberar_estado(temp_carga);
            }
        }
    } else {
        if (estado_actual->paquetes_cargados_count == 0 && !todos_entregados(estado_actual)) {
            EstadoRuta *temp_vuelta = copiar_estado(estado_actual);
            double costo_viaje = obtener_distancia_minima(g, u, DEPOSITO_ID);
            
            temp_vuelta->distancia_recorrida_parcial += costo_viaje;
            temp_vuelta->costo_total_parcial = temp_vuelta->costo_hubs_fijo + temp_vuelta->distancia_recorrida_parcial;
            temp_vuelta->ubicacion_actual_id = DEPOSITO_ID;
            temp_vuelta->ruta_secuencia[temp_vuelta->largo_ruta++] = DEPOSITO_ID;
            
            buscar_ruta_optima(g, paquetes, temp_vuelta);
        }
    }
    
    liberar_estado(estado_actual);
}