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
    // Verifica se o número de argumentos é correto
    if (argc > 2) {
        fprintf(stderr, "Uso: %s [-a]\n", argv[0]);
        return 1;
    }

    // Verifica se o diretório de tarefas existe
    struct stat st = {0};
    if (stat(TASK_DIR, &st) == -1) {
        fprintf(stderr, "O diretório de tarefas não existe.\n");
        return 1;
    }

    // Obtém informações do usuário
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


    // Monta o caminho do arquivo
    char filepath[256];
    sprintf(filepath, "%scm%d.txt", TASK_DIR, random_number);

    // Abre o arquivo de tarefa para escrita
    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        perror("Falha ao abrir arquivo de tarefa");
        return 1;
    }
    chmod(filepath, 0004); // Define permissões para o arquivo

    // Escreve no arquivo baseado nos argumentos
    fprintf(file, "tarefa: concordia-listar\n");
    fprintf(file, "origem: %s\n", user);

    // Verifica se o argumento '-a' foi fornecido
    if (argc == 2 && strcmp(argv[1], "[-a]") == 0) {
        fprintf(file, "some/all: all\n"); // Usuário especificou a opção -a para listar todos
    } else {
        fprintf(file, "some/all: some\n"); // Senão, lista apenas alguns
    }

    fclose(file); // Fecha o arquivo após terminar de escrever

    printf("Tarefa enviada com sucesso.\n");

    return 0;
}






/*#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#define MESSAGES_DIR "/home/%s/message/"  // Caminho para o diretório de mensagens
#define READ_PREFIX "lido_"  // Prefixo para arquivos de mensagens lidas

// Checa se a mensagem foi lida verificando o prefixo no nome do arquivo
int is_message_read(const char *filename) {
    return strncmp(filename, READ_PREFIX, strlen(READ_PREFIX)) == 0;
}

// Função para listar mensagens
void list_messages(int all) {
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        fprintf(stderr, "Failed to get user info.\n");
        exit(EXIT_FAILURE);
    }

    char path[256];
    snprintf(path, sizeof(path), MESSAGES_DIR, "core");

    DIR *dir;
    struct dirent *entry;
    struct stat filestat;
    char filepath[512];

    if ((dir = opendir(path)) == NULL) {
        perror("Failed to open messages directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')  // Ignora arquivos ocultos e diretórios especiais . e ..
            continue;

        snprintf(filepath, sizeof(filepath), "%s%s", path, entry->d_name);
        stat(filepath, &filestat);

        // Se não for -a, lista somente as mensagens não lidas
        if (!all && is_message_read(entry->d_name)) {
            continue;
        }

        printf("Message ID: %s, Size: %ld bytes, Last Modified: %ld\n", entry->d_name, filestat.st_size, filestat.st_mtime);
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    int all = 0;

    if (argc > 1 && strcmp(argv[1], "-a") == 0) {
        all = 1;
    }

    list_messages(all);

    return EXIT_SUCCESS;
}
*/