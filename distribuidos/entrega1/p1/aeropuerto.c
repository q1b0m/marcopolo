#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>

#include "cola.h"
#include "util.h"



#define MAX_PISTAS    4
#define MAX_AVIONES   8
#define MAX_VUELOS    4

// ====================================================================
// VARIABLES GLOBALES
// ====================================================================


// Estado de cada pista del aeropuerto 0 indica libre, 1 ocupada
int estado_pistas[MAX_PISTAS];

// Mutex para evitar el acceso de dos hilos simultáneos al array anterior
pthread_mutex_t manipular_pistas;

// Semáforo para bloquear al controlador de entrada si no hay pistas
// libres hasta que se libere alguna
sem_t pistas_libres; 


// Colas para comunicar aviones y controladores
Cola cola_peticion, cola_concesion, cola_liberacion;


// ====================================================================
// FUNCIONES de manejo del array de pistas
// ====================================================================
void  
SoltarPista(int pista_liberada)
{
	// Se pone un 0 en la posición de la pista liberada
	// Pero el acceso al array de estados de la pista
	// debe estar protegido por un cerrojo  
	if (pthread_mutex_lock(&manipular_pistas) != 0)
		Error("Mutex Lock SoltarPista");

	estado_pistas[pista_liberada] = 0;
	
	if (pthread_mutex_unlock(&manipular_pistas))
		Error("Mutex Unlock Soltar Pista");

	// Y se señala que hay un pista libre más, mediante el semáforo
	if (sem_post(&pistas_libres) == -1)
		Error("Post SoltarPista");

	return;
}


int 
CogerPista(void)
{
	int pista_encontrada = 0;

	// Esperar semáforo a que haya pistas libres
	if (sem_wait(&pistas_libres) == -1)
		Error("Wait CogerPista");

	// Buscar el número de pista libre, protegiendo el acceso al
	// array de pistas con el cerrojo
	if (pthread_mutex_lock(&manipular_pistas) != 0)
		Error("Mutex Lock CogerPista");
	
	// Iterar mientras la pista este ocupada y estemos dentro del array
	while (pista_encontrada < MAX_PISTAS && estado_pistas[pista_encontrada] != 0) 
		pista_encontrada++;
	
	// Poner un 1 en la posición encontrada
	estado_pistas[pista_encontrada] = 1;
	
	if (pthread_mutex_unlock(&manipular_pistas) != 0)
		Error("Mutex Unlock CogerPista");

	// Retornar la pista encontrada
	return pista_encontrada;
}


// ====================================================================
// Implementación de los hilos
// ====================================================================

void 
*controlador_entrada(void * nada)  // No recibe parámetro
{
	int concedida;
	int avion;
	char msg[100];

	//Codigo del controlador de entrada
	while (1)
	{
		// Esperar a que un avión solicite pista
		avion = obtener_dato_cola(&cola_peticion); 

		// Información de depuración
		sprintf(msg, "Controlador entrada busca pista para avion %d\n", avion);
		log_debug(msg);

		// Buscar pista libre llamando a CogerPista()
		concedida = CogerPista();

		// Información de depuración
		sprintf(msg, "Controlador entrada concede pista %d a avion %d\n", 
						concedida, avion);
		log_debug(msg);

		// Notificar al avión qué pista se le ha concedido
		insertar_dato_cola(&cola_concesion, concedida); 
	}

	return NULL;
}

void *controlador_salida(void *nada)   // No recibe parámetro
{
	int pista_liberada;
	char msg[100];

	// Código del controlador de salida
	while (1)
	{
		// Esperar a que un avión nos indique que ha liberado una pista
		pista_liberada = obtener_dato_cola(&cola_liberacion);

		// Marcar pista como libre llamando a SoltarPista()
		SoltarPista(pista_liberada);

		// Información de depuración
		sprintf(msg, "Controlador de salida libera pista %d\n", pista_liberada);
		log_debug(msg);
	}
	return NULL;
}


// Hilo que simula al avión. Recibe un número que es el ID del avión
// Efectúa un bucle en el que varias veces despega y aterriza. Antes de
// cada una de esas operaciones solicita pista (a través de una cola)
// y espera por ella (a través de otra cola).
//
// Luego simula que despega, y vuela un tiempo.
//
// Tras ello solicita pista para aterrizar (de nuevo en la primera cola)
// y espera a que se le conceda (de nuevo en la segunda cola)
//
// Cada vez que abandona la pista lo notifica en una tercera cola
void *Avion(int *id)
{
	int id_avion;      
	int n_vuelo;       // Contador de vuelos realizados
	char msg[200];     // Buffer auxiliar para crear mensajes de log

	id_avion = *id;    // Capturar el id del avión en una variable local
	free(id);          // Ya no necesitamos el parámetro recibido, lo liberamos

	// Bucle de simulación de cada vuelo
	for (n_vuelo=0; n_vuelo<MAX_VUELOS; n_vuelo++)
	{
		int pista;  // Para recoger el numero de pista concedida
		double t;   // Tiempo de uso de la pista, o de vuelo (aleatorio)

		// Mostrar información de depuración
		sprintf(msg, "Avion %d solicita pista para despegue (vuelo %d)\n", 
						id_avion, n_vuelo);
		log_debug(msg);


		// Solicitar pista al controlador de entrada (enviándole id_avion)
		insertar_dato_cola(&cola_peticion, id_avion);

		// Esperar concesión y obtener la pista concedida
		pista = obtener_dato_cola(&cola_concesion);

		// Simular que se usa la pista durante un tiempo aleatorio
		t = randRange(1, 3);
		sprintf(msg, "Avion %d usa pista %d para despegar durante %fs (vuelo %d)\n", 
						id_avion, pista, t, n_vuelo); 
		log_debug(msg);
		sleep(t);
		sprintf(msg, "Avion %d libera pista %d tras despegar (vuelo %d)\n", 
						id_avion, pista, n_vuelo);
		log_debug(msg);

		// Notificar al controlador de salida que la pista ya está libre
		insertar_dato_cola(&cola_liberacion, pista);


		// Simular vuelo durante un tiempo aleatorio
		t = randRange(3, 8);
		sprintf(msg, "Avion %d esta volando durante %fs (vuelo %d)\n",
						id_avion, t, n_vuelo);
		log_debug(msg);
		sleep(t); 

		// Aterrizaje
		sprintf(msg, "Avion %d solicita pista para aterrizaje (vuelo %d)\n", 
						id_avion, n_vuelo);
		log_debug(msg);


		// Solicitar pista al controlador de entrada (enviándole id_avion)
		insertar_dato_cola(&cola_peticion, id_avion);

		// Esperar concesión y obtener la pista concedida
		pista = obtener_dato_cola(&cola_concesion);

		// Simular que se usa durante un tiempo
		t = randRange(1, 3);
		sprintf(msg, "Avion %d usa pista %d para aterrizar durante %fs (vuelo %d)\n", 
						id_avion, pista, t, n_vuelo);
		log_debug(msg);
		sleep(t);
		sprintf(msg, "Avion %d libera pista %d tras aterrizar (vuelo %d)\n", 
						id_avion, pista, n_vuelo);
		log_debug(msg);


		// Notificar al controlador de salida que la pista ya está libre
		insertar_dato_cola(&cola_liberacion, pista);
	
	}
	
	return NULL;
}

// ====================================================================
// PROGRAMA PRINCIPAL
// ====================================================================

// Su misión es crear e inicializar todos los recursos de sincronización, 
// lanzar todos los hilos y esperar a que los hilos-avión finalicen

int main(void)
{
	register int i;   // Indice para bucles
	int *id_avion;    // Para ir creando dinámicamente los id de aviones

	// Variables para almacenar los identificadores de hilos
	pthread_t c_entrada, c_salida;
	pthread_t datos_hilo[MAX_AVIONES];
	
	setbuf(stdout,NULL); //quitamos el buffer de la salida estandar

	// Inicializar semáforo y mutex
	if (pthread_mutex_init(&manipular_pistas, NULL) != 0) 
		Error("Mutex create");

	if (sem_init(&pistas_libres, 0 , MAX_PISTAS) == -1)
		Error("Semaphore Init");

	// Inicializar todas las colas utilizadas
	inicializar_cola(&cola_peticion, MAX_AVIONES);
	inicializar_cola(&cola_concesion, MAX_AVIONES);
	inicializar_cola(&cola_liberacion, MAX_AVIONES);

	// Rellenar con ceros el estado de las pistas (todas libres)
	for (i=0;i<MAX_PISTAS;i++) estado_pistas[i] = 0;

	// Creación de los hilos controladores de entrada y de salida
	// No reciben parámetros por lo que se pasa NULL
	pthread_create(&c_entrada, NULL,
									(void *) controlador_entrada,      
									(void *) NULL);
	pthread_create(&c_salida, NULL,
									(void *) controlador_salida,
									(void *) NULL);

	// Creación de un hilo para cada avión. Estos sí reciben como parámetro
	// un puntero a entero que será su id_avion. Se crea dinámicamente uno
	// para cada hilo y se le asigna el contador del bucle
	for (i=0; i < MAX_AVIONES; i++)
	{
		id_avion = (int *) malloc (sizeof(int));
		*id_avion = i;    
		
		pthread_create(&datos_hilo[i], NULL,
											(void*) Avion,
											(void*)id_avion);
	}

	// main espera a que terminen todos los aviones
	for (i=0;i<MAX_AVIONES;i++)
	{
		pthread_join(datos_hilo[i],NULL);
	}

	// Finalización. Destruir el mutex, el semáforo y las colas
	if (pthread_mutex_destroy(&manipular_pistas) != 0)
		Error("Mutex destroy");

	if (sem_destroy(&pistas_libres) == -1)
		Error("Free Semaphore Destroy");
	
	//destruir colas
	destruir_cola(&cola_peticion);
	destruir_cola(&cola_concesion);
	destruir_cola(&cola_liberacion);

	return 0;
}
