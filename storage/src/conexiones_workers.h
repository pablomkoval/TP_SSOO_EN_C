#ifndef CONEXIONES_WORKERS_H_
#define CONEXIONES_WORKERS_H_

#include <utils/utils.h>
#include <storage.h>
#include <blocks.h>

extern t_dictionary* worker_id_por_socket;

void* lanzar_servidor(int socket_servidor);
void* manejar_servidor(void* socket_ptr) ;
void* manejar_conexiones_storage(void* socket_ptr);
void* manejar_conexion_worker(void* arg);

void manejar_create(int socket_worker);
void manejar_truncate(int socket_worker);
void manejar_commit(int socket_worker);
void manejar_tag(int socket_worker);
void manejar_write(int socket_worker);
void manejar_read(int socket_worker);
void manejar_delete(int socket_worker);
void manejar_desconexion(int* socket_worker);

#endif