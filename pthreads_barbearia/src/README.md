# Trabalho realizado na disciplina de Sistemas Operacionais
O Trabalho faz uma simulação de uma barbearia utilizando a biblioteca pthreads em C, onde os barbeiros e os clientes representam 
threads que se comunicam para liberação de recursos, os resultados finais não são determinísticos devido a aleatóriedade
de tempo lançamento das threads, duração de processos e o escalonamento de threads de acordo com o sistema.

## Observações
O trabalho foi desenvolvido no sistema linux, e até o momento só é compativel com o mesmo! <br>
Caso rode por linha de comando não se esqueça de incluir a biblioteca pthreads. <br>
Ex: *gcc -o main.c -lpthreads*

## Colaboradores
[Andersson de Souza](https://github.com/anderssonslv)<br>
[Franklin Sales](https://github.com/Frankl1sales)

## Principios básicos

### Granularidade
No contexto do problema do Barbeiro Dorminhoco, a prática de granularização foi 
a introdução de diversos mutexes para melhorar o entendimento e evitar possíveis 
condições de corrida, quando o resultado é indefinido porque duas ou mais threads estão concorrendo para acessar e modificar os mesmo dados compartilhados,
ou deadlock, quando duas threads estão impossibilitadas de processeguir porque cada uma está aguardando que a outra libere um recurso.

### Especificidades
Número de cadeiras é o mesmo número de barbeiros
Numero de pentes e tesouras é o numero de barbeiro/2




## Função do Barbeiro - Barber_func() 
É uma função que modela a ação de cada barbeiro no sistema

### Parâmetro de Entrada
A função recebe um parametro de entra "arg" que é um ponteiro para um inteiro (int *). O código
converte esse ponteiro para um inteiro (id) para facilitar operações e evitar alterações no ponteiro.

### Condição de Saída
Se a var global clientes_num ficar igual a 0, a função é encerrada

### Loop Principal
O código entra em um loop enquanto o número total de cortes (haircuts) somado ao número de clientes que 
desistiram (drops) for menor que o número total de clientes (clients_num) -  (haircuts + drops < clients_num). 

### Espera por CLiente
O barbeiro aguarda (sem_wait(&barber_sem)) até que um cliente esteja disponível. 
Essa semáforo geralmente é usado para controlar o acesso à região crítica compartilhada entre os barbeiros.

### Tratamento da última Thread 
Há uma seção crítica protegida por mutex (pthread_mutex_lock(&race_stop_mut)) que lida com a situação em que o último cliente é atendido. 
Nesse caso, todos os barbeiros são liberados e a função é encerrada.


