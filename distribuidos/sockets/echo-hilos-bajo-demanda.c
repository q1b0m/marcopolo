
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <netinet/ip.h>


#define PUERTO 4444  // Puerto por defecto (el estándar 7 está protegido)
#define MAX_LINEA 80

typedef struct Client_Data{
  char *origen;       
  int puerto_origen; 
  int sock;          
  unsigned int numhilo;  
} Client_Data, *pClient_Data;

int sockEscucha, sockDatos;

void
Error(const char * error) {
    perror(error);
    exit(EXIT_FAILURE);
}


__attribute__((always_inline))
void 
show_menu() 
{
    printf("Usage: server <port>\n");
    exit(EXIT_FAILURE);
}

void 
Handler(int s)
{
    printf("Interrupción desde teclado. Terminando servidor.\n");
    close(sockEscucha);
    exit(0);
}


void * 
servicioEcho(pClient_Data Client)
{
    int count = -1;
    unsigned char data[100] = {0};
    printf("Hilo %d: Recibida conexión desde %s(%d)\n", Client->numhilo, Client->origen, Client->puerto_origen);
    
    //do {

        /* TODO: Handle correctly recieve and send for big chunks */
        if ((count = recv(Client->sock, &data, sizeof(data), 0)) == -1)
            Error("Socket recieve");
        
        data[count]=0; 

        printf("Hilo %d: Recibida un mensaje de longitud %d\n", Client->numhilo, count);
        printf("Hilo %d: Contenido: %s\n", Client->numhilo, data);

        if ((count = send(Client->sock, &data, count, 0)) == -1)
            Error("Socket send");

    //} while (count != 0);
    
    printf("Hilo %d: El cliente ha cerrado. Muero.\n", Client->numhilo);

    if (close(Client->sock) == -1)
        Error("Close client socket");
    
    free(Client->origen);
    Client->origen = NULL;

    free(Client);
    Client = NULL;

    return 0;
}

int 
main(int argc, char *argv[])
{
    int server_sock = -1, client_sock = -1, port = -1;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    pClient_Data Client;
    
    unsigned int client_addrlen = sizeof(client_addr);
    
    unsigned int n_hilos = 0;
    pthread_t tid = 0;

    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    memset(&client_addr, 0, sizeof(struct sockaddr_in));


    if (argc != 2)
        show_menu();

    if ((port = strtol(argv[1], NULL, 10)) == 0) 
        Error("Invalid port string");

    if ((server_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        Error("Socket create");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        Error("Socket bind");

    if (listen(server_sock, SOMAXCONN) == -1)
        Error("Socket listen");

    signal(SIGINT, Handler);
    
    while (1) {

        if ((client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addrlen)) == -1)
            Error("Socket accept");
               
        Client =  (pClient_Data) malloc(sizeof(Client_Data));
        Client->origen = (char *) malloc(100);

        if(!inet_ntop(AF_INET, &(client_addr.sin_addr), Client->origen, 100))
            Error("Inet ntop");

        Client->puerto_origen = ntohs(client_addr.sin_port);
        Client->sock = client_sock;
        Client->numhilo = n_hilos;
        
        pthread_create(&tid, NULL, (void *)servicioEcho, (void *)Client);
        pthread_detach(tid);

        printf("Servidor principal: Lanzado el hijo %d\n", n_hilos);

        n_hilos++;
    }

    return 0;  
}