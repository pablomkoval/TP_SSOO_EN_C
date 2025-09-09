#include <conexiones_worker.h>


void handshake_master(int socket, int worker_id){
    enviar_handshake(socket);
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

void handshake_storage(int socket){
    enviar_handshake(socket);
    log_info(logger, "Envié el handshake a Storage");

    int respuesta;
    if(0 >= recv(socket, &respuesta, sizeof(int), MSG_WAITALL)){
        log_error(logger, "Fallo al recibir OK de Storage");
        return;
    }
    if(respuesta == OK){
        log_trace(logger, "Recibi el OK de Storage");
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
int conectar_storage(){
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
    handshake_storage(socket_servidor);

    return socket_servidor;
}
