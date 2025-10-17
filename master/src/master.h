#ifndef MASTER_H_
#define MASTER_H_

typedef enum{
    READY,
    EXEC,
    EXIT
} estados_query;

#include<utils/utils.h>
#include<master_config.h>
#include<conexiones_master.h>
#include<qcb.h>

extern t_dictionary* diccionario_workers;   // key: id_worker, valor: socket_worker
extern t_dictionary* diccionario_querys;    
extern t_dictionary* diccionario_exec;      //key: id_worker, valor: (qcb)query a procesar

extern pthread_mutex_t mutex_diccionario;

extern t_log* logger;

extern int id_query;

void cambiar_estado();

#endif