
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#include "cola.h"
#include "util.h"


void 
inicializar_cola(Cola *cola, int tam_cola)
{

	cola->datos = (int*) malloc(sizeof(int) * tam_cola);

	if(cola->datos == NULL)
		Error("Malloc NULL inicializar_cola");

	cola->head = 0;
	cola->tail = 0;
	cola->tam_cola = tam_cola;

	if (pthread_mutex_init(&(cola->mutex), NULL) != 0) 
		Error("Mutex create inicializar_cola");

	if (sem_init(&(cola->num_huecos), 0, tam_cola) == -1)
		Error("Free Semaphore inicializar_cola");

	if (sem_init(&(cola->num_ocupados), 0 , 0) == -1)
		Error("Busy Semaphore inicializar_cola");

	return;
}


void 
destruir_cola(Cola *cola)
{

	free(cola->datos);

	if (pthread_mutex_destroy(&(cola->mutex)) != 0)
		Error("Mutex destroy destruir_cola");

	if (sem_destroy(&(cola->num_huecos)) == -1)
		Error("Free Semaphore Destroy destruir_cola");

	if (sem_destroy(&(cola->num_ocupados)) == -1)
		Error("Busy Semaphore Destroy destruir_cola");

	return;
}


void 
insertar_dato_cola(Cola *cola, int dato)
{

	// sem_wait() decrements (locks) the semaphore pointed to by sem
	if (sem_wait(&(cola->num_huecos)) == -1)
		Error("Wait insertar_dato_cola");

	if (pthread_mutex_lock(&(cola->mutex)) != 0)
		Error("Mutex Lock insertar_dato_cola");

	cola->datos[cola->head] = dato;
	cola->head = (cola->head + 1) % cola->tam_cola;

	if (pthread_mutex_unlock(&(cola->mutex)) != 0)
		Error("Mutex Unluck Insert");

	// sem_post() increments (unlocks) the semaphore pointed to by sem.
	if (sem_post(&(cola->num_ocupados)) == -1)
		Error("Post insertar_dato_cola");

	return;
}


int 
obtener_dato_cola(Cola *cola)
{
	
	int dato = -1;

	// sem_wait() decrements (locks) the semaphore pointed to by sem
	if (sem_wait(&(cola->num_ocupados)) == -1)
		Error("Wait obtener_dato_cola");
	
	if (pthread_mutex_lock(&(cola->mutex)) != 0)
		Error("Mutex Lock obtener_dato_cola");
	
	dato = cola->datos[cola->tail];
	cola->tail = (cola->tail + 1) % cola->tam_cola;
	
	if (pthread_mutex_unlock(&(cola->mutex)) != 0)
		Error("Mutex Unluck Insert");
	
	// sem_post() increments (unlocks) the semaphore pointed to by sem.
	if (sem_post(&(cola->num_huecos)) == -1)
		Error("Post Insert");   

	return dato;
}