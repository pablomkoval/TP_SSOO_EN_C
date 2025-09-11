#ifndef ESTRUCTURA_STORAGE_H
#define ESTRUCTURA_STORAGE_H

#include <storage.h>
#include <utils/utils.h>

extern int fd;
extern t_config* metadata;
extern t_bitarray* bitmap;
extern t_config* hash;

void crear_directorios_y_archivos();
void* inicializar_bitmap();
void crear_archivo_hash_bloques();
void crear_tag(char* file_path, char* tag);
char* concatenar_path(char* path, char* suma);
void crear_directorio(const char* path);
void crear_bloque_logico(char* path, int numero);
void crear_metadata_config(char* archivo);
void crear_bloques_fisicos();
void formatear_volumen();
void crear_file(char* file, char* tag);
char* leer_archivo(char* path);
int escribir_archivo(char* path, char* contenido);
int obtener_numero_bloque(char* path);

#endif