//Nastasescu George-Silviu, 321CB

#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

void get_cookies(char* response, char* add, char* linie)
{
	char *aux = strdup(response);

	// Luam primul cookie
	strtok(response, "\n");
	strtok(NULL, "\n");
	char *p1 = strtok(NULL, ";");
	sprintf(linie, "%s; ", p1 + 4);
	strcat(add, linie);

	// Luam al doilea cookie
	strtok(aux, "\n");
	strtok(NULL, "\n");
	strtok(NULL, "\n");
	strtok(NULL, ":");
	char *p2 = strtok(NULL, ";");
	sprintf(linie, "%s", p2);
	compute_message(add, linie);
	
	free(aux);
}

int main()
{
	int i, nr, sockfd, new_sockfd;
	char add[LINELEN] = "";		// Partea aditionala a header-elor trimise
	char body[LINELEN] = "";	// Body-ul mesajelor trimise
	char *message, *response, url[100], type[30], method[7];
	char jwt[LINELEN] = "", linie[LINELEN] = "", url_par[50] = "";
	char new_url[100], new_method[5], *p, *ip, *resp;
	JSON_Value *root_value;
	JSON_Object *commit, *obj, *query;

	strcpy(url, "/task1/start");
	strcpy(method, "GET");

	for(i = 1; i <= 5; i++) {
		printf("\n-----------------------TASK %d-----------------------\n", i);

		// deschidem conexiunea cu server-ul
		sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM);
		if(strcmp(method, "GET") == 0)
			message = compute_get_request(IP_SERVER, url, add, body, url_par);
		else
			message = compute_post_request(IP_SERVER, url, add, body);

		bzero(add, LINELEN);
		bzero(body, LINELEN);
		bzero(url_par, sizeof(url_par));
		send_to_server(sockfd, message);
		response = receive_from_server(sockfd);
		close_connection(sockfd);
		free(message);

		if(i != 5) {
			// Luam string-ul in format json din body-ul raspunsului
			p = strchr(response, '{');
			printf("\n%s\n", p);
			root_value = json_parse_string(p);
			commit = json_value_get_object(root_value);

			// Obtinem url-ul, metoda si cookie-urile folosite
			strcpy(url, json_object_get_string(commit, "url"));
			strcpy(method, json_object_get_string(commit, "method"));
			get_cookies(response, add, linie);
		}
		else
			printf("\n%s\n", response);

		switch(i) {
			case 1:
				// Preluam tipul continutului, username-ul si parola
				// dupa care le punem in header-ul cererii in formatul dorit
				strcpy(type, json_object_get_string(commit, "type"));
				obj = json_object_get_object(commit, "data");
				sprintf(linie, "username=%s&password=%s",
						json_object_get_string(obj, "username"),
						json_object_get_string(obj, "password"));
				nr = strlen(linie);
				compute_message(body, linie);
				sprintf(linie, "Content-Type: %s", type);
				compute_message(add, linie);
				sprintf(linie, "Content-Length: %d", nr);
				compute_message(add, linie);
				break;

			case 2:
				// Preluam id-ul si JWT-ul dupa care le punem in
				// header-ul cererii in formatul corespunzator
				obj = json_object_get_object(commit, "data");
				query = json_object_get_object(obj,"queryParams");
				sprintf(url_par, "raspuns1=omul&raspuns2=numele&id=%s",
						json_object_get_string(query, "id"));
				sprintf(jwt, "Authorization: Bearer %s",
						json_object_get_string(obj, "token"));
				compute_message(add, jwt);
				break;

			case 3:
				// Adaugam token-ul in header
				compute_message(add, jwt);
				break;

			case 4:
				// Adaugam token-ul in header
				compute_message(add, jwt);

				// Obtinem tipul continutului
				strcpy(type, json_object_get_string(commit, "type"));

				// Preluam url-ul, metoda si parametrii pentru a obtine vremea
				obj = json_object_get_object(commit, "data");
				query = json_object_get_object(obj,"queryParams");
				strcpy(new_url, json_object_get_string(obj, "url"));
				strcpy(new_method, json_object_get_string(obj, "method"));
				sprintf(url_par, "q=%s&APPID=%s",
						json_object_get_string(query, "q"),
						json_object_get_string(query, "APPID"));

				// Obtinem IP-ul folosind o cerere DNS
				ip = get_ip(new_url);

				// Luam file path-ul din URL
				strtok(new_url, "/");
				p = strtok(NULL, "\r\n");
				sprintf(new_url, "/%s", p);

				// Trimitem cererea pentru obtinerea vremii
				new_sockfd = open_connection(ip, 80, AF_INET, SOCK_STREAM);
				if(strcmp(new_method, "GET") == 0)
					message = compute_get_request(ip, new_url, "", "",url_par);
				else
					message = compute_post_request(ip, new_url, "", "");
				bzero(url_par, sizeof(url_par));
				send_to_server(new_sockfd, message);
				resp = receive_from_server(new_sockfd);
				close_connection(new_sockfd);
				free(message);
				free(ip);

				// Formatam componentele cererii pentru a trimite server-ului
				// raspunsul primit cu vremea din Bucuresti
				p = strchr(resp, '{');
				sprintf(body, "%s\r\n", p);
				sprintf(linie, "Content-Type: %s", type);
				compute_message(add, linie);
				sprintf(linie, "Content-Length: %lu", strlen(p) + 1);
				compute_message(add, linie);
				break;

			case 5:
				free(resp);
				break;		
			}

		free(response);
	}

	return 0;
}