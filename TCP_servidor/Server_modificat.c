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
char 	arrayCircular[L_Array][TAM_MUESTRA]= {}; //Matriu dades temperatura a la llista
char 	valor_transportat[6];

//FUNCIONS
void Comunicacio_ControlSensor(char TempsMostreig[3], int NumerosMitjana);
void Desplasament_llista();

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

    //PROTOCOL DE COMUNICACIÓ
	//1. Acceptem connexió client
	//2. Llegim comanda.
	//3. Tanquem connexió.
	//4. Esperem nova connexió.
    while(1){

		/*Crear una cua per les peticions de connexió*/
		result = listen(sFd, SERVER_MAX_CONNECTIONS);
		printf("\nServidor esperant connexions\n");
		/*Esperar conexió. sFd: socket pare, newFd: socket fill*/
	   
		//Esperar i acceptar client
		newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
		printf("Connexió acceptada del client: adreça %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			
			/*Rebre*/
        memset(buffer, 0, 256 );
        result = read(newFd, buffer, 256); //Mirar si ha llegado algun mensaje. Guardarlo en BUFFER.
        bufferlen = strlen(buffer);

       // printf("El caràcter que estem enviat del buffer 2 és\n");
        printf("\nMissatge rebut del client(bytes %d): %s\n",	result, buffer);

        if (buffer[0] == '{' && buffer[bufferlen-1] == '}'){
            switch (buffer[1]) {
                case 'M':
                    switch (buffer[2]) {
                        case '0': //cas M0 ORDRE PARADA
                            
                            printf("Això funciona i estem a la etapa M0\n");//
							//FUNCIONS DE LA ETAPA:
							//1() Comprobar si el sistema ja està en mode parada.
							//2()Si no ho està, donar l'ordre de posar mode Parada. 
				
							//Enviar comprovació a client.
							memset(buffer, 0, strlen(buffer)+1);
                            strcpy(buffer, "{M0}");
                            result = write(newFd, buffer, strlen(buffer)+1);
                            printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                            printf("Adquisició finalitzada\n");    
                                break;
                        case '1': //cas M1 ORDRE MARXA
                            printf("Això funciona i estem a la etapa M1 \n");//*
							//FUNCIONS DE LA ETAPA:
							//1() Comprobar si el sistema ja està en mode Marxa.
							//2()Si no ho està, donar l'ordre de posar mode Marxa segons la configuració enviada. 
							if (buffer[3] == '0')
							{
								tempsstr[0] = buffer[4];
							}
							else
							{
								tempsstr[0] = buffer[3];//Agafem les dades de temps
								tempsstr[1] = buffer[4];
							}
                            temps = atoi(tempsstr);//Aquí posem els valors de temps a tempstr en int
                            numeromostra = (buffer[5]- 48); //Agafem la dada del número de mostres. Operació Char to Int
                            printf("Número de mostres: %i\n", numeromostra);//*Comprovació/ENVIAMENT VIA SERIE A CONTROL S
							//Obrir thread en cas de volguer consultar altres dades mentres es va analitzant.
							//Iniciar comunicació amb Control sensor
							Comunicacio_ControlSensor(tempsstr, numeromostra);
							
                                                       
							//Enviar comprovació a client.
							memset(buffer, 0, strlen(buffer)+1);
                            strcpy(buffer,"{M0}\n");//Retornar missatge amb codi de validació 0 -> OK
							//printf("ENVIAMENT: %s\n", buffer);
                            result = write(newFd, buffer,strlen(buffer) + 1); 
                            printf("Missatge enviat a client(bytes %d): %s\nEl temps és de: %d i el numero de mostres: %d \n", result, buffer, temps, numeromostra);//*
                               break;
						default:
                            printf("Això funciona i estem a la etapa error de paràmetres cas M\n");//*
							memset(buffer, 0, strlen(buffer)+1);
                            strcpy(buffer, "{M2}\n");
							//printf("ENVIAMENT: %s\n", buffer);
                            result = write(newFd, buffer,strlen(buffer) + 1); // el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                            printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
								break;
                        }
                        break;
                case 'U': // cas U DADA ANTIGA
                    printf("Això funciona i estem a la etapa U\n");//*
					//FUNCIONS DE LA ETAPA:
					//1()Agafar última dada del historial
					M_ANTIGA[0] = arrayCircular[nscans-1][0];
					M_ANTIGA[1] = arrayCircular[nscans-1][1];
					M_ANTIGA[2] = arrayCircular[nscans-1][2];
					M_ANTIGA[3] = arrayCircular[nscans-1][3];
					M_ANTIGA[4] = arrayCircular[nscans-1][4];
					
					//2()Prepararla enviament a client
                    memset(buffer, 0, strlen(buffer));
					strcpy(buffer, "{U0");
					strcat(buffer, M_ANTIGA);
                    strcat(buffer, "}");
                    //printf("ENVIAMENT: %s\n", buffer);
                    result = write(newFd, buffer,strlen(buffer) + 1);// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);	
					//Borrar mostra antiga per la seguent
					arrayCircular[nscans][0] = 0;
					arrayCircular[nscans][1] = 0;
					arrayCircular[nscans][2] = 0;
					arrayCircular[nscans][3] = 0;
					arrayCircular[nscans][4] = 0;
					if(nscans!= 0){
						nscans--;
					}
					/*int i=0;
					for(i = 0 ; i < (L_Array);i++ ){
						printf("%d Prova Valor afegit: %s\n", i, arrayCircular[i]);*/
												
					
					
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
                    //printf("ENVIAMENT: %s\n", buffer);
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
					//printf("ENVIAMENT: %s\n", buffer);
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
					char x[5];
					sprintf(x,"%i", nscans);
			    /* Enviar sempre 4 xifres procedents del número de mostres*/
			    if (strlen(x)<4){ 
			    	int d=0;
				    for (d= 0; d < (4-strlen(x)); d++){
					  strcat(buffer, "0");  
				    }
				strcat(buffer, x);
			    }
			    else{
				    strcat(buffer, x);
			    }
         
                    strcat(buffer, "}");
                    printf("ENVIAMENT: %s\n", buffer);
                    result = write(newFd, buffer, strlen(buffer) + 1);// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);//*
                        break;

		default:
					printf("Això funciona i estem a la etapa d'error de paràmetres\n");
					memset(buffer,0,strlen(buffer));
					strcpy(buffer, "{E2}");
					result=write(newFd, buffer, strlen(buffer)+1);
					printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);//*
            }


        } else if (buffer[0] != '{' || buffer[bufferlen-1] != '}'){
			memset(buffer, 0, strlen(buffer));
		    strcpy(buffer, "{E1}");
            result = write(newFd, buffer, strlen(buffer)+1);// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
            printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);//*


        }
	
        /*Tancar el socket fill*/
        result = close(newFd);
    }
}
void Comunicacio_ControlSensor(char TempsMostreig[3], int NumerosMitjana)
{
	int contador=0;
    int i=0;
	//time_t t;
        	
		//Entrada de informació 
		/*GENERACIÓ DE NOMBRES PER OMPLIR ARRAY CIRCULAR*/
		//(No es te en compte el temps ni el numero de mostres per fer mitjana).
		/* Intializes random number generator */
		
		srand((unsigned) time(NULL));
		float r =0;
		nscans=0;
		/* Print L_array random numbers from 0 to 70 */
		for(i = 0 ; i < (L_Array);i++ ) {
			r = ((float)rand()/(float)RAND_MAX)*70;
			sprintf(valor_transportat, "%.2f", r);
			Desplasament_llista();
			/*if (nscans > (L_Array-1))	//DESPLAÇAR VALORS DE L'ARRAY UNA POSICIÓ CAP A LA DRETA, EN CAS DE TROBARSE PLENA I BORRAR L'ÚLTIM VALOR.
			{
				
				contador= L_Array;
			}*/
			//ANALISIS DE LES MOSTRES* (pendent)
			
			//GUARDAR DADA
			arrayCircular[0][0]=valor_transportat[0];
			arrayCircular[0][1]=valor_transportat[1];
			arrayCircular[0][2]=valor_transportat[2];
			arrayCircular[0][3]=valor_transportat[3];
			arrayCircular[0][4]=valor_transportat[4];
			printf("%d Valor afegit: %s\n", nscans, arrayCircular[0]);
			contador++;

			/*Mostrar array actual	
			for (i=0; i< L_Array; i++){ //VISUALIZAR ARRAY DE PARTIDA
				printf("[%s] ",arrayCircular[i]);
			}
			printf("\n");
			*/
		}
}
void Desplasament_llista(){
	int i;
	i=0;
	for(i=0; i<(L_Array); i++) 
	{
		arrayCircular[((L_Array-1)-i)][0]= arrayCircular[((L_Array-1)-(i+1))][0];
		arrayCircular[((L_Array-1)-i)][1]= arrayCircular[((L_Array-1)-(i+1))][1];
		arrayCircular[((L_Array-1)-i)][2]= arrayCircular[((L_Array-1)-(i+1))][2];
		arrayCircular[((L_Array-1)-i)][3]= arrayCircular[((L_Array-1)-(i+1))][3];
		arrayCircular[((L_Array-1)-i)][4]= arrayCircular[((L_Array-1)-(i+1))][4];
				}
	 nscans++;
	 if (nscans > 100)
	 nscans=100;
	}
