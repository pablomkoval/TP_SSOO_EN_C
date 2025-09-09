#include <conexiones_workers.h>



void* manejar_conexion_worker(void* arg) {
    int socket_cliente = *((int*)arg);
    free(arg);
    while(1) {
        int codigo_operacion = recibir_operacion(socket_cliente);
        if (codigo_operacion < 0) {
            log_warning(logger, "CPU desconectada");
            break;
        }

        switch(codigo_operacion) {
            case CERRADO:
                log_trace(logger, "Se cerro la conexion con CPU");
                break;
            default:
                log_error(logger, "Operación CPU desconocida: %d", codigo_operacion);
                break;
        }
    }
    close(socket_cliente);
    return NULL;
}



void* manejar_conexiones_memoria(void* socket_ptr) {
    int socket_cliente = *((int*)socket_ptr);
    free(socket_ptr);

    int codigo_operacion = recibir_operacion(socket_cliente);
    
    if (codigo_operacion == HANDSHAKE) {
        log_trace(logger, "Recibi el handshake de un WORKER");
        t_paquete* paquete = crear_paquete();
        cambiar_opcode_paquete(paquete, OK);

        op_code respuesta = OK;
        send(socket_cliente, &respuesta, sizeof(int), 0);
    

        int* socket_worker = malloc(sizeof(int));
        *socket_worker = socket_cliente;
        log_trace(logger, "socket worker: %d", *socket_worker);
        
        pthread_t hilo_worker;
        pthread_create(&hilo_worker, NULL, (void*)manejar_conexion_worker, socket_worker);
        pthread_detach(hilo_worker);
        return NULL;
    }
    else {
        log_error(logger, "Handshake invalido: %d", codigo_operacion);
        close(socket_cliente);
        return NULL;
    }
}

void* manejar_servidor(void* socket_ptr) 
{
    //por cada accept esta funcion tira un hilo
    int socket_servidor = *((int*)socket_ptr);
    free(socket_ptr);
    while (1) {
        int socket_cliente = esperar_cliente(socket_servidor, logger);

        int* socket_cliente_ptr = malloc(sizeof(int));
        *socket_cliente_ptr = socket_cliente;

        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente, NULL, manejar_conexiones_memoria, socket_cliente_ptr);
        pthread_detach(hilo_cliente);
    }
    return NULL;
}

void* lanzar_servidor(int socket_servidor)
{
    //este es el hilo main que lanza todas las conexiones
    pthread_t hilo_conexion;

    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket_servidor;

    pthread_create(&hilo_conexion, NULL, manejar_servidor, socket_ptr);
    pthread_detach(hilo_conexion);

    return NULL;
}