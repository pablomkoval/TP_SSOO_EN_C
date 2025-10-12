#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <master.h>
#include <qcb.h>

void* planificador();

extern t_queue* cola_ready;
extern t_list* workers_libres;

#endif