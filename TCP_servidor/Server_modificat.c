/***************************************************************************
                          main.c  -  server
                             -------------------
    begin                : lun feb  4 15:30:41 CET 2002
    copyright            : (C) 2002 by A. Moreno
    copyright            : (C) 2020 by A. Fontquerni
    email                : amoreno@euss.es
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4
#define REQUEST_MSG_SIZE	1024
#define L_Array 100 // Longitud llista de mostres
#define TAM_MUESTRA 6 //Precisio de la mostra + 0 + (indicar final array)

/************************
*
*
* tcpServidor
*
*
*/
//DADES REGISTRE / variables Globals / intentar fer-les locals?
char 	MAXIMA[6]="99.99"; //*
char 	MINIMA[6]="00.00";//*
int 	nscans=0;
char 	M_ANTIGA[6]="12.10";//*
char 	arrayCircular[L_Array][TAM_MUESTRA]= {00.00,00.00,00.00}; //Matriu dades temperatura a la llista
char 	valor_transportat[6];

//FUNCIONS
void Comunicacio_ControlSensor(char TempsMostreig[3], int NumerosMitjana);


int main(int argc, char *argv[])
{
    struct 		sockaddr_in	serverAddr;
    struct 		sockaddr_in	clientAddr;
    unsigned int			sockAddrSize;
    int			sFd;
    int			newFd;
    int 		result;
    char		buffer[256];
    int         bufferlen=0;
    int         temps = 0;
    char        tempsstr[2];
    int         numeromostra = 0;
    //float     valorTemp = 46.00;
    //char      valorTempstr[6];
    //char		missatgesdeprova[] = "Hola\n";
	//int       Mode=0; //Indicador: 1 marxa 0 parada
	
    /*Preparar l'adreça local*/
    sockAddrSize=sizeof(struct sockaddr_in);
    bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT_NUM);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*Crear un socket*/
    sFd=socket(AF_INET, SOCK_STREAM, 0);

    /*Nominalitzar el socket*/
    result = bind(sFd, (struct sockaddr *) &serverAddr, sockAddrSize);

    /*Crear una cua per les peticions de connexió*/
    result = listen(sFd, SERVER_MAX_CONNECTIONS);
    printf("\nServidor esperant connexions\n");
    /*Esperar conexió. sFd: socket pare, newFd: socket fill*/
   
    //Esperar i acceptar client
    newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
    printf("Connexió acceptada del client: adreça %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    
    //PROTOCOL DE COMUNICACIÓ
    while(1){

        /*Rebre*/
        memset(buffer, 0, 256 );
        result = read(newFd, buffer, 256); //Mirar si ha llegado algun mensaje. Guardarlo en BUFFER.
        bufferlen = strlen(buffer);

       // printf("El caràcter que estem enviat del buffer 2 és\n");
        printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);

        if (buffer[0] == '{' && buffer[bufferlen-1] == '}'){
            switch (buffer[1]) {
                case 'M':
                    switch (buffer[2]) {
                        case '0': //cas M0 ORDRE PARADA
                            
                            //printf("Això funciona i estem a la etapa M0");//
							//FUNCIONS DE LA ETAPA:
							//1() Comprobar si el sistema ja està en mode parada.
							//2()Si no ho està, donar l'ordre de posar mode Parada. 
				
							//Enviar comprovació a client.
							memset(buffer, 0, strlen(buffer)+1);
                            strcpy(buffer, "{M0}\n");
                            result = write(newFd, buffer, strlen(buffer)+1);
                            printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);//*
                                break;
                        case '1': //cas M1 ORDRE MARXA
                            printf("Això funciona i estem a la etapa M1 \n");//*
							//FUNCIONS DE LA ETAPA:
							//1() Comprobar si el sistema ja està en mode Marxa.
							//2()Si no ho està, donar l'ordre de posar mode Marxa segons la configuració enviada. 
                            tempsstr[0] = buffer[3];//Agafem les dades de temps
                            tempsstr[1] = buffer[4];
                            temps = atoi(tempsstr);//Aquí posem els valors de temps a tempstr en int
                            numeromostra = (buffer[5]- 48); //Agafem la dada del número de mostres. Operació Char to Int
                            printf("Número de mostres: %i\n", numeromostra);//*Comprovació/ENVIAMENT VIA SERIE A CONTROL S
							//Obrir thread en cas de volguer consultar altres dades mentres es va analitzant.
							//Iniciar comunicació amb Control sensor
							Comunicacio_ControlSensor(tempsstr, numeromostra);


							//Enviar comprovació a client.
							memset(buffer, 0, strlen(buffer)+1);
                            strcpy(buffer, "{M0}\n");//Retornar missatge amb codi de validació 0 -> OK
							printf("ENVIAMENT: %s\n", buffer);
                            result = write(newFd, buffer,strlen(buffer) + 1); 
                            printf("Missatge enviat a client(bytes %d): %s\n El temps es de: %d i el numero de mostres: %d \n", result, buffer, temps, numeromostra);//*
                               break;
                        default:
                            printf("Això funciona i estem a la etapa error cas M\n");//*
							memset(buffer, 0, strlen(buffer)+1);
                            strcpy(buffer, "{M1}\n");
						printf("ENVIAMENT: %s\n", buffer);
                            result = write(newFd, buffer,strlen(buffer) + 1); // el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                            printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);//*
                               break;
                        }
                        break;
                case 'U': // cas U DADA ANTIGA
                    printf("Això funciona i estem a la etapa U\n");//*
					//FUNCIONS DE LA ETAPA:
					//1()Agafar última dada del historial
					//2()Prepararla enviament a client
                    memset(buffer, 0, strlen(buffer));
					strcpy(buffer, "{U0");
                    strcat(buffer, M_ANTIGA);
                    strcat(buffer, "}");
                    printf("ENVIAMENT: %s\n", buffer);
                    result = write(newFd, buffer,strlen(buffer) + 1);// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);//*
                        break;
                case 'X': //cas X MAXIMA REGISTRE
		    printf("Això funciona i estem a la etapa X\n");//*
		    //FUNCIONS DE LA ETAPA:
		    //1()Agafar dada màxima del registre. (MAXIMA)
		    //2()Prepararla enviament a client
                    memset(buffer, 0, strlen(buffer));
					strcpy(buffer, "{X0");
                    strcat(buffer, MAXIMA);
                    strcat(buffer, "}");
                    printf("ENVIAMENT: %s\n", buffer);
                    result = write(newFd, buffer, strlen(buffer) + 1);// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);//*
                        break;
                case 'Y': // cas Y MINIMA REGISTRE
					printf("Això funciona i estem a la etapa Y\n");//*
		    //FUNCIONS DE LA ETAPA:
		    //1()Agafar dada mínima del registre. (MINIMA)
		    //2()Prepararla enviament a client
                    memset(buffer, 0, strlen(buffer));
					strcpy(buffer, "{Y0");
                    strcat(buffer, MINIMA);
                    strcat(buffer, "}");
                    printf("ENVIAMENT: %s\n", buffer);
                    result = write(newFd, buffer, strlen(buffer) + 1);// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);//*
                        break;
                case 'R': //cas R RESET REGISTRES
					printf("Això funciona i estem a la etapa R\n");//*
                    //resetejar el máxim i mínim
					sprintf(MAXIMA,"00.00");//*
					sprintf(MINIMA,"00.00");//*
		    //Comprovant
					printf("MAXIMA: %s\nMINIMA: %s\n", MAXIMA, MINIMA);//*
					memset(buffer, 0, strlen(buffer));
					strcpy(buffer, "{R0}");
					printf("ENVIAMENT: %s\n", buffer);
                    result = write(newFd, buffer,strlen(buffer) + 1); // el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                            printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);//*
                        break;
                case 'B': // cas B NUMERO DE MOSTRES
					printf("Això funciona i estem a la etapa B\n");//*
						//FUNCIONS DE LA ETAPA:
						//1()Agafar variable que conta el numero de mostres guardades (nscans)
						//2()Preparar enviament a client
                    memset(buffer, 0, strlen(buffer));
					strcpy(buffer, "{B0");
					char x[3];
					sprintf(x,"%i", nscans);
                    strcat(buffer, x);
                    strcat(buffer, "}");
                    printf("ENVIAMENT: %s\n", buffer);
                    result = write(newFd, buffer, strlen(buffer) + 1);// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);//*
                        break;
            }


        } else if (buffer[0] != '{' || buffer[bufferlen-1] != '}'){
            strcpy(buffer, "{E2}");
            result = write(newFd, buffer, strlen(buffer)+1);// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
            printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);//*


        }
        /*Enviar*/
       /* strcpy(buffer,missatge); //Copiar missatge a buffer
        result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
        printf("Missatge enviat a client(bytes %d): %s\n",	result, missatge);*/

        /*Tancar el socket fill*/
       // result = close(newFd);
    }
}
void Comunicacio_ControlSensor(char TempsMostreig[3], int NumerosMitjana)
{
	int contador=0;
    int i;
	nscans= 0;



	while (1) //Mientras no haya una interrupción. 
	{
	//Entrada de informació 
		scanf("%s", valor_transportat);
		if (nscans > (L_Array-1))	//DESPLAÇAR VALORS DE L'ARRAY UNA POSICIÓ CAP A LA DRETA, EN CAS DE TROBARSE PLENA I BORRAR L'ÚLTIM VALOR.
		{
			for(i=0; i<L_Array; i++) 
			{
				arrayCircular[((L_Array-1)-i)][0]= arrayCircular[((L_Array-1)-(i+1))][0];
				arrayCircular[((L_Array-1)-i)][1]= arrayCircular[((L_Array-1)-(i+1))][1];
				arrayCircular[((L_Array-1)-i)][2]= arrayCircular[((L_Array-1)-(i+1))][2];
				arrayCircular[((L_Array-1)-i)][3]= arrayCircular[((L_Array-1)-(i+1))][3];
				arrayCircular[((L_Array-1)-i)][4]= arrayCircular[((L_Array-1)-(i+1))][4];
			}
			contador= L_Array-1;
		}
		//ANALISIS DE LES MOSTRES* (pendent)
	
		//GUARDAR DADA
		arrayCircular[((L_Array-1)-contador)][0]=valor_transportat[0];
		arrayCircular[((L_Array-1)-contador)][1]=valor_transportat[1];
		arrayCircular[((L_Array-1)-contador)][2]=valor_transportat[2];
		arrayCircular[((L_Array-1)-contador)][3]=valor_transportat[3];
		arrayCircular[((L_Array-1)-contador)][4]=valor_transportat[4];
		nscans++;
		contador++;
		
		//Mostrar array actual	
		for (i=0; i< L_Array; i++){ //VISUALIZAR ARRAY DE PARTIDA
			printf("[%s] ",arrayCircular[i]);
		}
		printf("\n");
	}
	
}

