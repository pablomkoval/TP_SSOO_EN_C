#ifndef MASTER_H_
#define MASTER_H_

#include<utils/utils.h>
#include<master_config.h>
#include<conexiones_master.h>

extern t_dictionary* diccionario_workers;   // key: id_worker, valor: socket_worker
extern t_dictionary* diccionario_querys;    //key: id_worker, valor: socket del query a procesar
extern pthread_mutex_t mutex_diccionar_workers;

extern t_log* logger;

#endif