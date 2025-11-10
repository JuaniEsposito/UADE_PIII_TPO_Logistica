TARGET = mi_programa

CC = gcc 

CFLAGS = -Wall -Werror -std=c99 -O2 -g

OBJS = main.o grafo.o logistica.o utilidades.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

main.o: main.c grafo.h logistica.h utilidades.h
	$(CC) $(CFLAGS) -c main.c

grafo.o: grafo.c grafo.h
	$(CC) $(CFLAGS) -c grafo.c

logistica.o: logistica.c logistica.h grafo.h
	$(CC) $(CFLAGS) -c logistica.c

utilidades.o: utilidades.c utilidades.h grafo.h logistica.h
	$(CC) $(CFLAGS) -c utilidades.c

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: run
run: $(TARGET)
	./$(TARGET) caso_pequeno.txt