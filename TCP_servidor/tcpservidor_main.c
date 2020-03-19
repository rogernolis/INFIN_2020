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
    char        tempsstr[3];
    char        numeromostrastr[3];
    int         numeromostra = 0;
    int         valorTemp = 46000;
    char        valorTempstr[6];
    char		missatgesdeprova[] = "me la suda bastant\n";

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

    /*Bucle s'acceptació de connexions*/
    while(1){
        printf("\nServidor esperant connexions\n");

        /*Esperar conexió. sFd: socket pare, newFd: socket fill*/
        newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
        printf("Connexión acceptada del client: adreça %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        /*Rebre*/
        memset( buffer, 0, 256 );
        result = read(newFd, buffer, 256);
        bufferlen = strlen(buffer);

       // printf("El caràcter que estem enviat del buffer 2 és\n");
        printf("Missatge rebut del client(bytes %d): %c\n",	result, buffer);

        if (buffer[0] == '{' && buffer[bufferlen-1] == '}'){
            switch (buffer[1]) {
                case '1':
                    switch (buffer[2]) {
                        case '0': //cas M0
                            printf("Això funciona i estem a la etapa M0");
                            strcpy(buffer, "{M0}\n");
                            result = write(newFd, buffer, strlen(bufferlen));
                            printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                                break;
                        case '1': //caso M
                            printf("Això funciona i estem a la etapa M1");
                            strcpy(numeromostrastr, "000");
                            strcpy(numeromostra, "000");
                            tempsstr[0] = '0';
                            tempsstr[1] = buffer[3];
                            tempsstr[2] = buffer[4];
                            temps = atoi(tempsstr);   //Aquí posem els valors de temps a tempstr en ints
                            numeromostrastr[0] = '0';
                            numeromostrastr[1] = '0';
                            numeromostrastr[2] = buffer[5];
                            printf("Número de mostres %s\n", numeromostrastr);
                            strcpy(buffer, "{M0}\n");
                            result = write(newFd, buffer,strlen(buffer) + 1); // el +1 el posem per enviar el 0 al final de la cadena
                            printf("Missatge enviat a client(bytes %d): %s\n El temps és de: %d i el número de mostres: %f \n", result, buffer, temps, numeromostra);
                               break;
                        default:
                            printf("Això funciona i estem a la etapa error cas M\n");
                            strcpy(buffer, "{E1}\n");
                            result = write(newFd, buffer,strlen(buffer) + 1); // el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                            printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
                               break;
                        }
                        break;
                case '2': // caso U
                    printf("Això funciona i estem a la etapa U de moment");
                    memset(buffer, 0, strlen(buffer));
                    printf("\nLa temperatura és de: %d\n", valorTemp);
                    sprintf(valorTempstr, "%d",valorTemp);
                    strcpy(buffer, "{U0");
                    strcat(buffer, valorTempstr);
                    strcat(buffer, "}");
                    printf("%s\n", buffer);
                    memset(buffer, 0, strlen(buffer)+1);
                    //strcpy(buffer, buffer);
                    result = write(newFd, buffer, 50);// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
                        break;
                case '3': //caso X
                    //mostrar la lectura maxima en el registre circular

                        break;
                case '4': // caso Y
                    //mostrar la lectura mínima en el registre circular

                        break;
                case '5': //caso R
                    //resetejar el máxim i mínim

                        break;
                case '6': // caso B
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

    
