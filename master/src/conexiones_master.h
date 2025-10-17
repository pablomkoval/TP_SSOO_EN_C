#ifndef CONEXIONES_MASTER_H_
#define CONEXIONES_MASTER_H_

#include <utils/utils.h>
#include <master.h>
#include <qcb.h>
#include <planificador.h>

typedef struct{
    int socket;
    int id;
    //bool disponible; -> y con este coso se tienen los workers libres y ocupados para la planificacion anasheiii
} t_argumentos_worker;

void* funcion_main_escucha (void* args);
void* manejar_servidor_worker(void* arg);
void* manejar_servidor_querycontrol(void* arg);
void manejar_desconexion_query_control(int socket_cliente);

#endif