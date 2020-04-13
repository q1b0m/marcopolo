#include <stdio.h>   
#include <errno.h>    
#include <stdlib.h>   
#include <stdint.h>

#include <rpc/types.h>
#include <rpc/rpc.h> 

#include "tipos.h"


void
Error(const char * error) {
	perror(error);
	exit(EXIT_FAILURE);
}


void 
anade_elemento(int dato, Lista **primero)
{
  // Lo primero crear el nuevo Elemento
  Lista *nuevo;

  nuevo = malloc(sizeof(Lista));

  if (!nuevo) Error("Malloc failed add element");

  // Meter dentro de él el dato
  nuevo->dato = dato;
  // Y ya que será el nuevo primer elemento, apunta al antiguo nuevo
  nuevo->siguiente = *primero;

  // El nuevo primero es ahora el recién creado
  *primero = nuevo;
}

void 
trata_datos(Lista *primero, Lista **output){

	Lista *elemento;
  elemento = primero;
  
  while (elemento != NULL) {

  	anade_elemento(elemento->dato*2, output);
    elemento = elemento->siguiente;
  }
}

void 
imprime_datos_lista(Lista *primero)
{
  
  Lista *elemento;
  elemento = primero;
  
  while (elemento != NULL) {
    printf("Dato: %d\n", elemento->dato);
    elemento = elemento->siguiente;
  }
  
  printf("No hay más elementos\n");
}

void 
destruir_lista(Lista *primero)
{
  Lista *elemento;
  Lista *siguiente;

  elemento = primero;
  
  while (elemento != NULL) {
    printf("liberando dato %d\n", elemento->dato);
    siguiente = elemento->siguiente;
    free(elemento);
    elemento = siguiente;
  }
}

int 
main(int argc, char *argv[]) 
{
	FILE *fInput, *fOutput;   
	XDR opRead, opWrite;

	Lista input, *output = NULL;
	input.siguiente = NULL;


	
	if (argc != 3) Error("Incorrect arguments");

	fInput = fopen(argv[1], "r");  
	fOutput = fopen(argv[2], "w+");

	if (!fInput || !fOutput) Error("File not found"); 
 
 
	xdrstdio_create(&opRead, fInput, XDR_DECODE);
	xdrstdio_create(&opWrite, fOutput, XDR_ENCODE);


	if(!xdr_Lista (&opRead, &input))
		Error("Cant read the input file");

	imprime_datos_lista(&input);

	trata_datos(&input, &output);

	xdr_Lista(&opWrite, output);
	destruir_lista(output);


	xdr_free((xdrproc_t)xdr_Lista, (void*)&input);

  xdr_destroy(&opRead);
	xdr_destroy(&opWrite);

  fclose(fInput);
  fclose(fOutput);
  
  return 0;
}