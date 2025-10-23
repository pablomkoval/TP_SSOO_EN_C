#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <master.h>
#include <qcb.h>

void* planificador();
void inicializar_planificador();
void enviar_qcb_a_worker(t_qcb* qcb, int socket_worker);
int obtener_worker_libre();
void enviar_query_a_worker(t_qcb* query_a_ejecutar, int worker_asignado_id);
void obtener_query_worker_fifo(t_qcb* query_a_ejecutar, int *worker_libre_id);
t_qcb* obtener_query_worker_priori(int *worker_libre_id);

extern t_list* cola_ready;
extern t_list* workers_libres;

extern sem_t sem_queries_ready;
extern sem_t sem_workers_libres;

extern pthread_mutex_t mutex_ready;
extern pthread_mutex_t mutex_workers_libres;

#endif