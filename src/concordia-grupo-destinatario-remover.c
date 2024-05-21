#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
#include <sys/stat.h>

#define TASK_DIR "/var/queue/new/"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s userDelete groupName\n", argv[0]);
        return 1;
    }

    struct stat st = {0};
    if (stat(TASK_DIR, &st) == -1) {
        fprintf(stderr, "O diretório de tarefas não existe.\n");
        return 1;
    }

    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    if (pw == NULL) {
        perror("Falha ao obter informações do usuário");
        return 1;
    }
    char *user = pw->pw_name;

    // Inicializa o gerador de números aleatórios
    srand(time(NULL));
    int random_number = rand();  // Gera um número aleatório


    char filepath[256];
    sprintf(filepath, "%sgr%d.txt", TASK_DIR, random_number);

    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        perror("Falha ao abrir arquivo de tarefa");
        return 1;
    }
    chmod(filepath, 0004);

    fprintf(file, "tarefa: concordia-grupo-destinatario-remover\n");
    fprintf(file, "origem: %s\n", user);
    fprintf(file, "groupName: %s\n", argv[2]);
    fprintf(file, "user: %s\n", argv[1]);

    fclose(file);

    printf("Tarefa enviada com sucesso.\n");

    return 0;
}