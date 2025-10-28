#ifndef BLOCKS_H
#define BLOCKS_H

#include <storage.h>
#include <metadata.h>
#include <sincronizacion.h>
#include <bitmap.h>

void asociar_hash_block(char* bloque_fisico);
void crear_bloque_logico(char* path, int numero);
void crear_bloques_fisicos();
char* leer_archivo(char* path);
int escribir_archivo(char* path, char* contenido);
int obtener_numero_bloque(char* path);
int obtener_numero_bloque_metadata(char *path);
char* obtener_hash_block(char* bloque);
int obtener_referencias_bloque(int bloque_fisico);
int obtener_bloque_por_hash(char* md5);
void sumar_referencia(int nro_block);
void restar_referencia(int nro_block);
int truncar_archivo(int query_id, int nuevo_tamanio, char* file, char* tag);
char* obtener_bloque_fisico_asociado(char* bloque_logico);
void cambiar_hard_link(char* bloque_logico, char* bloque_fisico);
char* obtener_bloque_logico(char* path, int numero);
char* bloque_fisico_por_nro(int nro);
int escribir_bloque(int query_id, char* file, char* tag, int offset, char* contenido);
bool operacion_fuera_de_rango(int nro_bloque, char* path);
int leer_bloque(int query_id, char* file, char* tag, int tamanio, char** buffer);
void eliminar_bloque_logico(int query_id, char *file, char* tag, int nro);

#endif