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
#include <signal.h>
#include "aux.h"
#include "task.h"
#include "groupTask.h"

#define INPUT_DIR "/var/queue/new/"
#define OUTPUT_DIR_BASE "/home/"
#define READ_PREFIX "lido_"

// Processa a tarefa com base no nome do arquivo
void process_task(const char *filename) {
    const char *basename = strrchr(filename, '/');
    basename = (basename ? basename + 1 : filename);  // Obtém apenas o nome do arquivo

    if (strncmp(basename, "ce", 2) == 0) { // enviar msg
        process_send_task(filename);
    } else if (strncmp(basename, "cl", 2) == 0) { // ver msg
        process_read_task(filename);
    } else if (strncmp(basename, "ca", 2) == 0) { // montar mail
        process_ative_task(filename);
    } else if (strncmp(basename, "cd", 2) == 0) { // apagar mail
        process_desative_task(filename);
    } else if (strncmp(basename, "cr", 2) == 0) { // remover msg
        process_remove_task(filename);
    } else if (strncmp(basename, "cm", 2) == 0) { // mostrar/listar msg
        process_listar_task(filename);
    } else if (strncmp(basename, "rm", 2) == 0) { // responde msg
        process_responde_task(filename);
    } else if (strncmp(basename, "gc", 2) == 0) { // criar grupo
        process_createG_task(filename);
    } else if (strncmp(basename, "gd", 2) == 0) { // delete grupo
        process_deleteG_task(filename);
    } else if (strncmp(basename, "gl", 2) == 0) { // listar grupo
        process_listarG_task(filename);
    } else if (strncmp(basename, "ga", 2) == 0) { // adicionar pessoa ao grupo
        process_addG_task(filename);
    } else if (strncmp(basename, "gr", 2) == 0) { // remover pessoa do grupo
        process_removeG_task(filename);
    } else {
        char log_message_buffer[256];
        snprintf(log_message_buffer, sizeof(log_message_buffer), "Unknown task type from file: %s", basename);
        log_message(log_message_buffer);
    }
}

// Função de daemonização simplificada
void daemonize() {
    pid_t pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE); // Falha ao criar o processo filho
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS); // Termina o processo pai
    }

    // Torna o processo filho um novo líder de sessão
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    // Ignora sinais específicos
    signal(SIGCHLD, SIG_IGN); // Ignora os sinais filhos
    signal(SIGHUP, SIG_IGN);  // Ignora o sinal de hangup

    // Garante que não haja um segundo fork
    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0); // Altera a máscara de modo de arquivo para garantir permissões completas para novos arquivos

    // Muda o diretório de trabalho para a raiz
    if (chdir("/") < 0) {
        exit(EXIT_FAILURE);
    }

    // Fecha os descritores de arquivo padrão
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main() {
    daemonize(); // Torna o processo um daemon

    // Registra que o daemon foi iniciado
    log_message("Daemon iniciado");

    DIR *dir;
    struct dirent *entry;
    struct stat filestat;
    char filepath[512];

    // Salva a umask atual
    mode_t old_umask = umask(0);

    // Loop infinito para o daemon
    while (1) {
        // Se não existe, cria o diretório
        if (mkdir(INPUT_DIR, 0703) == -1) {
            //log_message("A pasta da fila existe");
        }
        

        if ((dir = opendir(INPUT_DIR)) != NULL) {
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_name[0] == '.' || entry->d_name[1] == '.') continue;

                snprintf(filepath, sizeof(filepath), "%s%s", INPUT_DIR, entry->d_name);
                if (stat(filepath, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
                    process_task(filepath);
                }
            }
            closedir(dir);
        }

        // Aguarde um segundo antes de verificar novamente
        sleep(1);
    }

    // Restaura a umask anterior
    umask(old_umask);

    return 0;
}
