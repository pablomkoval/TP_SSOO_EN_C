#ifndef BLOCKS_H
#define BLOCKS_H

#include <storage.h>
#include <metadata.h>

void asociar_hash_block(char* bloque_fisico);
void crear_bloque_logico(char* path, int numero);
void crear_bloques_fisicos();
char* leer_archivo(char* path);
int escribir_archivo(char* path, char* contenido);
int obtener_numero_bloque(char* path);
char* obtener_hash_block(char* bloque);
int obtener_referencias_bloque(int bloque_fisico);
int obtener_bloque_por_hash(char* md5);
void sumar_referencia(int nro_block);
void restar_referencia(int nro_block);
void truncar_archivo(int nuevo_tamanio, char* file_tag);
char* obtener_bloque_fisico_asociado(char* bloque_logico);
void cambiar_hard_link(char* bloque_logico, char* bloque_fisico);
char* obtener_bloque_logico(char* path, int numero);
char* bloque_fisico_por_nro(int nro);

typedef struct{
    int nro;
    int referencias;
}t_block;

#endif