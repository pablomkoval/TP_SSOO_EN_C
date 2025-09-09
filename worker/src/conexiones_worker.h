#ifndef CONEXIONES_WORKER_H_
#define CONEXIONES_WORKER_H_

#include <utils/utils.h>

int conectar_master(int worker_id);
int conectar_storage();

#endif