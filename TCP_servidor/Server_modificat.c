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


/************************
*
*
* tcpServidor
*
*
*/

int main(int argc, char *argv[])
{
    struct sockaddr_in	serverAddr;
    struct sockaddr_in	clientAddr;
    unsigned int			sockAddrSize;
    int			sFd;
    int			newFd;
    int 		result;
    char		buffer[256];
    int         bufferlen=0;
    int         temps = 0;
    char        tempsstr[2];
    int         numeromostra = 0;
    float         valorTemp = 46.00;
    char        valorTempstr[6];
    char		missatgesdeprova[] = "Hola\n";
    pthread_t Hilos[5];

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
    //Aceptar client
    newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
    printf("Connexión acceptada del client: adreça %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    
    //PROTOCOL DE COMUNICACIÓ
    while(1){

        /*Rebre*/
        memset(buffer, 0, 256 );
        result = read(newFd, buffer, 256); //Mirar si ha llegado algun mensaje.
        bufferlen = strlen(buffer);

       // printf("El caràcter que estem enviat del buffer 2 és\n");
        printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);

        if (buffer[0] == '{' && buffer[bufferlen-1] == '}'){
            switch (buffer[1]) {
                case 'M':
                    switch (buffer[2]) {
                        case '0': //cas M0
                            printf("Això funciona i estem a la etapa M0");
				//FUNCIONS DE LA ETAPA:
				//1() Comprobar si el sistema ja està en mode parada.
				//2()Si no ho està, donar l'ordre de posar mode Parada. 
				

				//Enviar comprovació a client.
                            strcpy(buffer, "{M0}\n");
                            result = write(newFd, buffer, strlen(buffer)+1);
                            printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                                break;
                        case '1': //cas M1
                            printf("Això funciona i estem a la etapa M1 \n");
				//FUNCIONS DE LA ETAPA:
				//1() Comprobar si el sistema ja està en mode Marxa.
				//2()Si no ho està, donar l'ordre de posar mode Marxa segons la configuració enviada. 
                            tempsstr[0] = buffer[3];//Agafem les dades de temps
                            tempsstr[1] = buffer[4];
                            temps = atoi(tempsstr);//Aquí posem els valors de temps a tempstr en int
                            numeromostra = (buffer[5]- 48); //Agafem la dada del número de mostres i fem operació Char-Int
                            printf("Número de mostres: %i\n", numeromostra);//*Comprovació




				//Enviar comprovació a client.
                            strcpy(buffer, "{M0}\n");//Retornar missatge amb codi de validació 0 -> OK
                            result = write(newFd, buffer,strlen(buffer) + 1); 
                            printf("Missatge enviat a client(bytes %d): %s\n El temps es de: %d i el numero de mostres: %d \n", result, buffer, temps, numeromostra);
                               break;
                        default:
                            printf("Això funciona i estem a la etapa error cas M\n");
                            strcpy(buffer, "{M1}\n");
                            result = write(newFd, buffer,strlen(buffer) + 1); // el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                            printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
                               break;
                        }
                        break;
                case 'U': // caso U
                    printf("Això funciona i estem a la etapa U de moment");
		    //FUNCIONS DE LA ETAPA:
		    //1()Agafar última dada del historial
		    //2()Prepararla per enviar-la a client
                    memset(buffer, 0, strlen(buffer));
                    printf("\nLa temperatura és de: %f\n", valorTemp);
                    sprintf(valorTempstr, "%d",valorTemp);
                    


		    //Enviar dades a client
		    strcpy(buffer, "{U0");
                    strcat(buffer, valorTempstr);
                    strcat(buffer, "}");
                    printf("%s\n", buffer);
                    result = write(newFd, buffer, 50);// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
                        break;
                case 'X': //caso X
                    //mostrar la lectura maxima en el registre circular

                        break;
                case 'Y': // caso Y
                    //mostrar la lectura mínima en el registre circular

                        break;
                case 'R': //caso R
                    //resetejar el máxim i mínim

                        break;
                case 'B': // caso B
                    //demanar el número de mostres guardares a la array auesta circular

                        break;
            }


        } else if (buffer[0] != '{' || buffer[bufferlen-1] != '}'){
            strcpy(buffer, "{E2}");
            result = write(newFd, buffer, strlen(buffer)+1);// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
            printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);


        }
        /*Enviar*/
       /* strcpy(buffer,missatge); //Copiar missatge a buffer
        result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
        printf("Missatge enviat a client(bytes %d): %s\n",	result, missatge);*/

        /*Tancar el socket fill*/
       // result = close(newFd);
    }
}


