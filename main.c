#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>

#include "word.h"
#include "tcp.h"


void printDay(char* city, weatherInfo* info, WT* wt_info, bool single)
{
	printf("City: %s  Today is: %d/%d/%d  Weather information is as follows:\n", city, ntohs(info->year), info->month, info->day);
	int num = info->num;
	int i = 1;
	if(single == true)
		i = num;
	for(; i<=num; i++)
	{
		char* weather;
		if(wt_info->weather == 0x00)
			weather = "shower";
		else if(wt_info->weather == 0x01)
			weather = "clear";
		else if(wt_info->weather == 0x02)
			weather = "cloudy";
		else if(wt_info->weather == 0x03)
			weather = "rain";
		else if(wt_info->weather == 0x04)
			weather = "fog";

		printf("The %dth day's Weather is: %s;  Temp:%d\n", i, weather, wt_info->temperature);
		wt_info++;
	}
}

int main()
{
	int state = 0;
	int sockfd;
	struct sockaddr_in servaddr;
	char sendline[MAXLINE], recvline[MAXLINE];
	char current_city[20];	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(SERV_IP);
	servaddr.sin_port = htons(SERV_PORT);
	
	connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	system("clear");
	printf("%s",welcome);

	while(fgets(sendline, MAXLINE, stdin) != NULL)
	{
		int i = strlen(sendline);
		if(sendline[i-1]=='\n')
			sendline[i-1] = '\0';
		if(strlen(sendline) > 0)
		{
			if(strcmp(sendline, "#")==0)
			{
				close(sockfd);
				exit(0);
			}
			char message[23];
			//State for inputing city name
			if(state == 0)
			{
				if(strcmp(sendline, "c")==0)
				{
					system("clear");
					printf("%s",welcome);
					continue;
				}
				message[0] = 0x01;
				message[1] = 0x00;
				int j = 2;
				for(j=2; j<i+2; j++)
					message[j] = sendline[j-2];
				for(;j<22;j++)
					message[j] = 0x00;

				send(sockfd, message, 23, 0);
				if(recv(sockfd, recvline, MAXLINE, 0) == 0)
				{
					perror("The server terminated prematurely");
					exit(4);
				}

				if(recvline[0] == 0x02)	//no weather information
				{
					printf(sorry_message, recvline+2);
					printf(welcome);
				}
				else if(recvline[0] == 0x01) //find weather information
				{
					state = 1;
					strcpy(current_city, (const char*)(recvline+2));
					system("clear");
					printf(city_message);
				}
			}

			else if(state == 1)
			{
				if(strcmp(sendline, "r") == 0)
				{
					state = 0;
					system("clear");
					printf(welcome);
					continue;
				}
				else if(strcmp(sendline, "c") == 0)
				{
					system("clear");
					printf(city_message);
				}
				else if(strcmp(sendline, "1") == 0 || strcmp(sendline, "2") == 0)
				{
					int temp_l = strlen(current_city);
					message[0] = 0x02;
					if(strcmp(sendline, "1") == 0)
					{
						message[1] = 0x01;
						message[22] = 0x01;
					}
					else
					{
						message[1] = 0x02;
						message[22] = 0x03;
					}
					int j = 2;
					for(; j<temp_l+2; j++)
						message[j] = current_city[j-2];
					for(; j<22; j++)
						message[j] = 0x00;
					send(sockfd, message, 23,0);
					if(recv(sockfd, recvline, MAXLINE, 0) == 0)
					{	
						perror("The server terminated prematurely");
						exit(4);
					}

					weatherInfo* info;
					info = (weatherInfo*)malloc(sizeof(weatherInfo));
					char* city_name = recvline+2;
					char* temp = city_name + strlen(city_name);
					while(*temp == 0x00)
						temp++;
					memcpy(info, temp, sizeof(weatherInfo));
					temp += sizeof(weatherInfo);
					WT* wt;
					wt = (WT*)malloc(sizeof(WT)*info->num);
					memcpy(wt, temp, sizeof(WT)*info->num);
					
					printDay(current_city, info, wt, false);
				}
				else if(strcmp(sendline, "3") == 0)
				{
					printf("Please enter the day number(below 10, e.g. 1 means today):");
					char number[100];
					while(1)
					{
						fgets(number, 100, stdin);
						if(strlen(number)!=2 || number[0]<'0' || number[0]>'9')
						{
							printf("Input error\n");
							printf("Please enter the day number(below 10, e.g. 1 means today):");
						}
						else
						{
							message[0] = 0x02;
							message[1] = 0x01;
							message[22] = number[0] - '0';
							int temp_l = strlen(current_city);
							int j = 2;
							for(; j<temp_l+2; j++)
								message[j] = current_city[j-2];
							send(sockfd, message, 23,0);
							if(recv(sockfd, recvline, MAXLINE, 0) == 0)
							{	
								perror("The server terminated prematurely");
								exit(4);
							}

							weatherInfo* info;
							info = (weatherInfo*)malloc(sizeof(weatherInfo));
							char* city_name = recvline+2;
							char* temp = city_name + strlen(city_name);
							while(*temp == 0x00)
							temp++;
							memcpy(info, temp, sizeof(weatherInfo));
							temp += sizeof(weatherInfo);
							WT* wt;
							wt = (WT*)malloc(sizeof(WT)*info->num);
							memcpy(wt, temp, sizeof(WT)*info->num);
							if(recvline[0] == 0x04)
								printf("Sorry, no given day's weather information for city %s!\n",current_city);
							else
								printDay(current_city, info, wt, true);

							break;

						}
					}


				}

				else
				{
					printf("input error!\n");
					continue;
				}
			}
	
		}
		memset(sendline, 0, MAXLINE);
		memset(recvline, 0, MAXLINE);
		
	}

	return 0;
}
