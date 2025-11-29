#ifndef CONEXIONES_WORKER_H_
#define CONEXIONES_WORKER_H_

#include <utils/utils.h>
#include <worker.h>

extern bool interpreter_ocupado;
extern pthread_mutex_t mutex_interpreter;

int conectar_master(int worker_id);
int conectar_storage(int worker_id);
void* funcion_escucha_master();
#endif