#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include "tcp_socket.h"

char *session_username[CLIENT_MAX+5];

/*
@function: send message to client and print it
@parameter [IN] message: message to send
           [IN] connfd: socket connects to client
@return: true if success
         false if fail
*/
bool send_msg_to_client(char *message, int connfd) {
  char send_data[MSG_SIZE];
  int sent_bytes;
  unsigned int msglen = (unsigned int) strlen(message);
  if (msglen > MSG_SIZE - DELIMITER_SIZE) {
    fprintf(stderr, "send_msg(): message length must be less than %d character\n", MSG_SIZE - DELIMITER_SIZE);
    return false;
  }
  strcpy(send_data, message);
  strcat(send_data, DELIMITER);
  // memmove(send_data, message, msglen);
  // memmove(send_data + msglen, DELIMITER, DELIMITER_SIZE);
  sent_bytes = send(connfd, send_data, msglen + DELIMITER_LEN, 0);
  if(sent_bytes < 0) {
    perror("\nsend()");
    close(connfd);
    return false;
  }
  if (session_username[connfd] == NULL) {
    printf("[%d:]-> %s\n", connfd, message);
  } else printf("[%d:%s]-> %s\n", connfd, session_username[connfd], message);
  return true;
}
/*
@function: receive 1 message from client - connfd and print it
@parameter [IN] connfd: socket connects to client
           [IN] recv_buffer: save remaining data after receiving message
          [OUT] message: save message have been received
@return: 0 if success
         1 if success and unprocessed information will be put into recv_buffer
        -1 have error
        -2 Connection closed
        -3 Message overlength
*/
int get_msg_from_client(int connfd, char *message, char *recv_buffer) {
	char recv_data[MSG_SIZE], recv_msg[MSG_SIZE];
  int received_bytes;
  recv_data[0] = '\0';
  memset(recv_msg, 0, MSG_SIZE);

  do {
    if (recv_buffer[0] != '\0') { // get residual messages from recv_buffer
      memmove(recv_data, recv_buffer, strlen(recv_buffer) + 1);
      received_bytes = strlen(recv_buffer);
      recv_buffer[0] = '\0';
    } else {
      received_bytes = recv(connfd, recv_data, MSG_SIZE - 1, 0);
      if (received_bytes <= 0) {
        if (received_bytes == 0 || errno == ECONNRESET) return -2; // connection closed
        else {
          perror("\nrecv()");
          return -1;
        }
      }
    }
    recv_data[received_bytes] = '\0';
    char *ptr = strstr(recv_data, DELIMITER);
    char *optr = recv_data;
    while (1) {
      if (ptr == NULL) {
        if ( strlen(recv_msg) + strlen(optr) > MSG_SIZE - 1) return -3; // message overlength
        memmove(recv_msg + strlen(recv_msg), optr, strlen(optr) );
        break;
      } else {
        if ( strlen(recv_msg) + ptr - optr > MSG_SIZE - 1) return -3;
        memmove(recv_msg + strlen(recv_msg), optr, ptr - optr);
        memmove(message, recv_msg, strlen(recv_msg) + 1 );
        if (session_username[connfd] == NULL) {
          printf("[%d:]: %s\n", connfd, message);
        } else printf("[%d:%s]: %s\n", connfd, session_username[connfd], message);
        optr = ptr + DELIMITER_LEN;
        if (*optr != '\0') {
          memmove(recv_buffer, optr, strlen(optr) + 1);
          return 1;
        } else return 0;
      }
    }
  } while (received_bytes > 0);
  return -1;
}
//______________________________________________________________________________
/*
@function: send message to server
@parameter [IN] message: message to send
           [IN] connfd: socket connects to server
@return: true if success
         false if fail
*/
bool send_msg_to_server(char *message, int connfd) {
  char send_data[MSG_SIZE];
  int sent_bytes;
  unsigned int msglen = (unsigned int) strlen(message);
  if (msglen > MSG_SIZE - DELIMITER_SIZE) {
    fprintf(stderr, "send_msg(): message length must be less than %d character\n", MSG_SIZE - DELIMITER_SIZE);
    return false;
  }
  memmove(send_data, message, msglen);
  memmove(send_data + msglen, DELIMITER, DELIMITER_SIZE);
  sent_bytes = send(connfd, send_data, msglen + DELIMITER_LEN, 0);
  if(sent_bytes < 0) {
    perror("\nsend()");
    close(connfd);
    return false;
  }
  return true;
}
/*
@function: receive 1 message from server - connfd
@parameter [IN] connfd: socket connects to server
           [IN] recv_buffer: save remaining data after receiving message
          [OUT] message: save message have been received
@return: 0 if success
         1 if success and unprocessed information will be put into recv_buffer
        -1 have error
        -2 Connection closed
        -3 Message overlength
*/
int get_msg_from_server(int connfd, char *message, char *recv_buffer) {
	char recv_data[MSG_SIZE], recv_msg[MSG_SIZE];
  int received_bytes;
  recv_data[0] = '\0';
  memset(recv_msg, 0, MSG_SIZE);

  do {
    if (recv_buffer[0] != '\0') { // get residual messages from recv_buffer
      memmove(recv_data, recv_buffer, strlen(recv_buffer) + 1);
      received_bytes = strlen(recv_buffer);
      recv_buffer[0] = '\0';
    } else {
      received_bytes = recv(connfd, recv_data, MSG_SIZE - 1, 0);
      if (received_bytes <= 0) {
        if (received_bytes == 0 || errno == ECONNRESET) return -2; // connection closed
        else {
          perror("\nrecv()");
          return -1;
        }
      }
    }
    recv_data[received_bytes] = '\0';
    char *ptr = strstr(recv_data, DELIMITER);
    char *optr = recv_data;
    while (1) {
      if (ptr == NULL) {
        if ( strlen(recv_msg) + strlen(optr) > MSG_SIZE - 1) return -3; // message overlength
        memmove(recv_msg + strlen(recv_msg), optr, strlen(optr) );
        break;
      } else {
        if ( strlen(recv_msg) + ptr - optr > MSG_SIZE - 1) return -3;
        memmove(recv_msg + strlen(recv_msg), optr, ptr - optr);
        memmove(message, recv_msg, strlen(recv_msg) + 1 );
        optr = ptr + DELIMITER_LEN;
        if (*optr != '\0') {
          memmove(recv_buffer, optr, strlen(optr) + 1);
          return 1;
        } else return 0;
      }
    }
  } while (received_bytes > 0);
  return -1;
}
// Return a string describing the meaning of the server's message code
char *msg_str(char *recv_msg) {
  char *ptr;
  int code = strtol(recv_msg, &ptr, 10);
  if (ptr != recv_msg + 3) return NULL;
  switch (code) {
    case 100: return M100; break;
    case 101: return M101; break;
    case 102: return M102; break;
    case 103: return M103; break;

    case 200: return M200; break;
    case 201: return M201; break;
    case 202: return M202; break;
    case 203: return M203; break;
    case 204: return M204; break;
    case 205: return M205; break;
    case 206: return M206; break;
    case 207: return M207; break;
    case 208: return M208; break;
    case 209: return M209; break;
    
    case 300: return M300; break;
    case 301: return M301; break;
    case 302: return M302; break;
    case 303: return M303; break;
    case 304: return M304; break;
    case 305: return M305; break;
    case 306: return M306; break;
    case 307: return M307; break;

    case 400: return M400; break;
    case 401: return M401; break;
    case 402: return M402; break;
    case 403: return M403; break;
    case 404: return M404; break;

    case 500: return M500; break;
    default: return NULL;
  }
}
// Print message by receive message code
bool print_msg(char *recv_msg) {
  char *message = msg_str(recv_msg);
  if (message == NULL) {
    printf("Can not determine meaning of this message: %s\n", recv_msg);
    return false;
  }
  if (message[0] == '4')
    printf("!! %s !!\n", message);
  else printf("%s\n", message);
  return true;
}
