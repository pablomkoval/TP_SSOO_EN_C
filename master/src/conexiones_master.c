#include <conexiones_master.h>

void *manejar_servidor_worker(void *arg){
    t_argumentos_worker *worker_args = (t_argumentos_worker *)arg;
    int socket_worker = worker_args->socket;
    int worker_id = worker_args->id;
    free(arg);

    while (1){
        int op_code = recibir_opcode(socket_worker);
        char* worker_id_str = string_itoa(worker_id);

        switch (op_code){
            case -1:
                hacer_desconexion_worker(worker_id);
                break;

            case READ:
                hacer_read_worker(socket_worker, worker_id_str);
                break;

            case END:
                hacer_end_worker(worker_id_str, worker_id);
                break;

            default:
                log_info(logger, "Error al recibir opcode, %d", op_code);
                break;
        }

        free(worker_id_str);
    }
    return NULL;
}

void *manejar_servidor_querycontrol(void *arg){
    int socket_cliente = *(int *)arg;
    free(arg);

    t_qcb* qcb = NULL;

    while(1){
        int op_code = recibir_opcode(socket_cliente);

        switch (op_code){
            case -1:
                manejar_desconexion_query_control(socket_cliente, qcb->qid);
                break;
                
            case PAQUETE:
                qcb = hacer_qcb_nueva(socket_cliente);
                break;

            default:
                log_error(logger, "Error al recibir opcode, %d", op_code);
                break;
        }
    }
    return NULL;
}

void *funcion_main_escucha(void *socket_arg){
    int socket_main_escucha = *(int *)socket_arg;

    while (1){
        int socket_cliente = esperar_cliente(socket_main_escucha, logger);
        int tipo_conexion = recibir_handshake(socket_cliente);

        pthread_t hilo_cliente;
        
        switch (tipo_conexion){
            case WORKER:
                log_info(logger, "Recibi handshake de un worker");
                int worker_id;
                recv(socket_cliente, &worker_id, sizeof(int), MSG_WAITALL);
                log_info(logger, "Conexion de Worker ID: %d", worker_id);
            
                t_argumentos_worker *args = malloc(sizeof(t_argumentos_worker));
                args->socket = socket_cliente;
                args->id = worker_id;

                char* worker_id_str = string_itoa(worker_id);
                int *socket_worker_ptr = malloc(sizeof(int));
                *socket_worker_ptr = socket_cliente;
                
                pthread_mutex_lock(&mutex_diccionario_workers);
                dictionary_put(diccionario_workers, worker_id_str, (void *)socket_worker_ptr);
                pthread_mutex_unlock(&mutex_diccionario_workers);

                int* worker_id_ptr = malloc(sizeof(int));
                *worker_id_ptr = worker_id;

                pthread_mutex_lock(&mutex_workers_libres);
                list_add(workers_libres, worker_id_ptr);
                pthread_mutex_unlock(&mutex_workers_libres);

                sem_post(&sem_workers_libres);

                pthread_create(&hilo_cliente, NULL, manejar_servidor_worker, (void *)args);
                pthread_detach(hilo_cliente);

                free(worker_id_str);
                break;

            case QUERY_CONTROL:
                log_trace(logger, "Recibi handshake de un query control");
                int *socket_query_ptr = malloc(sizeof(int));
                *socket_query_ptr = socket_cliente;

                pthread_create(&hilo_cliente, NULL, manejar_servidor_querycontrol, (void *)socket_query_ptr);
                pthread_detach(hilo_cliente);
                
                free(socket_query_ptr);
                break;

            case SIN_DEFINIR:
                break;
        }
    }
}

void hacer_desconexion_worker(int worker_id){
    log_info(logger, "Se cerro la conexion de un worker");
    
    pthread_mutex_lock(&mutex_diccionario_workers);
    dictionary_remove(diccionario_workers, string_itoa(worker_id));
    pthread_mutex_unlock(&mutex_diccionario_workers);
}

void hacer_read_worker(int socket_worker, char* worker_id_str){
    t_list* recibido = recibir_paquete(socket_worker);
    char* mensaje_worker = list_get(recibido, 0);

    pthread_mutex_lock(&mutex_diccionario_exec);
    t_qcb* qcb = dictionary_get(diccionario_exec, worker_id_str);
    pthread_mutex_unlock(&mutex_diccionario_exec);   

    log_info(logger,"Recibi mensaje de worker: %s, id: %s", mensaje_worker, worker_id_str);

    if(qcb->socket > 0){
        log_info(logger, "Reenviando mensaje a Query Control con socket: %d", qcb->socket);
        t_paquete* paquete = crear_paquete();
        cambiar_opcode_paquete(paquete, READ);
        agregar_a_paquete(paquete, mensaje_worker, strlen(mensaje_worker) + 1);
        enviar_paquete(paquete, qcb->socket, logger);
        borrar_paquete(paquete);
    } else{
        log_error(logger, "No se encontro qcb asociado a un worker id: %s", worker_id_str);
    }

    list_destroy_and_destroy_elements(recibido, free);
}

void hacer_end_worker(char* worker_id_str, int worker_id){
    
    pthread_mutex_lock(&mutex_diccionario_exec);
    t_qcb* qcb = dictionary_remove(diccionario_exec, worker_id_str);
    pthread_mutex_unlock(&mutex_diccionario_exec);

    log_info(logger, "## Se terminó la Query %d en el Worker %d", qcb->qid, worker_id);

    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, END);
    char* motivo = "Fin de instrucciones.";
    agregar_a_paquete(paquete, motivo, strlen(motivo) + 1);
    enviar_paquete(paquete, qcb->socket, logger);
    borrar_paquete(paquete);

    int* worker_id_ptr = malloc(sizeof(int));
    *worker_id_ptr = worker_id;

    pthread_mutex_lock(&mutex_workers_libres);
    list_add(workers_libres, worker_id_ptr);
    pthread_mutex_unlock(&mutex_workers_libres);

    sem_post(&sem_workers_libres);
}

t_qcb* hacer_qcb_nueva(int socket_cliente){
    log_info(logger, "Recibi paquete de query");

    t_list* elementos = recibir_paquete(socket_cliente);
    char* path_query = list_get(elementos, 0);
    int prioridad_query = *(int*)list_get(elementos,1);

    t_qcb* qcb = crear_qcb(path_query, prioridad_query, socket_cliente);
    char* qid_str = string_itoa(qcb->qid);

    pthread_mutex_lock(&mutex_diccionario_querys);
    dictionary_put(diccionario_querys, qid_str, qcb);
    pthread_mutex_unlock(&mutex_diccionario_querys);

    pthread_mutex_lock(&mutex_ready);
    list_add(cola_ready, qcb);
    pthread_mutex_unlock(&mutex_ready);

    sem_post(&sem_queries_ready);

    free(qid_str);

    log_info(logger, "Query recibida con id: %d, path: %s, prioridad: %d", id_query, path_query, prioridad_query);

    list_destroy_and_destroy_elements(elementos, free);

    return qcb;
}


void manejar_desconexion_query_control(int socket_cliente, int qid){
    log_info(logger, "Query control de qid %d, socket %d se desconecto. Iniciando desconexion.", qid, socket_cliente);

    
}


void encolar_qcb(t_list* cola_ready, t_qcb* qcb){ // if segun algoritmo de planificacion
//     if(strcmp(algoritmo_planificacion, "FIFO")){
//         t_list_add
//     }
} //me void a cenar