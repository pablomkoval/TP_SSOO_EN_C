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
                char* query_id;
                char* file;
                char* tag;

                if(crear_file(file, tag) == 1)
                {
                    log_info(logger, "##<%s> - File Creado <%s>:<%s>", query_id, file, tag);
                    log_info(logger, "##<%s> - Tag Creado <%s>:<%s>", query_id, file, tag);
                }else
                {

                }

                break;
            case TRUNCATE:

                char* query_id;
                int tamanio;
                char* file;
                char* tag;

                if(truncar_archivo(tamanio, file, tag) == 1)
                {
                    log_info(logger, "##<%s> - File Truncado <%s>:<%s> - Tamaño: <%s>", query_id, file, tag, tamanio);
                }

                break;
            case TAG:
                char* query_id;
                char* file_origen;
                char* file_destino;
                char* tag_origen;
                char* tag_destino;

                if(copiar_tag(origen, origen, destino, destino) == 1)
                {
                    log_info(logger, "##<%s> - File Creado <%s>:<%s>", query_id, file_destino, tag_destino);
                    log_info(logger, "##<%s> - Tag Creado <%s>:<%s>", query_id, file_destino, tag_destino);
                }

                break;
            case COMMIT:
                char* query_id;

                if(commmit_file(file, tag) == 1)
                {
                    log_info(logger, "##<%s> - Commit de File:tag <%s>:<%s>", query_id, file, tag);
                }

                break;
            case WRITE:
                char* query_id;

                if(escribir_bloque(path, offet, contenido) == 1)
                {
                    log_info(logger, "##<%s> - Bloque Lógico Escrito <%s>:<%s> - Número de Bloque <%s>", query_id, file, tag, bloque);
                }

                break;
            case READ:
                char* query_id;
                char* data;
                char* path;
                int offset;
                int size;
                char* bloque;

                if(leer_bloque(path, offset, size, &data) == 1)
                {
                    log_info(logger, "##<%s> - Bloque Lógico Leído <%s>:<%s> - Número de Bloque <%s>", query_id, file, tag, bloque);
                }

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