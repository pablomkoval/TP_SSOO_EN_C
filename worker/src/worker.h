#ifndef WORKER_H_
#define WORKER_H_

#include <utils/utils.h>

extern char* ip_master;
extern int puerto_master;
extern char* ip_storage;
extern int puerto_storage;
extern int tam_memoria;
extern int retardo_memoria;
extern char* algoritmo_reemplazo;
extern char* path_queries;


extern t_log* logger;

#include <conexiones_worker.h>

#endif