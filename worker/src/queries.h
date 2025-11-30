#ifndef QUERIES_H_
#define QUERIES_H_

#include <utils/utils.h>
#include <worker.h>
#include <memoria_interna.h>

void ejecutar_create(char* file, char* tag, int qid);
void ejecutar_truncate(char* file, char* tag, int tamanio, int qid);
int ejecutar_write(char* file_tag, int direccion_base, char* contenido, int qid);
int ejecutar_read(char* file_tag, int direccion_base, int tamanio, int qid);
void ejecutar_tag(char* file_origen, char* tag_origen, char* file_tag_destino, int qid);
int ejecutar_flush(char* file, char* tag, char* file_tag, int qid);
int ejecutar_commit(char* file, char* tag, char* file_tag, int qid);
void ejecutar_delete(char* nombre_file, char* tag, int qid);
void ejecutar_end();

#endif