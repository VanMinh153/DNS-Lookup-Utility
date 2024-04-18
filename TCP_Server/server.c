#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <poll.h>
#include "../include/utility.h"
#include "../include/tcp_socket.h"
#include "../include/session.h"
#include "../include/resolver.h"

#define TIME_OUT 3       // 3ms, timeout when call poll() function
#define send_f send_msg_to_client
#define get_f get_msg_from_client

account_t *accounts = NULL;
unsigned int num_account = 0;

int PORT;
extern char *session_username[CLIENT_MAX+5];
int handle_msg(char *msg, session_t *session);
int read_account();

int main(int argc, char **argv) {
  accounts = (account_t*) malloc(ACCOUNT_MAX*sizeof(account_t));
  memset(session_username, 0, sizeof(session_username));
  int listenfd, connfd;
  struct sockaddr_in server, client;
  char recv_msg[MSG_SIZE];
  recv_msg[0] = '\0';
  socklen_t sin_size = sizeof(struct sockaddr);
  int retval = -1;

  memset(&server, 0, sizeof(server));
  memset(&client, 0, sizeof(client));
  memset(&recv_msg, 0, sizeof(recv_msg));

  if (argc != 2) {
    fprintf(stderr, "Please write: ./server <Port_Number>\n");
    exit(EXIT_FAILURE);
  }
  if (!str_to_port(argv[1], &PORT)) {
    fprintf(stderr, "Error: Port number is invalid for user server applications\n");
    exit(EXIT_FAILURE);
  }
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == -1) {
    perror("\nsocket()");
    exit(EXIT_FAILURE);
  }
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(listenfd, (struct sockaddr*) &server, sizeof(server)) == -1) {
    perror("\nbind()");
    exit(EXIT_FAILURE);
  }
  if (listen(listenfd, BACKLOG) == -1) {
    perror("\nlisten()");
    exit(EXIT_FAILURE);
  }
  printf("TCP Server started!\n");

  // Communicate with client
  // num_account = read_account(accounts);
  // printf("Information of %d accounts have been read successful\n", num_account);

  struct pollfd poll_listenfd;
  poll_listenfd.fd = listenfd;
  poll_listenfd.events = POLLIN;

  session_t *sessfd[CLIENT_MAX];
  struct pollfd poll_sessfd[CLIENT_MAX];
  for (int i = 0; i < CLIENT_MAX; i++) {
    poll_sessfd[i].fd = -1;
    poll_sessfd[i].events = POLLIN;
  }
  int count_sess = 0;
  int nready = 0;
  session_t sess_info;
  int idx = 0;

  while (1) {
    while (1) {
      retval = poll(&poll_listenfd, 1, TIME_OUT);
      if (retval == -1) {
        perror("\npoll():poll_listenfd");
        return -1;
      }
      if (poll_listenfd.revents & POLLIN) {
        for (idx = 0; idx <= CLIENT_MAX; idx++)
          if (poll_sessfd[idx].fd == -1) break;

        if (idx == CLIENT_MAX) {
          fprintf(stderr, "Number of connections exceeds the capacity (%d)\n", CLIENT_MAX);
          break;
        } else {
          connfd = accept(listenfd, (struct sockaddr*) &client, &sin_size);
          if (connfd == -1) {
            perror("\naccept()");
            return -1;
          }
          sess_info.recv_buffer[0] = '\0';
          sess_info.logged = false;
          sess_info.username[0] = '\0';
          sess_info.connfd = connfd;
          sess_info.prev = NULL;
          sess_info.next = NULL;
          sessfd[idx] = add_session(&sess_info);
          poll_sessfd[idx].fd = sess_info.connfd;
          count_sess++;
          send_f(CONNECT_SUCCESS, poll_sessfd[idx].fd);
        }
      } else break;
    }

    // Get message and response to client
    nready = poll(poll_sessfd, CLIENT_MAX, TIME_OUT);

    if (nready == 0) continue;
    else if (nready == -1) {
      perror("\npoll():poll_sessfd");
      continue;
    }

    for (idx = 0; idx < CLIENT_MAX; idx++) {
      if (poll_sessfd[idx].revents & POLLIN) {
        connfd = poll_sessfd[idx].fd;
        do {
          retval = get_f(connfd, recv_msg, sessfd[idx]->recv_buffer);
          if (retval < 0) {
            if (retval == -3) {
              fprintf(stderr, "Error: message received exceed the maximum message size\n");
              fprintf(stderr, "Notice: message length is limited to %d characters\n", MSG_SIZE);
              send_f(MSG_NOT_DETERMINED, connfd);
            } else if (retval == -2) {
              close(connfd);
              if (session_username[connfd] == NULL) {
                printf("[%d:]: (Disconnected)\n", connfd);
              } else {
                printf("[%d:%s]: (Disconnected)\n", connfd, session_username[connfd]);
                session_username[connfd] = NULL;
              }
              delete_session(sessfd[idx]);
              poll_sessfd[idx].fd = -1;
              count_sess--;
              break;
            } else {
              fprintf(stderr, "\nget_f(): Error\n");
              return -1;
            }
          } else handle_msg(recv_msg, sessfd[idx]);

          if (poll(poll_sessfd + idx, 1, 0) > 0) continue;
        } while (sessfd[idx]->recv_buffer[0] != '\0');

        nready--;
        if (nready == 0) break;
      }
    }
  }
  fprintf(stderr, "\nServer has error!!");
	close(listenfd);
	return 0;
}

/*
@function: handle message from client
@parameter: [IN] msg: message from client
            [IN] session: session's infomation
@return: 0 if success
         -1 if fail
*/
int handle_msg(char *msg, session_t *session_p) {
  session_t session = *session_p;
  int connfd = session.connfd;
  char command[COMMAND_LEN + 1];
  char type_of_msg[31];
  memset(command, 0, COMMAND_LEN + 1);
  memset(type_of_msg, 0, 31);

  char overcheck = '\0';
  int retval = -1;

  // get command from message
  snprintf(type_of_msg, sizeof(type_of_msg), "%%%ds", COMMAND_LEN);
  retval = sscanf(msg, type_of_msg, command);
  if (retval != 1) {
    send_f(MSG_NOT_DETERMINED, connfd);
    return -1;
  }

//---------------------------------Handle command---------------------------------
// __________________ Any session state can use this command _____________________
// LOGIN <username> <password>
// > LOGIN_SUCCESS
// > LOGGED_IN, PASSWORD_INCORRECT, ACCOUNT_LOCKED, ACCOUNT_NOT_EXIST
  if (strcmp(command, "LOGIN") == 0) {
    char username[USERNAME_LEN + 2];
    char password[PASSWORD_LEN + 2];
    memset(username, 0, USERNAME_LEN + 2);
    memset(password, 0, PASSWORD_LEN + 2);

    bool acc_exist = false;

    if (session.logged == true) { //[Warning] Unusual case for client
      send_f(LOGGED_IN, connfd);
      return -1;
    }

    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds %%%ds%%c", USERNAME_LEN+1, PASSWORD_LEN+1);
    retval = sscanf(msg, type_of_msg, username, password, &overcheck);
    if (strlen(username) == USERNAME_LEN+1 || strlen(password) == PASSWORD_LEN+1) {
      send_f(INVALID_ARGS, connfd);
      return -1;
    }
    if (retval != 2) {
      send_f(MSG_NOT_DETERMINED, connfd);
      return -1;
    }

    for (int i = 0; i < num_account; i++) {
      if (strcmp(username, accounts[i].username) == 0) {
        acc_exist = true;
        if (strcmp(password, accounts[i].password) == 0) {
          session_p->logged = true;
          memmove(session_p->username, username, USERNAME_LEN);
          session_username[connfd] = accounts[i].username;
          send_f(LOGIN_SUCCESS, connfd);
          return 0;
        } else {
          send_f(PASSWORD_INCORRECT, connfd);
          return -1;
        }
      }
    }
    if (acc_exist == false) {
      send_f(ACCOUNT_NOT_EXIST, connfd);
      return -1;
    }
  }
// REGISTER <username> <password>
// > REGISTER_SUCCESS
// > ACCOUNT_EXISTED
  else if (strcmp(command, "REGISTER") == 0) {
    char username[USERNAME_LEN + 2];
    char password[PASSWORD_LEN + 2];
    memset(username, 0, USERNAME_LEN + 2);
    memset(password, 0, PASSWORD_LEN + 2);

    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds %%%ds%%c", USERNAME_LEN+1, PASSWORD_LEN+1);
    retval = sscanf(msg, type_of_msg, username, password, &overcheck);
    if (strlen(username) == USERNAME_LEN+1 || strlen(password) == PASSWORD_LEN+1) {
      send_f(INVALID_ARGS, connfd);
      return -1;
    }
    if (retval != 2) {
      send_f(MSG_NOT_DETERMINED, connfd);
      return -1;
    }

    for (int i = 0; i < num_account; i++) {
      if (strcmp(username, accounts[i].username) == 0) {
        send_f(ACCOUNT_EXISTED, connfd);
        return -1;
      }
    }
    memmove(accounts[num_account].username, username, USERNAME_LEN);
    memmove(accounts[num_account].password, password, PASSWORD_LEN);
    session_username[connfd] = accounts[num_account].username;
    num_account++;

    session_p->logged = true;
    memmove(session_p->username, username, USERNAME_LEN);
    send_f(REGISTER_SUCCESS, connfd);
    return 0;
  }
//____________________ Only login state can use this command ____________________
  if (session.logged == false) {
    send_f(NOT_LOGGED_IN, connfd);
    return -1;
  }
// HOST <domain/ip>
// > GET_IP_SUCCESS <list_ip>, GET_DOMAIN_SUCCESS <domain>
// > GET_IP_FAIL, GET_DOMAIN_FAIL
  else if (strcmp(command, "HOST") == 0) {
    bool has_alpha = false;
    char send_msg[MSG_SIZE];
    char domain_ip[HOSTNAME_LEN + 2];
    memset(send_msg, 0, MSG_SIZE);
    memset(domain_ip, 0, HOSTNAME_LEN + 2);

    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds%%c", HOSTNAME_LEN+1);
    retval = sscanf(msg, type_of_msg, domain_ip, &overcheck);
    if (strlen(domain_ip) == HOSTNAME_LEN+1) {
      send_f(INVALID_ARGS, connfd);
      return -1;
    }
    if (retval != 1) {
      send_f(MSG_NOT_DETERMINED, connfd);
      return -1;
    }

    // Check whether domain_ip is domain name or ip address
    // If domain_ip has alphabetic characters, we consider it a domain name
    for (int i = 0; i < strlen(domain_ip); i++) {
      if (isalpha(domain_ip[i])) {
        has_alpha = true;
        break;
      }
    }
      
    if (has_alpha) {              // Get IPv4 address from domain name
      struct addrinfo *list_ip = NULL;
      retval = get_ip_address(domain_ip, &list_ip);
      if (retval != 0) {
        fprintf(stderr, "getaddrinfo() failed: %s\n", strerror(errno));
        send_f(GET_IP_FAIL, connfd);
        return -1;
      }

      strcpy(send_msg, GET_IP_SUCCESS);
      struct addrinfo *result = (struct addrinfo*) list_ip;
      struct sockaddr_in *addr_p = NULL;
      char ip[INET_ADDRSTRLEN];
      memset(ip, 0, INET_ADDRSTRLEN);

      for (result = list_ip; result != NULL; result = result->ai_next) {
        addr_p = (struct sockaddr_in*) result->ai_addr;
        inet_ntop(AF_INET, &addr_p->sin_addr, ip, sizeof(ip));
        send_msg[strlen(send_msg)] = ' ';
        strncat(send_msg, ip, MSG_SIZE-1 - strlen(send_msg));
        memset(ip, 0, INET_ADDRSTRLEN);
      }
      send_f(send_msg, connfd);
      freeaddrinfo(list_ip);
    } else {                  // get domain name from IPv4 address
      char *domain = NULL;
      retval = get_domain_name(domain_ip, &domain);
      if (retval == 0) {
        strcpy(send_msg, GET_DOMAIN_SUCCESS);
        send_msg[strlen(send_msg)] = ' ';
        strncat(send_msg, domain, MSG_SIZE-1 - strlen(send_msg));
        send_f(send_msg, connfd);
        free(domain);
      } else if (retval == 1) {
        send_f(INVALID_IP, connfd);
        return -1;
      } else {
        send_f(GET_DOMAIN_FAIL, connfd);
      }
    }
    return 0;
  }
// UPLOAD <filename> <filesize>
// > UPLOAD_SUCCESS
// > RECV_DATA_FAIL, FILE_TOO_LARGE <MAX_FILE_SIZE>
  else if (strcmp(command, "UPLOAD") == 0) {
    char filename[FILENAME_LEN + 2];
    unsigned long filesize = 0;
    char send_msg[MSG_SIZE];

    memset(filename, 0, FILENAME_LEN + 2);
    memset(send_msg, 0, MSG_SIZE);

    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds %%ld%%c", FILENAME_LEN+1);
    retval = sscanf(msg, type_of_msg, filename, &filesize, &overcheck);
    if (strlen(filename) == FILENAME_LEN+1) {
      send_f(INVALID_ARGS, connfd);
      return -1;
    }
    if (retval != 2) {
      send_f(MSG_NOT_DETERMINED, connfd);
      return -1;
    }
    if (filesize > FILESIZE_MAX) {
      sprintf(send_msg, "%s %ld", FILE_TOO_LARGE, FILESIZE_MAX);
      send_f(send_msg, connfd);
      return -1;
    }
    // Create storage folder if not exist
    if (access(STORAGE_FOLDER, F_OK) == -1) {
      if (mkdir(STORAGE_FOLDER, 0777) == -1) {
        fprintf(stderr, "Failed to create the directory %s", STORAGE_FOLDER);
        perror("\nmkdir()");
        send_f(SERVER_ERROR, connfd);
        return -1;
      }
    }
    // Create user folder if not exist
    char directory[strlen(STORAGE_FOLDER) + USERNAME_LEN + 4];
    snprintf(directory, sizeof(directory), "./%s/%s", STORAGE_FOLDER, session.username);
    if (access(directory, F_OK) == -1) {
      if (mkdir(directory, 0777) == -1) {
        fprintf(stderr, "Failed to create the directory ./%s/%s", STORAGE_FOLDER, session.username);
        perror("\nmkdir()");
        send_f(SERVER_ERROR, connfd);
        return -1;
      }
    }

    char filepath[PATH_MAX];
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);
    FILE *file = fopen(filepath, "wb");
    if (file == NULL) {
      perror("\nfopen()");
      send_f(SERVER_ERROR, connfd);
      return -1;
    }
    send_f(UPLOAD_READY, connfd);
    printf("[%d:%s]: (Uploading file \"%s\")\n", connfd, session.username, filename);

    long received_bytes = 0;
    long total_received = 0;
    char *data = (char*) malloc(FILE_UPLOAD_ALLOC_SIZE);
    while (total_received < filesize) {
      received_bytes = recv(connfd, data, FILE_UPLOAD_ALLOC_SIZE, 0);
      if (received_bytes < 0) {
        perror("\nrecv()");
        fclose(file);
        send_f(RECV_DATA_FAIL, connfd);
        return -1;
      } else if (received_bytes == 0) {
        fprintf(stderr, "Warning: Connection closed when download file\n");
        fclose(file);
        return -1;
      }
      total_received += received_bytes;
      if (total_received > filesize) {
        fprintf(stderr, "Warning: Received data > filesize\n");
        break;
      }
      fwrite(data, 1, received_bytes, file);
    }
    fclose(file);
    free(data);
    send_f(UPLOAD_SUCCESS, connfd);
    return 0;
  }
// FEEDBACK <content>
// > FEEDBACK_SUCCESS
  else if (strcmp(command, "FEEDBACK") == 0) {
    char *content = NULL;
    char filepath[PATH_MAX];

    if (msg[strlen("FEEDBACK")] == ' ' && msg[strlen("FEEDBACK") + 1] != '\0') {
      content = msg + strlen("FEEDBACK") + 1;
      // Create feedback folder if not exist
      if (access(FEEDBACK_FOLDER, F_OK) == -1) {
        if (mkdir(FEEDBACK_FOLDER, 0777) == -1) {
          fprintf(stderr, "Failed to create the directory %s", FEEDBACK_FOLDER);
          perror("\nmkdir()");
          send_f(SERVER_ERROR, connfd);
          return -1;
        }
      }
      snprintf(filepath, sizeof(filepath), "./%s/%s", FEEDBACK_FOLDER, session.username);
      FILE *feedback_file = fopen(filepath, "a");

      fprintf(feedback_file, "%s $ %s\n", get_time(), content);
      fclose(feedback_file);
    } else {
      send_f(INVALID_ARGS, connfd);
      return -1;
    }
    // Can print feedback content here
    send_f(FEEDBACK_SUCCESS, connfd);
    return 0;
  }
// CHANGE_PASSWORD <old_password> <new_password>
// > PASSWORD_CHANGE_SUCCESS
// > PASSWORD_INCORRECT
  if (strcmp(command, "CHANGE_PASSWORD") == 0) {
    char old_password[PASSWORD_LEN + 2];
    char new_password[PASSWORD_LEN + 2];
    memset(old_password, 0, PASSWORD_LEN + 2);
    memset(new_password, 0, PASSWORD_LEN + 2);

    snprintf(type_of_msg, sizeof(type_of_msg), "%%*s %%%ds %%%ds%%c", PASSWORD_LEN+1, PASSWORD_LEN+1);
    retval = sscanf(msg, type_of_msg, old_password, new_password, &overcheck);
    if (strlen(old_password) == PASSWORD_LEN+1 || strlen(new_password) == PASSWORD_LEN+1) {
      send_f(INVALID_ARGS, connfd);
      return -1;
    }
    if (retval != 2) {
      send_f(MSG_NOT_DETERMINED, connfd);
      return -1;
    }
    
    for (int i = 0; i < num_account; i++) {
      if (strcmp(session.username, accounts[i].username) == 0) {
        if (strcmp(old_password, accounts[i].password) == 0) {
          memmove(accounts[i].password, new_password, PASSWORD_LEN);
          send_f(CHANGE_PASSWORD_SUCCESS, connfd);
          return 0;
        } else {
          send_f(PASSWORD_INCORRECT, connfd);
          return -1;
        }
      }
    }
  }
// LOGOUT
// > LOGOUT_SUCCESS
  else if (strcmp(command, "LOGOUT") == 0) {
    if (session.logged == false) {
      send_f(NOT_LOGGED_IN, connfd);
      return 0;
    }
    session_p->logged = false;
    *session_p->username = '\0';
    send_f(LOGOUT_SUCCESS, connfd);
    session_username[connfd] = NULL;
    return 0;
  }

  send_f(INVALID_COMMAND, connfd);
  return 0;
}

/*
@function: Read ACCOUNT_FILE and save account infomation to account_t accounts[]
          Account's infomation have been read successful will be save into accounts array
@return: number account have been read successful
*/
int read_account() {
  int fd = open(ACCOUNT_FILE, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Cannot open file %s: %s", ACCOUNT_FILE, strerror(errno));
    exit(1);
  }

  char *buffer = (char*) malloc(FILE_READ_ALLOC_SIZE);
  int read_bytes;
  int retval = -1;

  int num_account = 0;
  int line = 0;
  char *token = NULL;
  char username[USERNAME_LEN+1];
  char status = '\0';
  char overcheck = '\0';
  char type_of_line[17];
  retval = snprintf(type_of_line, sizeof(type_of_line), "%%%ds %%c %%c", USERNAME_LEN);
  if (retval < 0) perror("\nsnprintf()");
  else if ((unsigned int) retval > sizeof(type_of_line) - 1)
    fprintf(stderr, "Warning: type_of_line string length exceeded\n");

  do {
    read_bytes = read(fd, buffer, FILE_READ_ALLOC_SIZE - 1);
    if (read_bytes < 0) {
      perror("\nread()");
      free(buffer);
      exit(1);
    }
    if (read_bytes == 0) break;
    buffer[read_bytes] = '\0';

    token = strtok(buffer, "\n");
    status = '\0';
    username[0] = '\0';
    while (token != NULL) {
      line ++;
      overcheck = '\0';
      retval = sscanf(token, type_of_line, username, &status, &overcheck);
      if (strlen(username) == USERNAME_LEN+1) {
        fprintf(stderr, "Note: the account name cannot be more than %d characters\n", USERNAME_LEN);
        fprintf(stderr, "Hint: change USERNAME_LEN in the server.c to be able to have more characters\n");
        fprintf(stderr, "Warning: this line will be ignore\n");
        fprintf(stderr, "%7d | %s\n", line, token);
        token = strtok(NULL, "\n");
        continue;
      }
      if (retval < 2 || (overcheck != '\0') || (status != '0' && status != '1')) {
        fprintf(stderr, "Warning: cannot parsing, this line will be ignore\n");
        fprintf(stderr, "%7d | %s\n", line, token);
        token = strtok(NULL, "\n");
        continue;
      }
      memmove(accounts[num_account].username, username, strlen(username) + 1);
      // accounts[num_account].status = (status == '1');
      num_account++;
      if (num_account == ACCOUNT_MAX) {
        fprintf(stderr, "Note: cannot save more than %d accounts\n", ACCOUNT_MAX);
        fprintf(stderr, "Hint: change ACCOUNT_MAX in the server.c to be able to save more accounts\n");
        fprintf(stderr, "Warning: stopped reading the file %s from line %d\n", ACCOUNT_FILE, line);
        free(buffer);
        close(fd);
        return num_account;
      }
      token = strtok(NULL, "\n");
    }
  } while (read_bytes > 0);
  // Can print account infomation here
  free(buffer);
  close(fd);
  return num_account;
}