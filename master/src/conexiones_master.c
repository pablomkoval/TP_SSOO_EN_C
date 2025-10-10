#include <conexiones_master.h>

void *manejar_servidor_worker(void *arg){
    t_argumentos_worker *worker_args = (t_argumentos_worker *)arg;
    int socket_cliente = worker_args->socket;
    int worker_id = worker_args->id;
    free(arg);

    while (1){
        int op_code = recibir_opcode(socket_cliente);

        if (op_code == -1){
            log_info(logger, "Se cerro la conexion de un worker");
            //mutex
            dictionary_remove(diccionario_workers, string_itoa(worker_id));
            //mutex
            break;
        }

        switch (op_code){
            case MENSAJE:
                char* mensaje_worker = recibir_mensaje(socket_cliente);

                //mutex¿ o uno mas abajo nose
                char* worker_id_str = string_itoa(worker_id);

                int* socket_qc_ptr = dictionary_get(diccionario_querys, worker_id_str);
                //mutex

                log_info(logger,"Recibi mensaje de worker: %s, id: %s", mensaje_worker, worker_id_str);

                free(worker_id_str);

                if(socket_qc_ptr != NULL) {
                    log_info(logger, "Reenviando mensaje a Query Control con socket: %d", *socket_qc_ptr);
                    enviar_mensaje(*socket_qc_ptr, mensaje_worker);
                } else{
                    log_error(logger, "No se encontró Query Control asociado al Worker ID: %d", worker_id);
                }

                free(mensaje_worker);

                break;
            default:
                log_info(logger, "Error al recibir opcode, %d", op_code);
                break;
        }
    }
    return NULL;
}

void *manejar_servidor_querycontrol(void *arg){
    int socket_cliente = *(int *)arg;
    free(arg);

    while(1){
        int op_code = recibir_opcode(socket_cliente);

        if (op_code == -1){
            log_info(logger, "Se cerro la conexion de un query control");
            close(socket_cliente);
            break;
        }

        switch (op_code){
            case PAQUETE:
                log_info(logger, "Recibi paquete de query");

                t_list* elementos = recibir_paquete(socket_cliente);
                char* path_query = list_get(elementos, 0);
                int prioridad_query = *(int*)list_get(elementos,1);

                //guardar este valor en algun lado, una lista o paquete o algo
                int id_query = id_query++

                log_info(logger, "Query recibida con id: %d, path: %s, prioridad: %d", id_query, path_query, prioridad_query);

                list_destroy_and_destroy_elements(elementos, free);
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

                dictionary_put(diccionario_workers, worker_id_str, (void *)socket_worker_ptr);

                pthread_create(&hilo_cliente, NULL, manejar_servidor_worker, (void *)args);
                pthread_detach(hilo_cliente);

                // agregar el socket a una lista de workers

                free(worker_id_str);
                break;

            case QUERY_CONTROL:
                log_trace(logger, "Recibi handshake de un query control");
                int *socket_query_ptr = malloc(sizeof(int));
                *socket_query_ptr = socket_cliente;
                // agregar el socket a una lista de qc's
                pthread_create(&hilo_cliente, NULL, manejar_servidor_querycontrol, (void *)socket_query_ptr);
                pthread_detach(hilo_cliente);
                
                free(socket_query_ptr);
                break;

            case SIN_DEFINIR:
                break;
        }
    }
}