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
	printf("\n\nMenu:\n");
	printf("Introduiu nombre per fer la selecció\n");
	printf("--------------------\n");
	printf("1:Posar en marxa/parar adquisició {M}\n");
	printf("2:Mostra més antiga {U}\n");
	printf("3:Demanar màxim {X}\n");
	printf("4:Demanar mínim {Y}\n");
	printf("5:Reset màxim i mínim {R}\n");
	printf("6:Demanar comptador mostres guardades {B}\n");
	printf("s:sortir\n");
	printf("--------------------\n");
}
/*programa*/
int main(int argc, char *argv[]){
	struct sockaddr_in	serverAddr;
	char	    serverName[] = "127.0.0.1"; //Adreça IP on est� el servidor
	int			sockAddrSize;
	int			sFd;
	int 		h=0;
	//int			mlen;
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
			printf("\n\n*****************************************************");
			ImprimirMenu();
			//scanf("%s",input1);
			//input1 = getchar();
			//getchar();
			
			do 
			{
				if(h>0)
				{
					printf("VALOR ERRONI\n");
				}
				h++;
				memset(input1, 0, strlen(input1));
				scanf("%s",input1);
			}
			while (((isalpha(input1[0]) > 0) && input1[0] != 's' ) || input1[0]=='0' || (input1[0] > '6' && input1[0] != 's') || strlen(input1) > 1);				h=0;
				if (input1[0] != 's')
				{
					/*Conexió*/
					/*Crear el socket*/
					sFd=socket(AF_INET,SOCK_STREAM,0);
			
					/*Construir l'adreça*/
					sockAddrSize = sizeof(struct sockaddr_in);
					bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
					serverAddr.sin_family=AF_INET;
					serverAddr.sin_port=htons (SERVER_PORT_NUM);
					serverAddr.sin_addr.s_addr = inet_addr(serverName);
					result = connect (sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
				
				if (result < 0)
				{
					printf("Error en establir la connexió\n");
					//exit(-1);
				}
						else
						{
							printf("\nConnexió establerta amb el servidor: adreça %s, port %d\n",	inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
							switch (input1[0]){				//seleccionar les diferents opcions a enviar al servidor
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
										scanf("%s",input2);
									}
									while ((isalpha(input2[0])>0) || (input2[0]>'1')|| strlen(input2) > 1);
									
									switch (input2[0]){
										
										case '0':
											cadena[0]='{';			
											cadena[1]='M';
											cadena[2]='0';
											cadena[3]='}';
											printf("Heu seleccionat parar l'adquisició\n");
												break;		
										case '1':
											cadena[0]='{';
											cadena[1]='M';
											cadena[2]='1';
											printf("Heu seleccionat posada en marxa\n");
											do{
												if(i!=0){
													printf("VALOR ERRONI\n");
												}
												printf("Quants segons de mostreig voleu(1..20)?\n");
												memset(cadena1, 0, strlen(cadena1)+1);
												scanf("%s",&cadena1[0]);
												i++;
											}
											while((cadena1[0]>='2' && cadena1[1]>'0')|| (strlen(cadena1)>2) || (cadena1[0]> '2' && cadena1[1]>='0') ||(isalpha(cadena1[0])!=0)); 
												if(cadena1[1]=='\0')
												{
													//printf("una xifra\n");
													cadena1[1]=cadena1[0];
													cadena1[0]='0';
											}
											do{
												if(t!=0){
													printf("VALOR ERRONI\n");
												}
												printf("Número de mostres fer la mitjana(1..9)?\n");
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
											printf("Opció incorrecta\n");	
											printf("He llegit 0x%hhx \n",input1[0]);
											cadena2[1] = input1[0];
											strcat(cadena,cadena2);
												break;	
									}
									memset(buffer, 0, strlen(buffer));
									strcpy(buffer,cadena);					//enviar al servidor la cadena
									result = write(sFd, buffer, strlen(buffer));
									printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer); 
									result = read(sFd, buffer, 256);       	//rebre resposta servidor
									printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
									printf("L'adquisició esta en marxa");           
										break;
								case '2':
									printf("Heu seleccionat l'opció {U}:Mostra més antiga\n");
									memset(buffer, 0, strlen(buffer));
									strcpy(buffer,"{U}");
									result = write(sFd, buffer, strlen(buffer));
									printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer); 
									result = read(sFd, buffer, 256);
									if (buffer[1]!='U' || buffer[2]!='0'){ 	//si la resposta del servidor no es la esperada s'indica al client i se li comunica al servidor,i es torna al menú(provar amb opcio 2 i extrendre a les altres posteriorment)
										printf("ERROR PROTOCOL");
										strcpy(buffer,"ERROR PROTOCOL");
										result = write(sFd, buffer, strlen(buffer));
										goto menu;
									}
									printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
									buffer[strlen(buffer)-1]='\0';
									printf("La mostra més antiga és:%s",&buffer[3]);              
										break;
								case '3':
									printf("Heu seleccionat l'opció {X}:Demanar màxim\n");
									strcpy(buffer,"{X}");
									result = write(sFd, buffer, strlen(buffer));
									printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);  
									result = read(sFd, buffer, 256);
									printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer); 
									buffer[strlen(buffer)-1]='\0';
									printf("La mostra més antiga és:%s",&buffer[3]);
									break;
								case '4':
									printf("Heu seleccionat l'opció {Y}:Demanar mínim\n");
									strcpy(buffer,"{Y}");
									result = write(sFd, buffer, strlen(buffer));
									printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);
									result = read(sFd, buffer, 256);
									printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
									buffer[strlen(buffer)-1]='\0';
									printf("La mostra més antiga és:%s",&buffer[3]);
									break;
								case '5':
									printf("Heu seleccionat l'opció {R}:Reset màxim i mínim\n");
									strcpy(buffer,"{R}");
									result = write(sFd, buffer, strlen(buffer));
									printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);
									result = read(sFd, buffer, 256);
									printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
									printf("S'han resetejat els registres de màxim i mínim");
									break;
								case '6':
									printf("Heu seleccionat l'opció {B}:Demanar comptador mostres guardades\n");
									strcpy(buffer,"{B}");
									result = write(sFd, buffer, strlen(buffer));
									printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);
									result = read(sFd, buffer, 256);
									printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
									buffer[strlen(buffer)-1]='\0';
									printf("La mostra més antiga és:%s",&buffer[3]);
										break;
								
								case 0x0a: //Això és per enviar els 0x0a (line feed) que s'envia quan li donem al Enter
								
									break;
								default:
									printf("Opció incorrecta\n");	
									printf("He llegit 0x%hhx \n",input1[0]);
									//goto menu;
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
