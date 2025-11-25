#ifndef CONEXIONES_MASTER_H_
#define CONEXIONES_MASTER_H_

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
bool qcb_esta_en_cola_ready(void* arg, int socket_buscado);
void hacer_desconexion_query_control(int socket_cliente, t_qcb* qcb);

void hacer_desconexion_worker(int worker_id);
void hacer_read_worker(int socket_worker, char* worker_id_str);
void hacer_end_worker(char* worker_id_str, int worker_id, int socket_worker);

t_qcb* hacer_qcb_nueva(int socket_cliente);
void encolar_qcb(t_list* cola_ready, t_qcb* qcb);
bool comparar_qcb_por_prioridad(void* qcb1, void* qcb2);
int workers_conectados();

void hacer_chequeo_desalojo(t_qcb* qcb_entrante);

#endif