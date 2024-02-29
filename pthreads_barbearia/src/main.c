#include <stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Problema dos resultados deterministicos resolvidos

// OBS: Este código tem compatibilidade apenas com Linux no momento

// 2 barbeiros -> Instavel, mesmo com poucos clientes ex 12
// 4 barbeiros -> Estavel, com poucas desistências mesmo com 50 clientes
// Se clientes menor ou igual barbeiros então 100% de atendimento como esperado

#define BARBERS_NUM 2 // padrão = 4
void *barber_func(void *arg);
void *client_func(void *arg);
void haircut_duration();
void client_entry();

int haircuts = 0;
int drops = 0;
// Waitchair pode ser qlqr valor para fazer testes
int waitchair_num = BARBERS_NUM; // Assumindo que são apenas para clientes e que os barbeiros podem esperar de qualquer forma
int comb_num = BARBERS_NUM / 2;
int scissor_num = BARBERS_NUM / 2;
int clients_num = 0;
sem_t barber_sem;
sem_t client_sem;
sem_t comb_sem;
sem_t scissor_sem;
sem_t waitchair_sem;
pthread_mutex_t clients_num_mut;
pthread_mutex_t waitchair_num_mut;
pthread_mutex_t haircuts_num_mut;
pthread_mutex_t drops_num_mut;
pthread_mutex_t race_stop_mut;

int main(int argc, char const *argv[]){
    system("clear");
    printf("\nBem vindo a Barbearia pthreads!\n");
    srand(time(NULL));
    //clients_num = (rand() % 31) +1; // QTD clientes indefinida
    clients_num = 8;
    int total = clients_num;
    printf("QTD CLIENTES: %d\n",clients_num);
    printf("QTD BARBEIROS: %d\n",BARBERS_NUM);

    pthread_t barbers_t[BARBERS_NUM];
    pthread_t clients_t[clients_num];
    int barbers_id[BARBERS_NUM];
    int clients_id[clients_num];

    // Argumentos da função -> sem_init()
    // ( semáforo / 0 - não compartilhado entre processos / quantidade de recursos )
    // Iniciando os semáforos
    sem_init(&barber_sem,0,0); // faz os barbeiros dormirem   
    sem_init(&comb_sem,0,comb_num); //começa com o valor de recursos incialmente disponíveis  
    sem_init(&scissor_sem,0,scissor_num);
    sem_init(&waitchair_sem,0,waitchair_num);
    sem_init(&client_sem,0,0);

    // Criando threads para barbeiros e clientes
    // Após criar pode executar a qualquer momento, depende do escalonador
    if (BARBERS_NUM > 0)
        printf("Os Barbeiros começam a chegar...\n");
    else printf("Os Barbeiros não vieram...\n");

    sleep(1);

    if (clients_num > 0)
        printf("Clientes começam a chegar...\n"); 
    else printf("Nenhum Cliente apareceu\n");
    

    for (int i = 0; i < BARBERS_NUM; i++){
        barbers_id[i] = i;
        pthread_create(&barbers_t[i],NULL,barber_func,&barbers_id[i]);
    }
    
    for (int i = 0; i < clients_num; i++){
        clients_id[i] = i;
        client_entry();
        pthread_create(&clients_t[i],NULL,client_func,&clients_id[i]);
    }
    
    // Aguarda as últimas threads para sincronização
    for (int i = 0; i < BARBERS_NUM; i++)
        pthread_join(barbers_t[i],NULL);

    for (int i = 0; i < clients_num; i++)
        pthread_join(clients_t[i],NULL);
    

    printf("\n===== Relatorio do dia =====\n");
    printf("Total de Clientes -> %d\n",total);
    printf("Concluidos -> %d\n",haircuts);
    printf("Desistências -> %d\n",drops);
    printf("Obrigado e volte sempre!\n");
    return 0;
}

void *barber_func(void *arg){
    int id = *((int *)arg);

    if (clients_num == 0) {
        printf("O Barbeiro [%d] chegou, e foi embora\n",id);
        pthread_exit(NULL);
    }
    
    while (haircuts + drops < clients_num) {
        printf("Barbeiro [%d] esta dormindo...\n",id);
        sem_wait(&barber_sem);

        sem_wait(&comb_sem); // Se houver pente continua
        sem_wait(&scissor_sem); // Se houver tesoura continua

        // Resolvendo a condição de corrida dos últimos barbeiros
        pthread_mutex_lock(&race_stop_mut);
        if (haircuts+drops == clients_num){ // Caso seja o ultimo cliente, libera os barbeiros do semáforo
            for (int i = 0; i < BARBERS_NUM; i++) // Não importa se todas ultimas threads liberarem mais recursos, pois está tudo finalizado
                sem_post(&barber_sem); // Este último barbeiro libera os que estão dormindo 
            sem_post(&comb_sem);
            sem_post(&scissor_sem);
            pthread_mutex_unlock(&race_stop_mut); // Libera o mutex para as últimas threads passarem por aqui
            pthread_exit(NULL); // E finaliza a execução
        } else {
            pthread_mutex_lock(&haircuts_num_mut);  
            haircuts++;
            pthread_mutex_unlock(&haircuts_num_mut);
            pthread_mutex_unlock(&race_stop_mut);
        }

        printf("Barbeiro [%d] está fazendo o corte...\n",id);
        pthread_mutex_lock(&waitchair_num_mut);        
        waitchair_num++;
        pthread_mutex_unlock(&waitchair_num_mut); 
        sem_post(&waitchair_sem); // Libera uma cadeira de espera para atender o cliente

        haircut_duration(); // Faz o Corte
        sem_post(&client_sem); // Libera o Cliente

        sem_post(&comb_sem); // Devolve os pentes
        sem_post(&scissor_sem); // Devolve as tesouras
        printf("Barbeiro [%d] devolve os recursos e finaliza!\n",id);

        if (haircuts+drops == clients_num) // Caso tenha mais barbeiros que clientes
            for (int i = 0; i < BARBERS_NUM; i++) 
                sem_post(&barber_sem);
    }
}


void *client_func(void *arg){
    int id = *((int *)arg);

    if (BARBERS_NUM == 0) {
        printf("O Cliente [%d] chegou e foi embora\n",id);
        pthread_exit(NULL);
    }
    
    if (waitchair_num > 0){
        pthread_mutex_lock(&waitchair_num_mut);        
        waitchair_num--;
        pthread_mutex_unlock(&waitchair_num_mut);  
        sem_post(&barber_sem); // Libera um barbeiro para atende-lo
              
        printf("Cliente [%d] aguarda sentado...\n",id);
        sem_wait(&waitchair_sem); // Aguarda na cadeira até o barbeiro atender

        printf("Cliente [%d] esta cortando o cabelo...\n",id);
        sem_wait(&client_sem); // Está cortando o cabelo e o barbeiro irá libera-lo
        
        printf("O cliente [%d] foi embora feliz!\n",id);
        printf("CORTES: %d\n",haircuts);
    } else {
        sem_post(&barber_sem);
        printf("Cliente [%d] foi embora triste...\n",id);
        pthread_mutex_lock(&drops_num_mut);
        drops++;
        pthread_mutex_unlock(&drops_num_mut);
        printf("DESISTENCIAS: %d\n",drops);
    }
} 

void haircut_duration(){ // tempo variado para cada corte
    sleep((rand() % 3)  + 2);
}

void client_entry() { // tempo variado para entrada dos clientes
    sleep((rand() % 2) + 1);
}
