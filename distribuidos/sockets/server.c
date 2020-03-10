
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <netinet/ip.h>

void 
Error(const char * error) {
  perror(error);
  exit(EXIT_FAILURE);
}

int 
main(int argc, char *argv[])
{
	int server_sock = -1, client_sock = -1, count = -1, port = -1;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	unsigned char data[100];
	char* end;

	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	memset(&client_addr, 0, sizeof(struct sockaddr_in));

	if (argc != 2) {
		printf("Usage: server <port>\n");
		exit(EXIT_FAILURE);
	}

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

	while (1) 
	{

		memset(&data, 0, sizeof(data));

		if ((client_sock = accept(server_sock, 0, 0)) == -1)
			Error("Socket accept");

		/* TODO: Handle correctly recieve and send for big chunks */
		if ((count = recv(client_sock, &data, sizeof(data), 0)) == -1)
			Error("Socket recieve");

		if ((count = send(client_sock, &data, count, 0)) == -1)
			Error("Socket send");

		if (close(client_sock) == -1)
			Error("Close client socket");
	}
}
