# ==============================================================================
# Makefile para el Trabajo Práctico Obligatorio (TPO) - Programación III
# ==============================================================================

# Nombre del ejecutable final, según el ejemplo de la consigna: ./mi_programa caso_mediano.txt
TARGET = mi_programa

# Compilador (usa 'gcc' si instalaste GCC o 'clang' si instalaste Clang)
# Asegúrate de que este comando funcione en tu terminal.
CC = gcc 

# Banderas de compilación:
# -Wall: Habilita todas las advertencias (buenas prácticas)
# -Werror: Trata las advertencias como errores (ideal para calidad de código)
# -std=c99: Usa el estándar C99 (o -std=c11, si prefieres)
# -O2: Nivel de optimización 2 (mejora la eficiencia, crucial para el Caso Grande)
# -g: Incluye información de debugging (opcional, útil para gdb)
CFLAGS = -Wall -Werror -std=c99 -O2 -g

# Archivos objeto (.o) que se generarán a partir de los archivos fuente (.c)
OBJS = main.o grafo.o logistica.o utilidades.o

# ==============================================================================
# REGLAS DE COMPILACIÓN
# ==============================================================================

# Regla default (compilar el ejecutable)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Regla para compilar main.c -> main.o
main.o: main.c grafo.h logistica.h utilidades.h
	$(CC) $(CFLAGS) -c main.c

# Regla para compilar grafo.c -> grafo.o
grafo.o: grafo.c grafo.h
	$(CC) $(CFLAGS) -c grafo.c

# Regla para compilar logistica.c -> logistica.o
logistica.o: logistica.c logistica.h grafo.h
	$(CC) $(CFLAGS) -c logistica.c

# Regla para compilar utilidades.c -> utilidades.o
utilidades.o: utilidades.c utilidades.h grafo.h logistica.h
	$(CC) $(CFLAGS) -c utilidades.c

# ==============================================================================
# REGLAS AUXILIARES
# ==============================================================================

# Regla 'clean': Limpia todos los archivos compilados (.o) y el ejecutable
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)

# Regla 'run': Compila y ejecuta con un caso de ejemplo (ajustar el nombre del archivo)
.PHONY: run
run: $(TARGET)
	./$(TARGET) caso_pequeno.txt