/* 	JAVIER PRIETO CEPEDA		100307011
	PEDRO ROBLEDO QUINTERO	 	100XXXXXX
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>


#include "read_line.h"
#include "database.h"

/*PARTE 2 RPC*/
#include "almacenamientoTemas.h"

#define MAX_MESSAGE 1154
#define IP_SIZE 16
#define MAX_TOPIC 128
#define MAX_TEXT 1024
#define MAX_OPERATION 12

int sdE;		// Descriptor de socket editores
int sdS;		// Descriptor de socket suscriptores
char ipAux[IP_SIZE];	// Ip del suscriptor
int copiado1 = 0;
int copiado2 = 0;
pthread_mutex_t m1;	// mutex global Editores
pthread_mutex_t m2;	// mutex global Suscriptores
pthread_cond_t cv1;	// variable condicion Editores
pthread_cond_t cv2;	// variable condicion Suscriptores
char ipRpc[IP_SIZE]; // Ip del servidor RPC
CLIENT *clnt;
enum clnt_stat retval;
int res;

/*OBTENER DIRECCION IP DE LOCALHOST*/
void getLocalAddress(char *ip)
{
        char hostname[128];
        struct hostent *he;

        gethostname(hostname, sizeof hostname);

        he = gethostbyname(hostname);

        if (!he) { // do some error checking
                //printf("Error al obtener IP\n");
                return;
        }

        // Copiar ip address
        strcpy(ip, inet_ntoa(*(struct in_addr*)he->h_addr));
}

void manejador_exit(int sig)
{
	// Cerrar socket y finalizar
	close(sdE);
	close(sdS);
	//int destroy();
	pthread_mutex_destroy(&m1);
	pthread_cond_destroy(&cv1);
	pthread_mutex_destroy(&m2);
	pthread_cond_destroy(&cv2);
	/* Destruir el manejador de rpc*/
	clnt_destroy( clnt );
	printf("\nClosing broker...\n");
	exit(0);
}

/*FUNCION DE TRATAMIENTO DE PETICIONES DE SUSCRIPTORES*/
void tratarSuscriptores(int *sock)
{
	int bytesLeidos, sc, resultado, resultadoEnvio;
	char operation[MAX_OPERATION];
	u_short port;	// Puerto de escucha del suscriptor
	char ipSuscriptor[IP_SIZE];
	char topic[MAX_TOPIC + 1];
	char respuesta;
	char ipRpcAux[16];
	strcpy(ipRpcAux,ipRpc);
	/*SECCION SERVER*/
	/* --- SECCION CRITICA - COPIA PETICION --- */
       	pthread_mutex_lock(&m2);
	sc= *sock;
	strcpy(ipSuscriptor,ipAux);
	copiado2 = 1;			// socket copiada
        pthread_cond_signal(&cv2);	// Despertamos a servidor de peticiones
        pthread_mutex_unlock(&m2);
        /* --- FIN SECCION CRITICA --- */
	bytesLeidos = readLine(sc, operation, MAX_OPERATION);
	//printf("OPERACION: .%s.\n",operation);
	if(strcmp((const char *)operation, "SUBSCRIBE") == 0)
	{
		bytesLeidos = readLine(sc, topic, MAX_TOPIC + 1);
		bytesLeidos = recibir(sc, (char *)&port, 2);

		if(bytesLeidos>0)
		{
			port = ntohs((u_short) port);
			//printf("Tema: %s - ip: %s - puerto: %d\n",topic,ipSuscriptor,port); // prueba
			resultado = altaSuscriptor(topic, ipSuscriptor, port);

		}else{
			resultado = 1;
		}
		switch(resultado)
		{
			case 0:
				respuesta = 0;
				resultadoEnvio=enviar(sc, &respuesta, 1);
				//printf("RESULTADO 0\n"); // prueba
				break;
			case -1:
				respuesta = 1;
				resultadoEnvio=enviar(sc, &respuesta, 1);
				//printf("RESULTADO 1\n"); // prueba
				break;
		}
		if(resultadoEnvio!=0)
		{
			perror("Error al responder a suscriptor\n");
		}
    		close(sc);
		/*PEDIMOS TEXTOS AL SERVIDOR RPC*/

    		respTextos *prueba = (respTextos *)malloc(sizeof(respTextos));
    		retval = gettexts_1(topic, prueba, clnt);
  		if (retval != RPC_SUCCESS) {
  			clnt_perror(clnt, "call failed:");
  		}
		/*FIN SERVICIO RPC*/
		if(prueba->textos>0)
		{
			/*ENVIAMOS TEXTOS*/
	  		char *text;
	    		char *textEnviar;
	  		char messageEditor[MAX_MESSAGE];
	    		struct sockaddr_in server_addr;
	  		struct hostent *hp;
	    		int 	sd, val;
	  		int error = 0;
	    		int i=0;
	    		text = strdup(prueba->texto);
	    		for(i=0;i<prueba->textos;i++)
	    		{
				memset(messageEditor,'\0',MAX_MESSAGE);
	  	   		textEnviar = strsep(&text, ":");
	       			strcpy(messageEditor,topic);
	   		 	strcat(messageEditor, ":");
	   		 	strcat(messageEditor, textEnviar);
	       			error = 0;
	       			if((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0){
		 			perror("Error in the creation of socket\n");
					error = 1;
	       			}else{
		 			val = 1;
		 			setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof (int));
		 			bzero((char *) &server_addr, sizeof(server_addr));
		 			server_addr.sin_family = AF_INET;
		 			hp = (struct hostent*)gethostbyname(ipSuscriptor);
		 			memcpy((void *)&(server_addr.sin_addr), (const void *)hp->h_addr, hp->h_length);
		 			server_addr.sin_port = (u_short)htons(port);
		 			if(connect(sd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
		 			{
		   				perror("Error in the connection to the suscriptor");
		   				error = 1;
		 			}
		 			if(error==0)
		 			{
		   				if(enviar(sd, messageEditor, strlen(messageEditor)+1)!=0)	//envio el mensaje acabado en /0
		   				{
		     					perror("Error in the send to the suscriptor\n");	//prueba
		     					error = 1;
		   				}
		 			}
	       			}
	       			close(sd);
	    		}
	    		/*FIN ENVIO TEXTOS*/
		}
	}else
	{
		if(strcmp((const char *)operation, "UNSUBSCRIBE") == 0)
		{
			bytesLeidos = readLine(sc, topic, MAX_TOPIC + 1);
			bytesLeidos = recibir(sc, (char *)&port, 2);
			if(bytesLeidos>0)
			{
				port = ntohs((u_short) port);
				//printf("Tema: %s - ip: %s - puerto: %d\n",topic,ipSuscriptor,port); // prueba
				resultado = bajaSuscriptor(topic, ipSuscriptor, port);
			}else{
				resultado = 2;
			}
			switch(resultado)
			{
				case 0:
					respuesta = 0;
					enviar(sc, &respuesta, 1);
					//printf("RESULTADO 0\n"); // prueba
					break;
				case 1:
					respuesta = 1;
					enviar(sc, &respuesta, 1);
					//printf("RESULTADO 1\n"); // prueba
					break;
				case -1:
					respuesta = 2;
					enviar(sc, &respuesta, 1);
					//printf("RESULTADO 2\n"); // prueba
					break;
			}
		}
		close(sc);
	}
	//printf("FIN CONEXION PUBLISHER\n");	//prueba
	/*FINAL SECCION SERVER*/
	pthread_exit(NULL);
}

/*HILO PARA ESCUCHAR A SUSCRIPTORES*/
void peticionesSuscriptores(int *puertoSuscriptores)
{
	char ipLocal[IP_SIZE];
	pthread_t threadSuscriptor;     // Hilo para suscriptores
	pthread_attr_t attr1;		// atributos de hilo
	struct sockaddr_in server_addr, client_addr;
	int sc, size;

	/*CREAMOS THREAD PARA ATENDER A SUSCRIPTORES*/
	/* Hilo independiente*/
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);

	/*ABRIMOS EL SOCKET*/
	if((sdS=socket(AF_INET, SOCK_STREAM, 0)) ==-1){
		perror("Error en la creacion del socket\n");
		pthread_exit(NULL);
	}

	/*DAR VALORES INICIALES AL SOCKET*/
	int val=1;
	setsockopt(sdE, SOL_SOCKET, SO_REUSEADDR,(char *)&val, sizeof(int));

	/*VACIAMOS LA ESTRUCTURA*/


	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(*puertoSuscriptores);
	server_addr.sin_addr.s_addr=INADDR_ANY;

	//printf("El puerto para suscriptores del servidor es %d\n", *puertoSuscriptores);	//prueba

	/*ASIGNAMOS DIRECCION AL SOCKET*/
	if(bind(sdS,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
		perror("Error en el bind\n");
		pthread_exit(NULL);
	}

	/*EMPEZAMOS A ESCUCHAR LAS COMUNICACIONES*/
	if((listen(sdS,SOMAXCONN) < 0))
	{
		perror("Error listen\n");
		pthread_exit(NULL);
	}
	size = sizeof(client_addr);
	getLocalAddress(ipLocal);

	printf("s> init server suscriptors %s: -S <%d>\n", ipLocal, *puertoSuscriptores); // prueba
	while(1)
	{
		/*ACEPTAMOS LA CONEXION*/
		if((sc=accept(sdS, (struct sockaddr *)&client_addr, (socklen_t*)&size))<0)
		{
			perror("Error en el accept de suscriptores\n");
		}
		//printf("s> accept %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
		//printf("IP DEL CLIENTE: %s\n",inet_ntoa(client_addr.sin_addr));
		strcpy(ipAux, inet_ntoa(client_addr.sin_addr));
		//printf("IP DEL CLIENTE: %s\n",ipAux);
		pthread_create(&threadSuscriptor, &attr1, (void *)tratarSuscriptores, (void *)&sc); // creacion de hilo

		/* --- SECCION CRITICA - COPIA PETICION --- */
                pthread_mutex_lock(&m2);
		while(copiado2 == 0)
		{
			pthread_cond_wait(&cv2, &m2);	// bloquea hasta que hilo ha copiado los datos
		}
		copiado2 = 0;
		pthread_mutex_unlock(&m2);
		/* --- FIN SECCION CRITICA --- */
	}
	close(sdS);
	pthread_exit(NULL);
}

/*FUNCION DE TRATAMIENTO DE PETICIONES DE EDITORES*/
void tratarEditores(int *socket)
{
	int bytesLeidos, sc, resultadoEnvio, scSu;
	char 	*topic;
	char 	*text;
	char 	messageEditor[MAX_MESSAGE];
	char ipRpcAux[16];
	strcpy(ipRpcAux,ipRpc);
	/*SECCION SERVER*/
	/* --- SECCION CRITICA - COPIA PETICION --- */
       	pthread_mutex_lock(&m1);
	sc= *socket;
	copiado1 = 1;			// socket copiada
        pthread_cond_signal(&cv1);	// Despertamos a servidor de peticiones
        pthread_mutex_unlock(&m1);
        /* --- FIN SECCION CRITICA --- */
	bytesLeidos = readLine(sc, messageEditor, MAX_MESSAGE);
	if(bytesLeidos>0)
	{
		text = strdup(messageEditor);
		topic = strsep(&text, ":");
		// printf("Recibo: %s -- Tema: %s -- Texto: %s\n", messageEditor,topic,text);	//prueba
		close(sc);
		// printf("FIN CONEXION PUBLISHER\n");	//prueba
		/*FINAL SECCION SERVER*/
		/*COMIENZO SECCION CLIENTE*/

    	/*ENVIAMOS NUEVO TEXTO AL SERVIDOR RPC*/
	
    	retval = putpair_1(topic,text, &res, clnt);

  	if (retval != RPC_SUCCESS) {
  		clnt_perror(clnt, "call failed:");
  	}
    	/*FIN SERVICIO RPC*/

	scSu = sdS;
	/*BUSCAMOS EL TEMA QUE QUEREMOS*/
	resultadoEnvio = enviarMensajeTema(messageEditor, topic, scSu);
	if(resultadoEnvio < 0)
	{
		perror("Error al hacer el broadcast\n");
	}
	// printf("RESULTADO ENVIO: %d\n",resultadoEnvio);	//prueba

	/*FINAL SECCION CLIENTE*/
	}else{
		perror("Error, no he leido ningun byte\n");
		close(sc);
	}
	pthread_exit(NULL);
}

int  main(int argc, char **argv)
{
	int sc, size;
	char ipLocal[IP_SIZE];
  	/*rpc*/
  	/*---*/


	/*EDITORES*/
	pthread_t threadEditores;       // Hilo para editores
	pthread_attr_t attr1;		// atributos de hilo
	int 	puertoEditores;
	struct sockaddr_in server_addr, client_addr;

	/*SUSCRIPTORES*/
	pthread_t threadSuscriptores;       // Hilo que para suscriptores
	pthread_attr_t attr2;		    // atributos de hilo
	int 	puertoSuscriptores;
	/*COMPROBAMOS ENTRADAS*/
	if(argc != 4)
	{
		perror("Usage: ./broker <puertoEditores> <puertoSuscriptores> <host-rpc>\n");
		exit(0);
	}

	/* ASIGNAR MANEJADOR A Ctrl + C PARA FINALIZAR EL BROKER*/
	if (signal(SIGINT, manejador_exit) == SIG_ERR) {
		perror("error en la señal SIGUSR1");
		exit(EXIT_FAILURE);
	}

	/*OBTENEMOS LOS PUERTOS PARA ATENDER PETICIONES*/
	puertoEditores = atoi(argv[1]);
	puertoSuscriptores = atoi(argv[2]);
  	strcpy(ipRpc,argv[3]);

	/*COMPROBAMOS QUE LOS PUERTO DEL BROKER ESTAN EN EL RANGO PERMITIDO*/
	if ((puertoEditores < 1024) || (puertoSuscriptores < 1024) ||
	(puertoEditores > 65535) || (puertoSuscriptores > 65535))
	{
			printf("Error: Ports must be in the range 1024 <= port <= 65535\n");
			return -1;
	}

  	/*INICIAMOS SERVICIO DE RPC*/
  	clnt = clnt_create(ipRpc, ALMACENAMIENTOTEMAS, ALMACENAMIENTO_V1, "tcp");
	if (clnt == NULL) {
		clnt_pcreateerror(ipRpc);
		exit(1);
	}
	retval = init_1(1, &res, clnt);

	if (retval != RPC_SUCCESS) {
		clnt_perror(clnt, "call failed:");
	}
  /*FIN INICIO SERVICIO RPC*/

	/*CREAMOS THREAD PARA SUSCRIPTORES*/
	/* Hilo independiente*/
	pthread_attr_init(&attr2);
	pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
	pthread_create(&threadSuscriptores, &attr2, (void *)peticionesSuscriptores, (void *)&puertoSuscriptores);

	/***********************************/
	/*FIN SUSCRIPTORES -- SOLO EDITORES*/
	/***********************************/

	/*CREAMOS THREAD PARA ATENDER A EDITORES*/
	/* Hilo independiente*/
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);

	/*ABRIMOS EL SOCKET*/
	if((sdE=socket(AF_INET, SOCK_STREAM, 0)) ==-1){
		perror("Error en la creacion del socket\n");
		return(-1);
	}

	/*DAR VALORES INICIALES AL SOCKET*/
	int val=1;
	setsockopt(sdE, SOL_SOCKET, SO_REUSEADDR,(char *)&val, sizeof(int));

	/*VACIAMOS LA ESTRUCTURA*/


	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(puertoEditores);
	server_addr.sin_addr.s_addr=INADDR_ANY;

	// printf("El puerto para editores del servidor es %s\n", argv[1]);	//prueba

	/*ASIGNAMOS DIRECCION AL SOCKET*/
	if(bind(sdE,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
		perror("Error en el bind\n");
		return(-1);
	}

	/*EMPEZAMOS A ESCUCHAR LAS COMUNICACIONES*/
	if((listen(sdE,SOMAXCONN) < 0))
	{
		perror("Error listen de editores\n");
		return -1;
	}
	size = sizeof(client_addr);
	getLocalAddress(ipLocal);
	printf("s> init server editors %s: -E <%d>\n", ipLocal, puertoEditores);

	while(1)
	{
		/*ACEPTAMOS LA CONEXION*/
		if((sc=accept(sdE, (struct sockaddr *)&client_addr, (socklen_t*)&size))<0)
		{
			perror("Error en el accept de editores\n");
			return(-1);
		}

		pthread_create(&threadEditores, &attr1, (void *)tratarEditores, (void *)&sc); // creacion de hilo

		/* --- SECCION CRITICA - COPIA PETICION --- */
                pthread_mutex_lock(&m1);
		while(copiado1 == 0)
		{
			pthread_cond_wait(&cv1, &m1);	// bloquea hasta que hilo ha copiado los datos
		}
		copiado1 = 0;
		pthread_mutex_unlock(&m1);
		/* --- FIN SECCION CRITICA --- */
	}
	close(sdE);
	return(0);
}
