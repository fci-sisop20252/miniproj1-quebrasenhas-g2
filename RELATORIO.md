# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

**Aluno(s):** Sofia Castelli (10443550), Enrique Cipolla Martins (10427834), Henrique Ferreira Marciano (10439797), Matheus Guion (10437693)  
---

## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

O coordenador recebe como entrada o hash alvo, o tamanho da senha, o charset e o número de workers.
Ele calcula o espaço total de busca C^L (onde C = tamanho do charset e L = tamanho da senha) e divide esse espaço igualmente entre os workers.
Cada worker recebe um intervalo definido por senha inicial e senha final. Esse intervalo é calculado convertendo índices em base C para strings no charset. Assim, os workers percorrem senhas em ordem lexicográfica sem sobreposição.
Exemplo:
	•	Para charset="abc", tamanho=3 → total de 27 combinações.
	•	Com 2 workers:
	•	Worker 0 recebe de aaa até acb.
	•	Worker 1 recebe de acc até ccc.

Isso garante balanceamento de carga e exploração completa do espaço de busca.

**Código relevante:** 
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

---

## 2. Implementação das System Calls

Descreva como você usou fork(), execl() e wait() no coordinator:

fork(): Usado em um loop para criar a quantidade de processos worker solicitada.

execl(): Chamado dentro de cada processo filho (pid == 0) para substituir sua execução pelo programa worker, passando os argumentos necessários (hash, senhas, charset, etc).

wait(): Usado pelo processo pai após o loop de criação para esperar que todos os workers terminem, evitando processos zumbis.

O coordenador cria os processos filhos em um loop usando fork().
	•	Se pid == 0 (processo filho), ele prepara os argumentos (hash, senha inicial, senha final, charset, tamanho, ID) e chama execl() para substituir a imagem do processo pelo executável worker.
	•	Se pid > 0 (processo pai), armazena o PID do worker e continua o loop.
	•	Após criar todos os workers, o coordenador executa um loop de wait() para garantir que todos terminem, evitando processos zumbis.   

**Código do fork/exec:**
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

---

## 3. Comunicação Entre Processos

**Como você garantiu que apenas um worker escrevesse o resultado?**


Utilizamos um arquivo found.txt como "trava". O primeiro worker que encontra a senha usa a chamada open() com as flags O_CREAT | O_EXCL. Essa operação é atômica: apenas um processo consegue criar o arquivo com sucesso. Os outros falharão, saberão que a senha foi encontrada e encerrarão. Isso evita condições de corrida

Leia sobre condições de corrida (aqui)[https://pt.stackoverflow.com/questions/159342/o-que-%C3%A9-uma-condi%C3%A7%C3%A3o-de-corrida]

**Como o coordinator consegue ler o resultado?**

Após todos os workers terminarem (confirmado pelo wait()), o coordenador verifica se o arquivo found.txt existe. Se existir, ele lê a senha de dentro do arquivo, a imprime na tela e apaga o arquivo. Caso contrário, informa que a senha não foi encontrada.

---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | ___s | ___s | ___s | ___ |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | ___s | ___s | ___s | ___ |

**O speedup foi linear? Por quê?**
Não. O speedup raramente é linear devido ao overhead:

Criação de Processos: fork() e execl() consomem tempo.

Comunicação: A verificação do arquivo de resultado gera I/O.
Para tarefas mais longas, o speedup se aproxima do ideal, pois o tempo de processamento supera o overhead.

---

## 5. Desafios e Aprendizados
**Qual foi o maior desafio técnico que você enfrentou?**
O maior desafio foi implementar a lógica de incremento de senha. Tratamos a senha como um contador em uma base customizada (o tamanho do charset), o que exigiu atenção para fazer o "carry" (o "vai um") corretamente entre os caracteres, especialmente em casos como aazz -> abzaa.

---

## Comandos de Teste Utilizados

```bash
# Teste básico
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Teste de performance
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 1
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 4

# Teste com senha maior
time ./coordinator "5d41402abc4b2a76b9719d911017c592" 5 "abcdefghijklmnopqrstuvwxyz" 4
```
---

**Checklist de Entrega:**
- [x] Código compila sem erros
- [x] Todos os TODOs foram implementados
- [x] Testes passam no `./tests/simple_test.sh`
- [x] Relatório preenchido
