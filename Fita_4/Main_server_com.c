
/*main.c  -  threads */
 
//DEFINICIONS
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>                                                        
#include <termios.h>       
#include <sys/ioctl.h> 

#define BAUDRATE B9600                                                
//#define MODEMDEVICE "/dev/ttyS0"      								//Conexió IGEP - Arduino
#define MODEMDEVICE "/dev/ttyACM0"      								//Conexió directa PC(Linux) - Arduino                                   
#define _POSIX_SOURCE 1 												// POSIX compliant source */                       
#define L_Array 100
#define TAM_MUESTRA 6
#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4
#define REQUEST_MSG_SIZE	1024

//DADES per variables Globals

int 	nscans=0;														// Nombre de mostres en l'array circular
int 	Temps;															// Variable global pel temps de les mostres
int 	l;																// Variables comunicació parametres longitud missatges
float 	maximo=00.00, minimo=70.00, temperatura;						// Variables glovals pelr les temperatures màximes i mínimes
char 	Entrada[10];													// Variable comunicació entre server i Comunicació_ArduinoC
char	codi_retorn[1];													// Variable global de comunicació ok/error protocol/error parametres arduino i server
char 	arrayCircular[L_Array][TAM_MUESTRA]={};							// Array circular gran de 100 mitjanes i 4 xifres significatives
int 	num_lectures=0;													// Numero lectures fetes per l'Arduino, inidferenment de les mitjanes 
int 	NumerodemostresM = 0;											// Numero de mostres per fer les mitjanes
float 	Arraymostres[10];  												// Mida array per fer les mitjanes
char 	missatge_enviat[255];											// Missatge que s'envia al Arduino
char 	missatge_rebut[255];											// Missatge que es rep del Arduino
int 	fd,F=0,res=0; 
pthread_mutex_t varmutex;										//Definicio variable mutex dels threads

// FUNCIONS                                                                                           
struct termios oldtio,newtio;                                            

int	ConfigurarSerie(void)												//Configuracio port rs232
{
	int fd;                                                           

	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
	
	if (fd <0) 
	{
		perror(MODEMDEVICE); exit(-1);
	}
	
	tcgetattr(fd,&oldtio); 												/* save current port settings */                 
	bzero(&newtio, sizeof(newtio));                                         
																		//newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;             
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;             
	newtio.c_iflag = IGNPAR;                                                
	newtio.c_oflag = 0;                                                     

	/* set input mode (non-canonical, no echo,...) */                       
	newtio.c_lflag = 0;                                                     
	newtio.c_cc[VTIME]    = 0;  										 /* inter-character timer unused */         
	newtio.c_cc[VMIN]     = 1;  										 /* blocking read until 1 chars received */ 
	tcflush(fd, TCIFLUSH);                                                  
	tcsetattr(fd,TCSANOW,&newtio);
	
 	sleep(2); 															//Per donar temps a que l'Arduino es recuperi del RESET
		
	return fd;
}               

void TancarSerie(int fd)												//Tancar port.
{
	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
}
void Enviar(void)														//Enviar dades. 
{
	res = write(fd,missatge_enviat,strlen(missatge_enviat));			//Esciure dades al port serie.
	
	if (res <0) 
	{
		tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); 	
	}
	
	if(missatge_enviat[1]!='S'){										// No mostrar els missatges d'encendre i apagar el led cada cop al agafar una mostra.
		printf("Enviats %d bytes: ",res);								// La resta de missatges s'ensenyen tots
										
		for (int F = 0; F < res; F++)
		{
			printf("%c",missatge_enviat[F]);							//Comprovació del missatge enviat.
		}
		printf("\n");
	}
}
void Lectura( int l)													//Llegir dades del port serie.
{
	res = read(fd,missatge_rebut,1); 
	for (int i = 1; i < l; i++)
	{
		res = res + read(fd,missatge_rebut+i,1);						//Càlcul de bytes per llegir.
	}
	if(missatge_rebut[1]!='S'){											// No mostrar els missatges d'encendre i apagar el led cada cop al agafar una mostra.
		printf("Rebuts %d bytes: ",res);								// La resta de missatges s'ensenyen tots
		
		for (int i = 0; i < res; i++)
		{
			printf("%c",missatge_rebut[i]);									//Mostar per pantalla el missatge rebut.
		}
		printf("\n");
	}
}
float map(int x, int in_min, int in_max, float out_min, float out_max)	//Funció para rescalado de datos. 
{
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Desplasament_llista() 												//Desplaçar una posició tots els elements de la array circular. 
{
	int i;
	i=0;
	for(i=0; i<(L_Array-1); i++) 
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
void Comunicacio_ControlSensor(float Temp) 								//Gestor array circular 
{
	int i=0;
    char valor_transportat[6];
        
        pthread_mutex_lock(&varmutex);
		sprintf(valor_transportat, "%.2f", Temp); 						//Convertir valor float de temperatura a char[]. PREPAREM LA MOSTRA AMB EL FORMAT DE L'ARRAY CIRCULAR. (char[])
		Desplasament_llista();					  						//Desplaçar del llistat una posició per deixar la primera posició buida i llesta per ser omplida per una nova mostra.
			
		//GUARDAR DADA [posició Y de la llista][byte= caràcter de la paraula]
		arrayCircular[0][0]=valor_transportat[0];			
		arrayCircular[0][1]=valor_transportat[1];
		arrayCircular[0][2]=valor_transportat[2];
		arrayCircular[0][3]=valor_transportat[3];
		arrayCircular[0][4]=valor_transportat[4];

		//Mostrar array actual
		printf("Array circular:\n"); 	
		for (i=0; i< L_Array; i++){ 									//Visualitzar array sencera, els valors vuits no es veuen si no posem una marca
			printf("%sºC  ",arrayCircular[i]);							
		}
		pthread_mutex_unlock(&varmutex);
		printf("\n");
}

void Desplasament_mitjana() 											//Desplaçar una posició tots els elements de la array de la mitjana.  
{
	int i;
	i=0;
	
	for(i=0; i<(NumerodemostresM-1); i++) 								//Recorregut de tots els elements. Quantitat d'elements de l'array = Numerodemostres.
	{
		Arraymostres[((NumerodemostresM-1)-i)]= Arraymostres[((NumerodemostresM-1)-(i+1))];
	}
}
void Mitjana(float Temperatura,int NumerodemostresM)					//Generació de valors per introduir a l'array circular. Valor= mitjana de Numerodemostres
{	
	int i=0;
	float Suma=0;
	float Media=0;
	
	Desplasament_mitjana();       								   	  	//Desplaçar valors per deixar el primer lliure.
	Arraymostres[0]=Temperatura;   									  	//Introducció d'un nou valor a la primera posició.
	
	if(NumerodemostresM <= num_lectures){							  	//Detectar array omplerta per fer mitjana. 
		for (i = 0; i < NumerodemostresM; i++)						  	//Suma de valors
		{
			Suma = Suma + Arraymostres[i];								//Suma dels valors de l'array
		}
		Media = (Suma/NumerodemostresM);								//Divisió pel nombre d'elements sumats.
		Comunicacio_ControlSensor(Media);								//Enviament del valor a l'array circular.
		printf("Valor media: %.2f ºC\n",Media);
		
	}	
	printf("-----------------------------------------------------------------------\n");
}		

void* Control_SensorC()     //més coneguda com Cum_Arduino  FITA 3 AQUEST és el thread!                                                       
{     
		
//Declaració de variables																																	
fd = ConfigurarSerie();													//Configuració del port sèrie.
int 	muestra;														//Variable de la mostra rebuda
char T1[10];															//variables per enviar el valor del temps al arduino
char T2[10];
int tiempo_arduino=0;													
long int i;
	
	//Fer bucle
	switch (Entrada[0]) {												//Es rep un missage del client amb protocol"{...}"
        case'M':														//Cas en que es vol posar en marxa/parada la maquina (nomes marxa en fita3).
            switch (Entrada[1]) {
				case '1':
								
					if (Temps == 1){									//corretgir l'error quan volem 1 segon de temps
						tiempo_arduino=1;
					}
					else{
						tiempo_arduino=Temps/2;
					}																
					int Dec=tiempo_arduino/10;							//Extreure decenes.
					int Uni=tiempo_arduino-(Dec*10);					//Extreure unitats.
					sprintf(T1,"%d",Dec);	
					sprintf(T2,"%d",Uni);
					
					//Missatge de marcha
					missatge_enviat[0]='A';
                    missatge_enviat[1]='M';
					missatge_enviat[2]='1';
					missatge_enviat[3]=T1[0];							//Incloure decenes al missatge d'enviament.
					missatge_enviat[4]=T2[0];							//Incloure unitats al missatge d'enviament.
					missatge_enviat[5]= 'Z';
					l=4;												//Nombre de bytes que s'esperen rebre del missatge de Marcha.
                    
                    //Enviar missatge        
					Enviar();																							
	
					sleep(1);											//Donar temps per enviar el missatge.
					
					//BUCLE PER BYTES DE LECTURA
					
					Lectura(l);											//Lectura del missatge de marcha. 
					codi_retorn[0] = missatge_rebut[2];
					
					//UN COP DINS DE MARCHA LLEGIR INDEFINIDAMENT
					for (i = 0;Entrada[1]=='1'; i++)								
					{
						//Enviar ordre de conversió
						sprintf(missatge_enviat,"ACZ");
						l=8;
						
						//Enviar
						Enviar();						
						
						//Recibir
						Lectura(l);
						codi_retorn[0] = missatge_rebut[2];
						
						//Convertidor graus
						char convert[5];
						convert [0]= missatge_rebut[3];
						convert [1]= missatge_rebut[4];
						convert [2]= missatge_rebut[5];
						convert [3]= missatge_rebut[6];
						convert [4]=0;
						muestra = atoi(convert);
						temperatura= map(muestra, 0,1023,0,110);		//Posem 110ºC per tenir tot el rang de l'entrada de 1,1V del arduino.
						printf("Temperatura: %.2fºC\n",temperatura);
						
						if (temperatura>maximo){						//Comparador max/min
							maximo=temperatura;
						}
						if (temperatura<minimo){
							minimo=temperatura;
						}
						// Comptador lectures fetes dins l'array
					
							num_lectures++;
																	
						//ORDRE ENCENDRE-APAGAR LED13
						sprintf(missatge_enviat,"AS131Z");				//Copiar missatge LED encés
						l=4;
						Enviar();										//Enviar missatge LED encés					
						Lectura(l);										//Rebre missatge LED encés
						codi_retorn[0] = missatge_rebut[2];
						sleep(0.5);										//Mantenir led encès 0,5 segons						
						sprintf(missatge_enviat,"AS130Z");				//Copiar missatge LED per apagar
						l=4;
						Enviar();										//Enviar missatge LED apagat
						Lectura(l);										//Rebre missatge LED apagat
						codi_retorn[0] = missatge_rebut[2];																	
						
						//Visualitzacions
						printf("Temperatura actual: %.2f ºC\n", temperatura);
						printf("Temperatura MAX: %.2f ºC\n", maximo);
						printf("Temperatura MIN: %.2f ºC\n",minimo);
						printf("Número de lectures: %d\n", num_lectures);
												
						Mitjana(temperatura,NumerodemostresM);			//Guardar dada. 			
						
						sleep(Temps*2-0.5);								//Esperar el doble del temps enviat a l'arduino menys el blink del led.	
					}
				}
                            				
				case '0':
					TancarSerie(fd);											//Tancar serie. 
					break;	
	}
	pthread_exit(NULL);
	}

int main(int argc, char *argv[])										//Main, part del servidor
{
	pthread_t thread;

    printf("Proces pare PID(%d) \n",getpid() );							//Proces Pare
  	
  	//Variables locals 
    struct 		sockaddr_in	serverAddr;
    struct 		sockaddr_in	clientAddr;
    unsigned int			sockAddrSize;
    int			sFd;
    int			newFd;
    int 		result;
    char		buffer[256];
    int         bufferlen=0;
    char 		tempsstr[2];
    char 		M_ANTIGA[6]; 
    char		Max[5];
    char		Min[5];
    
    /*Preparar l'adreça local*/
    sockAddrSize=sizeof(struct sockaddr_in);
    bzero ((char *)&serverAddr, sockAddrSize); 							//Posar l'estructura a zero
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT_NUM);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*Crear un socket*/
    sFd=socket(AF_INET, SOCK_STREAM, 0);

    /*Nominalitzar el socket*/
    result = bind(sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
	
    while(1){
		/*Crear una cua per les peticions de connexió*/
		result = listen(sFd, SERVER_MAX_CONNECTIONS);
		printf("\nServidor esperant connexions\n");
		/*Esperar conexió. sFd: socket pare, newFd: socket fill*/
	   
		//Esperar i acceptar client
		newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
		printf("Connexió acceptada del client: adreça %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			
		/*Rebre missatge client*/
        memset(buffer, 0, 256 );
        result = read(newFd, buffer, 256); 									//Mirar si ha llegado algun mensaje. Guardarlo en BUFFER.
        bufferlen = strlen(buffer);

       // printf("El caràcter que estem enviat del buffer 2 és\n");
        printf("\nMissatge rebut del client(bytes %d): %s\n",	result, buffer);

        if (buffer[0] == '{' && buffer[bufferlen-1] == '}'){
            switch (buffer[1]) {
                case 'M':
					switch (buffer[2]) {
                        case '0': 										//cas M0 ORDRE PARADA
                            
							//FUNCIONS DE LA ETAPA:
							//1() Comprobar si el sistema ja està en mode parada.
							//2()Si no ho està, donar l'ordre de posar mode Parada. 
							
							//Copiar missatge per Control_SensorC
							//Entrada = buffer[10];
							Entrada [0]= 'M';
							Entrada [1]= '0';				
									
							//Enviar comprovació a client.
							memset(buffer, 0, strlen(buffer)+1);
                            strcpy(buffer, "{M");
                            strcat(buffer, codi_retorn);
                            strcat(buffer, "}");
                            result = write(newFd, buffer, strlen(buffer)+1);
                            printf("Missatge enviat a client(bytes %d): %s\n", result, buffer);
                            printf("Adquisició finalitzada\n");    
                            break;
                            
                        case '1': 										//cas M1 ORDRE MARXA
							//FUNCIONS DE LA ETAPA:
							//1() Comprobar si el sistema ja està en mode Marxa.
							//2()Si no ho està, donar l'ordre de posar mode Marxa segons la configuració enviada. 
							
							Entrada [0]= 'M';
							Entrada [1]= '1';	

							if (buffer[3] == '0')
							{
								tempsstr[0] = buffer[4];
							}
							else
							{
								tempsstr[0] = buffer[3];							//Agafem les dades de temps
								tempsstr[1] = buffer[4];
							}
                            Temps = atoi(tempsstr);									//Aquí posem els valors de temps a tempstr en int
                            
                            NumerodemostresM = (buffer[5]- 48); 					//Agafem la dada del número de mostres. Operació Char to Int
									
                            printf("Número de mostres: %i\n", NumerodemostresM);	//*Comprovació/ENVIAMENT VIA SERIE A CONTROL S
                            
							//Obrir thread en cas de volguer consultar altres dades mentres es va analitzant.
							
							//Iniciar comunicació amb Control sensor	
                            pthread_create(&thread, NULL,Control_SensorC, NULL); 					//Es crea el thread fill 
                            
							//Enviar comprovació a client.
							memset(buffer, 0, strlen(buffer)+1);
                            strcpy(buffer,"{M");												//Retornar missatge amb codi de validació 0 -> OK
                            strcat(buffer, codi_retorn);
                            strcat(buffer, "}");
							//printf("ENVIAMENT: %s\n", buffer);
                            result = write(newFd, buffer,strlen(buffer) + 1); 
                            printf("Missatge enviat a client(bytes %d): %s\nEl temps és de: %d i el numero de mostres: %d \n", result, buffer, Temps, NumerodemostresM);//*
                               break;
                               
						default:
                           	memset(buffer, 0, strlen(buffer)+1);
                            strcpy(buffer, "{M2}");
							//printf("ENVIAMENT: %s\n", buffer);
                            result = write(newFd, buffer,strlen(buffer) + 1); 						// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                            printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
								break;
                        }
                        break;
                        
                case 'U': 												// Cas U DADA ANTIGA
					pthread_mutex_lock(&varmutex);						//bloquejem les variables globals
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
                    
                    result = write(newFd, buffer,strlen(buffer) + 1);							// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
						
					//Borrar mostra antiga per la seguent
					arrayCircular[nscans-1][0] = 0;
					arrayCircular[nscans-1][1] = 0;
					arrayCircular[nscans-1][2] = 0;
					arrayCircular[nscans-1][3] = 0;
					arrayCircular[nscans-1][4] = 0;
					
					if(nscans!= 0){
						nscans--;
					}
					pthread_mutex_unlock(&varmutex);						//desbloquejem les variables globals
					break;
						
                case 'X': 												//Cas X MAXIMA REGISTRE
					pthread_mutex_lock(&varmutex);						//bloquejem les variables globals
					sprintf(Max, "%.2f", maximo);
					pthread_mutex_unlock(&varmutex);						//desbloquejem les variables globals
					//1()Agafar dada màxima del registre. (MAXIMA)
					//2()Prepararla enviament a client
					memset(buffer, 0, strlen(buffer));
                    strcpy(buffer, "{X0");
					strcat(buffer, Max);
                    strcat(buffer, "}");
                    result = write(newFd, buffer, strlen(buffer) + 1);						// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
						break;
                        
                case 'Y': 												// Cas Y MINIMA REGISTRE
					pthread_mutex_lock(&varmutex);						//bloquejem les variables globals
					sprintf(Min, "%.2f", minimo);
					pthread_mutex_unlock(&varmutex);						//desbloquejem les variables globals

					//1()Agafar dada mínima del registre. (MINIMA)
					//2()Prepararla enviament a client
                    memset(buffer, 0, strlen(buffer));
					strcpy(buffer, "{Y0");
                    strcat(buffer, Min);
                    strcat(buffer, "}");
                    result = write(newFd, buffer, strlen(buffer) + 1);						// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
                    break;
                        
                case 'R': 												//Cas R RESET REGISTRES
					pthread_mutex_lock(&varmutex);						//bloquejem les variables globals
					maximo=00.00;										
					minimo=70.00;
					pthread_mutex_unlock(&varmutex);						//desbloquejem les variables globals
					//Comprovant
					printf("MAXIMA: %f\nMINIMA: %f\n", maximo, minimo);
					memset(buffer, 0, strlen(buffer));
					strcpy(buffer, "{R0");
					strcat(buffer, "}");
					result = write(newFd, buffer,strlen(buffer) + 1); 									// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
                    break;
                        
                case 'B': 												// Cas B NUMERO DE MOSTRES
                    memset(buffer, 0, strlen(buffer));
					strcpy(buffer, "{B0");								//Missatge de tot OK ja que els erros de Arduino no tenen a veure amb aquest cas
					char x[5];											//1()Agafar variable que conta el numero de mostres guardades (nscans)
					pthread_mutex_lock(&varmutex);						//bloquejem les variables globals
					sprintf(x,"%i", nscans);
					pthread_mutex_unlock(&varmutex);						//desbloquejem les variables globals
					
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
                    result = write(newFd, buffer, strlen(buffer) + 1);									// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
                    printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
                        break;
                        
		default:
					memset(buffer,0,strlen(buffer));
					strcpy(buffer, "{E2}");
					result=write(newFd, buffer, strlen(buffer)+1);
					printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);//*
            }

        } else if (buffer[0] != '{' || buffer[bufferlen-1] != '}'){
			memset(buffer, 0, strlen(buffer));
		    strcpy(buffer, "{E1}");
            result = write(newFd, buffer, strlen(buffer)+1);											// el +1 el posem per enviar el 0 al final de la cadena i aixi saber que és el final de la cadena
            printf("Missatge enviat a client (bytes %d): %s\n", result, buffer);
        }
        
        /*Tancar el socket fill*/
        result = close(newFd);
    }
    pthread_join(thread, NULL);

 	printf("Proces pare PID(%d)\n",getpid());
}
