#include <conexiones_workers.h>



void* manejar_conexion_worker(void* arg) {
    int socket_cliente = *((int*)arg);
    free(arg);

    char* query_id;
    char* file;
    char* tag;
    int tamanio;
    char* file_origen;
    char* file_destino;
    char* tag_origen;
    char* tag_destino;
    char* path;
    int offset;
    int contenido;
    int bloque;


    while(1) {
        int codigo_operacion = recibir_opcode(socket_cliente);
        if (codigo_operacion < 0) {
            log_warning(logger, "Worker desconectado");
            break;
        }

        switch(codigo_operacion) {
            case CREATE:


                if(crear_file(file, tag) == 1)
                {
                    log_info(logger, "##<%s> - File Creado <%s>:<%s>", query_id, file, tag);
                    log_info(logger, "##<%s> - Tag Creado <%s>:<%s>", query_id, file, tag);
                }else
                {

                }

                break;
            case TRUNCATE:



                if(truncar_archivo(tamanio, file, tag) == 1)
                {
                    log_info(logger, "##<%s> - File Truncado <%s>:<%s> - Tamaño: <%s>", query_id, file, tag, tamanio);
                }

                break;
            case TAG:


                if(copiar_tag(file_origen, tag_origen, file_destino, tag_destino) == 1)
                {
                    log_info(logger, "##<%s> - File Creado <%s>:<%s>", query_id, file_destino, tag_destino);
                    log_info(logger, "##<%s> - Tag Creado <%s>:<%s>", query_id, file_destino, tag_destino);
                }

                break;
            case COMMIT:

                if(commmit_file(file, tag) == 1)
                {
                    log_info(logger, "##<%s> - Commit de File:tag <%s>:<%s>", query_id, file, tag);
                }

                break;
            case WRITE:

                if(escribir_bloque(path, offset, contenido) == 1)
                {
                    log_info(logger, "##<%s> - Bloque Lógico Escrito <%s>:<%s> - Número de Bloque <%s>", query_id, file, tag, bloque);
                }

                break;
            case READ:
                char* data;
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
    if (recibir_opcode(socket_cliente) == HANDSHAKE) {

        t_list* recibido = recibir_paquete(socket_cliente);
        int worker_id = *((int*)list_get(recibido, 0));
        //ACA MANU TENES WORKER_ID Y SOCKET_CLIENTE, SOLO TENES QUE HACER UN DICCIONARIO

        t_paquete* paquete = crear_paquete();
        cambiar_opcode_paquete(paquete, OK);
        agregar_a_paquete(paquete, block_size, sizeof(int));
        enviar_paquete(paquete, socket_cliente, logger);
        borrar_paquete(paquete);

        log_trace(logger, "Recibi el handshake de un WORKER");

        int* socket_worker = malloc(sizeof(int));
        *socket_worker = socket_cliente;
        log_trace(logger, "socket worker: %d", *socket_worker);
        
        pthread_t hilo_worker;
        pthread_create(&hilo_worker, NULL, (void*)manejar_conexion_worker, socket_worker);
        pthread_detach(hilo_worker);

        list_destroy_and_destroy_elements(recibido, free);
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