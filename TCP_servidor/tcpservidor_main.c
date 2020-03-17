/*
 * XXXXXX.c
 * 
 * Copyright 2019 INFIN (EUSS) <euss@euss.cat>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

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
    char		missatge[] = "me la suda bastant\n";

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
        bufferlen = strlen(buffer[256]);

        printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);
        if (buffer[0] == '{' && buffer|bufferlen-1| == '}'){
            switch (buffer[1]) {
                case 'M':
                    switch (buffer[2]) {
                        case '0':
                            strcpy(buffer, "{M0}\n");
                            result = write(newFd, buffer, strlen(bufferlen) + 1);
                            printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                            break;
                        case '1':
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
                            result = write(newFd, buffer, strlen(buffer) + 1); // el +1 el posem per enviar el 0 al final de la cadena
                            printf("Missatge enviat a client(bytes %d): %s\n El temps és de: %d i el número de mostres: %f \n", result, buffer, temps, numeromostra);
                            break;
                        default:
                            strcpy("{M1}\n");
                            result = write(newFd, buffer,strlen(buffer) + 1); // el +1 el posem per enviar el 0 al final de la cadena
                            printf("Missatge enviat a client (bytes %d): %s\n", result, missatge);
                                break;
                        }
                        break;
                case'U':
                    memset(missatge, 0, strlen(missatge));
                    printf("%d\n", valorTemp);
                    sprintf(valorTempstr, "%d",valorTemp);

            }


        }
        /*Enviar*/
        strcpy(buffer,missatge); //Copiar missatge a buffer
        result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
        printf("Missatge enviat a client(bytes %d): %s\n",	result, missatge);

        /*Tancar el socket fill*/
        result = close(newFd);
    }
}


