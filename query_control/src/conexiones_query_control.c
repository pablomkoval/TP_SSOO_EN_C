#include <conexiones_query_control.h>


void handshake_master(int socket, t_paquete* paquete_query){
    enviar_handshake(socket, HANDSHAKE_QUERY_CONTROL);
    log_trace(logger, "Envié el handshake a Master");

    int respuesta;
    if(0 >= recv(socket, &respuesta, sizeof(int), MSG_WAITALL)){
        log_error(logger, "Fallo al recibir OK de Master");
        return;
    }

    if(respuesta == OK){
        log_trace(logger, "Recibi el OK de Master");
        log_info(logger, "## Conexión al Master exitosa. IP: <%s>, Puerto: <%s>", ip_master, puerto_master);
        //despues del ok hay que mandar un paquete con path del archivo query y prioridad
        enviar_paquete(paquete_query, socket, logger);
        
        while(1){
        int codigo_operacion;
        if(0 >= recv (socket, &codigo_operacion, sizeof(codigo_operacion), MSG_WAITALL) ){
            log_error(logger, "Conexión al Master cerrada.");
            break;
        }

        switch(codigo_operacion){
            case MENSAJE:
                char* mensaje = recibir_mensaje(socket);
                log_info(logger, "Mensaje del master: %s", mensaje);
                free(mensaje);
                break;

            case OK:
                log_info(logger, "Master indico fin de query. Cerrando la query...");
                close(socket);
                return;

            case ERROR:
                log_error(logger, "Master tuvo un error con la query");
                close(socket);
                return;
            default:
                log_error(logger, "Código desconocido recibido: %d", codigo_operacion);
                break;
        }
        }
        return;
    }else {
        log_error(logger, "Falló en el handshake con Master, recibí %d", respuesta);
        return;
    }

    return;
}

//devuelve el socket del servidor master
int conectar_master(t_paquete* paquete_query){
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
    handshake_master(socket_servidor, paquete_query);

    return socket_servidor;
}

t_paquete* empaquetar_query(char* archivo_query, int prioridad){
    log_trace(logger, "Comencé a empaquetar");
    
    int* prioridad_ptr = &prioridad;
    void *prioridad_ptr_void = (void*)prioridad_ptr;

    t_paquete* paquete_query = crear_paquete();
    agregar_a_paquete(paquete_query, archivo_query, strlen(archivo_query) + 1);
    agregar_a_paquete(paquete_query, prioridad_ptr_void, __SIZEOF_INT__);

    return paquete_query;
}