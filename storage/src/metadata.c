#include <metadata.h>

t_config* metadata;

void crear_metadata_config(char* path)
{
    char* config = concatenar_path(punto_montaje, path);
    FILE* archivo = fopen(config, "w+");
    fclose(archivo);

    metadata = config_create(config);

    config_set_value(metadata, "TAMAÑO", "0");
    config_set_value(metadata, "BLOCKS", "[]");
    config_set_value(metadata, "ESTADO", "WORK_IN_PROGRESS");

    config_save(metadata);

    free(config);
}

void cambiar_estado_metadata(char *path, char *nuevo_estado)
{
    char* path_config = concatenar_path(path,"metadata.config");
    t_config* meta = config_create(path_config);
    config_set_value (meta, "ESTADO", nuevo_estado);
    config_save(meta);
    config_destroy(meta);
    free(path_config);
}

int cambiar_tamanio_metadata(char* path, char* nuevo_tamanio)
{
    t_config* meta = config_create(path);
    int tamanio_viejo = config_get_int_value(meta, "TAMAÑO");
    config_set_value (meta, "TAMAÑO", nuevo_tamanio);
    config_save(meta);
    config_destroy(meta);
    return tamanio_viejo;
}

char* estado_metadata(char* path)
{
    t_config* meta = config_create(path);
    char* estado = config_get_string_value(meta, "ESTADO");
    config_destroy(meta);

    return estado;
}

void agregar_bloque_metadata(char* path, int bloque)
{
    t_config* meta = config_create(path);
    char** blocks = config_get_array_value(meta, "BLOCKS");

    int contador = 0;

    while (blocks && blocks[contador]) contador++;

    char** nuevos = malloc(sizeof(char*) * (contador + 2));
    for (int i = 0; i < contador; i++) {
        nuevos[i] = string_duplicate(blocks[i]);
    }

    nuevos[contador] = string_from_format("%d", bloque);
    nuevos[contador + 1] = NULL;

    char* blocks_str = string_new();
    string_append(&blocks_str, "[");
    for (int i = 0; i <= contador; i++) {
        string_append(&blocks_str, nuevos[i]);
        if (i < contador) string_append(&blocks_str, ",");
    }
    string_append(&blocks_str, "]");

    config_set_value(metadata, "BLOCKS", blocks_str);
    config_save(metadata);

    
    free(blocks_str);
    for (int i = 0; i <= contador; i++) free(nuevos[i]);
    free(nuevos);
    string_array_destroy(blocks);
}

int cant_bloques_logicos(char* path)
{
    t_config* meta = config_create(path);
    char** bloques = config_get_array_value(meta, "BLOCKS");
    config_destroy(meta);

    int cant = 0;
    while (bloques[cant] != NULL) 
    {
        cant++;
    }

    return cant;
}

int commmit_file(char* path)
{

    if(!file_tag_existe(path)) return -2;

    if(strcmp(estado_metadata(path), "COMMITED") == 1)
    {
        return 1; 
    }

    cambiar_estado_metadata(path, "COMMITED");

    int cant = cant_bloques_logicos(path);

    for(int i = 0; i < cant; i++)
    {
        char* bloque_logico = obtener_bloque_logico(path, i);
        char* md5 = obtener_hash_block(bloque_logico);
        char* bloque_fisico = obtener_bloque_fisico_asociado(bloque_logico);
        int nro_bloque = obtener_bloque_por_hash(md5);
        char* bloque_fisico_nuevo = bloque_fisico_por_nro(nro_bloque);

        if(nro_bloque != -1)  // obtener bloque por hash devuelve -1 si no hay ninguno :p
        {
            cambiar_hard_link(bloque_logico, bloque_fisico_nuevo);

            if(obtener_referencias_bloque(nro_bloque) <= 1)
            {
                bitarray_clean_bit(bitmap, nro_bloque);
            }
            
        }else
        {
            asociar_hash_block(bloque_fisico);
        }
    }
    return 1;
}

bool escritura_no_permitida(char* file_tag)
{
    if(strcmp(estado_metadata(file_tag), "COMMITED") == 1);

    return 1;
}