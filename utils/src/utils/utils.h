#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/config.h>
#include<commons/crypto.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/collections/queue.h>
#include<commons/collections/dictionary.h>
#include<commons/temporal.h>
#include<readline/readline.h>
#include<commons/bitarray.h>
#include<semaphore.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<sys/mman.h>
#include<pthread.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<fcntl.h>

typedef enum
{
    HANDSHAKE,
    HANDSHAKE_WORKER,
    HANDSHAKE_QUERY_CONTROL,
    PAQUETE,
    MENSAJE,
    OK,
    ERROR,
    CREATE,
    TRUNCATE,
    TAG,
    COMMIT,
    WRITE,
    READ,
    DELETE,
    FLUSH,
    END,
    SOLICITUD_NUEVA_QUERY,
    RESPUESTA_STORAGE
} op_code;

typedef enum 
{
    FILE_TAG_PREEXISTENTE = -1,
    FILE_TAG_INEXISTENTE = -2,
    ESPACIO_INSUFICIENTE = -3,
    ESCRITURA_NO_PERMITIDA = -4,
    LECTURA_O_ESCRITURA_FUERA_DE_RANGO = -5
} errores_storage;

typedef enum
{
    SIN_DEFINIR,
    WORKER,
    QUERY_CONTROL
} tipo_conexion;

typedef struct
{
    int size;
    void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

void* atender_cliente(void* fd_conexion_ptr, t_log* logger);
int iniciar_servidor(char* puerto, t_log* logger);
int esperar_cliente(int socket_servidor, t_log* logger);
int crear_conexion(char* ip, char* puerto);

t_paquete* crear_paquete(void);
void crear_buffer(t_paquete*);
void agregar_a_paquete(t_paquete*, void* contenido, int tamanio);
void enviar_paquete(t_paquete*, int socket, t_log* logger);
int recibir_opcode(int socket_cliente);
void borrar_paquete(t_paquete*);
void* serializar(t_paquete*, int bytes_a_enviar);
t_list* deserializar(t_buffer* buffer);
void enviar_handshake(int socket_servidor, int tipo_handshake);
int recibir_handshake(int socket_cliente);
t_list* recibir_paquete (int socket_cliente);
t_paquete* cambiar_opcode_paquete(t_paquete* paquete, op_code codigo);
void enviar_mensaje(int socket, char* mensaje);
void enviar_cod_op(int socket, int opcode);
char* recibir_mensaje(int socket);


#endif