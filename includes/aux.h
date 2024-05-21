// src/aux.h
#ifndef AUX_H
#define AUX_H

// Define os diretórios e prefixos usados
#define INPUT_DIR "/var/queue/new/"
#define OUTPUT_DIR_BASE "/home/"
#define READ_PREFIX "lido_"

// Declaração das funções auxiliares
void log_message(const char *message);
void mark_message_as_read(const char *filepath);
void read_and_display_message(const char *filepath);
void delete_file(const char *file_path);
void delete_all_files_in_directory(const char *directory_path);
int get_sender_from_message(const char *message_path, char *sender);
int send_response(const char *dest, const char *response, const char *origin);
int check_group_member(const char *groupName, const char *userName);
int check_group_and_first_member(const char *groupName, const char *userName);
char** fetch_group_members(const char *groupName);
int is_user_in_group(const char *groupName, const char *userName);

#endif // AUX_H
