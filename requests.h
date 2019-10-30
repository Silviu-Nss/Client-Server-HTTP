#ifndef _REQUESTS_
#define _REQUESTS_

char *compute_get_request(char *host, char *url, char *add, char *body, char *url_par);
char *compute_post_request(char *host, char *url, char *add, char *body);
char* get_ip(char* host);

#endif