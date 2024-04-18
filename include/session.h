#include <stdbool.h>
#include <arpa/inet.h>
#include "tcp_socket.h"
#ifndef SESSION_H
#define SESSION_H

struct account_t {
  char username[USERNAME_LEN+1];
  char password[PASSWORD_LEN+1];
};
typedef struct account_t account_t;

struct session_t {
  char username[USERNAME_LEN+1];
  bool logged;
  int connfd;
  char recv_buffer[MSG_SIZE];
  struct session_t *prev;
  struct session_t *next;
};
typedef struct session_t session_t;

extern unsigned int numSession;
extern session_t *sess_head;
extern session_t *sess_tail;

session_t *add_session(session_t *data);

void delete_session(session_t *node);

#endif // SESSION_H
