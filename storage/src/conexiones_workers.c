#include <conexiones_workers.h>



void* manejar_conexion_worker(void* arg) {
    int socket_cliente = *((int*)arg);
    free(arg);
    while(1) {
        int codigo_operacion = recibir_opcode(socket_cliente);
        if (codigo_operacion < 0) {
            log_warning(logger, "Worker desconectado");
            break;
        }

        switch(codigo_operacion) {
            case CREATE:

                char* file;
                char* tag;

                crear_file(file, tag);

                break;
            case TRUNCATE:

                int tamanio;
                char* file_tag;

                truncar_archivo(tamanio, file_tag);

                break;
            case TAG:

                char* origen;
                char* destino;

                copiar_tag(origen, origen, destino, destino);

                break;
            case COMMIT:

                break;
            case WRITE:

                break;
            case READ:

                char* data;
                char* path;
                int offset;
                int size;

                leer_bloque(path, offset, size, &data);

                break;
            case DELETE:

                break;
            default:
                log_error(logger, "Operación worker desconocida: %d", codigo_operacion);
                break;
        }
    }
    close(socket_cliente);
    return NULL;
}



void* manejar_conexiones_memoria(void* socket_ptr) {
    int socket_cliente = *((int*)socket_ptr);
    free(socket_ptr);

    if (recibir_handshake(socket_cliente) == SIN_DEFINIR) {
        log_trace(logger, "Recibi el handshake de un WORKER");

        int* socket_worker = malloc(sizeof(int));
        *socket_worker = socket_cliente;
        log_trace(logger, "socket worker: %d", *socket_worker);
        
        pthread_t hilo_worker;
        pthread_create(&hilo_worker, NULL, (void*)manejar_conexion_worker, socket_worker);
        pthread_detach(hilo_worker);
        return NULL;
    }
    return NULL;
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