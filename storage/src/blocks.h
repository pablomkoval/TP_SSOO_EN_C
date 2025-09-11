#ifndef BLOCKS_H
#define BLOCKS_H

#include <storage.h>

void asociar_hash_block(char* bloque_fisico);
void crear_bloque_logico(char* path, int numero);
void crear_bloques_fisicos();
char* leer_archivo(char* path);
int escribir_archivo(char* path, char* contenido);
int obtener_numero_bloque(char* path);

#endif