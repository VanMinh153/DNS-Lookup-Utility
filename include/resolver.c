#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <netdb.h>
#include <arpa/inet.h>
// #include <resolver.h>

/*
@function getIPAdress: this function is used to request IPv4 address using getaddrinfo()
@param [IN] domain: Domain name
      [OUT] list_ip: Pointer to linked-list containing host's IP information
@return: 0 if success
         or nonzero error codes from getaddrinfo() if getaddrinfo() has failed
*/
int get_ip_address(const char *domain, struct addrinfo **list_ip) {
  struct addrinfo hints;
  struct addrinfo *result = NULL;
  struct sockaddr_in *ptrAddr = NULL;
  int retval = -1;
  memset(&hints, 0, sizeof(struct addrinfo) );
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  // get IPv4 address by input domain name
  retval = getaddrinfo(domain, NULL, &hints, &result);
  if (retval != 0) return retval; // getaddrinfo() has failed

  *list_ip = result;
  return 0;
}
/*
@function getDomainName: this function is used to request domain name using getnameinfo()
@param [IN] ip: IPv4 address
      [OUT] domain: Pointer to Domain name if success
@return: 0 if success
         1 if input is invalid IPv4 address
         or nonzero error codes from getnameinfo() if getnameinfo() has failed
*/
int get_domain_name(const char *ip, char **domain) {
  //convert IP address from dots-and-number string to a struct in_addr
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  int retval = -1;
  retval = inet_pton(AF_INET, ip, &address.sin_addr);
  if (retval == 0) return 1;     // Invalid IPv4 address

  // search IP information
  char *hostname = (char*) malloc(NI_MAXHOST);
  char servInfo[NI_MAXSERV];
  retval = getnameinfo((struct sockaddr *) &address, sizeof(struct sockaddr),
                       hostname, NI_MAXHOST, servInfo, NI_MAXSERV, 0);
  if (retval != 0) {
    free(hostname);
    return retval; // getnameinfo() has failed
  }
  
  *domain = hostname;
  return 0;
}

// /*
// @function resolver: translate request for Domain name into IPv4 address or IPv4 address to Domain name
// @param [IN] host: Domain name or IPv4 address
//       [OUT] info: Pointer to Domain name or linked-list containing host's IP information
// @return: 0 if success to get IPv4 address from Domain name
//          1 if success to get Domain name from IPv4 address
//          2 if input is invalid IPv4 address
//          or nonzero error codes from getaddrinfo()/getnameinfo() if getaddrinfo()/getnameinfo() has failed
// */
// int resolver(const char *domain_ip, void **info) {
//   bool hasAlpha = false;
//   int retval = -1;

//   for (int i = 0; domain_ip[i] != '\0'; i++)
//     if (isalpha(domain_ip[i]) ) {
//       hasAlpha = true;
//       break;
//     }

//   if (hasAlpha) {
//     struct addrinfo *ipaddr = NULL;
//     retval = get_ip_address(domain_ip, &ipaddr);
//     if (retval == 0) {
//       *info = ipaddr;
//       return 0;
//     }
//     return retval;
//   } else {
//     char *domain = NULL;
//     retval = get_domain_name(domain_ip, &domain);
//     if (retval == 0) {
//       *info = domain;
//       return 1;
//     }
//     if (retval == 1) return 2;
//     // if (strlen(domain) == 0 || strcmp(domain,ip) == 0) {
//     //   free(domain);
//     //   return -2; // Not found information
//     // }
//     return retval;
//   }
//   return -1;
// }
//________________________________________________________________
// int main() {
//   bool hasAlpha = false;
//   int retval = -1;
//   char *host = "9s8dsd8fs.m";
//   void *info = NULL;

//   for (int i = 0; host[i] != '\0'; i++)
//     if (isalpha(host[i]) ) {
//       hasAlpha = true;
//       break;
//     }

//   if (hasAlpha) {
//     struct addrinfo *ipaddr = NULL;
//     retval = get_ip_address(host, &ipaddr);
//   } else {
//     char *domain = NULL;
//     retval = get_domain_name(host, &domain);
//   }
//   printf("retval = %d\n", retval);

//     return 0;
//   }