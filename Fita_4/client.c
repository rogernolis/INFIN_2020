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
#include <ctype.h>

#define REQUEST_MSG_SIZE	1024
#define REPLY_MSG_SIZE		500
#define SERVER_PORT_NUM		5001

/*menu*/
void ImprimirMenu(void)
{
	printf("\n\nMenú:\n");
	printf("Introduiu un nombre per fer la selecció: \n");
	printf("--------------------\n");
	printf("1:Posar en marxa/parada adquisició {M}\n");
	printf("2:Mostra més antiga {U}\n");
	printf("3:Demanar màxim {X}\n");
	printf("4:Demanar mínim {Y}\n");
	printf("5:Reset màxim i mínim {R}\n");
	printf("6:Demanar comptador mostres guardades {B}\n");
	printf("s:Sortir\n");
	printf("--------------------\n");
}

//Programa
int main(int argc, char *argv[]){
	
	//Variables locals
	struct sockaddr_in	serverAddr;
	char	    serverName[] = "127.0.0.1"; 							//Adreça IP on està el servidor
	int			sockAddrSize;											//vabiables connexió  TCP/IP
	int			sFd;
	int 		h=0;
	int 		result, i=0,j=0,t=0;
	char		buffer[256];
	char 		input1[5] = {},input2[5] = {};

	while(j==0){
		char  		cadena[50]={};
		char		cadena1[50]={};
		char 		cadena2[50]={};
		menu:
		i=0;
		h=0;
		t=0;
		ImprimirMenu();													//Imprimir el menú a seleccionar els diferents casos
					
		do 
		{
			if(h>0)
			{
				printf("VALOR ERRONI\n");
			}
			h++;
			memset(input1, 0, strlen(input1));
			scanf("%s",input1);											//llegir el que s'ha introduït per consola
		}
		while (((isalpha(input1[0]) > 0) && input1[0] != 's' ) || input1[0]=='0' || (input1[0] > '6' && input1[0] != 's') || strlen(input1) > 1);		
		h=0;
		
		if (input1[0] != 's')
		{
			/*Conexió*/
			/*Crear el socket*/
			sFd=socket(AF_INET,SOCK_STREAM,0);
	
			/*Construir l'adreça*/
			sockAddrSize = sizeof(struct sockaddr_in);
			bzero ((char *)&serverAddr, sockAddrSize); 					//Posar l'estructura a zero
			serverAddr.sin_family=AF_INET;
			serverAddr.sin_port=htons (SERVER_PORT_NUM);
			serverAddr.sin_addr.s_addr = inet_addr(serverName);
			result = connect (sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
		
		if (result < 0)
		{
			printf("Error en establir la connexió\n");					//Error de conexió si no és capaç de connectar-se
			//exit(-1);
		}
		else
		{
			printf("\nConnexió establerta amb el servidor: adreça %s, port %d\n",	inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
			
			switch (input1[0]){		
													//Seleccionar les diferents opcions a enviar al servidor
				case '1':
					printf("Heu seleccionat l'opció {M}:Posar en marxa/parar adquisició\n");
					printf("Seleccioneu:\n");
					printf("0:Parada\n");
					printf("1:Marxa\n");
					h=0;
					
					do 
					{
						if(h>0)
						{
							printf("VALOR ERRONI\n");
						}
					h++;
					memset(input2, 0, strlen(input2));
					scanf("%s",input2);									//Legir per consola que s'ha posat
					}
					while ((isalpha(input2[0])>0) || (input2[0]>'1')|| strlen(input2) > 1);
					
					switch (input2[0]){
						
						case '0':										//Seleccionar parada
							cadena[0]='{';								//Copiar missatge de parada per poder ser enviat
							cadena[1]='M';
							cadena[2]='0';
							cadena[3]='}';
							printf("Heu seleccionat parar l'adquisició\n");
								break;
										
						case '1':										//Seleccionar marxa
							cadena[0]='{';								//Copiar missatge de marxa per poder ser enviat
							cadena[1]='M';
							cadena[2]='1';
							printf("Heu seleccionat posada en marxa\n");
					
							do{
								if(i!=0){
									printf("VALOR ERRONI\n");			//Donar error de paràmetres
								}
							printf("Quants segons de mostreig voleu(1..20)?\n");
							memset(cadena1, 0, strlen(cadena1)+1);		//Posar el temps al missatge de marxa
							scanf("%s",&cadena1[0]);
							i++;
							}
							while((cadena1[0]>='2' && cadena1[1]>'0')|| (strlen(cadena1)>2) || (cadena1[0]> '2' && cadena1[1]>='0') ||(isalpha(cadena1[0])!=0)); 
								if(cadena1[1]=='\0')
								{
									cadena1[1]=cadena1[0];				//Solució del problema d'un sol número
									cadena1[0]='0';
								}
							do{
								if(t!=0)
								{
									printf("VALOR ERRONI\n");			//Donar error de paràmetres 
								}
								printf("Número de mostres per fer la mitjana(1..9)?\n");
								memset(cadena2, 0, strlen(cadena2)+1);
								scanf("%s",&cadena2[0]);
								t++;
							}
							while((isalpha(cadena2[0])!=0) || (cadena2[0]>='1' && cadena2[1]>='0') || (strlen(cadena2)>1));
								cadena2[1]='}';
								strncat(cadena, cadena1,2);
								strcat(cadena, cadena2);
									break;
						default:
							printf("Opció incorrecta\n");													//Donar error de paràmetres
							cadena2[1] = input1[0];
							strcat(cadena,cadena2);
								break;	
							}
							
							memset(buffer, 0, strlen(buffer));
							strcpy(buffer,cadena);															//Enviar al servidor la cadena
							result = write(sFd, buffer, strlen(buffer));
							printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer); 
							result = read(sFd, buffer, 256);       											//Rebre resposta del servidor
							printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);    
								break;
								
						case '2':																			//Selecció de mostra més antiga
							printf("Heu seleccionat l'opció {U}: Mostra més antiga\n"); 
							memset(buffer, 0, strlen(buffer));
							strcpy(buffer,"{U}");
							result = write(sFd, buffer, strlen(buffer));									//Enivar missatge al servidor
							printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer); 
							result = read(sFd, buffer, 256);												//Rebre resposta del servidor
							
							if (buffer[1]!='U' || buffer[2]!='0'){ 											//si la resposta del servidor no es la esperada s'indica al client i se li comunica al servidor.
								printf("ERROR PROTOCOL");													//es torna al menú (provar amb opcio 2 i extrendre a les altres posteriorment)
								strcpy(buffer,"ERROR PROTOCOL");
								result = write(sFd, buffer, strlen(buffer));
								goto menu;
							}
							
							printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
							buffer[strlen(buffer)-1]='\0';
							printf("La mostra més antiga és: %sºC",&buffer[3]);              
								break;
								
						case '3':																			//Selecció de mostrar el màxim
							printf("Heu seleccionat l'opció {X}: Demanar màxim\n");
							strcpy(buffer,"{X}");
							result = write(sFd, buffer, strlen(buffer));									//Enivar missatge al servidor
							printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);  
							result = read(sFd, buffer, 256);												//Rebre resposta del servidor
							printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
							buffer[strlen(buffer)-1]='\0';
							printf("El valor màxim de temperatura és: %sºC",&buffer[3]);
								break;
								
						case '4':																			//Selecció de mostrar el mínim
							printf("Heu seleccionat l'opció {Y}:Demanar mínim\n");
							strcpy(buffer,"{Y}");
							result = write(sFd, buffer, strlen(buffer));									//Enivar missatge al servidor
							printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);
							result = read(sFd, buffer, 256);												//Rebre resposta del servidor
							printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
							buffer[strlen(buffer)-1]='\0';
							printf("El valor mínim de temperatura és:%sºC",&buffer[3]);
								break;
								
						case '5':																			//Selecció de fer un Reset del màxim i mínim.
							printf("Heu seleccionat l'opció {R}:Reset màxim i mínim\n");
							strcpy(buffer,"{R}");
							result = write(sFd, buffer, strlen(buffer));									//Enivar missatge al servidor
							printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);
							result = read(sFd, buffer, 256);												//Rebre resposta del servidor
							printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
							printf("S'han resetejat els registres de màxim i mínim");
								break;
								
						case '6':																			//Selecció de mostrar el número de mostres dins l'array circular.
							printf("Heu seleccionat l'opció {B}:Demanar comptador mostres guardades\n");
							strcpy(buffer,"{B}");
							result = write(sFd, buffer, strlen(buffer));									//Enivar missatge al servidor
							printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);
							result = read(sFd, buffer, 256);												//Rebre resposta del servidor
							printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
							buffer[strlen(buffer)-1]='\0';
							printf("Tenim %s mostres",&buffer[3]);
								break;
						
						case 0x0a: 																			//Això és per enviar els 0x0a (line feed) que s'envia quan li donem al Enter
						
							break;
							
						default:
							printf("Opció incorrecta\n");	
							break;
					}
				}
			/*Tancar el socket*/
			close(sFd);			
	}
	else
	{
		j=1;
		/*Tancar el socket*/
		close(sFd);
	}
}
return 0;
}
/*Final programa*/
