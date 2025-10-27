#include <conexiones_master.h>

void *manejar_servidor_worker(void *arg){
    t_argumentos_worker *worker_args = (t_argumentos_worker *)arg;
    int socket_worker = worker_args->socket;
    int worker_id = worker_args->id;
    free(arg);

    while (1){
        int op_code = recibir_opcode(socket_worker);
        char *worker_id_str = string_itoa(worker_id);

        switch (op_code){
            case -1:
                hacer_desconexion_worker(worker_id);
                free(worker_id_str);
                return NULL;
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

void* manejar_servidor_querycontrol(void* arg){
    int socket_cliente = *((int*)arg);

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

            default:
                log_error(logger, "Error al recibir opcode, %d", op_code);
                break;
        }
    }
    free(arg);
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

                t_argumentos_worker *args = malloc(sizeof(t_argumentos_worker));
                args->socket = socket_cliente;
                args->id = worker_id;

                char *worker_id_str = string_itoa(worker_id);
                int *socket_worker_ptr = malloc(sizeof(int));
                *socket_worker_ptr = socket_cliente;

                pthread_mutex_lock(&mutex_diccionario_workers);
                dictionary_put(diccionario_workers, worker_id_str, (void *)socket_worker_ptr);
                pthread_mutex_unlock(&mutex_diccionario_workers);


                log_info(logger, "## Se conecta el Worker <%d> - Cantidad total de Workers: <%d>", worker_id, workers_conectados());

                int *worker_id_ptr = malloc(sizeof(int));
                *worker_id_ptr = worker_id;

                pthread_mutex_lock(&mutex_workers_libres);
                list_add(workers_libres, worker_id_ptr);
                pthread_mutex_unlock(&mutex_workers_libres);
                sem_post(&sem_workers_libres);
                
                pthread_create(&hilo_cliente, NULL, manejar_servidor_worker, (void *)args);
                pthread_detach(hilo_cliente);
                
                break;

            case QUERY_CONTROL:
                log_trace(logger, "Recibi handshake de un query control");
                int* socket_query_ptr = malloc(sizeof(int));
                *socket_query_ptr = socket_cliente;

                pthread_create(&hilo_cliente, NULL, manejar_servidor_querycontrol, socket_query_ptr);
                pthread_detach(hilo_cliente);
                break;

            case SIN_DEFINIR:
                break;
        }
    }
}

void hacer_desconexion_worker(int worker_id){
    
    char* wid_str = string_itoa(worker_id);
    

    pthread_mutex_lock(&mutex_diccionario_exec);
    t_qcb * qcb = dictionary_remove(diccionario_exec, wid_str);
    pthread_mutex_unlock(&mutex_diccionario_exec);

    t_paquete *paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, END);
    char *motivo = "Desconexion de Worker.";
    agregar_a_paquete(paquete, motivo, strlen(motivo) + 1);
    enviar_paquete(paquete, qcb->socket, logger);
    borrar_paquete(paquete);

    pthread_mutex_lock(&mutex_diccionario_workers);
    dictionary_remove(diccionario_workers, wid_str);
    pthread_mutex_unlock(&mutex_diccionario_workers);


    log_info(logger, "## Se desconecta el Worker <%d> - Se finaliza la Query <%d> - Cantidad total de Workers: <%d> ", worker_id, qcb->qid, workers_conectados());

}

void hacer_read_worker(int socket_worker, char *worker_id_str){
    t_list *recibido = recibir_paquete(socket_worker);
    char *mensaje_worker = list_get(recibido, 0);

    pthread_mutex_lock(&mutex_diccionario_exec);
    t_qcb *qcb = dictionary_get(diccionario_exec, worker_id_str);
    pthread_mutex_unlock(&mutex_diccionario_exec);

    if (qcb->socket > 0){

        
        t_paquete *paquete = crear_paquete();
        cambiar_opcode_paquete(paquete, READ);
        agregar_a_paquete(paquete, mensaje_worker, strlen(mensaje_worker) + 1);
        enviar_paquete(paquete, qcb->socket, logger);
        borrar_paquete(paquete);

        log_info(logger, "mensaje del worker a enviar: %s", mensaje_worker);
        log_info(logger, "## Se envía un mensaje de lectura de la Query <%d> en el Worker <%s> al Query Control", qcb->qid, worker_id_str);

    }   else{
        log_error(logger, "No se encontro qcb asociado a un worker id: %s", worker_id_str);
    }
    
    free(mensaje_worker);
    list_destroy_and_destroy_elements(recibido, free);
}

void hacer_end_worker(char *worker_id_str, int worker_id){

    pthread_mutex_lock(&mutex_diccionario_exec);
    t_qcb *qcb = dictionary_remove(diccionario_exec, worker_id_str);
    pthread_mutex_unlock(&mutex_diccionario_exec);

    log_info(logger, "## Se terminó la Query %d en el Worker %d", qcb->qid, worker_id);

    t_paquete *paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, END);
    char *motivo = "Fin de instrucciones.";
    agregar_a_paquete(paquete, motivo, strlen(motivo) + 1);
    enviar_paquete(paquete, qcb->socket, logger);
    borrar_paquete(paquete);

    int *worker_id_ptr = malloc(sizeof(int));
    *worker_id_ptr = worker_id;

    pthread_mutex_lock(&mutex_workers_libres);
    list_add(workers_libres, worker_id_ptr);
    pthread_mutex_unlock(&mutex_workers_libres);

    sem_post(&sem_workers_libres);
}

t_qcb *hacer_qcb_nueva(int socket_cliente){
    log_info(logger, "Recibi paquete de query");

    t_list *elementos = recibir_paquete(socket_cliente);
    char *path_query = list_get(elementos, 0);
    int prioridad_query = *(int *)list_get(elementos, 1);

    t_qcb *qcb = crear_qcb(path_query, prioridad_query, socket_cliente);
    char *qid_str = string_itoa(qcb->qid);

    pthread_mutex_lock(&mutex_diccionario_querys);
    dictionary_put(diccionario_querys, qid_str, qcb);
    pthread_mutex_unlock(&mutex_diccionario_querys);

    pthread_mutex_lock(&mutex_ready);
    encolar_qcb(cola_ready, qcb);
    pthread_mutex_unlock(&mutex_ready);

    

    free(qid_str);
    
    log_info(logger, "## Se conecta un Query Control para ejecutar la Query <%s> con prioridad <%d> - Id asignado: <%d>. Nivel multiprocesamiento <%d>",path_query, prioridad_query, qcb->qid, workers_conectados());

    /* if (strcmp(algoritmo_planificacion, "PRIORIDADES") == 0) { se termina moviendo a encolar_qcb
        //realizar_chequeo_desalojo(qcb); tikitititktkikitikt
    } */

    list_destroy_and_destroy_elements(elementos, free);

    return qcb;
}
// if segun algoritmo de planificacion
void hacer_desconexion_query_control(int socket_cliente, t_qcb *qcb){
    /* pthread_mutex_lock(&mutex_diccionario_querys);
    t_qcb* qcb = dictionary_get(diccionario_querys, qid_str);
    pthread_mutex_unlock(&mutex_diccionario_querys); */

    char* qid_str = string_itoa(qcb->qid);

    if(qcb->estado == READY){
    
        list_remove_element(cola_ready, qcb);   

        pthread_mutex_lock(&mutex_diccionario_querys);
        dictionary_remove(diccionario_querys,qid_str);
        pthread_mutex_unlock(&mutex_diccionario_querys);

        cambiar_estado(qcb, EXIT);

    } else if (qcb->estado == EXEC){
        char* wid_asociado_str = string_itoa(qcb->id_worker_asociado);
        pthread_mutex_lock(&mutex_diccionario_workers);
        int* socket_worker_asociado = dictionary_get(diccionario_workers, wid_asociado_str);
        pthread_mutex_unlock(&mutex_diccionario_workers);
        
        
        //enviar_cod_op(&socket_worker_asociado, DESALOJAR);
    }
    
    log_info(logger, "## Se desconecta un Query Control. Se finaliza la Query <%d> con prioridad <%d>. Nivel multiprocesamiento <%d>", qcb->qid, qcb->prioridad, workers_conectados());

    free(qid_str);

    //poner mutexs para desalojo en exec?
}

bool qcb_esta_en_cola_ready(void* arg, int socket_buscado){
    t_qcb* qcb = (t_qcb*)arg;
    return qcb->socket == socket_buscado;
}

void encolar_qcb(t_list *cola_ready, t_qcb *qcb){
    if (strcmp(algoritmo_planificacion, "PRIORIDADES") == 0){




        list_add_sorted(cola_ready, qcb, (void*)comparar_qcb_por_prioridad);
        log_info(logger, "qcb de qid: %d encolado en READY con Prioridad: %d", qcb->qid, qcb->prioridad);

    } else{

        list_add(cola_ready, qcb);
        log_info(logger, "qcb de qid: %d encolado en READY con FIFO.", qcb->qid);
    }

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

void realizar_chequeo_desalojo(t_qcb* qcb){
    ////tiki tiki tiki tiki
}


// void hacer_chequeo_desalojo(t_qcb* qcb){

//     int total_workers = workers_conectados();

//     pthread_mutex_lock(&mutex_diccionario_exec);
//     int querys_en_exec = dictionary_size(diccionario_exec);

//     if(total_workers == querys_en_exec && total_workers > 0){

//         t_qcb* qcb_menor_prioridad = NULL;


//         // void buscar_candidato_desalojo(char* wid_str, void* qcb_exec_ptr){
//         //     t_qcb* qcb_exec = (t_qcb)qcb_exec_ptr;

//         //     if (qcb_menor_prioridad == NULL || qcb_exec->prioridad > qcb_menor_prioridad->prioridad) {
//         //         qcb_menor_prioridad = qcb_exec;
//         //     }
//         // }

//         dictionary_iterator(diccionario_exec, buscar_candidato_desalojo);

//         if (qcb_menor_prioridad != NULL){
//             int socket_worker_asignado =((int)dictionary_get(diccionario_workers, qcb_menor_prioridad -> id_worker_asociado);

//             t_paquete paquete = crear_paquete();
//             cambiar_opcode_paquete(paquete, DESALOJAR);

//         }

//     } else{

//     }
// }