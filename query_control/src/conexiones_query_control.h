#ifndef CONEXIONES_QUERY_CONTROL_H
#define CONEXIONES_QUERY_CONTROL_H

#include <utils/utils.h>
#include <query_control.h>


void handshake_master(int socket, t_paquete* paquete_query);
int conectar_master(t_paquete* paquete_query);
t_paquete* empaquetar_query(char* archivo_query, int prioridad);



#endif