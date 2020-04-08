#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "util.h"


void
Error(const char * error) {
  perror(error);
  exit(EXIT_FAILURE);
}


// Función de utilidad, para generar los tiempos usados por los aviones
// Devuelve un número aleatorio comprendido entre min y max
double randRange(double min, double max)
{
  return min + (rand() / (double) RAND_MAX * (max - min + 1));
}


// Función de utilidad para depuración. Emite por pantalla el mensaje
// que se le pasa como parámetro, pero pone delante del mensaje un
// timestamp, para poder ordenar la salida por si saliera desordenada
//
// Ejemplo de uso:
//
//  log_debug("Avion en vuelo")
//
// Más ejemplos en el programa principal.
void log_debug(char *msg){
  struct timespec t;
  clock_gettime(_POSIX_MONOTONIC_CLOCK, &t);
  printf("[%ld.%09ld] %s", t.tv_sec, t.tv_nsec, msg);
}


// Función de utilidad para mostrar el array estado_pistas
// precedido de un mensaje que recibe como parámetro.
//
// Ejemplo de uso:
//
//   mostrar_estado_pistas("Antes de reservar pista", estado_pistas, MAX_PISTAS)
//   ...
//   mostrar_estado_pistas("Despues de reservar pista", estado_pistas, MAX_PISTAS)
//
void mostrar_estado_pistas(char * msg, int *estado_pistas, int num_pistas) {
    char buff[200];
    sprintf(buff, "%s -> ESTADO PISTAS: [ ", msg);
    flockfile(stdout);
    log_debug(buff);
    for (int i=0; i<num_pistas;i++) {
        printf("%d ", estado_pistas[i]);
    }
    printf("]\n");
    funlockfile(stdout);
}