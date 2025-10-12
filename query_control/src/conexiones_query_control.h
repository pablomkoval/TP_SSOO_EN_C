#ifndef CONEXIONES_QUERY_CONTROL_H
#define CONEXIONES_QUERY_CONTROL_H

#include <utils/utils.h>
#include <query_control.h>


int handshake_master(int socket);
int conectar_master();
t_paquete* empaquetar_query(char* archivo_query, int prioridad);
void recibir_mensajes_de_master(int socket_master);


#endif