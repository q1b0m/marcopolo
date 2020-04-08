
//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux  
#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
     
#define TRUE   1  
#define FALSE  0  
#define PORT 8888  



void
Error(const char * error) {
    perror(error);
    exit(EXIT_FAILURE);
}
     
int 
main(int argc , char *argv[])   
{   
    int opt = TRUE;   
    int master_socket, addrlen, new_socket, client_socket[5] ,  
          max_clients = 5, activity, i ,count , current_sock;   
    int max_sd;   
    struct sockaddr_in address;

    char buffer[1024];  
         
    //set of socket descriptors  
    fd_set readfds;   
         

    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < max_clients; i++)   
    {   
        client_socket[i] = 0;   
    }   
         

    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
        Error("socket failed");   
          

    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
        sizeof(opt)) < 0 )   
        Error("setsockopt");
       
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );

    addrlen = sizeof(address);
         

    if( bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0 )   
        Error("bind failed");   
       
    printf("Listener on port %d \n", PORT);   
         
    if (listen(master_socket, 5) < 0)   
        Error("listen");   
          
   
    while(TRUE)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
             
        //add active child sockets to readfds set  
        for ( i = 0 ; i < max_clients ; i++)   
        {   
            current_sock = client_socket[i];   
                 
            //if valid socket descriptor then add to read list  
            if(current_sock > 0)   
                FD_SET(current_sock, &readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(current_sock > max_sd)   
                max_sd = current_sock;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select(max_sd+1, &readfds, NULL, NULL, NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
            printf("select error");   
           
        //Master stuff
        //If something happened on the master socket, then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds))   
        {   
            if ((new_socket = accept(master_socket,  
                    (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)   
                Error("accept");   
               
            //inform user of socket number - used in send and receive commands  
            printf("New connection, socket FD:%d, IP:%s, Port:%d\n", 
                new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));   
                            
            //add new socket to array of sockets  
            for (i = 0; i < max_clients; i++)   
            {   
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                    printf("Adding to list of sockets as %d\n" , i);   

                    break;   
                }   
            }   
        }   


        //else its some IO operation on some other socket 
        for (i = 0; i < max_clients; i++)   
        {   
            current_sock = client_socket[i];   
                 
            if (current_sock != 0 && FD_ISSET(current_sock , &readfds))   
            {   
                //Check if it was for closing , and also read the  
                //incoming message  
                if ((count = read(current_sock, buffer, 1024)) == 0)   
                {   
                    //Somebody disconnected , get his details and print  
                    getpeername(current_sock, (struct sockaddr*)&address, (socklen_t*)&addrlen);   
                    printf("Host disconnected, IP:%s, Port:%d\n",  
                          inet_ntoa(address.sin_addr), ntohs(address.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close(current_sock);   
                    client_socket[i] = 0;  
                }   
                     
                //Echo back the message that came in  
                else 
                {   
                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    buffer[count] = '\0';   
                    send(current_sock, buffer, strlen(buffer), 0);   
                }   
            }   
        }  
    }   
         
    return 0;   
}   
