#include <conexiones_query_control.h>


void handshake_master(int socket)
{
    enviar_handshake(socket, HANDSHAKE_QUERY_CONTROL);
    log_info(logger, "Envié el handshake a Master");

    int respuesta;
    if(0 >= recv(socket, &respuesta, sizeof(int), MSG_WAITALL)){
        log_error(logger, "Fallo al recibir OK de Master");
        return;
    }
    if(respuesta == OK){
        log_trace(logger, "Recibi el OK de Master");
        //despues del ok hay que mandar un paquete con path del archivo query y prioridad
        return;
    }else {
        log_error(logger, "Fallo en el handshake con Master, recibí %d", respuesta);
        return;
    }

    return;
}

//devuelve el socket del servidor master
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
    handshake_master(socket_servidor);

    return socket_servidor;
}
