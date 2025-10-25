#include <conexiones_query_control.h>

// verifica la conexion con master, retorna si fue exitosa o fallo. en caso de exito logea en log_info
int handshake_master(int socket){
    enviar_handshake(socket, HANDSHAKE_QUERY_CONTROL);
    log_trace(logger, "Envié el handshake a Master");

    int respuesta;
    if(0 >= recv(socket, &respuesta, sizeof(int), MSG_WAITALL)){
        log_error(logger, "Fallo al establecer conexión Master");
        return -1;
    }

    if(respuesta == OK){
        log_trace(logger, "Recibi el OK de Master");
        log_info(logger, "## Conexión al Master exitosa. IP: <%s>, Puerto: <%s>", ip_master, puerto_master);
        return 0;
    }else {
        log_error(logger, "Master no envió el OK, recibí %d", respuesta);
        return -1;
    }

    return -1;
}

//devuelve el socket del servidor master o -1 si falló la conexion
int conectar_master(){
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip_master, puerto_master, &hints, &server_info);
    int socket_servidor = socket(server_info->ai_family,
                                server_info->ai_socktype,
                                server_info->ai_protocol);

    connect(socket_servidor, server_info->ai_addr, server_info->ai_addrlen);
    freeaddrinfo(server_info);

    //realizo un handhsake con master
    if (handshake_master(socket_servidor) == -1){
        return -1;
    } else{
        return socket_servidor;
    }
}

t_paquete* empaquetar_query(char* archivo_query, int prioridad){
    //log_trace(logger, "Comencé a empaquetar");
    
    //int* prioridad_ptr = &prioridad;
    //void *prioridad_ptr_void = (void*)prioridad_ptr;

    t_paquete* paquete_query = crear_paquete();
    agregar_a_paquete(paquete_query, archivo_query, strlen(archivo_query) + 1);
    //agregar_a_paquete(paquete_query, prioridad_ptr_void, __SIZEOF_INT__);
    agregar_a_paquete(paquete_query, &prioridad, sizeof(int));

    return paquete_query;
}

void recibir_mensajes_de_master(int socket){
    while(1){
        int codigo_operacion;
        if(0 >= recv (socket, &codigo_operacion, sizeof(codigo_operacion), MSG_WAITALL) ){
            log_error(logger, "Conexión al Master cerrada.");
            break;
        }

        switch(codigo_operacion){
            case READ:
                t_list* paquete_lectura = recibir_paquete(socket);
                char* file_tag = list_get(paquete_lectura, 0);
                char* contenido = list_get(paquete_lectura, 1);
                log_info(logger, "## Lectura realizada: File %s, contenido: %s", file_tag, contenido);
                list_destroy_and_destroy_elements(paquete_lectura, free);
                break;

            case END: // END Query
                char* motivo = recibir_mensaje(socket);
                log_info(logger, "## Query Finalizada - %s", motivo);
                close(socket);
                free(motivo);
                return;

            default:
                log_error(logger, "Código desconocido recibido: %d", codigo_operacion);
                break;
        }
    }
}