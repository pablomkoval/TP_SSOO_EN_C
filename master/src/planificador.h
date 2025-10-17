#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <master.h>
#include <qcb.h>

void* planificador();

extern t_queue* cola_ready;
extern t_list* workers_libres;

extern sem_t sem_queries_ready;
extern sem_t sem_workers_libres;

extern pthread_mutex_t mutex_ready;
extern pthread_mutex_t mutex_workers_libres;

#endif