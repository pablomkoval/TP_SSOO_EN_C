#ifndef QUERIES_H_
#define QUERIES_H_

#include <utils/utils.h>
#include <worker.h>
#include <memoria_interna.h>

void ejecutar_create(char* file, char* tag, int qid);
void ejecutar_write(char* file_tag, char* direccion_base_str, char* contenido, int qid);
void ejecutar_read(char* file_tag, int direccion_base, int tamanio, int qid);

#endif