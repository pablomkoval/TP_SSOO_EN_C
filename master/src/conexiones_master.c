#include <conexiones_master.h>

void *manejar_servidor_worker(void *arg){
    t_argumentos_worker *worker_args = (t_argumentos_worker *)arg;
    int socket_worker = worker_args->socket;
    int worker_id = worker_args->id;
    free(arg);

    while (1){
        int op_code = recibir_opcode(socket_worker);
        //char *worker_id_str = string_itoa(worker_id);
        //log_trace(logger, "Llego opcode de worker %d", op_code);
        log_trace(logger, "Llego opcode del worker [%d]: %s (%d)", worker_id ,obtener_nombre_opcode_simple(op_code), op_code);
        switch (op_code){
            case -1:
                hacer_desconexion_worker(worker_id);
                //free(worker_id_str);
                return NULL;

            case READ:
                pthread_mutex_lock(&mutex_diccionario_exec);
                hacer_read_worker(socket_worker, worker_id);
                break;

            case END:
                log_trace(logger, "## Se recibe END de Worker (%d)", worker_id);
                hacer_end_worker(worker_id, socket_worker);
                break;

            case INTERRUPCION_RTA:
                //log_warning(logger, "Recibi respuesta interrupcion de worker");
                t_list* recibido = recibir_paquete(socket_worker);


                int* qid_ptr = (int*)list_get(recibido, 0);
                int* pc = (int*)list_get(recibido, 1);
                
                char* qid_str = string_itoa(*qid_ptr);

                pthread_mutex_lock(&mutex_diccionario_querys);
                t_qcb* qcb_desalojada = dictionary_get(diccionario_querys, qid_str);
                pthread_mutex_unlock(&mutex_diccionario_querys);
                
                
                if (qcb_desalojada->estado != EXIT){ // qcb desalojada NO por desconexion de query control
                    char *worker_id_str = string_itoa(worker_id);

                    pthread_mutex_lock(&mutex_diccionario_exec);
                    dictionary_remove(diccionario_exec, worker_id_str);
                    log_trace(logger, "Quito de d_exec query [%d]", qcb_desalojada->qid);
                    pthread_mutex_unlock(&mutex_diccionario_exec);

                    //sem_post(&sem_permiso_desalojo);
                    log_debug(logger, "Libero semaforo desalojo");
                    qcb_desalojada->pc = *pc;
                    encolar_qcb(qcb_desalojada);
                }
                sumar_worker_libre(worker_id);

                pthread_mutex_lock(&mutex_ready);
                satisfacer_chequeos_desalojo();
                pthread_mutex_lock(&mutex_ready);

                list_destroy_and_destroy_elements(recibido, free);
                break;

            case 0:
                //hacer_desconexion_worker(worker_id);
                //return NULL;
                break;

            default:
                log_info(logger, "Error al recibir opcode (%d) de worker", op_code);
            break;
        }
        
        //free(worker_id_str);
    }
    return NULL;
}

void* manejar_servidor_querycontrol(void* arg){
    //int socket_cliente = *((int*)arg);
    int socket_cliente = (int)(intptr_t)arg;
    t_qcb* qcb = NULL;

    while (1){
        int op_code = recibir_opcode(socket_cliente);

        switch (op_code){
            case -1:
                if (qcb != NULL){
                    hacer_desconexion_query_control(socket_cliente, qcb);
                } else{
                    log_error(logger, "Error al recibir paquete de query conectada.");
                }
                return NULL;
            case PAQUETE:
                qcb = hacer_qcb_nueva(socket_cliente);
                break;
            case 0:
                break;
            default:
                log_trace(logger, "Error al recibir opcode (%d) de query control", op_code);
                break;
        }
    }
    //free(arg);
    return NULL;
}

void *funcion_main_escucha(void *socket_arg){
    int socket_main_escucha = *(int *)socket_arg;

    while (1){
    
        int socket_cliente = esperar_cliente(socket_main_escucha, logger);
        int tipo_conexion = recibir_handshake(socket_cliente);

        pthread_t hilo_cliente;

        switch (tipo_conexion){
            case WORKER: //se puede derivar lo de este case para que quede clean como el case query
                log_info(logger, "Recibi handshake de un worker");
                int worker_id;
                recv(socket_cliente, &worker_id, sizeof(int), MSG_WAITALL);

                t_argumentos_worker *args = malloc(sizeof(t_argumentos_worker));
                args->socket = socket_cliente;
                args->id = worker_id;

                char* worker_id_str = string_itoa(worker_id);
                int *socket_worker_ptr = malloc(sizeof(int));
                *socket_worker_ptr = socket_cliente;

                pthread_mutex_lock(&mutex_diccionario_workers);
                dictionary_put(diccionario_workers, worker_id_str, socket_worker_ptr);
                pthread_mutex_unlock(&mutex_diccionario_workers);

                free(worker_id_str);

                log_info(logger, "## Se conecta el Worker <%d> - Cantidad total de Workers: <%d>", worker_id, workers_conectados());

                sumar_worker_libre(worker_id);
                
                pthread_create(&hilo_cliente, NULL, manejar_servidor_worker, (void *)args);
                pthread_detach(hilo_cliente);
                
                break;

            case QUERY_CONTROL:
                log_trace(logger, "Recibi handshake de un query control");
                // int* socket_query_ptr = malloc(sizeof(int));
                // *socket_query_ptr = socket_cliente;

                //pthread_create(&hilo_cliente, NULL, manejar_servidor_querycontrol, socket_query_ptr);
                pthread_create(&hilo_cliente, NULL, manejar_servidor_querycontrol, (void*)(intptr_t)socket_cliente);
                pthread_detach(hilo_cliente);
                break;

            case SIN_DEFINIR:
                break;
        }
    }
}

void hacer_desconexion_worker(int worker_id){
    
    char* wid_str = string_itoa(worker_id);
    
    pthread_mutex_lock(&mutex_diccionario_workers);
    dictionary_remove(diccionario_workers, wid_str);
    pthread_mutex_unlock(&mutex_diccionario_workers);


    pthread_mutex_lock(&mutex_ready);
    pthread_mutex_lock(&mutex_diccionario_exec);
    t_qcb* qcb = dictionary_remove(diccionario_exec, wid_str);
    pthread_mutex_unlock(&mutex_diccionario_exec);

    pthread_mutex_lock(&mutex_diccionario_workers);
    dictionary_remove_and_destroy(diccionario_workers, wid_str, free);
    // int* socket_worker_ptr = dictionary_remove(diccionario_workers, wid_str);
    pthread_mutex_unlock(&mutex_diccionario_workers);
    //free(socket_worker_ptr);
    if(qcb != NULL){
        log_trace(logger, "Quito de d_exec query [%d]", qcb->qid);
        t_paquete *paquete = crear_paquete();
        cambiar_opcode_paquete(paquete, END);
        char *motivo = "Desconexion de Worker.";
        agregar_a_paquete(paquete, motivo, strlen(motivo) + 1);
        enviar_paquete(paquete, qcb->socket, logger);
        borrar_paquete(paquete);

        
        cambiar_estado(qcb, EXIT);

        log_info(logger, "## Se desconecta el Worker <%d> - Se finaliza la Query <%d> - Cantidad total de Workers: <%d> ", worker_id, qcb->qid, workers_conectados());
    }
    pthread_mutex_unlock(&mutex_ready);

    free(wid_str);
    log_info(logger, "## Se desconecta el Worker <%d> - Cantidad total de Workers: <%d> ", worker_id, workers_conectados());
}

void hacer_read_worker(int socket_worker, int worker_id){
    t_list *recibido = recibir_paquete(socket_worker);
    char* file_tag = list_get(recibido, 0);
    char *mensaje_worker = list_get(recibido, 1);

    char* worker_id_str = string_itoa(worker_id);

    //pthread_mutex_lock(&mutex_diccionario_exec);

    t_qcb *qcb = dictionary_get(diccionario_exec, worker_id_str);
    
    
    pthread_mutex_unlock(&mutex_diccionario_exec);
    
    if(qcb == NULL){
        log_error(logger, "Quise hacer un read pero fallo el diccionario exec");
        free(worker_id_str); 
        list_destroy_and_destroy_elements(recibido, free);
        return;
    }    

    if (qcb->socket > 0){

        log_trace(logger, "socket qcb es %d", qcb->socket);        
        t_paquete *paquete = crear_paquete();
        cambiar_opcode_paquete(paquete, READ);
        agregar_a_paquete(paquete, file_tag, strlen(file_tag) + 1);
        agregar_a_paquete(paquete, mensaje_worker, strlen(mensaje_worker) + 1);
        enviar_paquete(paquete, qcb->socket, logger);
        borrar_paquete(paquete);

        log_info(logger, "mensaje del worker a enviar: %s", mensaje_worker);
        log_info(logger, "## Se envía un mensaje de lectura de la Query <%d> en el Worker <%s> al Query Control", qcb->qid, worker_id_str);
        
    } else {
        log_error(logger, "Socket de query inexistente");
    }
    free(worker_id_str);
    list_destroy_and_destroy_elements(recibido, free);
}

void hacer_end_worker(int worker_id, int socket_worker){

    t_list* recibido = recibir_paquete(socket_worker);

    char* motivo_r = (char*)list_get(recibido, 0);
    char* motivo = strdup(motivo_r);
    list_destroy_and_destroy_elements(recibido, free);
    log_debug(logger, "## Query Finalizada - %s", motivo);

    char *worker_id_str = string_itoa(worker_id);

    pthread_mutex_lock(&mutex_diccionario_exec);
    t_qcb *qcb = dictionary_remove(diccionario_exec, worker_id_str);
    
    pthread_mutex_unlock(&mutex_diccionario_exec);

    if (qcb == NULL) {
        log_warning(logger, "Worker %d envió END, pero la Query ya fue procesada o no existía.", worker_id);
        free(worker_id_str); // Liberar la clave temporal antes de salir
        free(motivo); // Liberar la cadena duplicada
        return; 
    }

    log_trace(logger, "Quito de d_exec query [%d]", qcb->qid);

    log_info(logger, "## Se terminó la Query %d en el Worker %d", qcb->qid, worker_id);

    cambiar_estado(qcb, EXIT);

    t_paquete *paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, END);
    agregar_a_paquete(paquete, motivo, strlen(motivo) + 1);
    enviar_paquete(paquete, qcb->socket, logger);
    borrar_paquete(paquete);

    free(motivo);
    //free(worker_id_str);

    sumar_worker_libre(worker_id);

    free(worker_id_str);
}

t_qcb *hacer_qcb_nueva(int socket_cliente){
    log_info(logger, "Recibi paquete de query");

    t_list *elementos = recibir_paquete(socket_cliente);
    char *path_query = list_get(elementos, 0);
    int prioridad_query = *(int *)list_get(elementos, 1);

    t_qcb *qcb = crear_qcb(path_query, prioridad_query, socket_cliente);
    char *qid_str = string_itoa(qcb->qid);
    encolar_qcb(qcb);

    pthread_mutex_lock(&mutex_diccionario_querys);
    dictionary_put(diccionario_querys, qid_str, qcb);
    pthread_mutex_unlock(&mutex_diccionario_querys);
    log_debug(logger, "Llego query, aniadida a diccionario");

    //pthread_mutex_lock(&mutex_ready);
    //cambiar_estado(qcb, READY);
    //pthread_mutex_unlock(&mutex_ready);

    free(qid_str);


    log_info(logger, "## Se conecta un Query Control para ejecutar la Query <%s> con prioridad <%d> - Id asignado: <%d>. Nivel multiprocesamiento <%d>",path_query, prioridad_query, qcb->qid, workers_conectados());

    list_destroy_and_destroy_elements(elementos, free);

    return qcb;
}


void hacer_desconexion_query_control(int socket_cliente, t_qcb *qcb){

    char* qid_str = string_itoa(qcb->qid);

    log_debug(logger, "Lockeo mutex ready en linea 269 cm");
    pthread_mutex_lock(&mutex_ready);
    if(qcb->estado == READY){

        list_remove_element(cola_ready, qcb);    
        cambiar_estado(qcb, EXIT);
    } else if (qcb->estado == EXEC){

        char* wid_asociado_str = string_itoa(qcb->id_worker_asociado);
        log_trace(logger, "El Worker ID asociado a la query que va a terminar es: (%s)", wid_asociado_str);
        pthread_mutex_lock(&mutex_diccionario_workers);
        int* socket_worker_asociado_ptr = dictionary_get(diccionario_workers, wid_asociado_str);
        pthread_mutex_unlock(&mutex_diccionario_workers);

        pthread_mutex_lock(&mutex_diccionario_exec);
        dictionary_remove(diccionario_exec, wid_asociado_str);
        log_trace(logger, "Quito de d_exec query [%d]", qcb->qid);
        pthread_mutex_unlock(&mutex_diccionario_exec);


        int socket_worker_asociado = *socket_worker_asociado_ptr;

        enviar_cod_op(socket_worker_asociado, INTERRUPCION);
        cambiar_estado(qcb, EXIT);
    }
    pthread_mutex_unlock(&mutex_ready);
    
    log_info(logger, "## Se desconecta un Query Control. Se finaliza la Query <%d> con prioridad <%d>. Nivel multiprocesamiento <%d>", qcb->qid, qcb->prioridad, workers_conectados());

    free(qid_str);

}

bool qcb_esta_en_cola_ready(void* arg, int socket_buscado){
    t_qcb* qcb = (t_qcb*)arg;
    return qcb->socket == socket_buscado;
}

void encolar_qcb(t_qcb *qcb){
    log_debug(logger, "consumo mutex ready en encolar_qcb");
    if (strcmp(algoritmo_planificacion, "PRIORIDADES") == 0){
    
        pthread_mutex_lock(&mutex_ready);
        list_add_sorted(cola_ready, qcb, (void*)comparar_qcb_por_prioridad);
        hacer_chequeo_desalojo();
        cambiar_estado(qcb, READY);
        pthread_mutex_unlock(&mutex_ready);
        log_info(logger, "qcb de qid: %d encolado en READY con Prioridad: %d", qcb->qid, qcb->prioridad);

    } else{
        //log_warning(logger, "## Va a encolar qcb con fifo");
        pthread_mutex_lock(&mutex_ready);
        list_add(cola_ready, qcb);
        cambiar_estado(qcb, READY);
        pthread_mutex_unlock(&mutex_ready);
        log_info(logger, "qcb de qid: %d encolado en READY con FIFO.", qcb->qid);
    }
    log_debug(logger, "libero mutex ready en encolar_qcb");
    sem_post(&sem_queries_ready);
}

bool comparar_qcb_por_prioridad(void* qcb1, void* qcb2){
    t_qcb *qcb_menor = (t_qcb *)qcb1;
    t_qcb *qcb_mayor = (t_qcb *)qcb2;

    return qcb_menor->prioridad < qcb_mayor->prioridad; 
}

int workers_conectados(){
    pthread_mutex_lock(&mutex_diccionario_workers);
    int workers_conectados = dictionary_size(diccionario_workers);
    pthread_mutex_unlock(&mutex_diccionario_workers);
    return workers_conectados;
}




void sumar_worker_libre(int worker_id){
    
    int *worker_id_ptr = malloc(sizeof(int));
    *worker_id_ptr = worker_id;
    
    pthread_mutex_lock(&mutex_workers_libres);
    list_add(workers_libres, worker_id_ptr);
    pthread_mutex_unlock(&mutex_workers_libres);

    sem_post(&sem_workers_libres);
    log_debug(logger, "Se agrega el worker %d al diccionario de workers libres y se hace sem_post", worker_id);
}