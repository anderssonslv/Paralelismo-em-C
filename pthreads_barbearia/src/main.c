#include <stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

//OBS: Este código tem compatibilidade apenas com Linux no momento

#define BARBERS_NUM 4 //default -> 4

void *barber_func(void *arg);
void *client_func(void *arg);
void haircut_duration();
void client_wait();
int num_clients();

int haircuts = 0;
int drops = 0;
//Waitchair pode ser qlqr valor para fazer testes
int waitchair_num = 4; // Assumindo que são apenas para clientes e que os barbeiros podem esperar de qualquer forma
int comb_num = BARBERS_NUM / 2;
int scissor_num = BARBERS_NUM / 2;
int clients_num = 0;
sem_t barber_sem;
sem_t comb_sem;
sem_t scissor_sem;
sem_t waitchair_sem;
pthread_mutex_t mutex;

int main(int argc, char const *argv[]){
    system("clear");
    printf("\nBem vindo a Barbearia pthreads\n");
    srand(time(NULL));
    int clients_num = (rand() % 10) +1; // QTD clientes indefinida
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
    printf("Falhos -> %d\n",drops);
    printf("Obrigado e volte sempre!\n");
}

void *barber_func(void *arg){
    int id = *((int *)arg);

    printf("Barbeiro [%d] dorme...\n",id);
    sem_wait(&barber_sem);
    
    //O barbeiro irá atender, cliente liberou este recurso
    //Caso haja tesouras ele atende, se não houver ele espera
    sem_wait(&comb_sem);
    sem_wait(&scissor_sem);

    printf("Barbeiro [%d] atende um cliente...\n",id);
    //Cliente sai da cadeira de espera, e é atendido
    sem_post(&waitchair_sem); //Libera a cadeira de espera
    waitchair_num++;
    printf("QTD CADEIRAS DE ESPERA: %d\n",waitchair_num);
    haircut_duration();
    haircuts++;
    
    //após cortar o cabelo o barbeiro devolve os recursos de pente e tesoura
    sem_post(&comb_sem); //Libera um pente
    sem_post(&scissor_sem); //Libera uma tesoura
    clients_num--;
    printf("QTD CLIENTES: %d\n",clients_num);
    printf("Barbeiro [%d] devolve os materiais\n",id);
    if(clients_num < 1)
        pthread_exit(NULL); //termina a thread explicitamente
}

void *client_func(void *arg){
    int id = *((int *)arg);
    client_wait(); // testando colocar delay nos clientes
    if (waitchair_num > 0){
        waitchair_num--;
        printf("Cliente [%d] está na cadeira de espera...\n",id);
        printf("QTD CADEIRAS DE ESPERA: %d\n",waitchair_num);
        // Cliente libera o recurso para o barbeiro e vai para cadeira de espera
        sem_post(&barber_sem); // Libera um recurso para o barbeiro
        sem_wait(&waitchair_sem); // decrementa o recurso da cadeira do semáforo
    } else {
        printf("Cliente [%d] foi embora...\n",id);
        drops++;
        clients_num--;
        printf("QTD CLIENTES: %d\n",clients_num);
    }
    pthread_exit(NULL);
}


void haircut_duration() {
    sleep((rand() % 4)  + 3);
}

void client_wait() {
    sleep(1);
}
