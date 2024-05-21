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
#include "aux.h"

#define INPUT_DIR "/var/queue/new/"
#define OUTPUT_DIR_BASE "/home/"
#define READ_PREFIX "lido_"

/////////////////////////////////////////////////////////////////// ENVIAR //////////////////////////////////////////////////////////////////////////////////

void process_send_task(const char *filename) {
    char log_msg[1024];
    snprintf(log_msg, sizeof(log_msg), "Processing send task for file: %s", filename);
    log_message(log_msg);
    
    char task[256], origin[256], dest_dir[256], message[512], output_dir[512];
    FILE *fp = fopen(filename, "r");
    if (!fp) return;

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\ndestinatario: %s\nmensagem: \"%[^\"]\"", task, origin, dest_dir, message) != 4) {
        log_message("Erro ao ler as informações do arquivo.");
        fclose(fp);
        return;
    }
    fclose(fp);
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date_str[64];
    strftime(date_str, sizeof(date_str)-1, "%Y-%m-%d %H:%M:%S", t);


    // Verifica se o destinatário começa com "G_"
    if (strncmp(dest_dir, "G_", 2) == 0) {
        // Remove as primeiras duas letras de dest_dir e armazena em uma variável temporária
        char group_name[256];
        strncpy(group_name, dest_dir + 2, sizeof(group_name) - 1);
        group_name[sizeof(group_name) - 1] = '\0'; // Garante a terminação nula
        // Verifica se o usuário da origem pertence ao grupo
        if (is_user_in_group(group_name, origin)) {
            // Obtém os membros do grupo
            char **members = fetch_group_members(group_name);
            if (members) {
                for (int i = 0; members[i] != NULL; i++) {
                    // Monta o diretório de destino para cada membro
                    snprintf(output_dir, sizeof(output_dir), "%s%s/grupos/%s", OUTPUT_DIR_BASE, members[i], dest_dir);
                    mkdir(output_dir, 0700); // Cria o diretório se não existir

                    // Usar basename para extrair o nome base do arquivo
                    char *filename_copy = strdup(filename); // strdup para evitar modificar entrada original
                    char *filename_only = basename(filename_copy);

                    // Construir o caminho do novo arquivo de mensagem de forma segura
                    char filepath[1024];
                    snprintf(filepath, sizeof(filepath), "%s/%s_%s", output_dir, dest_dir, filename_only);

                    // Cria o arquivo de mensagem no diretório de destino
                    FILE *fp_member = fopen(filepath, "w");
                    if (fp_member) {
                        //fprintf(fp_member, "From: %s\nTo: %s\nMessage: %s\n", origin, dest_dir, message);
                        fprintf(fp_member, "From: %s\nTo: %s\nDate: %s\nMessage: %s\n", origin, dest_dir, date_str, message);
                        fclose(fp_member);
                        chmod(filepath, 0400);
                    } else {
                        perror("Falha ao abrir arquivo de mensagem");
                    }

                    free(filename_copy); // Libera a memória alocada
                }

                // Libera a memória alocada para os membros do grupo
                for (int i = 0; members[i] != NULL; i++) {
                    free(members[i]);
                }
                free(members);
            }
        } else {
            snprintf(log_msg, sizeof(log_msg), "Usuário %s não pertence ao grupo %s", origin, dest_dir);
            log_message(log_msg);
        }
    } else {
        // Monta o diretório de destino baseado no arquivo de tarefa
        snprintf(output_dir, sizeof(output_dir), "%s/%s/mail", OUTPUT_DIR_BASE, dest_dir);

        mkdir(output_dir, 0700); // Cria o diretório se não existir

        // Usar basename para extrair o nome base do arquivo
        char *filename_copy = strdup(filename); // strdup para evitar modificar entrada original
        char *filename_only = basename(filename_copy);

        // Construir o caminho do novo arquivo de mensagem de forma segura
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", output_dir, filename_only);

        // Cria o arquivo de mensagem no diretório de destino
        FILE *fp = fopen(filepath, "w");
        if (fp) {
            //fprintf(fp, "From: %s\nTo: %s\nMessage: %s\n", origin, dest_dir, message);
            fprintf(fp, "From: %s\nTo: %s\nDate: %s\nMessage: %s\n", origin, dest_dir, date_str, message);
            fclose(fp);
            chmod(filepath, 0400);
        } else {
            perror("Falha ao abrir arquivo de mensagem");
        }

        free(filename_copy); // Libera a memória alocada
    }

    remove(filename);
}

/////////////////////////////////////////////////////////////////// LER /////////////////////////////////////////////////////////////////////////////////

void process_read_task(const char *filename) {
    char log_msg[1024];
    snprintf(log_msg, sizeof(log_msg), "Processing read task for file: %s", filename);
    log_message(log_msg);

    char task[256], origin[256], id_message[256];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Falha ao abrir o arquivo");
        return;
    }

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\nid_mensagem: %s\n", task, origin, id_message) == 3) {
        snprintf(log_msg, sizeof(log_msg), "Origem: %s\nId Mensagem: %s", origin, id_message);
        log_message(log_msg);
    } else {
        log_message("Erro ao ler as informações do arquivo.");
        fclose(fp);
        return;
    }
    fclose(fp);
    

    char message_file_path[1024];

    // Verifica se o id da mensagem começa com "lido_G_" ou "G_"
    if (strncmp(id_message, "lido_G_", 7) == 0 || strncmp(id_message, "G_", 2) == 0) {
        char *group_start = id_message;

        // Ajusta o ponteiro se começar com "lido_"
        if (strncmp(id_message, "lido_", 5) == 0) {
            group_start = id_message + 5;
        }

        // Verifica se a parte após "lido_" começa com "G_"
        if (strncmp(group_start, "G_", 2) == 0) {
            char *first_underscore = strstr(group_start, "_");
            if (first_underscore) {
                char *second_underscore = strstr(first_underscore + 1, "_");
                if (second_underscore) {
                    // Calcula o comprimento da parte G_NOMEDOGRUPO
                    size_t length = second_underscore - group_start;
                    char group_id[256];
                    strncpy(group_id, group_start, length);
                    group_id[length] = '\0'; // Garante a terminação nula

                    snprintf(log_msg, sizeof(log_msg), "Grupo ID: %s", group_id);
                    log_message(log_msg);
                    snprintf(message_file_path, sizeof(message_file_path), "%s%s/grupos/%s/%s", OUTPUT_DIR_BASE, origin, group_id, id_message);
                    read_and_display_message(message_file_path);
                }
            }
        }
    } else {
        log_message("Id da mensagem não começa com 'G_' ou 'lido_G_'");
        // Monta o caminho para o arquivo de mensagem
        snprintf(message_file_path, sizeof(message_file_path), "%s%s/mail/%s", OUTPUT_DIR_BASE, origin, id_message);
        read_and_display_message(message_file_path);
    }

    remove(filename);
}

/////////////////////////////////////////////////////////////////// ATIVAR ////////////////////////////////////////////////////////////////////

void process_ative_task(const char *filename) {
    char log_msg[2080];
    snprintf(log_msg, sizeof(log_msg), "Processing ative task for file: %s", filename);
    log_message(log_msg);

    char task[256], origin[256];
    FILE *fp = fopen(filename, "r");
    if (!fp) return;

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\n", task, origin) == 2) {
        snprintf(log_msg, sizeof(log_msg), "tarefa: %s\norigem: %s", task, origin);
        log_message(log_msg);
    } else {
        log_message("Erro ao ler as informações do arquivo.");
    }
    fclose(fp);

    // Monta o caminho para o arquivo de mensagem
    char message_file_path[2048];
    snprintf(message_file_path, sizeof(message_file_path), "%s%s/mail/", OUTPUT_DIR_BASE, origin);

    // Cria o diretório se ele não existir
    struct stat st = {0};
    if (stat(message_file_path, &st) == -1) {
        if (mkdir(message_file_path, 0700) == -1) { //so o deamon é que pode mexer nisto mas os outro podem listar
            perror("Failed to create directory");
            return;
        }
    } else {
        snprintf(log_msg, sizeof(log_msg), "A pasta %s ja existe", message_file_path);
        log_message(log_msg);
    }
    // Monta o caminho para o arquivo de mensagem
    char group_file_path[1024];
    snprintf(group_file_path, sizeof(group_file_path), "%s%s/grupos/", OUTPUT_DIR_BASE, origin);

    // Cria o diretório se ele não existir
    struct stat stt = {0};
    if (stat(group_file_path, &stt) == -1) {
        if (mkdir(group_file_path, 0700) == -1) { //so o deamon é que pode mexer nisto mas os outro podem listar
            perror("Failed to create directory");
            return;
        }
    } else {
        snprintf(log_msg, sizeof(log_msg), "A pasta %s ja existe", group_file_path);
        log_message(log_msg);
    }
    remove(filename);
}

/////////////////////////////////////////////////////////////////// DESATIVAR /////////////////////////////////////////////////////////////////// 

void process_desative_task(const char *filename) {
    char log_msg[2080];
    snprintf(log_msg, sizeof(log_msg), "Processing desative task for file: %s", filename);
    log_message(log_msg);

    char task[256], origin[256];
    FILE *fp = fopen(filename, "r");
    if (!fp) return;

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\n", task, origin) == 2) {
        snprintf(log_msg, sizeof(log_msg), "tarefa: %s\norigem: %s", task, origin);
        log_message(log_msg);
    } else {
        log_message("Erro ao ler as informações do arquivo.");
    }
    fclose(fp);

    // Monta o caminho para o arquivo de mensagem
    char message_file_path[1024];
    snprintf(message_file_path, sizeof(message_file_path), "%s%s/mail/", OUTPUT_DIR_BASE, origin);

    delete_all_files_in_directory(message_file_path);

    // Tenta remover o diretório de mensagens
    if (rmdir(message_file_path) == 0) {
        snprintf(log_msg, sizeof(log_msg), "Directory deleted successfully: %s", message_file_path);
        log_message(log_msg);
    } else {
        perror("Failed to delete directory");
    }

    // Monta o caminho para o diretório de grupos
    char groups_directory_path[1024];
    snprintf(groups_directory_path, sizeof(groups_directory_path), "%s%s/grupos/", OUTPUT_DIR_BASE, origin);

    // Abre o diretório de grupos
    DIR *dir = opendir(groups_directory_path);
    struct dirent *entry;

    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            char path[2048];
            snprintf(path, sizeof(path), "%s/%s", groups_directory_path, entry->d_name);
            struct stat path_stat;
            stat(path, &path_stat);

            if (S_ISDIR(path_stat.st_mode)) { // Verifica se é um diretório
                // Ignora os diretórios "." e ".."
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }

                char subdir_path[2048];
                snprintf(subdir_path, sizeof(subdir_path), "%s/%s", groups_directory_path, entry->d_name);

                // Remove todos os arquivos no subdiretório
                delete_all_files_in_directory(subdir_path);

                // Remove o subdiretório
                if (rmdir(subdir_path) == 0) {
                    snprintf(log_msg, sizeof(log_msg), "Deleted directory: %s", subdir_path);
                    log_message(log_msg);
                } else {
                    perror("Failed to delete directory");
                }
            }
        }
        closedir(dir);

        // Remove o diretório grupos
        if (rmdir(groups_directory_path) == 0) {
            snprintf(log_msg, sizeof(log_msg), "Deleted directory: %s", groups_directory_path);
            log_message(log_msg);
        } else {
            perror("Failed to delete directory");
        }
    } else {
        perror("Failed to open directory");
    }

    remove(filename);
}

/////////////////////////////////////////////////////////////////// REMOVER /////////////////////////////////////////////////////////////////// 

void process_remove_task(const char *filename) {
    char log_msg[1024];
    snprintf(log_msg, sizeof(log_msg), "Processing remove task for file: %s", filename);
    log_message(log_msg);

    char task[256], origin[256], id_message[256];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Falha ao abrir o arquivo");
        return;
    }

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\nid_mensagem: %s\n", task, origin, id_message) == 3) {
        snprintf(log_msg, sizeof(log_msg), "Origem: %s\nId Mensagem: %s", origin, id_message);
        log_message(log_msg);
    } else {
        log_message("Erro ao ler as informações do arquivo.");
        fclose(fp);
        return;
    }
    fclose(fp);

    char message_file_path[1024];

    // Verifica se o id da mensagem começa com "lido_G_" ou "G_"
    if (strncmp(id_message, "lido_G_", 7) == 0 || strncmp(id_message, "G_", 2) == 0) {
        char *group_start = id_message;

        // Ajusta o ponteiro se começar com "lido_"
        if (strncmp(id_message, "lido_", 5) == 0) {
            group_start = id_message + 5;
        }

        // Verifica se a parte após "lido_" começa com "G_"
        if (strncmp(group_start, "G_", 2) == 0) {
            char *first_underscore = strstr(group_start, "_");
            if (first_underscore) {
                char *second_underscore = strstr(first_underscore + 1, "_");
                if (second_underscore) {
                    // Calcula o comprimento da parte G_NOMEDOGRUPO
                    size_t length = second_underscore - group_start;
                    char group_id[256];
                    strncpy(group_id, group_start, length);
                    group_id[length] = '\0'; // Garante a terminação nula

                    snprintf(log_msg, sizeof(log_msg), "Grupo ID: %s", group_id);
                    log_message(log_msg);
                    snprintf(message_file_path, sizeof(message_file_path), "%s%s/grupos/%s/%s", OUTPUT_DIR_BASE, origin, group_id, id_message);
                    delete_file(message_file_path);
                }
            }
        }
    } else {
        log_message("Id da mensagem não começa com 'G_' ou 'lido_G_'");
        // Monta o caminho para o arquivo de mensagem
        snprintf(message_file_path, sizeof(message_file_path), "%s%s/mail/%s", OUTPUT_DIR_BASE, origin, id_message);
        delete_file(message_file_path);
    }
    remove(filename);
}

/////////////////////////////////////////////////////////////////// LISTAR ///////////////////////////////////////////////////////////////////

void process_listar_task(const char *filename) {
    char log_msg[2080];
    snprintf(log_msg, sizeof(log_msg), "Processing list task for file: %s", filename);
    log_message(log_msg);

    char task[256], origin[256], some_all[256];
    FILE *fp = fopen(filename, "r");
    if (!fp) return;

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\nsome/all: %s\n", task, origin, some_all) == 3) {
        snprintf(log_msg, sizeof(log_msg), "tarefa: %s\norigem: %s\nsome/all: %s", task, origin, some_all);
        log_message(log_msg);
    } else {
        log_message("Erro ao ler as informações do arquivo.");
    }
    fclose(fp);

    // Monta o caminho para o diretório de mensagens
    char message_file_path[2048];
    snprintf(message_file_path, sizeof(message_file_path), "%s%s/mail/", OUTPUT_DIR_BASE, origin);

    // Lista mensagens no diretório de mensagens do usuário
    DIR *dir = opendir(message_file_path);
    if (dir == NULL) {
        perror("Failed to open mail directory");
        return;
    }

    struct dirent *entry;
    snprintf(log_msg, sizeof(log_msg), "Listing files in: %s", message_file_path);
    log_message(log_msg);
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Ignora as entradas '.' e '..'
        }
        // Filtra arquivos com base no prefixo "ce" se 'some' foi especificado
        if (strcmp(some_all, "some") == 0) {
            if (strncmp(entry->d_name, "ce", 2) == 0) {
                snprintf(log_msg, sizeof(log_msg), "%s", entry->d_name);
                log_message(log_msg);
            }
        } else { // Caso 'all', lista todos os arquivos
            snprintf(log_msg, sizeof(log_msg), "%s", entry->d_name);
            log_message(log_msg);
        }
    }
    closedir(dir);

    // Monta o caminho para o diretório de grupos
    char groups_directory_path[1024];
    snprintf(groups_directory_path, sizeof(groups_directory_path), "%s%s/grupos/", OUTPUT_DIR_BASE, origin);

    // Lista mensagens em cada subdiretório de grupos
    dir = opendir(groups_directory_path);
    if (dir == NULL) {
        perror("Failed to open groups directory");
        return;
    }

    snprintf(log_msg, sizeof(log_msg), "Listing group messages in: %s", groups_directory_path);
    log_message(log_msg);
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' || entry->d_name[1] == '.') continue; // Ignora entradas '.' e '..'
        
        // Monta o caminho completo para o subdiretório do grupo
        char group_subdir_path[2048];
        snprintf(group_subdir_path, sizeof(group_subdir_path), "%s/%s", groups_directory_path, entry->d_name);

        DIR *subdir = opendir(group_subdir_path);
        if (subdir == NULL) {
            perror("Failed to open group subdirectory");
            continue;
        }

        struct dirent *subentry;
        snprintf(log_msg, sizeof(log_msg), "Messages in group: %s", entry->d_name);
        log_message(log_msg);
        while ((subentry = readdir(subdir)) != NULL) {
            if (strcmp(subentry->d_name, ".") == 0 || strcmp(subentry->d_name, "..") == 0) {
                continue; // Ignora as entradas '.' e '..'
            }
            // Filtra arquivos com base no prefixo "ce" se 'some' foi especificado
            if (strcmp(some_all, "some") == 0) {
                if (strncmp(subentry->d_name, "ce", 2) == 0) {
                    snprintf(log_msg, sizeof(log_msg), "  %s", subentry->d_name);
                    log_message(log_msg);
                }
            } else { // Caso 'all', lista todos os arquivos
                snprintf(log_msg, sizeof(log_msg), "  %s", subentry->d_name);
                log_message(log_msg);
            }
        }
        closedir(subdir);
    }
    closedir(dir);
    remove(filename);
}

////////////////////////////////////////////////////////////////////////   RESPONDE    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void process_responde_task(const char *filename) {
    char log_msg[2048];
    snprintf(log_msg, sizeof(log_msg), "Processing responde task for file: %s", filename);
    log_message(log_msg);

    char task[256], origin[256], id_message[256], message[512];
    FILE *fp = fopen(filename, "r");
    if (!fp) return;

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\nid_message: %s\nmessage: %s\n", task, origin, id_message, message) == 4) {
        snprintf(log_msg, sizeof(log_msg), "tarefa: %s\norigem: %s\nid_message: %s\nmessage: %s", task, origin, id_message, message);
        log_message(log_msg);
    } else {
        log_message("Erro ao ler as informações do arquivo.");
    }
    fclose(fp);

    // Monta o caminho para o arquivo de mensagem
    char message_file_path[1024];
    snprintf(message_file_path, sizeof(message_file_path), "%s%s/mail/%s", OUTPUT_DIR_BASE, origin, id_message);

    char sender[1024];
    if (get_sender_from_message(message_file_path, sender) != 0) {
        log_message("Failed to get sender from message");
        return;
    }

    if (send_response(sender, message, origin) != 0) {
        snprintf(log_msg, sizeof(log_msg), "Failed to send response to %s", sender);
        log_message(log_msg);
    }
    remove(filename);
}
