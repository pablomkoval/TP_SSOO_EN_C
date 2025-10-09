#include <conexiones_master.h>

void *manejar_servidor_worker(void *arg){
    int socket_cliente = *(int *)arg;
    free(arg);

    while (1)
    {
        int op_code = recibir_opcode(socket_cliente);

        if (op_code == -1)
        {
            log_info(logger, "Se cerro la conexiopn de un worker");
            break;
        }

        switch (op_code)
        {

        default:
            log_debug(logger, "Error al recibir opcode, %d", op_code);
            break;
        }
    }
    return NULL;
}

void *manejar_servidor_querycontrol(void *arg){
    int socket_cliente = *(int *)arg;
    free(arg);

    while (1){
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

                //fijarse q se manda mal la prioridad debe ser algo de punteros seguro
                int prioridad_query = list_get(elementos,1);

                log_info(logger, "Query recibida con path: %s, prioridad: %d", path_query, prioridad_query);

                list_destroy_and_destroy_elements(elementos, free);
                break;

            default:
                log_error(logger, "Error al recibir opcode, %d", op_code);
                break;
        }
    }
    return NULL;
}

void *funcion_main_escucha(void *socket_arg)
{
    pthread_t hilo_cliente;
    int socket_main_escucha = *(int *)socket_arg;

    while (1)
    {
        int socket_cliente = esperar_cliente(socket_main_escucha, logger);
        int tipo_conexion = recibir_handshake(socket_cliente);

        switch (tipo_conexion)
        {
        case WORKER:
            log_trace(logger, "Recibi handshake de un worker");
            int worker_id;
            recv(socket_cliente, &worker_id, sizeof(int), MSG_WAITALL);
            log_trace(logger, "Conexion de Worker ID: %d", worker_id);
            t_argumentos_worker *args = malloc(sizeof(t_argumentos_worker));
            args->socket = socket_cliente;
            args->id = worker_id;

            pthread_create(&hilo_cliente, NULL, manejar_servidor_worker, (void *)args);
            pthread_detach(hilo_cliente);
            // agregar el socket a una lista de workers
            break;

        case QUERY_CONTROL:
            log_trace(logger, "Recibi handshake de un query control");
            int *socket_cliente_ptr = malloc(sizeof(int));
            *socket_cliente_ptr = socket_cliente;
            // agregar el socket a una lista de qc's
            pthread_create(&hilo_cliente, NULL, manejar_servidor_querycontrol, (void *)socket_cliente_ptr);
            pthread_detach(hilo_cliente);

            log_info(logger, "aaaa");
            break;

        case SIN_DEFINIR:
            break;
        }
    }
}