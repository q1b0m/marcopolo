
aeropuerto: aeropuerto.o cola.o util.o
	gcc -o aeropuerto aeropuerto.o cola.o util.o -pthread

aeropuerto.o: aeropuerto.c
	gcc -c aeropuerto.c

cola.o: cola.c cola.h
	gcc -c cola.c

util.o: util.c util.h
	gcc -c util.c

clean:
	-rm *.o
