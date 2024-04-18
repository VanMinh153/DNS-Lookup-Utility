#ifndef RESOLVER_H
#define RESOLVER_H

int get_ip_address(const char *domain, struct addrinfo **list_ip);
int get_domain_name(const char *ip, char **domain);

#endif // RESOLVER_H
