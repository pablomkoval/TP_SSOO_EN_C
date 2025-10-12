#ifndef CONEXIONES_MASTER_H_
#define CONEXIONES_MASTER_H_
#include <utils/utils.h>
#include <master.h>

typedef struct{
    int socket;
    int id;
    //bool disponible; -> y con este coso se tienen los workers libres y ocupados para la planificacion anasheiii
} t_argumentos_worker;

// typedef struct{
//     int socket_query_control;
//     int id;
//     int prioridad;
//     char* path;
//     int id_worker_asociado;
// }t_argumentos_query;

void* funcion_main_escucha (void* args);
void* manejar_servidor_worker(void* arg);
void* manejar_servidor_querycontrol(void* arg);

#include <qcb.h>
#include <planificador.h>

#endif