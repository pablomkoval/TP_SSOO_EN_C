#ifndef MASTER_H_
#define MASTER_H_



#include<utils/utils.h>
#include<master_config.h>
#include<conexiones_master.h>
#include<qcb.h>

extern t_dictionary* diccionario_workers;   // key: id_worker, valor: socket_worker
extern t_dictionary* diccionario_querys;    // key: qid, valor: qcb
extern t_dictionary* diccionario_exec;      //key: id_worker, valor: (qcb)query a procesar
<<<<<<< HEAD

extern pthread_mutex_t mutex_diccionario;
=======
extern pthread_mutex_t mutex_diccionario_workers;
extern pthread_mutex_t mutex_diccionario_querys;
extern pthread_mutex_t mutex_diccionario_exec;

>>>>>>> bf17a6e (laburo de planificador y  headers que rompian qcb)

extern t_log* logger;

extern int id_query;

void cambiar_estado();

#endif