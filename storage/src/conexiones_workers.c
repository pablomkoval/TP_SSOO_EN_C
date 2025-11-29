#include <conexiones_workers.h>

t_dictionary* worker_id_por_socket = NULL;

void* manejar_conexion_worker(void* arg) {
    int* socket_worker = arg;
    

    while(1) {
        int codigo_operacion = recibir_opcode(*socket_worker);
        if (codigo_operacion < 0) {

            manejar_desconexion(socket_worker);
            break;
        }

        switch(codigo_operacion) {
            case CREATE:
                log_debug(logger, "Llego un CREATE");
                //lo agrego el chad de blito
                manejar_create(*socket_worker);
                break;

            case TRUNCATE:
                log_debug(logger, "Llego un TRUNCATE");

                manejar_truncate(*socket_worker);
                break; 

            case TAG:
                log_debug(logger, "Llego un TAG");
                manejar_tag(*socket_worker);
                break;

            case COMMIT:
                log_debug(logger, "Llego un COMMIT");
                manejar_commit(*socket_worker);
                break;

            case WRITE:
                log_debug(logger, "Llego un WRITE");
                manejar_write(*socket_worker);
                break;

            case READ:
                log_debug(logger, "Llego un READ");
                manejar_read(*socket_worker);
                break;

            case DELETE:
                log_debug(logger, "Llego un DELETE");
                manejar_delete(*socket_worker);
                break;

            default:
                log_error(logger, "Operación worker desconocida: %d", codigo_operacion);
                break;
        }
     }
     free(arg);
     close(*socket_worker);
     return NULL;
}



void* manejar_conexiones_storage(void* socket_ptr)
{
    int* socket_cliente = socket_ptr;
    
    if (recibir_opcode(*socket_cliente) == HANDSHAKE) {

        t_list* recibido = recibir_paquete(*socket_cliente);
        int worker_id = *(int*)list_get(recibido, 0);
        char* socket_key = string_itoa(*socket_cliente);

        int* worker_id_ptr = malloc(sizeof(int));
        *worker_id_ptr = worker_id; 
        
        pthread_mutex_lock(&mutex_worker_id);
        dictionary_put(worker_id_por_socket, socket_key, worker_id_ptr);
        int cantidad_workers = dictionary_size (worker_id_por_socket);
        pthread_mutex_unlock(&mutex_worker_id);

        free(socket_key);

        log_info(logger, "##Se conecta el Worker <%i> - Cantidad de Workers: <%d>", worker_id, cantidad_workers);

        //ACA MANU TENES WORKER_ID Y SOCKET_CLIENTE, SOLO TENES QUE HACER UN DICCIONARIO

        t_paquete* paquete = crear_paquete();
        cambiar_opcode_paquete(paquete, OK);
        agregar_a_paquete(paquete, &block_size, sizeof(int));
        enviar_paquete(paquete, *socket_cliente, logger);
        borrar_paquete(paquete);

        log_trace(logger, "Recibi el handshake de un WORKER");

        log_trace(logger, "socket worker: %d", *socket_cliente);
        
        pthread_t hilo_worker;
        pthread_create(&hilo_worker, NULL, (void*)manejar_conexion_worker, socket_cliente);
        pthread_detach(hilo_worker);

        list_destroy_and_destroy_elements(recibido, free);

        return NULL;
    }
    free(socket_ptr);
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
    usleep(retardo_operacion);
    t_list* datos = recibir_paquete(socket_worker);
    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);

    int resultado = crear_file(file, tag);
    if(resultado == 1)
    {
        log_info(logger, "##<%i> - File Creado <%s>:<%s>", *query_id, file, tag);
        log_info(logger, "##<%i> - Tag Creado <%s>:<%s>", *query_id, file, tag);
        mandar_resultado(socket_worker, resultado);
    }
    else mandar_resultado(socket_worker, resultado);

    list_destroy_and_destroy_elements(datos, free);
    
}
void manejar_truncate(int socket_worker)
{
    usleep(retardo_operacion);
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);
    int* tamanio = list_get(datos, 3);

    int resultado = truncar_archivo(*query_id, *tamanio, file, tag);

    if(resultado == 1)
    {
        log_info(logger, "##<%i> - File Truncado <%s>:<%s> - Tamaño: <%i>", *query_id, file, tag, *tamanio);
        mandar_resultado(socket_worker, resultado);
    }
    else mandar_resultado(socket_worker, resultado);

    list_destroy_and_destroy_elements(datos, free);
   
}

void manejar_commit(int socket_worker)
{
    usleep(retardo_operacion);
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);

    log_debug(logger, "ABRO EL PAQUETE");

    int resultado = commmit_file(*query_id, file, tag);

    if(resultado == 1)
    {
        log_info(logger, "##<%i> - Commit de File:tag <%s>:<%s>", *query_id, file, tag);
        mandar_resultado(socket_worker, resultado);
    }
    else mandar_resultado(socket_worker, resultado);

    list_destroy_and_destroy_elements(datos, free);
}

void manejar_tag(int socket_worker)
{
    usleep(retardo_operacion);
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
        mandar_resultado(socket_worker, resultado);
    } 
    else mandar_resultado(socket_worker, resultado);

    list_destroy_and_destroy_elements(datos, free);
}

void manejar_write(int socket_worker)
{
    usleep(retardo_operacion);
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);
    int* nro_bloque = list_get(datos, 3);
    char* contenido = list_get(datos, 4);
    
    log_error(logger, "Write, contenido por escribir: %s", contenido);
    int resultado = escribir_bloque(*query_id, file, tag, *nro_bloque, contenido);
    usleep(retardo_acceso_bloque);

    if(resultado == 1)
    {
        //log_info(logger, "##<%s> - Bloque Lógico Escrito <%s>:<%s> - Número de Bloque <%i>", query_id, file, tag, bloque);
        mandar_resultado(socket_worker, resultado);
    }
    else mandar_resultado(socket_worker, resultado);

    list_destroy_and_destroy_elements(datos, free);
}

void manejar_read(int socket_worker)
{
    usleep(retardo_operacion);
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);
    int* bloque = list_get(datos, 3);

    char* data = NULL;

    
    int resultado = leer_bloque(*query_id, file, tag, *bloque, &data);
    log_debug(logger, "query_id: %d, file: %s, tag: %s, data: %s", *query_id, file, tag, data);
    usleep(retardo_acceso_bloque);

    if(resultado == 1)
    {
        log_info(logger, "##<%d> - Bloque Lógico Leído <%s>:<%s> - Número de Bloque <%d>", *query_id, file, tag, *bloque);
        mandar_read(socket_worker, resultado, data);
    }
    else mandar_resultado(socket_worker, resultado);

    free(data);

    list_destroy_and_destroy_elements(datos, free);
}

void manejar_delete(int socket_worker)
{
    usleep(retardo_operacion);
    t_list* datos = recibir_paquete(socket_worker);

    int* query_id = list_get(datos, 0);
    char* file = list_get(datos, 1);
    char* tag = list_get(datos, 2);

    int resultado = eliminar_tag(*query_id, file, tag);

    if(resultado == 1)
    {
        log_info(logger, "##<%i> - Tag Eliminado <%s>:<%s>", *query_id, file, tag);
        mandar_resultado(socket_worker, resultado);
    }
    else mandar_resultado(socket_worker, resultado);

    list_destroy_and_destroy_elements(datos, free);
}

void manejar_desconexion(int* socket_worker)
{
        char* socket_key = string_itoa(*socket_worker);

        pthread_mutex_lock(&mutex_worker_id);
        int* worker_id_ptr = dictionary_remove(worker_id_por_socket, socket_key);
    
        if (worker_id_ptr == NULL) {
            pthread_mutex_unlock(&mutex_worker_id);
            log_error(logger, "Intento de desconectar socket %d que no está registrado", *socket_worker);
            free(socket_key);
            return;
        }

        int worker_id = *worker_id_ptr;
        free(worker_id_ptr);                    
        int cantidad_workers = dictionary_size(worker_id_por_socket);
        pthread_mutex_unlock(&mutex_worker_id);

        log_info(logger, "##Se desconecta el Worker <%i> - Cantidad de Workers: <%d>", worker_id, cantidad_workers);
        
        free(socket_key);

}

void mandar_resultado(int socket_worker, int resultado)
{
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, RESPUESTA_STORAGE);
    agregar_a_paquete(paquete, &resultado, sizeof(int));
    enviar_paquete(paquete, socket_worker, logger);
    borrar_paquete(paquete);
}

void mandar_read(int socket_worker, int resultado, char* contenido)
{
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, RESPUESTA_STORAGE);
    
    if (contenido != NULL) {
        agregar_a_paquete(paquete, &resultado, sizeof(int));
        agregar_a_paquete(paquete, contenido, strlen(contenido) + 1); 
    } else {
        //char* vacio = "";
        // char* pagina_vacia = malloc(block_size);
        // memset(pagina_vacia, 0, block_size);
        //agregar_a_paquete(paquete, vacio, strlen(vacio) + 1);
        resultado = PAGINA_VACIA;
        agregar_a_paquete(paquete, &resultado, sizeof(int)); 
    }

    enviar_paquete(paquete, socket_worker, logger);
    borrar_paquete(paquete);
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