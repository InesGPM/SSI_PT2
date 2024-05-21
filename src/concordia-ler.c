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
        fprintf(stderr, "Uso: %s id_mensagem\n", argv[0]);
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
    sprintf(filepath, "%scl%d.txt", TASK_DIR, random_number);

    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        perror("Falha ao abrir arquivo de tarefa");
        return 1;
    }
    chmod(filepath, 0004);

    fprintf(file, "tarefa: concordia-ler\n");
    fprintf(file, "origem: %s\n", user);
    fprintf(file, "id_mensagem: %s\n", argv[1]);

    fclose(file);

    printf("Tarefa enviada com sucesso.\n");

    return 0;
}









/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <libgen.h>  // Necessário para funções dirname e basename

#define READ_PREFIX "lido_"

// Função para renomear o arquivo e marcar como lido
void mark_message_as_read(const char *file_path) {
    char new_path[512];
    char *dir = dirname(strdup(file_path));  // Obter diretório do arquivo
    char *base = basename(strdup(file_path));  // Obter nome base do arquivo

    snprintf(new_path, sizeof(new_path), "%s/%s%s", dir, READ_PREFIX, base);  // Constrói o novo caminho com o prefixo

    if (rename(file_path, new_path) != 0) {
        perror("Error marking message as read");
    } else {
        printf("Message marked as read: %s\n", new_path);
    }
}

// Função para ler uma mensagem
void read_message(const char *message_id) {
    struct passwd *pw = getpwuid(getuid());  // Obter informações do usuário atual
    char file_path[512];

    if (pw == NULL) {
        fprintf(stderr, "Failed to get user info.\n");
        return;
    }

    // Constrói o caminho do arquivo a partir do diretório home do usuário
    snprintf(file_path, sizeof(file_path), "/home/user1/mail/%s.txt", message_id);

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open message file at %s\n", file_path);
        perror("Reason");  // Mostra o motivo do erro ao abrir o arquivo
        return;
    }

    // Lê e exibe o conteúdo da mensagem
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    fclose(file);

    // Marcar a mensagem como lida após ser exibida
    mark_message_as_read(file_path);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [message_id]\n", argv[0]);
        return EXIT_FAILURE;
    }

    read_message(argv[1]);

    return EXIT_SUCCESS;
}
*/