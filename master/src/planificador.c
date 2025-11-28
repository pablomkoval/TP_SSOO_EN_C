#include <planificador.h>

t_list* cola_ready;
t_list* workers_libres;

sem_t sem_queries_ready;
sem_t sem_workers_libres;

pthread_mutex_t mutex_ready;
pthread_mutex_t mutex_workers_libres;

void inicializar_planificador(){
    cola_ready = list_create();
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
    //log_debug(logger, "##DEBUG: PRE-MUTEX");
    pthread_mutex_lock(&mutex_workers_libres);
    //log_debug(logger, "##DEBUG: POST-MUTEX");
    int* worker_id_ptr = list_remove(workers_libres, 0);
    //log_debug(logger, "##DEBUG: POST-LIST_REMOVE");
    pthread_mutex_unlock(&mutex_workers_libres);

    int worker_id = *worker_id_ptr;
    free(worker_id_ptr);
    log_debug(logger, "##DEBUG: RETORNA WORKER ID: (%d)", worker_id);
    return worker_id;
}

void* planificador(){
    while(1){
        log_info(logger,"Planificador esperando query....");
        sem_wait(&sem_queries_ready);
        log_debug(logger, "##DEBUG: PASO SEMAFORO 1");
        sem_wait(&sem_workers_libres);
        log_debug(logger, "##DEBUG: PASO SEMAFORO 2");
        
        t_qcb* query_a_ejecutar = NULL;
        int worker_seleccionado_id;
        query_a_ejecutar = obtener_query_y_worker(&worker_seleccionado_id);
        
        enviar_query_a_worker(query_a_ejecutar, worker_seleccionado_id);
    }
}

void enviar_query_a_worker(t_qcb* query_a_ejecutar, int worker_asignado_id){
    
    char* worker_asignado_id_ptr = string_itoa(worker_asignado_id); 

    
    
    pthread_mutex_lock(&mutex_diccionario_exec);
    dictionary_put(diccionario_exec, worker_asignado_id_ptr, query_a_ejecutar);
    pthread_mutex_unlock(&mutex_diccionario_exec);


    pthread_mutex_lock(&mutex_diccionario_workers);
    int socket_worker_asignado = *((int*)dictionary_get(diccionario_workers, worker_asignado_id_ptr));
    pthread_mutex_unlock(&mutex_diccionario_workers);


    enviar_qcb_a_worker(query_a_ejecutar, socket_worker_asignado);
    query_a_ejecutar->id_worker_asociado = worker_asignado_id;
    //cambiar_estado(query_a_ejecutar, EXEC); movido a obtener_query_y_worker para evitar solapamiento con aging

    //sem_post(&sem_queries_ready); //y este maquina?

    log_info(logger, "## Se envía la Query <%d> (<%d>) al Worker <%d>", query_a_ejecutar->qid, query_a_ejecutar->prioridad, worker_asignado_id);
}

t_qcb* obtener_query_y_worker(int *worker_libre_id){
    pthread_mutex_lock(&mutex_ready);
    t_qcb* query_a_ejecutar = list_remove(cola_ready, 0);
    cambiar_estado(query_a_ejecutar, EXEC);
    pthread_mutex_unlock(&mutex_ready);
    
    *worker_libre_id = obtener_worker_libre();
    return query_a_ejecutar;
}

/* bool chequear_y_hacer_aging(t_qcb* qcb){
    int tiempo_qcb = temporal_gettime(qcb->tiempo_aging);

    if(tiempo_qcb >= tiempo_aging){
        qcb->prioridad--;
        log_info(logger, "##<%d> Cambio de prioridad: <%d> - <%d>", qcb->qid, qcb->prioridad + 1, qcb->prioridad);
        temporal_destroy(qcb->tiempo_aging);

        if(qcb->prioridad > 0){
        qcb->tiempo_aging = temporal_create();
        }
        return true;
    }
    return false;
} */

void* hilo_aging_individual(void* arg){
    t_qcb* qcb = (t_qcb*)arg;

    int tiempo_aging_micro = tiempo_aging * 1000;

    log_info(logger, "qid %d: Hilo de aging individual iniciado. Intervalo: %d ms", qcb->qid, tiempo_aging);

    while(qcb->prioridad > 0){
        usleep(tiempo_aging_micro); //importante, controla aging y permite pthread_cancel

        pthread_mutex_lock(&mutex_ready); 
        if (qcb->estado == READY){

            qcb->prioridad--;
            log_info(logger, "##<%d> Cambio de prioridad: <%d> - <%d>", qcb->qid, qcb->prioridad + 1, qcb->prioridad);

            list_sort(cola_ready, (void*)comparar_qcb_por_prioridad);

            hacer_chequeo_desalojo(qcb); 
            
        } /* else{ //no necesario, se mata el hilo cuando sale de ready
            query_sigue_en_cola = false;
            //temporal_destroy(qcb->tiempo_aging);
        } */
        pthread_mutex_unlock(&mutex_ready);
    }
    
    log_info(logger, "qid %d: Hilo de aging individual finalizo.", qcb->qid);
    free(arg);
    return NULL;
}