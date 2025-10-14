#include <conexiones_workers.h>



void* manejar_conexion_worker(void* arg) {
    int socket_worker = *((int*)arg);
    free(arg);

    while(1) {
        int codigo_operacion = recibir_opcode(socket_worker);
        if (codigo_operacion < 0) {
            log_warning(logger, "Worker desconectado");
            break;
        }

        switch(codigo_operacion) {
            case CREATE:

                manejar_create(socket_worker);
                break;

            case TRUNCATE:

                manejar_truncate(socket_worker);
                break; 

            case TAG:

                manejar_tag(socket_worker);
                break;

            case COMMIT:

                manejar_commit(socket_worker);
                break;

            case WRITE:

                manejar_write(socket_worker);
                break;

            case READ:

                manejar_read(socket_worker);
                break;

            case DELETE:
                manejar_delete(socket_worker);
                break;

            default:
                log_error(logger, "Operación worker desconocida: %d", codigo_operacion);
                break;
        }
     }
     close(socket_worker);
     return NULL;
}



void* manejar_conexiones_storage(void* socket_ptr)
{
    int socket_cliente = *((int*)socket_ptr);
    free(socket_ptr);
    if (recibir_opcode(socket_cliente) == HANDSHAKE) {
        t_paquete* paquete = crear_paquete();
        cambiar_opcode_paquete(paquete, OK);
        agregar_a_paquete(paquete, &block_size, sizeof(int));
        enviar_paquete(paquete, socket_cliente, logger);
        borrar_paquete(paquete);

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
        pthread_create(&hilo_cliente, NULL, manejar_conexiones_storage, socket_cliente_ptr);
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

void manejar_create(int socket_worker)
{
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);

    int resultado = crear_file(file, tag);
    if(resultado == 1)
    {
        log_info(logger, "##<%i> - File Creado <%s>:<%s>", *query_id, file, tag);
        log_info(logger, "##<%i> - Tag Creado <%s>:<%s>", *query_id, file, tag);
    }else
    {

    }
}

void manejar_truncate(int socket_worker)
{
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);
    int* tamanio = list_get(datos, 3);

    int resultado = truncar_archivo(*query_id, *tamanio, file, tag);

    if(resultado == 1)
    {
        log_info(logger, "##<%i> - File Truncado <%s>:<%s> - Tamaño: <%i>", *query_id, file, tag, *tamanio);
    }
   
}

void manejar_commit(int socket_worker)
{
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);

    int resultado = commmit_file(*query_id, file, tag);

    if(resultado == 1)
    {
        log_info(logger, "##<%i> - Commit de File:tag <%s>:<%s>", *query_id, file, tag);
    }
}

void manejar_tag(int socket_worker)
{
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file_origen = list_get(datos, 1);
    char* tag_origen = list_get(datos, 2);
    char* file_destino = list_get(datos, 3);
    char* tag_destino = list_get(datos, 4);

    int resultado = copiar_tag(*query_id, file_origen, tag_origen, file_destino, tag_destino);

    if(resultado == 1)
    {
        log_info(logger, "##<%i> - File Creado <%s>:<%s>", *query_id, file_destino, tag_destino);
        log_info(logger, "##<%i> - Tag Creado <%s>:<%s>", *query_id, file_destino, tag_destino);
    }   
}

void manejar_write(int socket_worker)
{
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);
    int* offset = list_get(datos, 3);
    char* contenido = list_get(datos, 4);
    
    int resultado = escribir_bloque(*query_id, file, tag, *offset, contenido);

    if(resultado == 1)
    {
        //log_info(logger, "##<%s> - Bloque Lógico Escrito <%s>:<%s> - Número de Bloque <%i>", query_id, file, tag, bloque);
    }
}

void manejar_read(int socket_worker)
{
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);
    int* bloque = list_get(datos, 3);

    char* data;

    int resultado = leer_bloque(*query_id, file, tag, *bloque, &data);

    if(resultado == 1)
    {
        log_info(logger, "##<%d> - Bloque Lógico Leído <%s>:<%s> - Número de Bloque <%d>", *query_id, file, tag, *bloque);
    }
}

void manejar_delete(int socket_worker)
{
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);

    int resultado = eliminar_tag(file, tag);

    if(resultado == 1)
    {
        log_info(logger, "##<%i> - Tag Eliminado <%s>:<%s>", *query_id, file, tag);
    }
}
















// log_info(logger, "##Se conecta el Worker <%i> - Cantidad de Workers: <CANTIDAD", worker_id);
// log_info(logger, "##Se desconecta el Worker <%i> - Cantidad de Workers: <CANTIDAD", worker_id);
// log_info(logger, "##<%i> - File Creado <%s>:<%s>", query_id, file, tag);  SI
// log_info(logger, "##<%i> - File Truncado <%s>:<%s> - Tamaño: <%i>", query_id, file, tag, tamanio); SI
// log_info(logger, "##<%i> - Tag creado <%s>:<%s>", query_id, file, tag);  SI
// log_info(logger, "##<%i> - Commit de File:Tag <%s>:<%s>", query_id, file, tag);  SI
// log_info(logger, "##<%i> - Tag Eliminado <%s>:<%s>", query_id, file, tag);  SI
// log_info(logger, "##<%i> - Bloque Lógico Leído <%s>:<%s> - Número de Bloque: <%i>", query_id, file, tag, bloque);  SI
// log_info(logger, "##<%i> - Bloque Lógico Escrito <%s>:<%s> - Número de Bloque: <%i>", query_id, file, tag, bloque);  SI
// log_info(logger, "##<%i> - Bloque Físico Reservado - Número de Bloque: <%i>", query_id, bloque);
// log_info(logger, "##<%i> - Bloque Físico Liberado - Número de Bloque: <%i>", query_id, bloque);
// log_info(logger, "##<%i> - <%s>:<%s> Se agregó el hard link del bloque lógico <BLOQUE_LOGICO> al bloque físico <BLOQUE_FISICO>", query_id, file, tag, bloque_logico, bloque_fisico);
// log_info(logger, "##<%i> - <%s>:<%s> Se eliminó el hard link del bloque lógico <BLOQUE_LOGICO> al bloque físico <BLOQUE_FISICO>", query_id, file, tag, bloque_logico, bloque_fisico);
// log_info(logger, "##<%i> - <%s>:<%s> Bloque Lógico <BLOQUE> se reasigna de <BLOQUE_FISICO_ACTUAL> a <BLOQUE_FISICO_CONFIRMADO>", query_id, file, tag, bloque_logico, bloque_fisico_actual, bloque_fisico_nuevo);