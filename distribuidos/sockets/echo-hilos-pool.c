
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <netinet/ip.h>


#define PACKED __attribute__((__packed__))

#define PUERTO 4444  // Puerto por defecto (el estándar 7 está protegido)
#define MAX_LINEA 80
#define TAM_COLA 5
#define THREADS 4


typedef struct PACKED Client_Data {
  char *origen;       
  int puerto_origen; 
  int sock;          
  unsigned int numhilo;  
} Client_Data, *pClient_Data;

typedef struct PACKED ColaTrabajos{
    Client_Data **Clients;
    unsigned int Head;
    unsigned int Tail;
    pthread_mutex_t Lock;
    sem_t Free;
    sem_t Busy;
} ColaTrabajos, pColaTrabajos;

ColaTrabajos Cola;


void
Error(const char * error) {
    perror(error);
    exit(EXIT_FAILURE);
}


void 
Handler(int s)
{
    printf("Interrupción desde teclado. Terminando servidor.\n");
    exit(0);
}


void
inicializar_cola()
{
    Cola.Clients = (Client_Data**) malloc(sizeof(pClient_Data) * TAM_COLA);

    Cola.Head = 0;
    Cola.Tail = 0;

    if (pthread_mutex_init(&(Cola.Lock), NULL) != 0) 
        Error("Mutex create");

    if (sem_init(&(Cola.Free), 0 , TAM_COLA) == -1)
        Error("Free Semaphore Init");

    if (sem_init(&(Cola.Busy), 0 , 0) == -1)
        Error("Busy Semaphore Init");
}


void
destruir_cola()
{
    if (pthread_mutex_destroy((&Cola.Lock)) != 0)
        Error("Mutex destroy");

    if (sem_destroy(&(Cola.Free)) == -1)
        Error("Free Semaphore Destroy");

    if (sem_destroy(&(Cola.Busy)) == -1)
        Error("Busy Semaphore Destroy");
}


void 
hexdump(void *ptr, unsigned int len){
    while(len > 0){
        printf("%x", *((char*) ptr));
        len--;
    }
    printf("\n");
}


void
insertar_dato_cola(pClient_Data Client)
{
    if (sem_wait(&(Cola.Free)) == -1)
        Error("Wait Insert");

    if (pthread_mutex_lock(&(Cola.Lock)) != 0)
        Error("Mutex Lock Insert");

    //verify not to excedd Cola.Clients[TAM_COLA]
    if(!Cola.Clients[Cola.Head])
        Cola.Clients[Cola.Head] = (pClient_Data) malloc(sizeof(Client_Data));

    /*
    Here we can check if exists previous pointers on that client object
    and release them
    */

    memcpy(Cola.Clients[Cola.Head], Client, sizeof(Client_Data));
    Cola.Head = (Cola.Head + 1) % TAM_COLA;


    if (pthread_mutex_unlock(&(Cola.Lock)) != 0)
        Error("Mutex Unluck Insert");

    if (sem_post(&(Cola.Busy)) == -1)
        Error("Post Insert");
}


void
obtener_dato_cola(pClient_Data Client)
{
    memset(Client, 0, sizeof(Client_Data));

    if (sem_wait(&(Cola.Busy)) == -1)
        Error("Wait Insert");

    if (pthread_mutex_lock(&(Cola.Lock)) != 0)
        Error("Mutex Lock Insert");

    memcpy(Client, Cola.Clients[Cola.Tail], sizeof(Client_Data));

    Cola.Tail = (Cola.Tail + 1) % TAM_COLA;

    if (pthread_mutex_unlock(&(Cola.Lock)) != 0)
        Error("Mutex Unluck Insert");

    if (sem_post(&(Cola.Free)) == -1)
        Error("Post Insert");   

}


void
do_Work()
{
    Client_Data Client;
    while(1)
    {

        obtener_dato_cola(&Client);
        servicioEcho(&Client);

        /* TODO: Release correctly old clients heap pointer, problem with TAIL,
                here we are accessing new tail so not corresponds with current client
                object.
         */

        //free(Cola.Clients[Cola.Tail]->origen);
        //free(Cola.Clients[Cola.Tail]);
    }
}


void
servicioEcho(pClient_Data Client)
{
    int count = -1;
    unsigned char data[100] = {0};
    printf("Hilo %d: Recibida conexión desde %s (%d)\n", Client->numhilo, Client->origen, Client->puerto_origen);
    
    do {

        if ((count = recv(Client->sock, &data, sizeof(data), 0)) == -1)
            Error("Socket recieve");
        
        data[count]=0; 

        printf("Hilo %d: Recibida un mensaje de longitud %d\n", Client->numhilo, count);
        printf("Hilo %d: Contenido: %s", Client->numhilo, data);

        if ((count = send(Client->sock, &data, count, 0)) == -1)
            Error("Socket send");

    } while (count != 0);

    
    printf("Hilo %d: El cliente ha cerrado. Muero.\n", Client->numhilo);

    if (close(Client->sock) == -1)
        Error("Close client socket");

    return 0;
}


int 
main(int argc, char *argv[])
{
    int server_sock = -1, client_sock = -1, port = -1, i = 0;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    
    pClient_Data Client;
    
    unsigned int client_addrlen = sizeof(client_addr);
    
    unsigned int n_hilos = 0;
    pthread_t tid = 0;

    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    memset(&client_addr, 0, sizeof(struct sockaddr_in));


    //////////////////////////////////////////////////////////
    /*                 Handle Menu                          */
    //////////////////////////////////////////////////////////
    
    if (argc != 2)
    {
        printf("Usage: server <port>\n");
        exit(EXIT_FAILURE);
    }
    if ((port = strtol(argv[1], NULL, 10)) == 0) 
        Error("Invalid port string");

    //////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////
    /*                 Build TCP Server                     */
    //////////////////////////////////////////////////////////
    
    if ((server_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        Error("Socket create");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        Error("Socket bind");

    if (listen(server_sock, SOMAXCONN) == -1)
        Error("Socket listen");

    //////////////////////////////////////////////////////////
    
    signal(SIGINT, Handler);
    
    //////////////////////////////////////////////////////////
    /*                 Initilize queue                      */
    //////////////////////////////////////////////////////////
    inicializar_cola();


    for(i = 0; i < THREADS; i++){
        pthread_create(&tid, NULL, (void *)do_Work, NULL);
        pthread_detach(tid);
    }


    while (1) {

        if ((client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addrlen)) == -1)
            Error("Socket accept");
               

        Client = (pClient_Data) malloc(sizeof(Client_Data));
        Client->origen = (char *) malloc(100);

        if(!inet_ntop(AF_INET, &(client_addr.sin_addr), Client->origen, 100))
            Error("Inet ntop");

        Client->puerto_origen = ntohs(client_addr.sin_port);
        Client->sock = client_sock;
        Client->numhilo = n_hilos;

        //printf("%s %d %d\n",Client->origen, Client->puerto_origen, Client->sock, Client->numhilo);

        insertar_dato_cola(Client);

        printf("Servidor principal: Lanzado el hijo %d\n", n_hilos);

        n_hilos++;
    }

    return 0;  
}