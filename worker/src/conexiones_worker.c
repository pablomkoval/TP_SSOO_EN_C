#include <conexiones_worker.h>


void handshake_master(int socket, int worker_id){
    enviar_handshake(socket, HANDSHAKE_WORKER);
    log_info(logger, "Envié el handshake a Master");

    int respuesta;
    if(0 >= recv(socket, &respuesta, sizeof(int), MSG_WAITALL)){
        log_error(logger, "Fallo al recibir OK de Master");
        return;
    }
    if(respuesta == OK){
        log_trace(logger, "Recibi el OK de Master");
        send(socket, &worker_id, sizeof(int), 0);
        return;
    }else {
        log_error(logger, "Fallo en el handshake con Master, recibí %d", respuesta);
        return;
    }

    return;
}

void handshake_storage(int socket, int worker_id){
    //enviar_handshake(socket, HANDSHAKE);
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, HANDSHAKE);
    agregar_a_paquete(paquete, &worker_id, sizeof(int));
    enviar_paquete(paquete, socket, logger);
    borrar_paquete(paquete);
    log_info(logger, "Envié el handshake a Storage");
    
    int respuesta = recibir_opcode(socket);
    if(respuesta <= 0){
        log_error(logger, "Fallo al recibir OK de Storage");
        return;
    }
    if(respuesta == OK){
        log_trace(logger, "Recibi el OK de Storage");
        t_list* recibido = recibir_paquete(socket);
        int tam_bloque = *((int*)list_get(recibido, 0));
        tam_pagina = tam_bloque;
        return;
    }else {
        log_error(logger, "Fallo en el handshake con Storage, recibí %d", respuesta);
        return;
    }

    return;
}

//devuelve el socket del servidor master
int conectar_master(int worker_id){
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
    handshake_master(socket_servidor, worker_id);

    return socket_servidor;
}

//devuelve el socket del servidor storage
int conectar_storage(int worker_id){
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip_storage, puerto_storage, &hints, &server_info);
    int socket_servidor = socket(server_info->ai_family,
                                server_info->ai_socktype,
                                server_info->ai_protocol);

    connect(socket_servidor, server_info->ai_addr, server_info->ai_addrlen);
    freeaddrinfo(server_info);

    //realizo un handhsake con storage
    handshake_storage(socket_servidor, worker_id);

    return socket_servidor;
}

void* funcion_escucha_master(){
    while (1){
        int op_code = recibir_opcode(socket_master);
        switch (op_code)
        {
        case SOLICITUD_NUEVA_QUERY:
            log_debug(logger, "Se recibio una solicitud de nueva query");
            t_list* recibido = recibir_paquete(socket_master);
            t_args_query_interpreter* argumentos = malloc(sizeof(t_args_query_interpreter));

            int qid = *((int*)list_get(recibido, 0));
            void* nombre_elem = list_get(recibido, 1);
            int pc = *((int*)list_get(recibido, 2));
            argumentos->archivo = strdup((char*)nombre_elem);
            argumentos->pc = pc;
            argumentos->qid = qid;

            pthread_create(&thread_query_interpreter, NULL, iniciar_query_interpreter, (void*)argumentos);
            pthread_detach(thread_query_interpreter);

            list_destroy_and_destroy_elements(recibido, free);
            break;
        
        case INTERRUPCION:
            log_debug(logger, "Se recibio una interrupcion");
            pthread_mutex_unlock(&mutex_interrupcion);
            hay_interrupcion = true;
            pthread_mutex_lock(&mutex_interrupcion);
            //abrir mutex y cambiar booleano interrumpido
            break;
        default:
            break;
        }
    }
}