#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *add, char *body, char *url_params)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *linie = calloc(LINELEN, sizeof(char));

    if (url_params != NULL)
    {
        sprintf(linie, "GET %s?%s HTTP/1.1", url, url_params);
    }
    else
    {
        sprintf(linie, "GET %s HTTP/1.1", url);
    }
    compute_message(message, linie);


    sprintf(linie, "Host: %s", host);
    compute_message(message, linie);

    if(strlen(add) > 0)
        compute_message(message, add);

    if(strlen(body) > 0)
        compute_message(message, body);

    compute_message(message, "");

    free(linie);
    return message;
}

char *compute_post_request(char *host, char *url, char *add, char *body)
{

    char *message = calloc(BUFLEN, sizeof(char));
    char *linie = calloc(LINELEN, sizeof(char));


    sprintf(linie, "POST %s HTTP/1.1", url);
    compute_message(message, linie);


    sprintf(linie, "Host: %s", host);
    compute_message(message, linie);

    if(strlen(add) > 0)
        compute_message(message, add);

    if(strlen(body) > 0)
        strcat(message, body);

    else
        compute_message(message, "");

    free(linie);
    return message;
}

char* get_ip(char* host)
{
    struct addrinfo hints, *result;
    int errcode;
    char addrstr[100], *p, *name;
    void *ptr;
    name = strdup(host);

    // Luam numele domeniului caruia dorim sa ii aflam IP-ul
    p = strtok(name, "/");
    bzero(&hints, sizeof(hints));
    hints.ai_family= PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo(p, NULL, &hints, &result);
    if(errcode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        exit(-1);
    }
    ptr = &((struct sockaddr_in *)result->ai_addr)->sin_addr;
    inet_ntop(result->ai_family, ptr, addrstr, 100);

    return strdup(addrstr);
}