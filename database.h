/* 	JAVIER PRIETO CEPEDA		100307011 
	PEDRO ROBLEDO QUINTERO	 	100XXXXXX
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include "read_line.h"


/* CONSTANTES */

#define TOPIC_SIZE 128
#define TEXT_SIZE 1024
#define MESSAGE_SIZE 1154
#define IP_SIZE 16
#define MAX_SUBSCRIBES 10
#define TRUE 1
#define FALSE 0
#define OK 0


/* NODO DE UN SUSCRIPTOR */
typedef struct suscriptor{
	char ip[IP_SIZE];	// Ip en formato decimal punto del suscriptor
	u_short puerto;		// Puerto del suscriptor
	struct suscriptor *next;
} suscriptor;

/* NODO DE UN TEMA DE LA LISTA DE TEMAS*/
typedef struct tema{
	char nombre[TOPIC_SIZE + 1];			// Nombre del tema
	suscriptor *listaSuscritos; 			// Lista de suscritos al tema 
	int suscritos;					// Numero de suscritos al tema
	struct tema *next;
} tema;


/* PROTOTIPOS DE FUNCIONES */

int init();								// Iniciar base de datos
int destroy();								// Destruir base de datos
int altaSuscriptor(char *topic, char *ip, u_short puerto);		// Dar de alta a un suscriptor
int bajaSuscriptor(char *topic, char *ip, u_short puerto);		// Dar de baja a un suscriptor
int altaTema(char *topic);						// Dar de alta a un suscriptor
int bajaTema(char *topic);						// Dar de baja a un suscriptor
int enviarMensajeTema(char *messageEditor, char *topic, int scEs);	// Enviar Mensaje a suscriptores de un tema



