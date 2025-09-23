#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include "hash_utils.h"

/**
 * PROCESSO TRABALHADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 */

#define RESULT_FILE "password_found.txt"
#define PROGRESS_INTERVAL 100000

/**
 * Incrementa uma senha para a próxima na ordem lexicográfica.
 */
int increment_password(char *password, const char *charset, int charset_len, int password_len) {
    for (int i = password_len - 1; i >= 0; i--) {
        char *char_ptr = strchr(charset, password[i]);
        int charset_index = (char_ptr == NULL) ? -1 : (char_ptr - charset);

        if (charset_index < charset_len - 1) {
            password[i] = charset[charset_index + 1];
            return 1;
        } else {
            password[i] = charset[0];
        }
    }
    return 0; // Overflow
}

/**
 * Compara duas senhas lexicograficamente.
 */
int password_compare(const char *a, const char *b) {
    return strcmp(a, b);
}

/**
 * Verifica se o arquivo de resultado já existe.
 */
int check_result_exists() {
    return access(RESULT_FILE, F_OK) == 0;
}

/**
 * Salva a senha encontrada no arquivo de resultado.
 */
void save_result(int worker_id, const char *password) {
    int fd = open(RESULT_FILE, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd < 0) {
        return;
    }

    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "%d:%s\n", worker_id, password);
    if (len > 0) {
        write(fd, buffer, len);
    }
    close(fd);

    // Este printf pode não aparecer se o coordinator terminar primeiro.
    // printf("[Worker %d] SENHA ENCONTRADA: %s.\n", worker_id, password);
}

/**
 * Função principal do worker.
 */
int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Uso interno: %s <hash> <start> <end> <charset> <len> <id>\n", argv[0]);
        return 1;
    }

    const char *target_hash = argv[1];
    const char *start_password = argv[2];
    const char *end_password = argv[3];
    const char *charset = argv[4];
    int password_len = atoi(argv[5]);
    int worker_id = atoi(argv[6]);
    int charset_len = strlen(charset);

    // Buffer para a senha atual (tamanho dinâmico + 1 para o '\0')
    char current_password[password_len + 1];
    strcpy(current_password, start_password);

    char computed_hash[33];
    long long passwords_checked = 0;
    time_t start_time = time(NULL);

    // Loop principal de verificação
    // CORREÇÃO: O loop agora respeita a senha final designada.
    while (password_compare(current_password, end_password) <= 0) {
        if (passwords_checked > 0 && passwords_checked % PROGRESS_INTERVAL == 0) {
            if (check_result_exists()) {
                // printf("[Worker %d] Outro worker encontrou a senha. Encerrando.\n", worker_id);
                break;
            }
        }

        md5_string(current_password, computed_hash);

        if (strcmp(computed_hash, target_hash) == 0) {
            save_result(worker_id, current_password);
            return 0; // Sucesso, encontrou a senha
        }

        // Se a senha atual é a última do intervalo, paramos aqui
        if (password_compare(current_password, end_password) == 0) {
            break;
        }

        // Tenta incrementar para a próxima senha
        if (!increment_password(current_password, charset, charset_len, password_len)) {
            // Se não conseguiu incrementar, significa que chegamos ao fim de todas as combinações
            break; 
        }

        passwords_checked++;
    }

    time_t end_time = time(NULL);
    double total_time = difftime(end_time, start_time);

    fprintf(stderr, "[Worker %d] Finalizado. Verificou %lld senhas em %.2fs.\n",
            worker_id, passwords_checked, total_time);

    return 0;
}
