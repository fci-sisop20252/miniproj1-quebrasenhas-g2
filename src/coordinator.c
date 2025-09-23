#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) {
        total *= charset_len;
    }
    return total;
}

void index_to_password(long long index, const char *charset, int charset_len, 
                       int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

int main(int argc, char *argv[]) {
    // 1. Validar argumentos de entrada
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <hash_md5> <tamanho> <charset> <num_workers>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s \"900150983cd24fb0d6963f7d28e17f72\" 3 \"abc\" 4\n", argv[0]);
        exit(1);
    }
    
    // Parsing dos argumentos
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);
    
    // Validações dos parâmetros
    if (password_len < 1 || password_len > 10) {
        fprintf(stderr, "Erro: Tamanho da senha deve estar entre 1 e 10.\n");
        exit(1);
    }
    if (num_workers < 1 || num_workers > MAX_WORKERS) {
        fprintf(stderr, "Erro: Número de workers deve estar entre 1 e %d.\n", MAX_WORKERS);
        exit(1);
    }
    if (charset_len == 0) {
        fprintf(stderr, "Erro: Charset não pode ser vazio.\n");
        exit(1);
    }

    // Calcular espaço de busca total
    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n", total_space);
    
    unlink(RESULT_FILE);
    time_t start_time = time(NULL);
    
    // 2. Dividir o espaço de busca entre os workers
    long long passwords_per_worker = total_space / num_workers;
    long long remaining_passwords = total_space % num_workers;
    
    pid_t workers_pids[MAX_WORKERS];
    long long current_start_index = 0;

    printf("Iniciando %d workers...\n", num_workers);
    
    // Loop para criar os workers
    for (int i = 0; i < num_workers; i++) {
        long long chunk_size = passwords_per_worker + (i < remaining_passwords ? 1 : 0);
        if (chunk_size == 0) continue;

        long long start_index = current_start_index;
        long long end_index = start_index + chunk_size - 1;
        current_start_index += chunk_size;

        char start_pass[password_len + 1];
        char end_pass[password_len + 1];
        index_to_password(start_index, charset, charset_len, password_len, start_pass);
        index_to_password(end_index, charset, charset_len, password_len, end_pass);
        
        // 3. Criar processo filho com fork()
        pid_t pid = fork();

        if (pid < 0) { // Erro
            perror("Erro no fork()");
            exit(EXIT_FAILURE);
        } 
        else if (pid == 0) { // 4. Processo FILHO
            // Preparar argumentos para o execl
            char worker_id_str[10];
            char password_len_str[10];
            snprintf(worker_id_str, 10, "%d", i);
            snprintf(password_len_str, 10, "%d", password_len);

            // 5. Executar o worker com execl()
            execl("./worker", "worker", target_hash, start_pass, end_pass, charset, password_len_str, worker_id_str, NULL);
            
            // Se execl retornar, significa que houve um erro
            perror("Erro no execl()");
            exit(EXIT_FAILURE);
        } 
        else { // 6. Processo PAI
            workers_pids[i] = pid; // Armazena o PID do filho
        }
    }
    
    // 7. Aguardar todos os workers terminarem
    for (int i = 0; i < num_workers; i++) {
        waitpid(workers_pids[i], NULL, 0); 
    }
    
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);
    
    printf("\nProcessamento Concluído. Tempo total: %.2f segundos\n", elapsed_time);
    
    // 8. Verificar o resultado
    FILE *result_file = fopen(RESULT_FILE, "r");
    if (result_file != NULL) {
        char line_buffer[256];
        if (fgets(line_buffer, sizeof(line_buffer), result_file)) {
            int worker_id;
            char found_password[password_len + 2];
            sscanf(line_buffer, "%d:%s", &worker_id, found_password);
            printf("\n>>> Sucesso! Senha encontrada pelo Worker %d: %s\n", worker_id, found_password);
        }
        fclose(result_file);
    } else {
        printf("\n>>> Senha não encontrada no espaço de busca definido.\n");
    }
    
    return 0;
}
