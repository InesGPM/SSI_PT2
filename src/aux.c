// src/aux.c
#include "aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>


#define INPUT_DIR "/var/queue/new/"
#define OUTPUT_DIR_BASE "/home/"
#define READ_PREFIX "lido_"
#define LOG_DIR "/home/deamon"
#define LOG_FILE "/home/deamon/mydeamon.log"

// Registra uma mensagem de log em um arquivo de log.
void log_message(const char *message) {
    // Cria o diretório de log se não existir
    struct stat st = {0};
    if (stat(LOG_DIR, &st) == -1) {
        mkdir(LOG_DIR, 0700);
    }

    FILE *file = fopen(LOG_FILE, "a");  // Abre o arquivo de log para anexar
    if (file == NULL) {
        perror("Failed to open the log file");
        return;
    }

    // Obtém a hora atual e formata para a string de log
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0'; // Remove a nova linha no final de ctime

    fprintf(file, "[%s] %s\n", time_str, message);
    fclose(file);
}


void mark_message_as_read(const char *filepath) {
    char new_path[1024];
    char *dir = dirname(strdup(filepath));
    char *base = basename(strdup(filepath));

    // Verifica se a mensagem já está marcada como lida
    if (strncmp(base, READ_PREFIX, strlen(READ_PREFIX)) == 0) {
        log_message("Mensagem já está marcada como lida.");
        return;
    }

    // Monta o novo caminho com o prefixo "lido_"
    snprintf(new_path, sizeof(new_path), "%s/%s%s", dir, READ_PREFIX, base);

    // Renomeia o arquivo para marcar como lida
    if (rename(filepath, new_path) != 0) {
        perror("Erro ao marcar mensagem como lida");
    } else {
        char log_msg[2048];
        snprintf(log_msg, sizeof(log_msg), "Mensagem marcada como lida: %s", new_path);
        log_message(log_msg);
    }
}

void read_and_display_message(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        //fprintf(stderr, "Erro: Não foi possível abrir o arquivo de mensagem em %s\n", filepath);
        log_message("Erro: Não foi possível abrir o ficheiro de mensagem.");
        perror("Motivo");
        return;
    }

    // Lê e exibe o conteúdo da mensagem
    printf("Mensagem:\n");
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        //printf("%s", buffer);
        log_message(buffer);
    }

    fclose(file);

    // Marcar a mensagem como lida após ser exibida
    mark_message_as_read(filepath);
}



void delete_file(const char *file_path) {
    if (remove(file_path) == 0) {
        char log_msg[1024];
        snprintf(log_msg, sizeof(log_msg), "Ficheiro apagado com sucesso: %s", file_path);
        log_message(log_msg);
    } else {
        perror("Failed to delete file");
    }
}

void delete_all_files_in_directory(const char *directory_path) {
    DIR *dir;
    struct dirent *entry;
    struct stat filestat;
    char full_path[1024];

    if ((dir = opendir(directory_path)) == NULL) {
        perror("Failed to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' ||
            (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
            continue;  // Skip "." and ".."
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, entry->d_name);
        if (stat(full_path, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
            delete_file(full_path);  // Use the delete_file function to remove each file
        }
    }

    closedir(dir);
}




// Função para extrair o remetente de uma mensagem
int get_sender_from_message(const char *message_path, char *sender) {
    FILE *file = fopen(message_path, "r");
    if (!file) {
        perror("Failed to open message file");
        return -1;
    }

    char buffer[1024];
    while (fgets(buffer, 1024, file)) {
         if (strncmp(buffer, "From: ", 6) == 0) {
            size_t len = strlen(buffer);
            if (buffer[len - 1] == '\n') { // Se o último caractere é '\n', remova-o
                buffer[len - 1] = '\0';
            }
            strcpy(sender, buffer + 6); // Copia o remetente para a variável 'sender'
            fclose(file);
            return 0;
        }
    }

    fclose(file);
    return -1;
}

// Função para enviar uma resposta
int send_response(const char *dest, const char *response, const char *origin) {
    // Inicializa o gerador de números aleatórios
    srand(time(NULL));
    int random_number = rand();  // Gera um número aleatório
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date_str[64];
    strftime(date_str, sizeof(date_str)-1, "%Y-%m-%d %H:%M:%S", t);

    char filename[256];
    sprintf(filename, "cer%d.txt", random_number);
    log_message(filename);
    // Construir o caminho do novo arquivo de mensagem de forma segura
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s%s/mail/%s", OUTPUT_DIR_BASE, dest, filename);
    //printf("%s\n",filepath);
    log_message(filename);
    // Cria o arquivo de mensagem no diretório de destino
    FILE *fp = fopen(filepath, "w");
    if (fp) {
        //fprintf(fp, "From: %s\nTo: %s\nMessage: %s\n", origin, dest, response);
        fprintf(fp, "From: %s\nTo: %s\nDate: %s\nMessage: %s\n", origin, dest, date_str, response);

        fclose(fp);
        chmod(filepath, 0400);
        printf("Responding to %s with message: %s\n", dest, response);
    } else {
        perror("Falha ao abrir arquivo de mensagem");
        return -1;
    }
    return 0;  // Simulando sucesso
}




int check_group_member(const char *groupName, const char *userName) {
    FILE *file = fopen("/etc/group", "r");
    if (!file) {
        perror("Falha ao abrir /etc/group");
        return 0;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        char *saveptr;
        char *token = strtok_r(line, ":", &saveptr);
        if (token && strcmp(token, groupName) == 0) {
            for (int i = 0; i < 3; i++) { // Skip password, GID, and get to user list
                token = strtok_r(NULL, ":", &saveptr);
            }
            if (token) {
                token = strtok_r(token, "\n", &saveptr); // Remove newline from user list
                char *member = strtok_r(token, ",", &saveptr);
                while (member) {
                    if (strcmp(member, userName) == 0) {
                        fclose(file);
                        return 1; // User is a member of the group
                    }
                    member = strtok_r(NULL, ",", &saveptr);
                }
            }
            break; // Exit if group is found, regardless of user match
        }
    }

    fclose(file);
    return 0; // Group not found or user not in group
}

int check_group_and_first_member(const char *groupName, const char *userName) {
    FILE *file = fopen("/etc/group", "r");
    if (!file) {
        perror("Falha ao abrir /etc/group");
        return 0;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        char *saveptr;
        char *token = strtok_r(line, ":", &saveptr);
        if (token && strcmp(token, groupName) == 0) {
            for (int i = 0; i < 2; i++) { // Skip password and GID
                strtok_r(NULL, ":", &saveptr);
            }
            token = strtok_r(NULL, ":", &saveptr); // User list
            if (token) {
                char *firstMember = strtok_r(token, "\n", &saveptr); // Get first member, stop at comma or newline
                if (firstMember && strcmp(firstMember, userName) == 0) {
                    fclose(file);
                    return 1; // User is the first member or the only member
                }else{
                    firstMember = strtok_r(token, ",", &saveptr); // Get first member, stop at comma or newline
                    if (firstMember && strcmp(firstMember, userName) == 0) {
                    fclose(file);
                    return 1; // User is the first member or the only member
                    }
                }
            }
            break; // Exit if group is found, regardless of user match
        }
    }

    fclose(file);
    return 0; // Group not found or user not the first member
}



char** fetch_group_members(const char *groupName) {
    FILE *file;
    char line[1024];
    char *name, *members;
    char **memberList = NULL;
    int count = 0;

    file = fopen("/etc/group", "r");
    if (!file) {
        perror("Falha ao abrir o arquivo /etc/group");
        return NULL;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        // Obtém o nome do grupo
        name = strtok(line, ":");
        if (name && strcmp(name, groupName) == 0) {
            strtok(NULL, ":"); // Pula a senha
            strtok(NULL, ":"); // Pula o GID
            members = strtok(NULL, "\n"); // Obtém a lista de membros e remove a quebra de linha
            if (members) {
                // Conta quantos membros existem
                for (char *p = members; *p; p++) {
                    if (*p == ',') count++;
                }
                count++; // Adiciona mais um para o último membro sem vírgula

                // Aloca memória para os membros
                memberList = malloc(sizeof(char*) * (count + 1)); // +1 para o NULL no final
                if (!memberList) {
                    perror("Falha ao alocar memória");
                    fclose(file);
                    return NULL;
                }

                // Divide a string de membros e aloca para cada membro
                int idx = 0;
                char *member = strtok(members, ",");
                while (member) {
                    memberList[idx++] = strdup(member);
                    member = strtok(NULL, ",");
                }
                memberList[idx] = NULL; // Termina com NULL para marcar o fim do array

                fclose(file);
                return memberList;
            }
            break;
        }
    }

    //printf("Grupo '%s' não encontrado ou não tem membros.\n", groupName);
    log_message("Grupo não encontrado ou não tem membros.\n");
    fclose(file);
    return NULL;
}


// Função para verificar se o usuário já está no grupo
int is_user_in_group(const char *groupName, const char *userName) {
    FILE *file = fopen("/etc/group", "r");
    if (!file) {
        perror("Falha ao abrir /etc/group");
        return -1;  // Retorna -1 se falhar ao abrir o arquivo
    }

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        char *saveptr;
        char *token = strtok_r(line, ":", &saveptr);
        if (token && strcmp(token, groupName) == 0) {
            token = strtok_r(NULL, ":", &saveptr); // Skip password
            token = strtok_r(NULL, ":", &saveptr); // Skip GID
            token = strtok_r(NULL, ":", &saveptr); // User list
            if (token) {
                token = strtok_r(token, "\n", &saveptr); // Remove newline from user list
                char *member = strtok_r(token, ",", &saveptr);
                while (member) {
                    if (strcmp(member, userName) == 0) {
                        fclose(file);
                        return 1; // User is a member of the group
                    }
                    member = strtok_r(NULL, ",", &saveptr);
                }
            }
            break; // Group found, but user not in group
        }
    }

    fclose(file);
    return 0; // User not in group
}

