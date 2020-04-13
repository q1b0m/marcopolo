
#include <stdio.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>  
#include <string.h>

#include <arpa/inet.h>     
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> 

void
Error(const char * error) {
	perror(error);
	exit(EXIT_FAILURE);
}
    
int 
main(int argc , char *argv[])   
{   
  int opt = 1;   
  int s_datos, addrlen, new_client_socket, maximo, *client_sockets;      
  int port, max_clients;
  int i, count;

  struct sockaddr_in address;

  char buffer[1024];   
  fd_set conjunto;   

  if (argc != 3)
  {
  	printf("Usage: %s <max_clientes> <puerto>\n", argv[0]);
  	Error("Invalid Arguments");
  }

  if( !(max_clients = strtol(argv[1], NULL, 10)) || max_clients < 1)
  	Error("Invalid max_clients");
  
  if( !(port = strtol(argv[2], NULL, 10)) || port < 1024 || port > 65535)
  	Error("Invalid port");

  client_sockets = (int*) malloc(sizeof(int) * max_clients);
  if (!client_sockets)
  	Error("Malloc failed");
         
  //Inicializar array de clientes a 0
  for (i = 0; i < max_clients; i++)   
  {   
  	client_sockets[i] = 0;   
  }   
         
  if ((s_datos = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
  	Error("Socket failed");   
          
  if (setsockopt(s_datos, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
  		sizeof(opt)) < 0 )   
  	Error("Setsockopt failed");
       
  address.sin_family = AF_INET;   
  address.sin_addr.s_addr = INADDR_ANY;   
  address.sin_port = htons( port );

  addrlen = sizeof(address);
 

  if (bind(s_datos, (struct sockaddr *)&address, sizeof(address)) < 0 )
  	Error("Bind failed");   
       
  printf("Escuchando en el puerto %d \n", port);   
         
  if (listen(s_datos, 5) < 0)
  	Error("Listen failed");   
         
  while(1)   
  {   
  	// Vaciar conjunto de descriptores a vigilar
  	FD_ZERO(&conjunto);   
     
    //Añade el s_datos al conjunto
    FD_SET(s_datos, &conjunto);   
    maximo = s_datos;
             
    //Añade los socket hijos activos 
    for ( i = 0 ; i < max_clients ; i++)   
    {                     
      if (client_sockets[i] > 0)   
        FD_SET(client_sockets[i], &conjunto);   
                 
   		//Buscamos el descriptor mayor de todos          
      if (client_sockets[i] > maximo)   
       	maximo = client_sockets[i];   
    }   
    
    select(maximo+1, &conjunto, NULL, NULL, NULL);     
           
  	//Hay una nueva conexión
  	if (FD_ISSET(s_datos, &conjunto))   
  	{   
      if ((new_client_socket = accept(s_datos,  
      		(struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)   
        Error("Accept failed");   
               
    	//inform user of socket number - used in send and receive commands  
    	printf("Nueva conexion, socket FD:%d, IP:%s, Port:%d\n", 
    			new_client_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));   
                            
    	//Añadimos el nuevo socket a la lista de sockets hijos
    	for (i = 0; i < max_clients; i++)   
    	{   
    		if( client_sockets[i] == 0 )   
        {   
        	client_sockets[i] = new_client_socket;   
        	printf("Añadiendo a la lista de sockets %d\n", i);   
        	break;   
        }   
      }

      if(i == max_clients){
        const char* message = "Lo sentimos, el servidor está saturado\n";
        send(new_client_socket, message, strlen(message), 0);
        close(new_client_socket);
      
      }
    }   

    //Algún hijo esta recibindo datos 
    for (i = 0; i < max_clients; i++)   
    {              
    	if (client_sockets[i] != 0 && FD_ISSET(client_sockets[i], &conjunto))   
    	{   	
      	//Mensaje para cerrar conexion 
      	if ((count = read(client_sockets[i], buffer, 1024)) == 0)   
        {   
         	getpeername(client_sockets[i], (struct sockaddr*)&address, 
         		(socklen_t*)&addrlen);   
         	
         	printf("Host disconnected, IP:%s, Port:%d\n", 
         				inet_ntoa(address.sin_addr), ntohs(address.sin_port));   
                         
         	close(client_sockets[i]);   
         	client_sockets[i] = 0;  
        }   
                     
        //Servicio echo  
        else 
        {    
          buffer[count] = '\0';   
          send(client_sockets[i], buffer, count, 0);   
        }   
      }   
    }  
  }      

  free(new_client_socket);  
  return 0;   
}   
