#ifndef CONEXIONES_WORKER_H_
#define CONEXIONES_WORKER_H_

#include <utils/utils.h>
#include <worker.h>

int conectar_master(int worker_id);
int conectar_storage(int worker_id);

#endif