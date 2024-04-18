#include <stdbool.h>
#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#define BACKLOG 10       // Max number of pending connections
#define CLIENT_MAX 1024  // Max client in one thread
#define MSG_SIZE 4096    // Size of string message
#define DELIMITER "\r\n"
#define DELIMITER_LEN 2
#define DELIMITER_SIZE 3  // Size of string delimiter

#define FILE_UPLOAD_ALLOC_SIZE 10000000   // 10MB
#define FILE_READ_ALLOC_SIZE 1000000 // 1MB
#define ACCOUNT_FILE "account.txt"
#define ACCOUNT_MAX 5000  // Max account number
#define STORAGE_FOLDER "Upload_files"
#define FEEDBACK_FOLDER "Feedback_files"

#define COMMAND_LEN 20
#define USERNAME_LEN 20
#define PASSWORD_LEN 20
#define HOSTNAME_LEN 100
#define FILENAME_LEN 100
#define FILESIZE_MAX 9000000000 // 9GB

// LOGIN <username> <password>
// LOGOUT
// REGISTER <username> <password>
// CHANGE_PASSWORD <old_password> <new_password>
// FEEDBACK <content>
// HOST <hostname> // hostname is domain name or ip address
// UPLOAD <filename> <filesize>

//______________________________________________________________________________
#define LOGGED_IN "100"
#define NOT_LOGGED_IN "101"
#define ACCOUNT_NOT_EXIST "102"
#define ACCOUNT_LOCKED "103"

#define CONNECT_SUCCESS "200"
#define LOGIN_SUCCESS "201"
#define LOGOUT_SUCCESS "202"
#define REGISTER_SUCCESS "204"
#define CHANGE_PASSWORD_SUCCESS "205"
#define FEEDBACK_SUCCESS "206"
#define GET_IP_SUCCESS "207"
#define GET_DOMAIN_SUCCESS "208"
#define UPLOAD_SUCCESS "209"

#define PASSWORD_INCORRECT "300"
#define ACCOUNT_EXISTED "301"
#define INVALID_IP "302"
#define NOT_FOUND "303"
#define GET_IP_FAIL "304"
#define GET_DOMAIN_FAIL "305"
#define UPLOAD_READY "306" // Server is ready to receive file
#define FILE_TOO_LARGE "307"


#define MSG_NOT_DETERMINED "400"
#define MSG_OVERLENGTH "401"
#define RECV_DATA_FAIL "402"
#define INVALID_COMMAND "403"
#define INVALID_ARGS "404"

#define SERVER_ERROR "500"
//______________________________________________________________________________
#define M100 "Have been logged in"
#define M101 "Have not logged in"
#define M102 "Account does not exist"
#define M103 "Account is locked"

#define M200 "Connect successful"
#define M201 "Login successful"
#define M202 "Logout successful"
#define M203 "Post successful"
#define M204 "Register successful"
#define M205 "Change password successful"
#define M206 "Feedback successful"
#define M207 "Get IP address successful"
#define M208 "Get domain name successful"
#define M209 "Upload successful"

#define M300 "Password is incorrect"
#define M301 "Account existed"
#define M302 "Invalid IP address"
#define M303 "Not found"
#define M304 "IP address not found"
#define M305 "Domain name not found"
#define M306 "Server is ready to receive file"
#define M307 "File too large"

#define M400 "Message cannot determined"
#define M401 "Message exceed the maximum message size"
#define M402 "Get data fail"
#define M403 "Invalid command"
#define M404 "Invalid arguments"

#define M500 "Server error"

//______________________________________________________________________________

bool send_msg_to_client(char *msg, int connfd);
int get_msg_from_client(int connfd, char *message, char *recv_buffer);
bool send_msg_to_server(char *msg, int connfd);
int get_msg_from_server(int clientfd, char *message, char *recv_buffer);
char *msg_str(char *msg_code);
bool print_msg(char *msg_code);

#endif // TCP_SOCKET_H
