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


////////////////////////////////////////////////////////////////////////   CRIAR GRUPO   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void process_createG_task(const char *filename) {
    char log_msg[2048];
    snprintf(log_msg, sizeof(log_msg), "Processing list task for file: %s", filename);
    log_message(log_msg);

    char task[256], userName[256], groupName[256];
    char command[1024];
    char directory_path[1024];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        log_message("Failed to open file");
        return;
    }

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\ngroupName: %s\n", task, userName, groupName) == 3) {
        snprintf(log_msg, sizeof(log_msg), "tarefa: %s\norigem: %s\ngroupName: %s\n", task, userName, groupName);
        log_message(log_msg);
    } else {
        log_message("Erro ao ler as informações do arquivo.");
    }
    fclose(fp);

    if (groupName == NULL || strlen(groupName) == 0) {
        log_message("Nome do grupo não especificado.");
        return;
    }

    if (userName == NULL || strlen(userName) == 0) {
        log_message("Nome de usuário não especificado.");
        return;
    }

    // Cria o grupo
    snprintf(command, sizeof(command), "sudo groupadd %s", groupName);
    //snprintf(log_msg, sizeof(log_msg), "Comando grupo: %s", command);
    log_message(command);
    system(command);
    /*if (system(command) != 0) {
        snprintf(log_msg, sizeof(log_msg), "Falha ao criar o grupo '%s'.", groupName);
        log_message(log_msg);
        remove(filename); //talvez tire depois
        return;
    } else {
        log_message("Grupo criado");
    }*/

    // Adiciona o usuário ao grupo
    snprintf(command, sizeof(command), "sudo usermod -a -G %s %s", groupName, userName);
    //snprintf(log_msg, sizeof(log_msg), "Adicionando usuário ao grupo: %s", command);
    log_message(command);
    system(command);
    /*if (system(command) != 0) {
        snprintf(log_msg, sizeof(log_msg), "Falha ao adicionar o usuário '%s' ao grupo '%s'.", userName, groupName);
        log_message(log_msg);
        remove(filename); //talvez tire depois
        return;
    }*/

    // Cria o diretório no /home
    snprintf(directory_path, sizeof(directory_path), "/home/%s/grupos/G_%s", userName, groupName);
    snprintf(log_msg, sizeof(log_msg), "Criando diretório: %s", directory_path);
    log_message(log_msg);
    mkdir(directory_path, 0700);
    /*if (mkdir(directory_path, 0700) != 0) {
        log_message("Falha ao criar o diretório");
        remove(filename); //talvez tire depois
        return;
    }*/
    remove(filename);
}

////////////////////////////////////////////////////////////////////////   DELETE GRUPO   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void process_deleteG_task(const char *filename) {
    char log_msg[2048];
    snprintf(log_msg, sizeof(log_msg), "Processing list task for file: %s", filename);
    log_message(log_msg);

    char task[256], userName[256], groupName[256];
    char command[1024];
    char directory_path[1024];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        log_message("Failed to open file");
        return;
    }

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\ngroupName: %s\n", task, userName, groupName) == 3) {
        snprintf(log_msg, sizeof(log_msg), "tarefa: %s\norigem: %s\ngroupName: %s\n", task, userName, groupName);
        log_message(log_msg);
    } else {
        log_message("Erro ao ler as informações do arquivo.");
    }
    fclose(fp);

    if (strlen(groupName) == 0) {
        log_message("Nome do grupo não especificado.");
        return;
    }

    if (strlen(userName) == 0) {
        log_message("Nome de usuário não especificado.");
        return;
    }

    // Chama a função de verificação
    int result = check_group_and_first_member(groupName, userName); 
    if (result == 1) {
        snprintf(log_msg, sizeof(log_msg), "O grupo existe e o usuário '%s' é o primeiro membro do grupo '%s'.", userName, groupName);
        log_message(log_msg);

        char **members = fetch_group_members(groupName);
        if (members) {
            snprintf(log_msg, sizeof(log_msg), "Membros do grupo '%s':", groupName);
            log_message(log_msg);

            // Remover usuários do grupo
            for (int i = 0; members[i] != NULL; i++) {
                snprintf(directory_path, sizeof(directory_path), "/home/%s/grupos/G_%s", members[i], groupName);
                delete_all_files_in_directory(directory_path);

                // Tenta remover o arquivo especificado
                if (remove(directory_path) == 0) {
                    snprintf(log_msg, sizeof(log_msg), "File deleted successfully: %s", directory_path);
                    log_message(log_msg);
                } else {
                    log_message("Failed to delete file");
                }

                snprintf(command, sizeof(command), "sudo gpasswd -d %s %s", members[i], groupName);
                system(command);
                snprintf(log_msg, sizeof(log_msg), "%s", command);
                log_message(log_msg);
                free(members[i]); // Libera cada membro
            }
            free(members); // Libera a lista de membros
        } else {
            log_message("Nenhum membro encontrado ou erro na busca.");
        }

        // Apagar o grupo
        snprintf(command, sizeof(command), "sudo groupdel %s", groupName);
        system(command);
        snprintf(log_msg, sizeof(log_msg), "Grupo '%s' removido com sucesso.", groupName);
        log_message(log_msg);
    } else {
        snprintf(log_msg, sizeof(log_msg), "O grupo não existe ou o usuário '%s' não é o primeiro membro do grupo '%s'.", userName, groupName);
        log_message(log_msg);
    }
    remove(filename);
}

////////////////////////////////////////////////////////////////////////   LISTAR GRUPO   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void process_listarG_task(const char *filename) {
    char log_msg[2048];
    snprintf(log_msg, sizeof(log_msg), "Processing list task for file: %s", filename);
    log_message(log_msg);

    char task[256], userName[256], groupName[256];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        log_message("Failed to open file");
        return;
    }

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\ngroupName: %s\n", task, userName, groupName) == 3) {
        snprintf(log_msg, sizeof(log_msg), "tarefa: %s\norigem: %s\ngroupName: %s\n", task, userName, groupName);
        log_message(log_msg);
    } else {
        log_message("Erro ao ler as informações do arquivo.");
    }
    fclose(fp);

    if (strlen(groupName) == 0) {
        log_message("Nome do grupo não especificado.");
        return;
    }

    if (strlen(userName) == 0) {
        log_message("Nome de usuário não especificado.");
        return;
    }

    // Chama a função de verificação
    int result = check_group_member(groupName, userName); //tem de checar se te esta no grupo
    if (result == 1) {
        snprintf(log_msg, sizeof(log_msg), "O grupo existe e o usuário '%s' é membro do grupo '%s'.", userName, groupName);
        log_message(log_msg);

        char **members = fetch_group_members(groupName);
        if (members) {
            snprintf(log_msg, sizeof(log_msg), "Membros do grupo '%s':", groupName);
            log_message(log_msg);

            for (int i = 0; members[i] != NULL; i++) {
                snprintf(log_msg, sizeof(log_msg), "%s", members[i]);
                log_message(log_msg);
                free(members[i]); // Libera cada membro
            }
            free(members); // Libera a lista de membros
        } else {
            log_message("Nenhum membro encontrado ou erro na busca.");
        }
    } else {
        snprintf(log_msg, sizeof(log_msg), "O grupo não existe ou o usuário '%s' não é membro do grupo '%s'.", userName, groupName);
        log_message(log_msg);
    }
    remove(filename);
}

////////////////////////////////////////////////////////////////////////  ADICIONAR USER AO GRUPO   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void process_addG_task(const char *filename) {
    char log_msg[2048];
    snprintf(log_msg, sizeof(log_msg), "Processing list task for file: %s", filename);
    log_message(log_msg);

    char task[256], userName[256], groupName[256], userAdd[256];
    char command[1024];
    char directory_path[1024];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        log_message("Failed to open file");
        return;
    }

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\ngroupName: %s\nuser: %s\n", task, userName, groupName, userAdd) == 4) {
        snprintf(log_msg, sizeof(log_msg), "tarefa: %s\norigem: %s\ngroupName: %s\nuser: %s\n", task, userName, groupName, userAdd);
        log_message(log_msg);
    } else {
        log_message("Erro ao ler as informações do arquivo.");
    }
    fclose(fp);

    if (strlen(groupName) == 0) {
        log_message("Nome do grupo não especificado.");
        return;
    }

    if (strlen(userName) == 0) {
        log_message("Nome de usuário não especificado.");
        return;
    }

    // Chama a função de verificação
    int result = check_group_and_first_member(groupName, userName); //tem de checar se te esta no grupo
    if (result == 1) {
        snprintf(log_msg, sizeof(log_msg), "O grupo existe e o usuário '%s' é o primeiro membro do grupo '%s'.", userName, groupName);
        log_message(log_msg);

        // Verifica se o usuário já está no grupo
        int result = is_user_in_group(groupName, userAdd);
        if (result == 1) {
            snprintf(log_msg, sizeof(log_msg), "O usuário '%s' já é membro do grupo '%s'.", userAdd, groupName);
            log_message(log_msg);
        } else if (result == 0) {
            snprintf(command, sizeof(command), "sudo usermod -a -G %s %s", groupName, userAdd);
            snprintf(log_msg, sizeof(log_msg), "Adicionando usuário ao grupo: %s", command);
            log_message(log_msg);
            system(command);
            
                // Cria o diretório no /home
                snprintf(directory_path, sizeof(directory_path), "/home/%s/grupos/G_%s", userAdd, groupName);
                snprintf(log_msg, sizeof(log_msg), "Criando diretório: %s", directory_path);
                log_message(log_msg);
                if (mkdir(directory_path, 0700) != 0) {
                    log_message("Falha ao criar o diretório");
                    remove(filename); //talvez tire depois
                    return;
                }
            
        }
    } else {
        snprintf(log_msg, sizeof(log_msg), "O grupo não existe ou o usuário '%s' não é membro do grupo '%s'.", userName, groupName);
        log_message(log_msg);
    }
    remove(filename);
}

////////////////////////////////////////////////////////////////////////  REMOVER USER DO GRUPO   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void process_removeG_task(const char *filename) {
    char log_msg[2048];
    snprintf(log_msg, sizeof(log_msg), "Processing list task for file: %s", filename);
    log_message(log_msg);

    char task[256], userName[256], groupName[256], userDel[256];
    char command[1024];
    char directory_path[1024];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        log_message("Failed to open file");
        return;
    }

    // Lê as informações do arquivo
    if (fscanf(fp, "tarefa: %s\norigem: %s\ngroupName: %s\nuser: %s\n", task, userName, groupName, userDel) == 4) {
        snprintf(log_msg, sizeof(log_msg), "tarefa: %s\norigem: %s\ngroupName: %s\nuser: %s\n", task, userName, groupName, userDel);
        log_message(log_msg);
    } else {
        log_message("Erro ao ler as informações do arquivo.");
    }
    fclose(fp);

    if (strlen(groupName) == 0) {
        log_message("Nome do grupo não especificado.");
        return;
    }

    if (strlen(userName) == 0) {
        log_message("Nome de usuário não especificado.");
        return;
    }

    // Chama a função de verificação
    int result = check_group_and_first_member(groupName, userName); //tem de checar se te esta no grupo
    if (result == 1) {
        snprintf(log_msg, sizeof(log_msg), "O grupo existe e o usuário '%s' é o primeiro membro do grupo '%s'.", userName, groupName);
        log_message(log_msg);

        // Verifica se o usuário já está no grupo
        int result = is_user_in_group(groupName, userDel);
        if (result == 1) {
            snprintf(command, sizeof(command), "sudo deluser %s %s", userDel, groupName);
            snprintf(log_msg, sizeof(log_msg), "Apagar usuário ao grupo: %s", command);
            log_message(log_msg);

            if (system(command) != 0) {
                snprintf(log_msg, sizeof(log_msg), "Falha ao remover o usuário '%s' ao grupo '%s'.", userDel, groupName);
                log_message(log_msg);
            }
            snprintf(log_msg, sizeof(log_msg), "O usuário '%s' deixou é membro do grupo '%s'.", userDel, groupName);
            log_message(log_msg);

            snprintf(directory_path, sizeof(directory_path), "/home/%s/grupos/G_%s", userDel, groupName);
            delete_all_files_in_directory(directory_path);

            // Tenta remover o arquivo especificado
            if (remove(directory_path) == 0) {
                snprintf(log_msg, sizeof(log_msg), "File deleted successfully: %s", directory_path);
                log_message(log_msg);
            } else {
                log_message("Failed to delete file");
            }
        } else if (result == 0) {
            snprintf(log_msg, sizeof(log_msg), "O usuário '%s' nao faz parte do grupo desde o inicio '%s'.", userDel, groupName);
            log_message(log_msg);
        }
    } else {
        snprintf(log_msg, sizeof(log_msg), "O grupo não existe ou o usuário '%s' não é membro do grupo '%s'.", userName, groupName);
        log_message(log_msg);
    }
    remove(filename);
}
