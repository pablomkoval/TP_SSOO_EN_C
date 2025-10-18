#ifndef CONEXIONES_MASTER_H_
#define CONEXIONES_MASTER_H_

#include <utils/utils.h>
#include <master.h>
#include <qcb.h>
#include <planificador.h>

typedef struct{
    int socket;
    int id;
} t_argumentos_worker;

void* funcion_main_escucha (void* args);
void* manejar_servidor_worker(void* arg);
void* manejar_servidor_querycontrol(void* arg);
void manejar_desconexion_query_control(int socket_cliente, int qid);

void hacer_desconexion_worker(int worker_id);
void hacer_read_worker(int socket_worker, char* worker_id_str);
void hacer_end_worker(char* worker_id_str, int worker_id);

t_qcb* hacer_qcb_nueva(int socket_cliente);
void encolar_qcb(t_list* cola_ready, t_qcb* qcb);

#endif