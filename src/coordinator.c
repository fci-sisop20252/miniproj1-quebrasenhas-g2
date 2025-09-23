#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

/**
 * PROCESSO COORDENADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 * * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 * * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 * * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 * * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 * * @param charset_len Tamanho do conjunto de caracteres
 * @param password_len Comprimento da senha
 * @return Número total de combinações possíveis
 */
long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) {
        total *= charset_len;
    }
    return total;
}

/**
 * Converte um índice numérico para uma senha
 * Usado para definir os limites de cada worker
 * * @param index Índice numérico da senha
 * @param charset Conjunto de caracteres
 * @param charset_len Tamanho do conjunto
 * @param password_len Comprimento da senha
 * @param output Buffer para armazenar a senha gerada
 */
void index_to_password(long long index, const char *charset, int charset_len, 
                       int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

/**
 * Função principal do coordenador
 */
int main(int argc, char *argv[]) {
    // TODO 1: Validar argumentos de entrada
    // Verificar se argc == 5 (programa + 4 argumentos)
    // Se não, imprimir mensagem de uso e sair com código 1
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <hash_md5> <tamanho> <charset> <num_workers>\n", argv[0]);
        exit(1);
    }
    
    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);
    
    // TODO 2: Adicionar validações dos parâmetros
    // - password_len deve estar entre 1 e 10
    // - num_workers deve estar entre 1 e MAX_WORKERS
    // - charset não pode ser vazio
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
    printf("Espaço de busca total: %lld combinações\n\n", total_space);
    
    // Remover arquivo de resultado anterior se existir
    unlink(RESULT_FILE);
    
    // Registrar tempo de início
    time_t start_time = time(NULL);
    
    // TODO 2: Dividir o espaço de busca entre os workers
    // Calcular quantas senhas cada worker deve verificar
    // DICA: Use divisão inteira e distribua o resto entre os primeiros workers
    long long passwords_per_worker = total_space / num_workers;
    long long remaining_passwords = total_space % num_workers;
    
    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];
    long long current_index = 0;
    
    // TODO 3: Criar os processos workers usando fork()
    for (int i = 0; i < num_workers; i++) {
        // TODO: Calcular intervalo de senhas para este worker
        long long chunk_size = passwords_per_worker + (i < remaining_passwords ? 1 : 0);
        if (chunk_size == 0) continue;

        long long start_index = current_index;
        long long end_index = current_index + chunk_size - 1;
        current_index += chunk_size;

        // TODO: Converter indices para senhas de inicio e fim
        char start_pass[password_len + 1];
        char end_pass[password_len + 1];
        index_to_password(start_index, charset, charset_len, password_len, start_pass);
        index_to_password(end_index, charset, charset_len, password_len, end_pass);

        // TODO 4: Usar fork() para criar processo filho
        pid_t pid = fork();

        if (pid < 0) {
            // TODO 7: Tratar erros de fork()
            perror("Erro no fork()");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // TODO 6: No processo filho: usar execv() para executar worker
            char *worker_args[] = {
                "./worker", (char*)target_hash, (char*)charset,
                start_pass, end_pass, RESULT_FILE, NULL
            };
            execv("./worker", worker_args);
            
            // Se execv retornar, significa que houve um erro
            perror("Erro no execv do worker");
            exit(EXIT_FAILURE);

        } else {
            // TODO 5: No processo pai: armazenar PID
            workers[i] = pid;
            printf("Pai: Worker %d criado com PID %d.\n", i, workers[i]);
        }
    }
    
    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");
    
    // TODO 8: Aguardar todos os workers terminarem usando wait()
    // IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis
    
    // IMPLEMENTE AQUI:
    // - Loop para aguardar cada worker terminar
    // - Usar wait() para capturar status de saída
    // - Identificar qual worker terminou
    // - Verificar se terminou normalmente ou com erro
    // - Contar quantos workers terminaram
    for (int i = 0; i < num_workers; i++) {
        int status;
        pid_t terminated_pid = wait(&status);

        if (terminated_pid > 0) {
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                printf("Worker com PID %d terminou com status de saída %d.\n", terminated_pid, exit_code);
            } else if (WIFSIGNALED(status)) {
                int signal_num = WTERMSIG(status);
                printf("Worker com PID %d foi terminado pelo sinal %d.\n", terminated_pid, signal_num);
            }
        } else {
            perror("Erro ao aguardar worker");
        }
    }
    
    // Registrar tempo de fim
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);
    
    printf("\n=== Resultado ===\n");
    
    // TODO 9: Verificar se algum worker encontrou a senha
    // Ler o arquivo password_found.txt se existir
    
    // IMPLEMENTE AQUI:
    // - Abrir arquivo RESULT_FILE para leitura
    // - Ler conteúdo do arquivo
    // - Fazer parse do formato "worker_id:password"
    // - Verificar o hash usando md5_string()
    // - Exibir resultado encontrado
    FILE *result_file = fopen(RESULT_FILE, "r");
    if (result_file == NULL) {
        printf("Senha não encontrada no espaço de busca testado.\n");
    } else {
        char line_buffer[256];
        if (fgets(line_buffer, sizeof(line_buffer), result_file) != NULL) {
            char *colon_ptr = strchr(line_buffer, ':');
            if (colon_ptr != NULL) {
                char *found_password = colon_ptr + 1;
                found_password[strcspn(found_password, "\n")] = '\0';
                
                char calculated_hash[33];
                md5_string(found_password, calculated_hash);

                if (strcmp(calculated_hash, target_hash) == 0) {
                    printf("SUCESSO! Senha encontrada: %s\n", found_password);
                } else {
                    printf("ERRO: A senha '%s' encontrada no arquivo não gera o hash correto!\n", found_password);
                }
            } else {
                fprintf(stderr, "Erro: Arquivo de resultado mal formatado.\n");
            }
        }
        fclose(result_file);
    }
    
    // Estatísticas finais (opcional)
    // TODO: Calcular e exibir estatísticas de performance
    printf("Tempo total de execução: %.2f segundos\n", elapsed_time);
    if (elapsed_time > 0) {
        printf("Taxa de verificação: %.2f hashes/segundo\n", (double)total_space / elapsed_time);
    }
    
    return 0;
}
