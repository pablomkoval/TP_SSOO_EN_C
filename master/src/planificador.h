#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_



#include <master.h>
#include <utils.h>

void inicializar_fifo(t_queue* cola_ready);
void agregar_query_fifo(t_queue* cola_ready, t_query* query);
void* obtener_query_siguiente_fifo(t_queue* cola_ready);
void destruir_fifo(t_queue* cola_ready);

extern t_queue* cola_ready;
extern t_list* workers_libres;

#endif