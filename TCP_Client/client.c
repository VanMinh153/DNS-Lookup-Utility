#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include "../include/utility.h"
#include "../include/tcp_socket.h"
#include "../include/session.h"

#define FAIL_TO_SEND_MSG "Fail to send message to server\n"
#define send_f(msg) send_msg_to_server(msg, server_fd)
#define get_f(msg) get_msg_from_server(server_fd, msg, recv_buffer)

int SERVER_PORT;

int main(int argc, char **argv) {
  int server_fd;
  struct sockaddr_in server;
  char send_msg[MSG_SIZE];
  char recv_msg[MSG_SIZE], recv_buffer[MSG_SIZE];
  int retval = -1;

  memset(send_msg, 0, MSG_SIZE);
  memset(recv_msg, 0, MSG_SIZE);
  memset(recv_buffer, 0, MSG_SIZE);

  if (argc != 3) {
    fprintf(stderr, "Please write: ./client <Server_IP> <Port_Number>\n");
    return -1;
  }
  retval = inet_pton(AF_INET, argv[1], &server.sin_addr);
  if (retval == 0) {
    fprintf(stderr, "Warning: Invalid IPv4 address\n");
    return -1;
  } else if (retval == -1) {
    perror("\ninet_pton()");
    return -1;
  }
  if (!str_to_port(argv[2], &SERVER_PORT)) {
    fprintf(stderr, "Error: Port number is invalid for user server applications\n");
    return -1;
  }
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
    perror("\nsocket()");
    return -1;
  }
  server.sin_family = AF_INET;
  server.sin_port = htons(SERVER_PORT);
  if (connect(server_fd, (struct sockaddr*) &server, sizeof(struct sockaddr)) < 0) {
    perror("\nconnect()");
    return -1;
  }

// Communicate with server
  get_f(recv_msg);
  print_msg(recv_msg);

  const int FEEDBACK_MAX = MSG_SIZE - strlen("FEEDBACK ") - DELIMITER_LEN;
  char inp_str[MSG_SIZE];
  char *cleared_inp;
  memset(inp_str, 0, MSG_SIZE);


Menu1:
  while (1) {
    system("clear");
    printf("_____________________________________________________\n");
    printf("|                                                    |\n");
    printf("|         DNS Lookup & File Upload Utility           |\n");
    printf("|____________________________________________________|\n\n");
    printf("  1. Login\n");
    printf("  2. Register\n");
    printf("\n");
// Menu1_run:
    int user_choice = 0;
    while (1) {
      printf("Choose feature [1-2]: ");
      fgets(inp_str, MSG_SIZE, stdin);
      if (inp_str[0] == '\n') continue;
      if (get_last_char(inp_str) != '\n') {
        printf("Notice: Too long message\n\n");
        clear_stdin();
        continue;
      }
      cleared_inp = clear_f(inp_str);

      if (strlen(cleared_inp) != 1 || cleared_inp[0] < '1' || cleared_inp[0] > '2') {
        printf("Notice: ask to choose between 1 - 2\n\n");
        continue;
      }
      user_choice = cleared_inp[0] - '0';
      break;
    }
    switch (user_choice) {
// LOGIN <username> <password>
      case 1:
        char *username = NULL;
        char *password = NULL;
        while (1) {
          printf("Enter username: ");
          fgets(inp_str, USERNAME_LEN+2, stdin);
          if (inp_str[0] == '\n') continue;
          if (get_last_char(inp_str) != '\n') {
            printf("Notice: username must be less than %d character\n\n", USERNAME_LEN+1);
            clear_stdin();
            continue;
          }
          username = clear_f(inp_str);
          if (username == NULL) continue;
          if (!isalnum_f(username)) {
            printf("Notice: Username may only contain alphanumeric characters\n\n");
            continue;
          }
          break;
        }
        sprintf(send_msg, "LOGIN %s ", username);

        while (1) {
          printf("Enter password: ");
          fgets(inp_str, PASSWORD_LEN+2, stdin);
          if (inp_str[0] == '\n') continue;
          if (get_last_char(inp_str) != '\n') {
            printf("Notice: password must be less than %d character\n\n", PASSWORD_LEN+1);
            clear_stdin();
            continue;
          }
          password = clear_f(inp_str);
          if (password == NULL) continue;
          if (!isalnum_f(password)) {
            printf("Notice: Password may only contain alphanumeric characters\n\n");
            continue;
          }
          break;
        }
        strcat(send_msg, password);
        send_f(send_msg);
        get_f(recv_msg);
        print_msg(recv_msg);
        if (strcmp(recv_msg, LOGIN_SUCCESS) == 0) {
          clear_stdin();
          goto Menu2;
        }
        break;
// REGISTER <username> <password>
      case 2:
        // char *username = NULL;
        // char *password = NULL;
        while (1) {
          printf("Enter username: ");
          fgets(inp_str, USERNAME_LEN+2, stdin);
          if (inp_str[0] == '\n') continue;
          if (get_last_char(inp_str) != '\n') {
            printf("Notice: username must be less than %d character\n\n", USERNAME_LEN+1);
            clear_stdin();
            continue;
          }
          username = clear_f(inp_str);
          if (username == NULL) continue;
          if (!isalnum_f(username)) {
            printf("Notice: Username may only contain alphanumeric characters\n\n");
            continue;
          }
          break;
        }
        sprintf(send_msg, "REGISTER %s ", username);

        while (1) {
          printf("Enter password: ");
          fgets(inp_str, PASSWORD_LEN+2, stdin);
          if (inp_str[0] == '\n') continue;
          if (get_last_char(inp_str) != '\n') {
            printf("Notice: password must be less than %d character\n\n", PASSWORD_LEN+1);
            clear_stdin();
            continue;
          }
          password = clear_f(inp_str);
          if (password == NULL) continue;
          if (!isalnum_f(password)) {
            printf("Notice: Password may only contain alphanumeric characters\n\n");
            continue;
          }
          break;
        }
        strcat(send_msg, password);
        send_f(send_msg);
        get_f(recv_msg);
        print_msg(recv_msg);
        if (strcmp(recv_msg, REGISTER_SUCCESS) == 0) {
          clear_stdin();
          goto Menu2;
        }
        break;
      default: break;
    }
    clear_stdin();
  }

Menu2:
  while (1) {
    system("clear");
    printf("_____________________________________________________\n");
    printf("|                                                    |\n");
    printf("|         DNS Lookup & File Upload Utility           |\n");
    printf("|____________________________________________________|\n\n");
    printf("  1. DNS Lookup\n");
    printf("  2. File Upload\n");
    printf("  3. Feedback\n");
    printf("  4. Change Password\n");
    printf("  5. Logout\n");
    printf("\n");
// Menu2_run:
    int user_choice = 0;
    while (1) {
      printf("Choose feature [1-5]: ");
      fgets(inp_str, MSG_SIZE, stdin);
      if (inp_str[0] == '\n') continue;
      if (get_last_char(inp_str) != '\n') {
        printf("Notice: Too long message\n\n");
        clear_stdin();
        continue;
      }
      cleared_inp = clear_f(inp_str);

      if (strlen(cleared_inp) != 1 || cleared_inp[0] < '1' || cleared_inp[0] > '5') {
        printf("Notice: ask to choose between 1 - 5\n\n");
        continue;
      }
      user_choice = cleared_inp[0] - '0';
      break;
    }
    switch (user_choice) {
// HOST <domain/ip>
      case 1:
        char *domain_ip = NULL;
        while (1) {
          printf("Enter domain name or ip address: ");
          fgets(inp_str, MSG_SIZE, stdin);
          if (inp_str[0] == '\n') continue;
          if (get_last_char(inp_str) != '\n') {
            printf("Notice: hostname must be less than %d character\n\n", MSG_SIZE-2);
            clear_stdin();
            continue;
          }
          domain_ip = clear_f(inp_str);
          if (domain_ip == NULL) continue;
          break;
        }
        sprintf(send_msg, "HOST %s", domain_ip);
        send_f(send_msg);
        get_f(recv_msg);

        if (strncmp(recv_msg, GET_IP_SUCCESS, 3) == 0) {
          char *token = strtok(recv_msg + 4, " ");
          while (token != NULL) {
            printf("IP address: %s\n", token);
            token = strtok(NULL, "\n");
          }
        } else if (strncmp(recv_msg, GET_DOMAIN_SUCCESS, 3) == 0) {
          printf("Domain name: %s\n", recv_msg + 4);
        } else if (strcmp(recv_msg, INVALID_IP) == 0) {
          printf("Notice: Invalid IP address\n");
        } else if (strcmp(recv_msg, GET_IP_FAIL) == 0) {
          printf("IP address not found\n");
        } else if (strcmp(recv_msg, GET_DOMAIN_FAIL) == 0) {
          printf("Domain name not found\n");
        } else print_msg(recv_msg);
        break;
// UPLOAD <filename> <filesize>
      case 2:
        FILE *file = NULL;
        struct stat file_info;
        long filesize = 0;
        char *filepath = NULL;
        char *filename = NULL;

        while (1) {
          printf("Enter path to file (q-quit): ");
          fgets(inp_str, MSG_SIZE, stdin);
          if (inp_str[0] == '\n') continue;
          if (get_last_char(inp_str)!= '\n') {
            printf("Notice: file path must be less than %d character\n\n", MSG_SIZE-2);
            clear_stdin();
            continue;
          }
          filepath = clear_f(inp_str);
          if (filepath == NULL) continue;
          if (strcmp(filepath, "q") == 0) goto Menu2;
          file = fopen(filepath, "rb");
          if (file == NULL) {
            printf("Notice: File not found\n\n");
            continue;
          }
          // get file size use stat
          if (stat(filepath, &file_info) == -1) {
            perror("\nError");
            continue;
          }
          if (!S_ISREG(file_info.st_mode)) {
            printf("Notice: \"%s\" is not a regular file\n\n", filepath);
            continue;
          }
          filesize = file_info.st_size;
          break;
        }
        if (filesize > FILESIZE_MAX) {
          printf("Notice: File too large to send\n\n");
          break;
        }
        filename = strrchr(filepath, '/');  // get filename from path
        if (filename == NULL) filename = filepath;
        else filename++;
        
        sprintf(send_msg, "UPLOAD %s %ld", filename, filesize);
        send_f(send_msg);
        // receive ready message from server before sending file
        get_f(recv_msg);
        if (strcmp(recv_msg, UPLOAD_READY) != 0) {
          printf("Notice: Server is not ready to receive file\n\n");
          break;
        }

        // Upload file
        printf("Uploading file...\n");
        char *data = (char*) malloc(FILE_UPLOAD_ALLOC_SIZE);
        if (data == NULL) {
          perror("\nError");
          break;
        }
        int read_bytes = 0;
        int sent_bytes = 0;
        while (!feof(file)) {
          read_bytes = fread(data, 1, FILE_UPLOAD_ALLOC_SIZE, file);
          if (read_bytes < 0) {
            perror("\nError reading file");
            fclose(file);
            free(data);
            return -1;
          }
          sent_bytes = send(server_fd, data, read_bytes, 0);
          if (sent_bytes < 0) {
            perror("\nError sending data");
            fclose(file);
            free(data);
            return -1;
          }
        }

        get_f(recv_msg);
        print_msg(recv_msg);
        printf("%ld bytes was sent", filesize);
        fclose(file);
        free(data);
        break;
// FEEDBACK <content>        
      case 3:
        char *content = NULL;
        while (1) {
          printf("Enter feedback: ");
          fgets(inp_str, MSG_SIZE, stdin);
          if (inp_str[0] == '\n') continue;
          if (get_last_char(inp_str) != '\n') {
            printf("Notice: feedback must be less than %d character\n\n", MSG_SIZE-2);
            clear_stdin();
            continue;
          }
          content = clear_f(inp_str);
          if (content == NULL) continue;
          break;
        }
        sprintf(send_msg, "FEEDBACK %s", content);
        send_f(send_msg);
        get_f(recv_msg);
        print_msg(recv_msg);
        break;
// CHANGE_PASSWORD <old_password> <new_password>
      case 4:
        char *old_password = NULL;
        char *new_password = NULL;

        while (1) {
          printf("Enter old password: ");
          fgets(inp_str, PASSWORD_LEN+2, stdin);
          if (inp_str[0] == '\n') continue;
          if (get_last_char(inp_str) != '\n') {
            printf("Notice: password must be less than %d character\n\n", PASSWORD_LEN+1);
            clear_stdin();
            continue;
          }
          old_password = clear_f(inp_str);
          if (old_password == NULL) continue;
          if (!isalnum_f(old_password)) {
            printf("Notice: Password may only contain alphanumeric characters\n\n");
            continue;
          }
          break;
        }
        sprintf(send_msg, "CHANGE_PASSWORD %s ", old_password);

        while (1) {
          printf("Enter new password: ");
          fgets(inp_str, PASSWORD_LEN+2, stdin);
          if (inp_str[0] == '\n') continue;
          if (get_last_char(inp_str) != '\n') {
            printf("Notice: password must be less than %d character\n\n", PASSWORD_LEN+1);
            clear_stdin();
            continue;
          }
          new_password = clear_f(inp_str);
          if (new_password == NULL) continue;
          if (!isalnum_f(new_password)) {
            printf("Notice: Password may only contain alphanumeric characters\n\n");
            continue;
          }
          break;
        }
        strcat(send_msg, new_password);
        send_f(send_msg);
        get_f(recv_msg);
        print_msg(recv_msg);
        break;        
// LOGOUT
      case 5:
        send_f("LOGOUT");
        get_f(recv_msg);
        // print_msg(recv_msg);
        // clear_stdin();
        goto Menu1;        
      default: break;
    }
    clear_stdin();
  }
  return 0;
}
