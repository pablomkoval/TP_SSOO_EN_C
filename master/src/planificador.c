#include <planificador.h>

t_list* cola_ready;
t_list* workers_libres;

sem_t sem_queries_ready;
sem_t sem_workers_libres;
//sem_t sem_permiso_desalojo;

pthread_mutex_t mutex_ready;
pthread_mutex_t mutex_workers_libres;

int tiempo_chequeo_aging = 0;
bool se_aplico_aging = false;


void inicializar_planificador(){
    cola_ready = list_create();
    workers_libres = list_create();

    sem_init(&sem_queries_ready, 0, 0);
    sem_init(&sem_workers_libres, 0, 0);
    //sem_init(&sem_permiso_desalojo, 0, 1);
    pthread_mutex_init(&mutex_ready, NULL);
    pthread_mutex_init(&mutex_workers_libres, NULL);

    if(strcmp(algoritmo_planificacion, "PRIORIDADES") == 0 && tiempo_aging > 0){
        pthread_t hilo_aging_var; 
        pthread_create(&hilo_aging_var, NULL, hilo_aging, NULL);
        pthread_detach(hilo_aging_var);
    }

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
    log_trace(logger, "Agrego a d_exec query [%d]", query_a_ejecutar->qid);
    pthread_mutex_unlock(&mutex_diccionario_exec);


    pthread_mutex_lock(&mutex_diccionario_workers);
    int socket_worker_asignado = *((int*)dictionary_get(diccionario_workers, worker_asignado_id_ptr));
    pthread_mutex_unlock(&mutex_diccionario_workers);


    enviar_qcb_a_worker(query_a_ejecutar, socket_worker_asignado);
    query_a_ejecutar->id_worker_asociado = worker_asignado_id;
    //cambiar_estado(query_a_ejecutar, EXEC); movido a obtener_query_y_worker para evitar solapamiento con aging

    //sem_post(&sem_queries_ready); //y este maquina?

    free(worker_asignado_id_ptr);
    log_info(logger, "## Se envía la Query <%d> (<%d>) al Worker <%d>", query_a_ejecutar->qid, query_a_ejecutar->prioridad, worker_asignado_id);
}

t_qcb* obtener_query_y_worker(int *worker_libre_id){
    pthread_mutex_lock(&mutex_ready);
    t_qcb* query_a_ejecutar = list_remove(cola_ready, 0);
    log_debug(logger, "Query a ejecutar: %d", query_a_ejecutar->qid);
    cambiar_estado(query_a_ejecutar, EXEC);
    pthread_mutex_unlock(&mutex_ready);
    
    *worker_libre_id = obtener_worker_libre();
    return query_a_ejecutar;
}


void* hilo_aging(){
    tiempo_chequeo_aging = tiempo_aging / 5; //chequeo de aging a un 1/5 del tiempo aging (convencion)
    
    log_info(logger, "Hilo de aging iniciado. Intervalo Aging: %d ms. Intervalo de chequeo: %d ms.", tiempo_aging, tiempo_chequeo_aging);

    int tiempo_chequeo_aging_micro = tiempo_chequeo_aging * 1000; // pasado a milisegundos

    t_qcb* qcb_cabeza = NULL;
    int prioridad_maxima_anterior = -1;
    while(1){
        usleep(tiempo_chequeo_aging_micro); // intervalo de chequeo de aging

        pthread_mutex_lock(&mutex_ready); 

        
        if(list_size(cola_ready)>0){
            qcb_cabeza = list_get(cola_ready, 0);
            prioridad_maxima_anterior = qcb_cabeza->prioridad;

        }
        
        list_iterate(cola_ready, *evaluar_aging_individual);

        

        if(se_aplico_aging){
            log_debug(logger, "Se aplico aging");
            se_aplico_aging = false;
            log_debug(logger, "Ya no deberia hacer chequeo");

            
            list_sort(cola_ready, (void*)comparar_qcb_por_prioridad);
            t_qcb* nueva_qcb_cabeza = list_get(cola_ready, 0);

            log_trace(logger, "Comparacion de prioridades: Anterior: %d, Actual: %d", prioridad_maxima_anterior, nueva_qcb_cabeza->prioridad);

            if(prioridad_maxima_anterior != nueva_qcb_cabeza->prioridad){ //cambio la prioridad mayor de la lista
                log_trace(logger, "Se debe chequear desalojo");
                nueva_qcb_cabeza->chequeo_desalojo_pendiente = true;
                hacer_chequeo_desalojo();
            }
            
        }
        pthread_mutex_unlock(&mutex_ready);
    }

    return NULL;

}

void evaluar_aging_individual(void* arg){
    t_qcb* qcb = (t_qcb*)arg;
    
    if(qcb == NULL){
        log_error(logger, "Se intento evaluar aging en una qcb Nula");
        return;
    }
    if(qcb->prioridad == 0){    // se puede juntar con el if de arriba pero mepa que
        return;                 // es mejor diferenciar el de arriba con el log_error
    }

    if(qcb->tiempo_aging_qcb >= tiempo_aging && !qcb->chequeo_desalojo_pendiente){
        qcb->prioridad--;
        se_aplico_aging = true;
        qcb->tiempo_aging_qcb = 0;
        log_info(logger, "##<%d> Cambio de prioridad: <%d> - <%d>", qcb->qid, qcb->prioridad + 1, qcb->prioridad);
    }else if(qcb->tiempo_aging_qcb < tiempo_aging && !qcb->chequeo_desalojo_pendiente){
        qcb->tiempo_aging_qcb += tiempo_chequeo_aging;
    }



    return;
}


void hacer_chequeo_desalojo(/* t_qcb* qcb_entrante */){

    t_qcb* qcb_a_desalojar = NULL;

    void buscar_candidato_desalojo(char* wid_str, void* qcb_exec){ //funcion anonima para iteracion
        t_qcb* qcb_leida = (t_qcb*)qcb_exec;
        
        if (qcb_a_desalojar == NULL || qcb_leida->prioridad > qcb_a_desalojar->prioridad) {
            qcb_a_desalojar = qcb_leida;
        }
    }

    int total_workers = workers_conectados();

    //sem_wait(&sem_permiso_desalojo);
    //log_debug(logger, "Query %d obtuvo permiso para verificar desalojo", qcb_entrante->qid);
    log_debug(logger, "Se verifica el desalojo");

    t_qcb* query_de_mayor_prioridad = list_get(cola_ready, 0);
    pthread_mutex_lock(&mutex_diccionario_exec);
    int querys_en_exec = dictionary_size(diccionario_exec);
    
    if(total_workers == querys_en_exec && total_workers > 0){
        
        dictionary_iterator(diccionario_exec, *buscar_candidato_desalojo);
        pthread_mutex_unlock(&mutex_diccionario_exec); //mutex cierra aca o area critica mas grande?
        
        //no necesita mutex, chequeo desalojo siempre se hace en un mutex ready
        

        log_debug(logger, "Prioridad maxima en lista: %d", query_de_mayor_prioridad->prioridad);

        if(query_de_mayor_prioridad->prioridad < qcb_a_desalojar->prioridad){

            char *wid_str_asociado = string_itoa(qcb_a_desalojar->id_worker_asociado);

            //pthread_mutex_lock(&mutex_diccionario_exec);
            //dictionary_remove(diccionario_exec, wid_str_asociado);
            //pthread_mutex_unlock(&mutex_diccionario_exec);

            pthread_mutex_lock(&mutex_diccionario_workers);
            int* socket_worker_asignado_ptr = dictionary_get(diccionario_workers, wid_str_asociado);
            
            int socket_worker_asignado = *socket_worker_asignado_ptr;

            enviar_cod_op(socket_worker_asignado, INTERRUPCION); 
            
            log_info(logger, "## Se desaloja la Query <%d> (<%d>) del Worker <%d> - Motivo: <PRIORIDAD>", qcb_a_desalojar->qid, qcb_a_desalojar->prioridad, qcb_a_desalojar->id_worker_asociado);
            log_debug(logger, "## Se desaloja la Query <%d> (<%d>) del Worker <%d>", qcb_a_desalojar->qid, qcb_a_desalojar->prioridad, qcb_a_desalojar->id_worker_asociado);
            pthread_mutex_unlock(&mutex_diccionario_workers);
            //query_de_mayor_prioridad->
            free(wid_str_asociado);

        } else{
            log_warning(logger, "Hice chequeo desalojo pero no interrumpi");
            pthread_mutex_unlock(&mutex_diccionario_exec);
            query_de_mayor_prioridad->chequeo_desalojo_pendiente = false;
            
            //satisfacer_chequeos_desalojo();
            //sem_post(&sem_permiso_desalojo);
            //log_debug(logger, "Libero semaforo desalojo");
        }

    }else{
        log_warning(logger, "Quise chequear desalojo pero habian workers libres");
        pthread_mutex_unlock(&mutex_diccionario_exec);
        query_de_mayor_prioridad->chequeo_desalojo_pendiente = false;
        //satisfacer_chequeos_desalojo();
        //sem_post(&sem_permiso_desalojo);
        //log_debug(logger, "Libero semaforo desalojo");
    }

    
}

/* void satisfacer_chequeos_desalojo(){
    list_iterate(cola_ready, *satisfacer_chequeo_qcb);
}

void satisfacer_chequeo_qcb(void* arg){
    t_qcb* qcb = (t_qcb*)arg;
    qcb->chequeo_desalojo_pendiente = false;
} */



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

// void* hilo_aging_individual(void* arg){
//     t_qcb* qcb = (t_qcb*)arg;

//     int tiempo_aging_micro = tiempo_aging * 1000;

//     log_info(logger, "qid %d: Hilo de aging individual iniciado. Intervalo: %d ms", qcb->qid, tiempo_aging);

//     while(qcb->prioridad > 0){
//         usleep(tiempo_aging_micro); //importante, controla aging y permite pthread_cancel

//         pthread_mutex_lock(&mutex_ready); 
//         if (qcb->estado == READY){

//             qcb->prioridad--;
//             log_info(logger, "##<%d> Cambio de prioridad: <%d> - <%d>", qcb->qid, qcb->prioridad + 1, qcb->prioridad);

//             //list_sort(cola_ready, (void*)comparar_qcb_por_prioridad);

//             hacer_chequeo_desalojo(qcb); 
            
//         } /* else{ //no necesario, se mata el hilo cuando sale de ready
//             query_sigue_en_cola = false;
//             //temporal_destroy(qcb->tiempo_aging);
//         } */
//         pthread_mutex_unlock(&mutex_ready);
//     }
    
//     log_info(logger, "qid %d: Hilo de aging individual finalizo.", qcb->qid);
//     //free(arg);

//     pthread_mutex_lock(&mutex_aging);
//     qcb->aging_activo = false;
//     pthread_mutex_unlock(&mutex_aging);

//     return NULL;
// }
