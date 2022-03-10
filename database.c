/* 	JAVIER PRIETO CEPEDA		100307011 
	PEDRO ROBLEDO QUINTERO	 	100XXXXXX
*/

#include "database.h"

/* VARIABLES GLOBALES */
pthread_mutex_t mutex;	        		// Mutex
pthread_cond_t espera;       			// Variables condicionales
int numeroLectores = 0;				// Numero de lectores en el sistema

tema *lista = NULL;	// Lista de temas en el sistema

/* INICIALIZAR BASE DE DATOS */
int init()
{
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&espera, NULL);
	return OK;
}

/* DESTRUIR BASE DE DATOS */
int destroy()
{
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&espera);
	return OK;
}

/* DAR DE ALTA A UN SUSCRIPTOR EN UN TEMA */
/* RETURN 0  --> OK 
*  RETURN -1 --> YA SUSCRITO
*/
int altaSuscriptor(char *topic, char *ip, u_short puerto)
{
	tema *iterador;
	 /* --- SECCION CRITICA -- ESCRIBIR EN BBDD --- */
	pthread_mutex_lock(&mutex);
	while(numeroLectores>0)	
	{
		pthread_cond_wait(&espera, &mutex);	// Si hay lectores, esperar
	}
	iterador = lista;
	if(iterador ==NULL)	// Lista vacia -- Primer elemento
	{
		altaTema(topic); // Nos suscribimos al nuevo elemento
		iterador= lista;
		suscriptor *iteradorAux;

		iterador->listaSuscritos = (suscriptor *)malloc(sizeof(suscriptor)); 
		iteradorAux=iterador->listaSuscritos;
				
		strcpy((char *)iteradorAux->ip, (const char*) ip);
		iteradorAux -> puerto = puerto;
		iterador -> suscritos = (iterador -> suscritos) + 1;
		iteradorAux -> next = NULL;
		pthread_mutex_unlock(&mutex);
		/* --- FIN SECCION CRITICA --- */
		return OK;
	}else{
		while(iterador)
		{
			// printf("TEMA: %s\n",iterador->nombre); // prueba
			if((strcmp((const char *)iterador->nombre,(const char *)topic)==0))	// Tema encontrado
			{
				suscriptor *iteradorAux;
				iteradorAux = iterador->listaSuscritos;
				if(iteradorAux == NULL)	// Lista vacia -- Primer elemento
				{
					perror("ERROR, AQUI NUNCA\n");	// prueba					
					return -1;
					// ERROR, NUNCA DEBERIA ENTRAR AQUI
				}else{
					while(iteradorAux)
					{
						if(strcmp((const char *)iteradorAux->ip,(const char *)ip)==0
						&& (puerto == iteradorAux->puerto))
						{
							pthread_mutex_unlock(&mutex);
							/* --- FIN SECCION CRITICA --- */
							return OK;	// YA ESTA SUSCRITO AL TEMA
						}
						if(iteradorAux -> next == NULL)	// SUSCRIBIMOS AL TEMA
						{
							// SUSCRIBIMOS
							iteradorAux -> next  = (suscriptor *)malloc(sizeof(suscriptor));
							iteradorAux = iteradorAux -> next;
							strcpy((char *)iteradorAux->ip, (const char*) ip);
							iteradorAux -> puerto = puerto;
							iterador -> suscritos = (iterador -> suscritos) + 1;
							iteradorAux -> next = NULL;
							pthread_mutex_unlock(&mutex);
							/* --- FIN SECCION CRITICA --- */
							return OK;
						}
						iteradorAux = iteradorAux -> next;
					}
				}
			}
			if(iterador -> next == NULL)
			{
				altaTema(topic); // Nos suscribimos al nuevo elemento					
				iterador= iterador -> next;
				suscriptor *iteradorAux;

				iterador->listaSuscritos = (suscriptor *)malloc(sizeof(suscriptor)); 
				iteradorAux=iterador->listaSuscritos;
				
				strcpy((char *)iteradorAux->ip, (const char*) ip);
				iteradorAux -> puerto = puerto;
				iterador -> suscritos = (iterador -> suscritos) + 1;
				iteradorAux -> next = NULL;
				pthread_mutex_unlock(&mutex);
				/* --- FIN SECCION CRITICA --- */
				return OK;
			}
			iterador = iterador -> next;
		}
	}
	pthread_mutex_unlock(&mutex);
	/* --- FIN SECCION CRITICA --- */
	return -1;	// ERROR, NO DEBERIA LLEGAR AQUI 
}

/* DAR DE BAJA A UN SUSCRIPTOR EN UN TEMA */
/* RETURN 0  --> OK 
*  RETURN -1 --> NO EXISTE
*/
int bajaSuscriptor(char *topic, char *ip, u_short puerto)		// Dar de baja a un suscriptor
{
	tema *iterador;
	 /* --- SECCION CRITICA -- ESCRIBIR EN BBDD --- */
	pthread_mutex_lock(&mutex);
	while(numeroLectores>0)	
	{
		pthread_cond_wait(&espera, &mutex);	// Si hay lectores, esperar
	}
	iterador = lista;

	if(iterador ==NULL)	// Lista vacia -- Primer elemento
	{
		pthread_mutex_unlock(&mutex);
		/* --- FIN SECCION CRITICA --- */
		return 1;
	}else{
		while(iterador)
		{
			//printf("TEMA: %s\n",iterador->nombre);	// prueba
			if((strcmp((const char *)iterador->nombre,(const char *)topic)==0))	// Tema encontrado
			{
				//printf("ENTRO\n");	// prueba
				suscriptor *iteradorAux;
				suscriptor *iteradorPrev;
				iteradorPrev = NULL;
				iteradorAux = iterador->listaSuscritos;
				if(iteradorAux ==NULL)	// Lista vacia -- Primer elemento
				{
					// ERROR, NUNCA DEBERIA ENTRAR AQUI
					//printf("ENTRO1\n");	// prueba
				}else{
					while(iteradorAux)
					{
						// printf("BUSCADO: -IP: %s -puerto %d \n IP: %s PUERTO %d\n",ip,puerto,iteradorAux->ip,iteradorAux->puerto); // prueba
						if((strcmp((const char *)iteradorAux->ip,(const char *)ip)==0) && 
						(puerto == iteradorAux->puerto))
						{
							if(NULL == iteradorPrev) // Es el primero de la lista
							{
								iterador-> listaSuscritos = iteradorAux -> next ;
								free(iteradorAux);
								iterador -> suscritos = (iterador -> suscritos) - 1;
								
							}else			// No es el primero
							{
								iteradorPrev->next = iteradorAux->next;
								free(iteradorAux);
								iterador -> suscritos = (iterador -> suscritos) - 1;
							}	
							if((iterador -> suscritos) == 0) // Ningun suscrito al tema, borramos tema
							{
								bajaTema(topic);
							}
							pthread_mutex_unlock(&mutex);
							/* --- FIN SECCION CRITICA --- */						
							return OK;	//  ESTA SUSCRITO AL TEMA
						}
						if(iteradorAux -> next == NULL)	// NO ESTA SUSCRITO AL TEMA
						{
							pthread_mutex_unlock(&mutex);
							/* --- FIN SECCION CRITICA --- */
							return 1;
						}
						iteradorPrev = iteradorAux;
						iteradorAux = iteradorAux -> next;
					}
				}
			}
			if(iterador -> next == NULL)	// No existe el tema
			{
				pthread_mutex_unlock(&mutex);
				/* --- FIN SECCION CRITICA --- */
				return 1;
			}
			iterador = iterador -> next;
		}
	}
	pthread_mutex_unlock(&mutex);
	/* --- FIN SECCION CRITICA --- */
	return -1;	// ERROR, NO DEBERIA LLEGAR AQUI 
	
}

/* DAR DE BAJA A UN SUSCRIPTOR EN UN TEMA */
/* RETURN 0  --> OK 
*  RETURN -1 --> NO EXISTE
*/
int altaTema(char *topic)
{
	tema *iterador;
	
	iterador = lista;
	if(iterador ==NULL)	// Lista vacia -- Primer elemento
	{
		iterador  = (tema *)malloc(sizeof(tema));
		strcpy((char *)iterador->nombre, (const char*) topic);
		iterador -> suscritos = 0;
		iterador -> listaSuscritos = NULL;
		lista = iterador;
		iterador -> next = NULL;
	}else{			// Lista no vacia
		while(iterador)
		{
			if(iterador -> next == NULL)
			{
				iterador -> next = (tema *)malloc(sizeof(tema));
				iterador  = iterador -> next;
				strcpy((char *)iterador->nombre, (const char*) topic);
				iterador -> suscritos = 0;
				iterador -> listaSuscritos = NULL;

				iterador -> next = NULL;
			}
			iterador = iterador -> next;
		}
	}
	return OK; 
}

/* DAR DE BAJA A UN SUSCRIPTOR EN UN TEMA */
/* RETURN 0  --> OK 
*  RETURN -1 --> NO EXISTE
*/
int bajaTema(char *topic)		// Dar de baja a un suscriptor
{
	tema *iterador;
	tema *iteradorPrev;

	iterador = lista;
	if(iterador ==NULL)	// Lista vacia -- Nunca deberia llegar aqui
	{
		return -1;
	}else{			// Lista no vacia
		iteradorPrev = NULL;
		while(iterador)
		{
			if((strcmp((const char *)iterador->nombre,(const char *)topic)==0))	// Tema encontrado
			{
				if(NULL == iteradorPrev) // Es el primero de la lista
				{
					lista = iterador -> next ;
					free(iterador);
								
				}else			// No es el primero
				{
					iteradorPrev->next = iterador->next;
					free(iterador);
				}
				return OK;	
			}
			if(iterador -> next == NULL)
			{
				return -1;
			}
			iteradorPrev = iterador;
			iterador = iterador -> next;
		}
	}
	return -1; 
}



/* ENVIAR MENSAJE SUSCRIPTORES DE UN TEMA */
/* RETURN 0  --> OK 
*  RETURN -1 --> ERROR
*/
int enviarMensajeTema(char *messageEditor, char *topic, int scEs)
{
	struct sockaddr_in server_addr;
	struct hostent *hp;
	tema *iterador;
	tema *iteradorPrevTema;
	iteradorPrevTema= NULL;
	int 	sd, val;
	int error = 0;
	/* --- SECCION CRITICA -- ESCRIBIR EN BBDD --- */
	pthread_mutex_lock(&mutex);
	while(numeroLectores>0)	
	{
		pthread_cond_wait(&espera, &mutex);	// Si hay lectores, esperar
	}

	iterador = lista;
	if(iterador ==NULL)	// Lista vacia -- Primer elemento
	{
		//NO HAY TEMAS RESGISTRADOS, NO ENVIO NADA OK
		pthread_mutex_unlock(&mutex);
		/* --- FIN SECCION CRITICA --- */
		return 0;		
	}else{			// Lista no vacia
		while(iterador)
		{
			// printf("TEMA A MENSAJEAR: %s\n",iterador->nombre); // prueba
			if((strcmp((const char *)iterador->nombre,(const char *)topic)==0))	// Tema encontrado
			{
				suscriptor *iteradorAux;
				suscriptor *iteradorPrev;
				iteradorAux = iterador->listaSuscritos;
				iteradorPrev = NULL;
				//printf("Mensaje a mandar : %s -- length : %d\n", messageEditor, strlen(messageEditor));
				if(iteradorAux ==NULL)	// Lista vacia -- Primer elemento
				{
					// ERROR, NUNCA DEBERIA ENTRAR AQUI
					pthread_mutex_unlock(&mutex);
					/* --- FIN SECCION CRITICA --- */
					return -1;
				}else{
					while(iteradorAux)
					{
						error = 0;
						/*REALIZAMOS CONEXION Y ENVIO, SI NO CONEXION, LO BORRAMOS*/						
						/*ABRIMOS EL SOCKET*/
						if((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0){
							perror("Error in the creation of socket\n");	//prueba
							pthread_mutex_unlock(&mutex);
							/* --- FIN SECCION CRITICA --- */
							error = 1;
						}else{
							/*DAR VALORES INICIALES AL SOCKET*/
							val = 1;
							setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof (int));
							/*VACIAMOS LA ESTRUCTURA*/
							bzero((char *) &server_addr, sizeof(server_addr));
							server_addr.sin_family = AF_INET;

						
							hp = (struct hostent*)gethostbyname(iteradorAux->ip);
							memcpy((void *)&(server_addr.sin_addr), (const void *)hp->h_addr, hp->h_length); 
							//inet_aton(iteradorAux->ip, &server_addr.sin_addr.s_addr);

							server_addr.sin_port = (u_short)htons(iteradorAux->puerto);

							//printf("ANTES DE CONEXION ..\n"); // prueba
							//printf("IP: %s Puerto: %d\n",inet_ntoa(server_addr.sin_addr), iteradorAux->puerto); // prueba
							//printf("IP: %s Puerto: %d\n",iteradorAux->ip, iteradorAux->puerto);	// prueba
							//hp = gethostbyname(argv[1]); /*HAY QUE ACTUALIZARLO!!*/
							/*CONECTAMOS AL SUSCRIPTOR*/
							if(connect(sd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
							{
								perror("Error in the connection to the suscriptor");
								error = 1;
							}
							if(error==0)
							{
								//printf("Conexion Realizada\n");	//prueba
								/*ENVIAMOS EL MENSAJE*/
								if(enviar(sd, messageEditor, strlen(messageEditor)+1)!=0)	//envio el mensaje acabado en /0
								{
								 	perror("Error in the send to the suscriptor\n");	//prueba
									error = 1;	
								}
							}
						}
						close(sd);
						if(error==1) // BORRAMOS SUSCRIPCION
						{
							//printf("ERROR AL BROADCASTEAR\n"); // prueba
							if(NULL == iteradorPrev) // Es el primero de la lista
							{
								iterador-> listaSuscritos = iteradorAux -> next ;
								free(iteradorAux);
								iterador -> suscritos = (iterador -> suscritos) - 1;
								
							}else			// No es el primero
							{
								iteradorPrev->next = iteradorAux->next;
								free(iteradorAux);
								iterador -> suscritos = (iterador -> suscritos) - 1;
							}	
						}
						iteradorPrev = iteradorAux;
						iteradorAux = iteradorAux -> next;
					}
					if(iterador -> suscritos == 0)
					{
						/*BORRAMOS EL TEMA*/
						if(NULL == iteradorPrevTema) // Es el primer tema de la lista
						{
							lista = iterador -> next ;
							free(iterador);
								
						}else			// No es el primero
						{
							iteradorPrevTema->next = iterador->next;
							free(iterador);
						}
					}
					pthread_mutex_unlock(&mutex);
					/* --- FIN SECCION CRITICA --- */
					return 0;	//prueba
				}
				pthread_mutex_unlock(&mutex);
				/* --- FIN SECCION CRITICA --- */
				return -1;
			}
			iteradorPrevTema = iterador;
			iterador = iterador -> next;
		}
		// TEMA NO ENCONTRADO, NO ENVIO NADA, PORQUE NO HAY SUSCRIPTORES A ESE TEMA
		pthread_mutex_unlock(&mutex);
		/* --- FIN SECCION CRITICA --- */
		return 2;	//prueba
	}
	pthread_mutex_unlock(&mutex);
	/* --- FIN SECCION CRITICA --- */
	return 0;
}



