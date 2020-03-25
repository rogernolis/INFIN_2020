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
	//int			mlen;
	int 		result, i=0,j=0,t=0;
	char		buffer[256];
	char  		cadena[256];
	char 		cadena2[256];
	char 		input1,input2;
	

		/*Crear el socket*/
		sFd=socket(AF_INET,SOCK_STREAM,0);
		
		/*Construir l'adreça*/
		sockAddrSize = sizeof(struct sockaddr_in);
		bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
		serverAddr.sin_family=AF_INET;
		serverAddr.sin_port=htons (SERVER_PORT_NUM);
		serverAddr.sin_addr.s_addr = inet_addr(serverName);
		/*Conexió*/
		result = connect (sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
		if (result < 0)
		{
			printf("Error en establir la connexió\n");
			exit(-1);
		}
		printf("\nConnexió establerta amb el servidor: adreça %s, port %d\n",	inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

		/*Enviar*/
		while(j==0){
			menu:
			ImprimirMenu();
			input1 = getchar();
			getchar();
			switch (input1){				//seleccionar les diferents opcions a enviar al servidor
				case '1':
					printf("Heu seleccionat l'opció {M}:Posar en marxa/parar adquisició\n");
					printf("Seleccioneu:\n");
					printf("0:Parada\n");
					printf("1:Marxa\n");
					input2 = getchar();
					switch (input2){
						case '0':
							cadena[0]='{';			//posar primers valors de la cadena :{M0
							cadena[1]='M';
							cadena[2]='0';
							printf("Heu seleccionat parar l'adquisició\n");
							/*do{			//es demana introduir els segons de mostreig, si aquests són erronis, s'indicara i es demnara que es tornin a introduir
								if(i!=0){
									printf("VALOR ERRONI\n");
								}
								printf("Quants segons de mostreig voleu(1..20)?\n");
								scanf("%s",&cadena[3]);
								i++;
								}
							while((cadena[3]=='2' && cadena[4]>'0')|| (cadena[3]>'2' && cadena[4]!= '\0') || (isalpha(cadena[3])!=0)); 
							do{
								if(t!=0){
									printf("VALOR ERRONI\n");
								}
								printf("Número de mostres fer la mitjana(1..9)?\n");
								scanf("%s",&cadena2[0]);
								t++;
							}
							while(isalpha(cadena2[0])!=0);*/
							cadena2[0]='}';
							strcat(cadena,cadena2);			//s'uneixen les dues cadenes per formar una de sola
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
								scanf("%s",&cadena[3]);
								i++;
							}while((cadena[3]=='2' && cadena[4]>'0')|| (cadena[3]>'2' && cadena[4]!= '\0') || (isalpha(cadena[3])!=0)); 
							do{
								if(t!=0){
									printf("VALOR ERRONI\n");
								}
								printf("Número de mostres fer la mitjana(1..9)?\n");
								scanf("%s",&cadena2[0]);
								t++;
							}while(isalpha(cadena2[0])!=0);
							cadena2[1]='}';
							strcat(cadena,cadena2);
							break;
						default:
							printf("Opció incorrecta\n");	
							printf("He llegit 0x%hhx \n",input1);
							goto menu;
							break;	
					}
					strcpy(buffer,cadena);					//enviar al servidor la cadena
					result = write(sFd, buffer, strlen(buffer));
					printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer); 
					result = read(sFd, buffer, 256);       	//rebre resposta servidor
					printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);                   
					break;
				case '2':
					printf("Heu seleccionat l'opció {U}:Mostra més antiga\n");
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
					break;
				case '3':
					printf("Heu seleccionat l'opció {X}:Demanar màxim\n");
					strcpy(buffer,"{X}");
					result = write(sFd, buffer, strlen(buffer));
					printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);  
					result = read(sFd, buffer, 256);
					printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);  
					break;
				case '4':
					printf("Heu seleccionat l'opció {Y}:Demanar mínim\n");
					strcpy(buffer,"{Y}");
					result = write(sFd, buffer, strlen(buffer));
					printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);
					result = read(sFd, buffer, 256);
					printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
					break;
				case '5':
					printf("Heu seleccionat l'opció {R}:Reset màxim i mínim\n");
					strcpy(buffer,"{R}");
					result = write(sFd, buffer, strlen(buffer));
					printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);
					result = read(sFd, buffer, 256);
					printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
					break;
				case '6':
					printf("Heu seleccionat l'opció {B}:Demanar comptador mostres guardades\n");
					strcpy(buffer,"{B}");
					result = write(sFd, buffer, strlen(buffer));
					printf("Missatge enviat a servidor(bytes %d): %s\n",result,buffer);
					result = read(sFd, buffer, 256);
					printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
					break;
				case 's':
					j=1;
					break;
				case 0x0a: //Això és per enviar els 0x0a (line feed) que s'envia quan li donem al Enter
				
					break;
				default:
					printf("Opció incorrecta\n");	
					printf("He llegit 0x%hhx \n",input1);
					goto menu;
					break;
			
			}
			
	}
	
	/*Tancar el socket*/
	close(sFd);
	printf("S'ha tancat sessió\n");
	return 0;
	
}
