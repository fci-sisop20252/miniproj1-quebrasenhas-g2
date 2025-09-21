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
 * 
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 * 
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 * 
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 * 
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 * 
 * @param charset_len Tamanho do conjunto de caracteres
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
 * 
 * @param index Índice numérico da senha
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
    
    // IMPLEMENTE AQUI: verificação de argc e mensagem de erro

    if (argc != 5) {

        // Imprime a mensagem de uso no 'stderr' (saída de erro padrão).
        // Usar argv[0] torna a mensagem dinâmica, mostrando o nome real do executável.
        fprintf(stderr, "Erro: Número incorreto de argumentos.\n");
        fprintf(stderr, "Uso: %s <parametro1> <parametro2> <parametro3> <parametro4>\n", argv[0]);

        exit(1); // Sai do programa com código de erro 1.
    }

    printf("Argumentos validados com sucesso!\n");
    printf("Nome do programa: %s\n", argv[0]);
    printf("Argumento 1: %s\n", argv[1]);
    printf("Argumento 2: %s\n", argv[2]);
    printf("Argumento 3: %s\n", argv[3]);
    printf("Argumento 4: %s\n", argv[4]);

    return 0; // Retorna 0 para indicar sucesso na validação

    
    
    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);
    
    // TODO2: Adicionar validações dos parâmetros
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
        if (strlen(charset) == 0) {
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
    
    // IMPLEMENTE AQUI:
    // long long passwords_per_worker = ?
    // long long remaining = ?
    printf("Iniciando divisão do trabalho...\n");
    printf("Total de senhas a verificar: %lld\n", total_space);
    printf("Número de workers: %d\n\n", num_workers);

    // CORREÇÃO: Renomeado `total_passwords` para `total_space` para evitar confusão,
    // e usando a variável já calculada.
    long long passwords_per_worker;
    long long remaining_passwords;

    passwords_per_worker = total_space / num_workers; // Divisão inteira
    remaining_passwords = total_space % num_workers; // Resto da divisão
    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];
    
    // TODO 3: Criar os processos workers usando fork()
    printf("Iniciando %d workers para testar %lld senhas...\n", num_workers, total_passwords);
        
    pid_t pid; // Variável para armazenar o retorno do fork()

    // IMPLEMENTE AQUI: Loop para criar workers
    for (int i = 0; i < num_workers; i++) {
         // Cria um novo processo
        // TODO: Calcular intervalo de senhas para este worker
        long long chunk_size = total_passwords / num_workers;
        long long start_index = i * chunk_size;
        long long end_index = (i == num_workers - 1) ? (total_passwords - 1) : (start_index + chunk_size - 1);

        // TODO: Converter indices para senhas de inicio e fim

        // TODO 4: Usar fork() para criar processo filho
        pid = fork();

        if (pid > 0) {
            perror("Erro no fork()");
            exit(EXIT_FAILURE);
        }
        // TODO 5: No processo pai: armazenar PID
        else { // Este é o código que o processo PAI executa
            workers[i] = pid; // Armazena o PID do filho recém-criado
            printf("Pai: Processo worker %d criado com PID %d.\n", i, workers[i]);

        // TODO 6: No processo filho: usar execl() para executar worker
        else if (pid == 0) { // Código do processo FILHO
    // ... calcula senhas ...
            execl("./worker", "worker", start_pass, end_pass, NULL);
    // ... trata erro do execl ...
}
        // TODO 7: Tratar erros de fork() e execl()
            perror("Erro no fork()");
            // Em um caso real, seria bom matar os workers já criados
            exit(EXIT_FAILURE);
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
    
    // Estatísticas finais (opcional)
    // TODO: Calcular e exibir estatísticas de performance
    
    return 0;
}#include <stdio.h>
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
 * 
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 * 
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 * 
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 * 
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 * 
 * @param charset_len Tamanho do conjunto de caracteres
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
 * 
 * @param index Índice numérico da senha
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
    
    // IMPLEMENTE AQUI: verificação de argc e mensagem de erro

    if (argc != 5) {

        // Imprime a mensagem de uso no 'stderr' (saída de erro padrão).
        // Usar argv[0] torna a mensagem dinâmica, mostrando o nome real do executável.
        fprintf(stderr, "Erro: Número incorreto de argumentos.\n");
        fprintf(stderr, "Uso: %s <parametro1> <parametro2> <parametro3> <parametro4>\n", argv[0]);

        exit(1); // Sai do programa com código de erro 1.
    }

    printf("Argumentos validados com sucesso!\n");
    printf("Nome do programa: %s\n", argv[0]);
    printf("Argumento 1: %s\n", argv[1]);
    printf("Argumento 2: %s\n", argv[2]);
    printf("Argumento 3: %s\n", argv[3]);
    printf("Argumento 4: %s\n", argv[4]);

    return 0; // Retorna 0 para indicar sucesso na validação

    
    
    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);
    
    // TODO2: Adicionar validações dos parâmetros
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
        if (strlen(charset) == 0) {
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
    
    // IMPLEMENTE AQUI:
    // long long passwords_per_worker = ?
    // long long remaining = ?
    printf("Iniciando divisão do trabalho...\n");
    printf("Total de senhas a verificar: %lld\n", total_space);
    printf("Número de workers: %d\n\n", num_workers);

    // CORREÇÃO: Renomeado `total_passwords` para `total_space` para evitar confusão,
    // e usando a variável já calculada.
    long long passwords_per_worker;
    long long remaining_passwords;

    passwords_per_worker = total_space / num_workers; // Divisão inteira
    remaining_passwords = total_space % num_workers; // Resto da divisão
    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];
    
    // TODO 3: Criar os processos workers usando fork()
    printf("Iniciando %d workers para testar %lld senhas...\n", num_workers, total_passwords);
        
    pid_t pid; // Variável para armazenar o retorno do fork()

    // IMPLEMENTE AQUI: Loop para criar workers
    for (int i = 0; i < num_workers; i++) {
         // Cria um novo processo
        // TODO: Calcular intervalo de senhas para este worker
        long long chunk_size = total_passwords / num_workers;
        long long start_index = i * chunk_size;
        long long end_index = (i == num_workers - 1) ? (total_passwords - 1) : (start_index + chunk_size - 1);

        // TODO: Converter indices para senhas de inicio e fim

        // TODO 4: Usar fork() para criar processo filho
        pid = fork();

        if (pid > 0) {
            perror("Erro no fork()");
            exit(EXIT_FAILURE);
        }
        // TODO 5: No processo pai: armazenar PID
        else { // Este é o código que o processo PAI executa
            workers[i] = pid; // Armazena o PID do filho recém-criado
            printf("Pai: Processo worker %d criado com PID %d.\n", i, workers[i]);

        // TODO 6: No processo filho: usar execl() para executar worker
        else if (pid == 0) { // Código do processo FILHO
    // ... calcula senhas ...
            execl("./worker", "worker", start_pass, end_pass, NULL);
    // ... trata erro do execl ...
}
        // TODO 7: Tratar erros de fork() e execl()
            perror("Erro no fork()");
            // Em um caso real, seria bom matar os workers já criados
            exit(EXIT_FAILURE);
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
    
    // Estatísticas finais (opcional)
    // TODO: Calcular e exibir estatísticas de performance
    
    return 0;
}
