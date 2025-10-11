#ifndef QUERIES_H_
#define QUERIES_H_

#include <utils/utils.h>
#include <worker.h>
#include <memoria_interna.h>

void ejecutar_write(char* file_tag, int direccion_base, char* contenido);
void ejecutar_read(char* file_tag, int direccion_base, int tamanio);
#endif