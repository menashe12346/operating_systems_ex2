#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h> // For fd_set and select

#define BUFFER_SIZE 1024

int input_fd = STDIN_FILENO;
int output_fd = STDOUT_FILENO;
int input_port = 0;

/// @brief Function to create a TCP server
/// @param port
/// @return socket file descriptor
int create_tcp_server(int port) {
    // Socket file descriptor
    int sockfd; //This will be used to reference the socket throughout its lifetime.
    struct sockaddr_in server_addr; // A structure that holds the address and port on which the server socket will listen for incoming connections.

    /* High-level of struct sockaddr_in:

    struct sockaddr_in {
        short            sin_family;   // address family, e.g. AF_INET
        unsigned short   sin_port;     //  port number for the socket
        struct in_addr   sin_addr;     //  structure to hold the IPv4 address(binary representation of the IP address for the socket)
        char             sin_zero[8];  // so it will be competible with the generic struct sockaddr
    };

    struct in_addr {
        unsigned long s_addr; //where we hold the ip address 
    };
    */

    /*
    AF_INET: Specifies the address family (IPv4). we chose IPv4 because it ensures compatibility with a vast majority of existing networks and devices.
    SOCK_STREAM: Specifies the socket type (TCP).
    0: means that the system should automatically choose the appropriate protocol for the given socket type. For SOCK_STREAM with AF_INET, this will be TCP..
    */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    // allows the socket to reuse the local address
    int val = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
        perror("setsockopt");
    }

    memset(&server_addr, 0, sizeof(server_addr));  //ensures that the server_addr structure is fully initialized to zero

    //we chose AF_INET as the address family for the socket(specifying that the socket will use IPv4 addresses, a protocol for transmitting data over the internet)
    //there are many protocols in AF_INET, among them the most popular are TCP and UDP. we will use TCP.
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0 in IPv4, it will accept connections on all available networks.
    server_addr.sin_port = htons(port); //convert the representation of the port into big-endian(the expected network byte order).

    /* bind is associating a socket with a specific local address and port number
    sockfd - file descriptor of the socket.
    (struct sockaddr *)&server_addr - pointer to a sockaddr structure that contains the address to which the socket will bound.
    sizeof(server_addr) - size of the server_addr structure, tells the bind how much memory to read.
    */
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    /* tells the operating system that the program is ready to accept incoming connections on the socket sockfd.
    sockfd - file descriptor of the socket.
    1 - backlog parameter, 
    When a client attempts to connect to the server, the connection is placed in a queue if the server is not immediately ready to accept it. 
    The backlog parameter determines the size of this queue.
    */
    if (listen(sockfd, 1) < 0) {
        perror("listen");
        close(sockfd);
        return -1;
    }

    printf("Server listening on port %d\n", port);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)); //convert from a binary format into IPv4 format
    return client_fd;
}

// Function to start a TCP client
int start_tcp_client(char *hostname, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr)); //  reset server_addr to 0
    server_addr.sin_family = AF_INET; //set address family to AF_INET
    server_addr.sin_port = htons(port); // convert the port to big-endian.

    if (inet_pton(AF_INET, hostname, &server_addr.sin_addr) <= 0) {  //converts the hostname (a string) to an IP address and stores it in server_addr.sin_addr.
        fprintf(stderr, "Invalid address/ Address not supported\n");
        close(sockfd);
        return -1;
    }

    printf("Connecting to %s:%d\n", hostname, port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    printf("Client connected to %s on port %d\n", hostname, port);
    return sockfd;
}


/// @brief Function to parse TCP/UDP/UDS parameters
/// @param param TCP configuration information
/// @return 1 on Success, 0 on fail
void parse_parameter(char *param) {
    int port;
    if (strncmp(param, "TCPS", 4) == 0) {
        port = atoi(param + 4);
        input_port = port;
        printf("Creating TCP server on port %d\n", port);
        input_fd = create_tcp_server(port);
        if (input_fd == -1) {
            fprintf(stderr, "Failed to create TCP server\n");
            exit(EXIT_FAILURE);
        }
    } else if (strncmp(param, "TCPC", 4) == 0) {
        param += 4;
        char *hostname = strtok(param, ",");
        char *port_str = strtok(NULL, ",");
        if (hostname == NULL || port_str == NULL) {
            fprintf(stderr, "Invalid client parameters\n");
            exit(EXIT_FAILURE);
        }
        int port = atoi(port_str);
        if (port == input_port) {
            output_fd = input_fd;
            return;
        }
        printf("Connecting to TCP server at %s:%d\n", hostname, port);
        if (strcmp(hostname, "localhost") == 0) { hostname = "127.0.0.1"; }
        output_fd = start_tcp_client(hostname, port);
        if (output_fd == -1) {
            fprintf(stderr, "Failed to connect to TCP server\n");
            exit(EXIT_FAILURE);
        }
    }
}

// Signal handler for alarm
void handle_alarm(int sig) {
    printf("Timeout reached, terminating the process\n");
    exit(EXIT_SUCCESS);
}

void chat() {
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    int max_fd = (input_fd > output_fd) ? input_fd : output_fd;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(input_fd, &read_fds);
        FD_SET(output_fd, &read_fds);

        // Wait for input on either file descriptor
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // Check if there's data to read from input_fd
        if (FD_ISSET(input_fd, &read_fds)) {
            ssize_t bytes_read = read(input_fd, buffer, sizeof(buffer));
            if (bytes_read < 0) {
                perror("read from input_fd");
                exit(EXIT_FAILURE);
            } else if (bytes_read == 0) {
                printf("End of input from input_fd\n");
                break;
            }
            if (write(output_fd, buffer, bytes_read) < 0) {
                perror("write to output_fd");
                exit(EXIT_FAILURE);
            }
        }

        // Check if there's data to read from output_fd
        if (FD_ISSET(output_fd, &read_fds)) {
            ssize_t bytes_read = read(output_fd, buffer, sizeof(buffer));
            if (bytes_read < 0) {
                perror("read from output_fd");
                exit(EXIT_FAILURE);
            } else if (bytes_read == 0) {
                printf("End of input from output_fd\n");
                break;
            }
            if (write(input_fd, buffer, bytes_read) < 0) {
                perror("write to input_fd");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int main(int argc, char *argv[]) { 
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <program> [args...] -i <input> -o <output> -b <both> -t <timeout>\n", argv[0]);
        return 1;
    }

    char *program = NULL;
    char input_param[256] = "";
    char output_param[256] = "";
    int timeout = 0;
    int i = 1; // because the first argument is the program name.

    int length = 0;
    for (; i < argc && argv[i][0] != '-'; i++) {
        length += strlen(argv[i]) + 1; // length of the program
    }

    program = malloc(length);
    if (!program) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    program[0] = '\0';
    for (i = 1; i < argc && argv[i][0] != '-'; i++) { // extract the program string from argv.
        strcat(program, argv[i]);
        if (i < argc - 1) {
            strcat(program, " ");
        }
    }

    printf("Program to execute: %s\n", program);

    int isBoth = 0;
    for (; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) { // ensures also that there is another argument after -i.
            strcpy(input_param, argv[++i]);
            printf("Input parameter: %s\n", input_param);
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            strcpy(output_param, argv[++i]);
            printf("Output parameter: %s\n", output_param);
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            strcpy(input_param, argv[++i]);
            strcpy(output_param, argv[i]);
            isBoth = 1;
            printf("Both parameter: %s\n", input_param);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            timeout = atoi(argv[++i]);
            printf("Timeout parameter: %d\n", timeout);
        }
    }

    if (timeout > 0) {
        signal(SIGALRM, handle_alarm); // will be executed when the signal is received. will also execute the handle_alarm function.
        alarm(timeout); //an alarm signal set to be delivered after a specified number of seconds.
    }

    if (strlen(input_param) > 0) {
        parse_parameter(input_param);
    }

    if (strlen(output_param) > 0) {
        if (isBoth) {
            output_fd = input_fd;
        } else {
            parse_parameter(output_param);
        }
    }

    if (input_fd != STDIN_FILENO) {
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            perror("dup2 input");
            exit(EXIT_FAILURE);
        }
    }

    if (output_fd != STDOUT_FILENO) {
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            perror("dup2 output");
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid = fork(); // create a child and run the program(so after the program exit we will return to the parent and will be able to continue)
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        chat();
    }else {
        wait(NULL);
    }

    free(program);
    return EXIT_SUCCESS;
}

/* HOW TO RUN:

1)  ./mync3_5 "./ttt 123456789" -i TCPS4050
    nc localhost 4050
*/

/* Creating gcov:

1)  make
2) running the program
3) gcov -b mync3_5.c
*/