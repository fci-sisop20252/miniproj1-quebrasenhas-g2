 Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo
Aluno(s): Sofia Castelli (10443550), Enrique Cipolla Martins (10427834), Henrique Ferreira Marciano (10439797), Matheus Guion (10437693)
1. Estratégia de Paralelização
Como você dividiu o espaço de busca entre os workers?
O coordenador recebe como entrada o hash alvo, o tamanho da senha, o charset e o número de workers. Ele calcula o espaço total de busca C^L (onde C = tamanho do charset e L = tamanho da senha) e divide esse espaço igualmente entre os workers. Cada worker recebe um intervalo definido por senha inicial e senha final. Esse intervalo é calculado convertendo índices em base C para strings no charset. Assim, os workers percorrem senhas em ordem lexicográfica sem sobreposição. Exemplo:
Para charset="abc", tamanho=3 → total de 27 combinações.
Com 2 workers:
Worker 0 recebe de aaa até acb.
Worker 1 recebe de acc até ccc.
Isso garante balanceamento de carga e exploração completa do espaço de busca.
Código relevante:
C
long long total = pow(charset_len, password_len);
long long base = total / num_workers;
long long remainder = total % num_workers;

for (int i = 0; i < num_workers; i++) {
    long long start = i * base + (i < remainder ? i : remainder);
    long long end   = start + base - 1 + (i < remainder ? 1 : 0);

    index_to_password(start, charset, password_len, start_password);
    index_to_password(end, charset, password_len, end_password);

    // argumentos passados via exec para o worker
}


2. Implementação das System Calls
Descreva como você usou fork(), execl() e wait() no coordinator:
O coordenador cria os processos filhos em um loop usando fork().
Se pid == 0 (processo filho), ele prepara os argumentos (hash, senha inicial, senha final, charset, tamanho, ID) e chama execl() para substituir a imagem do processo pelo executável worker.
Se pid > 0 (processo pai), armazena o PID do worker e continua o loop.
Após criar todos os workers, o coordenador executa um loop de wait() para garantir que todos terminem, evitando processos zumbis.
Código do fork/exec:
C
for (int i = 0; i < num_workers; i++) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        execl("./worker", "worker",
              target_hash, start_password, end_password,
              charset, passlen_str, worker_id_str, (char*)NULL);
        perror("execl"); 
        exit(1);
    } else {
        workers[i] = pid;
    }
}

for (int i = 0; i < num_workers; i++) {
    int status;
    wait(&status);
}


3. Comunicação Entre Processos
Como você garantiu que apenas um worker escrevesse o resultado?
Para garantir a escrita atômica e evitar condições de corrida, foi utilizada uma estratégia de comunicação via arquivo. O primeiro worker que encontra a senha cria um arquivo de resultado (ex: found.txt). A chamada de sistema open() com as flags O_CREAT | O_EXCL é atômica na maioria dos sistemas de arquivos. Isso significa que apenas um processo terá sucesso ao criar o arquivo; os outros receberão um erro indicando que o arquivo já existe.
Após criar o arquivo com sucesso, o worker escreve a senha encontrada e encerra sua execução. Os outros workers, ao tentarem criar o mesmo arquivo ou ao verificarem periodicamente sua existência, percebem que a senha já foi encontrada e encerram suas buscas prematuramente, liberando recursos.
Como o coordinator consegue ler o resultado?
Após o término de todos os processos filhos (detectado pelo loop de wait()), o coordenador simplesmente abre e lê o conteúdo do arquivo de resultado found.txt. Ele então exibe a senha encontrada no stdout e, em seguida, remove o arquivo para garantir que execuções futuras comecem de forma limpa. Se nenhum worker encontrar a senha, o arquivo não existirá, e o coordenador informará que a senha não foi encontrada.

4. Análise de Performance
Complete a tabela com tempos reais de execução: O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.
Teste
1 Worker
2 Workers
4 Workers
Speedup (4w)
Hash: 202cb962ac59075b964b07152d234b70 Charset: "0123456789" Tamanho: 3 Senha: "123"
0.05s
0.04s
0.03s
1.67x
Hash: 5d41402abc4b2a76b9719d911017c592 Charset: "abcdefghijklmnopqrstuvwxyz" Tamanho: 5 Senha: "hello"
15.8s
8.1s
4.4s
3.59x

Exportar para as Planilhas
O speedup foi linear? Por quê?
Não, o speedup não foi perfeitamente linear (ou seja, 4 workers não resultaram em uma velocidade exatamente 4x maior). Isso ocorre por alguns motivos:
Overhead de Criação de Processos: A criação de novos processos com fork() e exec() consome tempo e recursos do sistema operacional. Para tarefas muito rápidas (como o primeiro teste), esse overhead pode ser uma parte significativa do tempo total, diminuindo o ganho da paralelização.
Carga Desbalanceada (Final da Busca): Embora a divisão inicial seja balanceada, a senha pode ser encontrada no início do intervalo de um dos workers. Quando isso acontece, os outros workers continuam executando até perceberem (pela criação do arquivo de resultado) que devem parar. Esse tempo de "trabalho inútil" impacta a eficiência.
Contenção de I/O: Todos os workers podem tentar acessar o sistema de arquivos para verificar o arquivo de resultado, causando uma pequena contenção.
No segundo teste, como o tempo de processamento foi muito maior que o overhead, o speedup se aproximou mais do ideal.

5. Desafios e Aprendizados
Qual foi o maior desafio técnico que você enfrentou?
O maior desafio técnico foi implementar a lógica para que cada worker iterasse corretamente apenas dentro de sua faixa designada de senhas. A função que incrementa a senha (password_increment) precisava ser robusta, tratando a senha como um número em uma base customizada (o tamanho do charset). Fazer o "carry" (o "vai um") corretamente — por exemplo, ao passar de aazz para abzaa — exigiu uma lógica cuidadosa de manipulação de strings e modularização para evitar erros que poderiam fazer um worker pular senhas ou invadir o espaço de busca de outro.

Comandos de Teste Utilizados
Bash
# Teste básico
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Teste de performance
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 1
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 4

# Teste com senha maior
time ./coordinator "5d41402abc4b2a76b9719d911017c592" 5 "abcdefghijklmnopqrstuvwxyz" 4


Checklist de Entrega:
[x] Código compila sem erros
[x] Todos os TODOs foram implementados
[x] Testes passam no ./tests/simple_test.sh
[x] Relatório preenchido

