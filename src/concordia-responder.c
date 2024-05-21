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
    if (argc < 3) {
        fprintf(stderr, "Uso: %s destino mensagem\n", argv[0]);
        return 1;
    }

    struct stat st = {0};

    // Verifica se o diretório de tarefas existe
    if (stat(TASK_DIR, &st) == -1) {
        perror("nao ha diretoria");
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
    sprintf(filepath, "%srm%d.txt", TASK_DIR, random_number);

    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        perror("Falha ao abrir arquivo de tarefa");
        return 1;
    }

    chmod(filepath, 0004); // que os outro podem ler (neste caso o deamon), mas nao alterar nada la dentro

    fprintf(file, "tarefa: concordia-enviar\n");
    fprintf(file, "origem: %s\n", user);
    fprintf(file, "id_message: %s\n", argv[1]);
    fprintf(file, "message: \"%s\"\n", argv[2]);


    printf( "tarefa: concordia-enviar\n");
    printf("origem: %s\n", user);
    printf("destinatario: %s\n", argv[1]);
    printf("mensagem: \"%s\"\n", argv[2]);

    fclose(file);

    printf("Tarefa enviada com sucesso.\n");

    return 0;
}