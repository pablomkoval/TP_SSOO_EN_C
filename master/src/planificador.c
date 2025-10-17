#include <planificador.h>

t_queue* cola_ready;
t_list* workers_libres;

sem_t sem_queries_ready;
sem_t sem_workers_libres;

pthread_mutex_t mutex_ready;
pthread_mutex_t mutex_workers_libres;

void inicializar_planificador(){
    cola_ready = queue_create();
    workers_libres = list_create();

    sem_init(&sem_queries_ready, 0, 0);
    sem_init(&sem_workers_libres, 0, 0);
    pthread_mutex_init(&mutex_ready, NULL);
    pthread_mutex_init(&mutex_workers_libres, NULL);
}

void enviar_qcb_a_worker(t_qcb* qcb, int socket_worker){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, SOLICITUD_NUEVA_QUERY);
    agregar_a_paquete(paquete, &qcb->qid, sizeof(int));
    agregar_a_paquete(paquete, qcb->path, strlen(qcb->path) + 1);
    agregar_a_paquete(paquete, &qcb->pc, sizeof(int));
    enviar_paquete(paquete, socket_worker, logger);
    borrar_paquete(paquete);
}

int obtener_worker_libre(){
    int* worker_id_ptr = list_remove(workers_libres, 0);
    int worker_id = *worker_id_ptr;
    return worker_id;
}

void* planificador(){
    while(1){
        log_info(logger,"Planificador esperando query....");
        sem_wait(&sem_queries_ready);
        sem_wait(&sem_workers_libres);
        
        pthread_mutex_lock(&mutex_ready);

        t_qcb* a_ejecutar;
        if(strcmp(algoritmo_planificacion, "FIFO") == 0){
            a_ejecutar = queue_pop(cola_ready);
        }   else{
            //planificar por prioridadess
            a_ejecutar = queue_pop(cola_ready); //por mientras esto para q no explote
        }
        pthread_mutex_unlock(&mutex_ready);

        cambiar_estado(a_ejecutar, EXEC);

        int socket_worker = obtener_worker_libre();
        enviar_qcb_a_worker(a_ejecutar, socket_worker);
    }
}
