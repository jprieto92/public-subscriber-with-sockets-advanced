/* 	JAVIER PRIETO CEPEDA		100307011 
	PEDRO ROBLEDO QUINTERO	 	100XXXXXX
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#include "read_line.h"

#define MAX_MESSAGE 1154
#define IP_SIZE 16
#define MAX_TOPIC 128
#define MAX_TEXT 1024



int  main(int argc, char **argv)
{
	struct sockaddr_in server_addr;
	struct hostent *hp;
	int 	sd, val;
	char 	ipBroker[IP_SIZE];
	char 	topic[MAX_TOPIC + 1];
	char 	text[MAX_TEXT + 1];
	char 	message[MAX_MESSAGE];
	int 	puertoBroker;
	
	
	int bytesLeidos;	
	if(argc != 4)
	{
		printf("Usage: ./publisher <topic> <ipIntermediario> <puertoIntermediario>\n");
		exit(0);
	}
	
	if(strlen(argv[1]) > MAX_TOPIC)	// Copiamos el tema del editor
	{
		memcpy(topic, argv[1], 128);			
	}else
	{
		memcpy(topic, argv[1], strlen(argv[1]) + 1);
	}
	strcpy(ipBroker, argv[2]);	// Copiamos IP del intermediario
	puertoBroker = atoi(argv[3]); 	// Copiamos el puerto del servidor

	/*COMPROBAMOS QUE EL PUERTO DEL BROKER ESTA EN EL RANGO PERMITIDO*/	
	if ((puertoBroker < 1024) || (puertoBroker > 65535)) 
	{
			printf("Error: Port must be in the range 1024 <= port <= 65535\n");
			return -1;
	}
	/*-PRUEBA
	*printf("TEMA: %s\n",topic);			
	*printf("IP: %s\n",ipBroker);		
	*printf("PUERTO: %d\n",puertoBroker);	
	*/
	bytesLeidos = readLine(0, text, MAX_TEXT + 1);	// Leo el texto -- > MAX 1024 bytes + /0 --> 1025
	while (bytesLeidos != -1 && (strcmp(text, "exit")!=0)) 	
	{
		/*FORMO EL MENSAJE tema:texto*/
		strcpy(message,topic);	
		strcat(message, ":");
		strcat(message, text);
		//printf("texto: .%s. tam %d\n",text,strlen(text));
		//printf("mensaje: .%s. tam %d\n",message,strlen(message));
		/*ABRIMOS EL SOCKET*/
		if((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0){
			perror("Error al crear socket\n");	//prueba
			return -1;
		}
		/*DAR VALORES INICIALES AL SOCKET*/
		val = 1;
		setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof (int));
		/*VACIAMOS LA ESTRUCTURA*/
		bzero((char *) &server_addr, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(puertoBroker);	
		//server_addr.sin_addr.s_addr = INADDR_ANY; // Direccion de mi maquina, localhost
		hp = (struct hostent*)gethostbyname(argv[2]);
		memcpy((void *)&(server_addr.sin_addr), (const void *)hp->h_addr, hp->h_length);

	
		//printf("Conexion Realizada\n");	//prueba
		if(connect(sd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
		{
			printf("Error in the connection to the server %s: %d\n",ipBroker,puertoBroker);
			return(-1);
		}
		if(enviar(sd, message, strlen(message) + 1)!=0)	//envio el mensaje acabado en /0
		{
		 	printf("Error in the connection to the server %s: %d\n",ipBroker,puertoBroker);	//prueba
	        	return -1;	
		}
		//printf("MENSAJE ENVIADO: .%s.\n",message);
		bytesLeidos = readLine(0, text, MAX_TEXT + 1);	// Leo el texto -- > MAX 1024 bytes + /0 --> 1025
	}
	close(sd);
	return(0);
}

	
