#include <utils/utils.h>


int iniciar_servidor(char* PUERTO, t_log* logger){
    int socket_servidor;

    struct addrinfo hints, *server_info;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, PUERTO, &hints, &server_info);

    socket_servidor = socket(server_info->ai_family,
                        server_info->ai_socktype,
                        server_info->ai_protocol);


    setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

    bind(socket_servidor,server_info->ai_addr,server_info->ai_addrlen);

    listen(socket_servidor, SOMAXCONN);
    freeaddrinfo(server_info);
    log_trace(logger, "Listo para escucha");
    return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger){
    int socket_cliente;
    socket_cliente = accept(socket_servidor, NULL, NULL);
    log_trace(logger, "Se conecto un cliente");
    return socket_cliente;
}

void* serializar(t_paquete* paquete, int bytes_a_enviar){
    //meto el contenido del paquete en un unico stream de datos para hacer el send
    //se serializa con el formato opcode / size / contenido
    void* stream_a_enviar = malloc(bytes_a_enviar);
    int offset = 0;

    memcpy(stream_a_enviar + offset, &(paquete->codigo_operacion), sizeof(int));
    offset += sizeof(int);

    memcpy(stream_a_enviar + offset, &(paquete->buffer->size), sizeof(int));
    offset += sizeof(int);

    if(paquete->buffer->size > 0 && paquete->buffer->stream != NULL){
        memcpy(stream_a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
        offset += paquete->buffer->size;
    }
    
    return stream_a_enviar;
}

t_list* deserializar(t_buffer* buffer){
    void* stream = buffer->stream;
    int size = buffer->size;


    t_list* elementos = list_create();
    int offset = 0;

    while(offset < size){
        if (offset + sizeof(int) > size) {
            printf("Buffer corrupto: no hay suficiente espacio para leer el tamaño del elemento");
            break;
        }
        int tamanio_elemento = 0;
        memcpy(&tamanio_elemento, stream + offset, sizeof(int));
        offset += sizeof(int);

        if (offset + tamanio_elemento > size) {
            printf("Buffer corrupto: no hay suficiente espacio para leer el elemento completo");
            break;
        }

        void* elemento = malloc(tamanio_elemento);
        memcpy(elemento, stream + offset, tamanio_elemento);
        offset += tamanio_elemento;

        list_add(elementos,elemento);
    }
    
    return elementos;
}

t_list* recibir_paquete (int socket_cliente){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    t_list* contenido;

    if(recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL) <= 0){
        printf("Fallo el recv del size del paquete");
    }
    
    buffer->stream = malloc(buffer->size);
    
    if(recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL) <= 0){
        printf("Fallo el recv del stream del paquete");
    }

    contenido = deserializar(buffer);

    free(buffer->stream);
    free(buffer);
    return contenido;
}

void crear_buffer(t_paquete* paquete){
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void){
    t_paquete* paquete;
    paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PAQUETE;
    crear_buffer(paquete);
    return paquete;
}
t_paquete* cambiar_opcode_paquete(t_paquete* paquete, op_code codigo){
    paquete->codigo_operacion = codigo;
    return paquete;
}
void enviar_mensaje(int socket, char* mensaje){
    int cod_op = MENSAJE;
    int size = strlen(mensaje) + 1;
    send(socket, &cod_op, sizeof(int), 0);
    send(socket, &size, sizeof(int), 0);
    send(socket, mensaje, size, 0);
}

void enviar_cod_op(int socket, int opcode){
    send(socket, &opcode, sizeof(int), 0);
}

char* recibir_mensaje(int socket){
    int size_mensaje;
    recv(socket, &size_mensaje, sizeof(int), MSG_WAITALL);
    char* mensaje = malloc(size_mensaje);
    recv(socket, mensaje, size_mensaje, MSG_WAITALL);
    return mensaje;
}



void enviar_handshake(int socket_servidor, int tipo_handshake){
    send(socket_servidor, &tipo_handshake, sizeof(int), 0);
}


int recibir_handshake(int socket_cliente){
    int op_code = recibir_opcode(socket_cliente);
    int respuesta = OK;
    switch(op_code){
        case HANDSHAKE:
            send(socket_cliente, &respuesta, sizeof(int), 0);
            return SIN_DEFINIR;
            break;
        case HANDSHAKE_QUERY_CONTROL:
            send(socket_cliente, &respuesta, sizeof(int), 0);
            return QUERY_CONTROL;
            break;
        case HANDSHAKE_WORKER:
            send(socket_cliente, &respuesta, sizeof(int), 0);
            return WORKER;
            break;
        default:
            printf("Se esperaba un handshake pero se obtuvo el Opcode (%d)", op_code);
            break;
    }
    return 0;
}

void agregar_a_paquete(t_paquete* paquete, void* contenido, int tamanio){
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), contenido, tamanio);

    paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket, t_log* logger){
    int bytes_a_enviar = paquete->buffer->size + 2*sizeof(int);
    void* stream_a_enviar = serializar(paquete, bytes_a_enviar);

    int resultado = send(socket, stream_a_enviar, bytes_a_enviar, 0);
    log_trace(logger, "Se envió un paquete al socket %d", socket);
    if (resultado == -1) {
        log_debug(logger, "Fallo el send. Probablemente el socket está cerrado.");
    }

    free(stream_a_enviar);
}
int recibir_opcode(int socket_cliente){
    int codigo_operacion;
    int resultado = recv(socket_cliente, &codigo_operacion, sizeof(int), MSG_WAITALL);
    if(resultado > 0){
        return codigo_operacion;
    }
    else if(resultado == 0){
        //el cliente cerro la conexion
        //printf("se cerro la conexion");
        close(socket_cliente);
        return 0;
    }
    else{
        close(socket_cliente);
        return -1;
    }
}

void borrar_paquete(t_paquete* paquete){
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}
