#ifndef WORKER_H_
#define WORKER_H_

#include <utils/utils.h>

extern char* ip_master;
extern char* puerto_master;
extern char* ip_storage;
extern char* puerto_storage;
extern int tam_memoria;
extern int retardo_memoria;
extern char* algoritmo_reemplazo;
extern char* path_queries;
extern int tam_pagina;
extern int worker_id;

extern pthread_t thread_query_interpreter;
extern int socket_storage;
extern int socket_master;
extern t_log* logger;
extern bool hay_interrupcion;
extern pthread_mutex_t mutex_interrupcion;

#include <conexiones_worker.h>
#include <query_interpreter.h>

#endif