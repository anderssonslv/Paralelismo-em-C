#include <stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// OBS: Este código tem compatibilidade apenas com Linux no momento

// Atendimentos sempre igual a(qtd de barbeiros + 1/2 qrd barbeiros)
#define BARBERS_NUM 4 //default -> 4
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
pthread_mutex_t mutex;


int main(int argc, char const *argv[]){
    system("clear");
    printf("\nBem vindo a Barbearia pthreads\n");
    srand(time(NULL));
    clients_num = (rand() % 31) +1; // QTD clientes indefinida
    //clients_num = 8;
    int total = clients_num;
    printf("QTD CLIENTES: %d\n",clients_num);

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
    printf("Barbeiros começam a chegar...\n");
    for (int i = 0; i < BARBERS_NUM; i++){
        barbers_id[i] = i;
        pthread_create(&barbers_t[i],NULL,barber_func,&barbers_id[i]);
    }
    printf("Clientes começam a chegar...\n"); 
    for (int i = 0; i < clients_num; i++){
        clients_id[i] = i;
        pthread_create(&clients_t[i],NULL,client_func,&clients_id[i]);
    }
    
    /// Aguarda as últimas threads para sincronização
    for (int i = 0; i < BARBERS_NUM; i++)
        pthread_join(barbers_t[i],NULL);

    for (int i = 0; i < clients_num; i++)
        pthread_join(clients_t[i],NULL);
    

    printf("\nRelatorio do dia\n");
    printf("Atendimentos -> %d\n",total);
    printf("Concluidos -> %d\n",haircuts);
    printf("Desistências -> %d\n",drops);
    printf("Obrigado e volte sempre!\n");
}

void *barber_func(void *arg){
    int id = *((int *)arg);

    while (clients_num > 0) {
        printf("Barbeiro [%d] dorme...\n",id);
        sem_wait(&barber_sem);

        // caso as últimos threads trancadas tenham sido liberadas pela qtd menor de clientes
        if (clients_num < 1)
            pthread_exit(NULL);
        
        //O barbeiro irá atender, cliente liberou este recurso
        //Caso haja tesouras ele atende, se não houver ele espera
        sem_wait(&comb_sem);
        sem_wait(&scissor_sem);

        printf("Barbeiro [%d] atende um cliente...\n",id);
        //Cliente sai da cadeira de espera, e é atendido
        sem_post(&waitchair_sem); //Libera a cadeira de espera

        pthread_mutex_lock(&mutex);
        waitchair_num++;
        haircuts++;
        clients_num--;
        pthread_mutex_unlock(&mutex);

        printf("QTD CADEIRAS DE ESPERA: %d\n",waitchair_num);
        
        haircut_duration();

        sem_post(&client_sem); // libera recurso para o cliente ir embora
        
        //após cortar o cabelo o barbeiro devolve os recursos de pente e tesoura
        sem_post(&comb_sem); //Libera um pente
        sem_post(&scissor_sem); //Libera uma tesoura

        printf("QTD CLIENTES: %d\n",clients_num);
        printf("Barbeiro [%d] devolve os materiais\n",id);
        
        // caso a qtd de clientes seja menor que os barbeiros, os ultimos barbeiros liberam os primeiros que estava dormindo
        if (clients_num == 0) {
            for (int i = 0; i < BARBERS_NUM; i++)
                sem_post(&barber_sem);
        }
    }   
}

void *client_func(void *arg){
    int id = *((int *)arg);
    client_entry();
    if (waitchair_num != 0){
        pthread_mutex_lock(&mutex);
        waitchair_num--;
        pthread_mutex_unlock(&mutex);
        printf("Cliente [%d] está na cadeira de espera...\n",id);
        printf("QTD CADEIRAS DE ESPERA: %d\n",waitchair_num);
        // Cliente libera o recurso para o barbeiro e vai para cadeira de espera
        sem_post(&barber_sem); // Libera um recurso para o barbeiro
        sem_wait(&waitchair_sem); // decrementa o recurso da cadeira do semáforo
        sem_wait(&client_sem); // Cliente está cortando o cabelo, barbeiro deve liberar recurso
        printf("O cliente vai embora feliz!\n");
    } else {
        printf("Cliente [%d] foi embora triste...\n",id);
        pthread_mutex_lock(&mutex);
        drops++;
        clients_num--;
        pthread_mutex_unlock(&mutex);
        printf("QTD CLIENTES: %d\n",clients_num);
        //sem_post(&barber_sem); // libera o recurso para um barbeiro que verifica se há mais clientes
    }
    pthread_exit(NULL);
}

void haircut_duration(){
    sleep((rand() % 4)  + 3);
}

void client_entry() { // tempo variado para entrada dos clientes
    sleep((rand() % 3)+3);
}
