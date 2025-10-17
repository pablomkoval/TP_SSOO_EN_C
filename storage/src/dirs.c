#include <dirs.h>

void crear_directorio(const char *path)
{
    mkdirat(fd, path, 0777);
}

int crear_file(char *file, char *tag)
{
    char *path = concatenar_path("files", file);
    char *path_tag = string_from_format("files/%s/%s", file, tag);
    char *chequeo = string_from_format("%s/files/%s", punto_montaje, file);

    if (file_tag_existe(chequeo))
        return -1;

    crear_directorio(path);
    crear_tag(path_tag);

    free(path);
    free(path_tag);
    free(chequeo);

    

    return 1;
}

void crear_tag(char *path_tag)
{
    crear_directorio(path_tag);

    char *path_blocks = concatenar_path(path_tag, "logical_blocks");
    crear_directorio(path_blocks);

    char *path_metadata = concatenar_path(path_tag, "metadata.config");
    crear_metadata_config(path_metadata);

    free(path_blocks);
    free(path_metadata);
}

char *concatenar_path(char *path, char *suma)
{
    return string_from_format("%s/%s", path, suma);
}

int copiar_tag(int query_id, char *file_origen, char *tag_origen, char *file_destino, char *tag_destino)
{

    char *path_origen = string_from_format("%s/files/%s/%s", punto_montaje, file_origen, tag_origen);

    char *path_destino = string_from_format("%s/files/%s/%s", punto_montaje, file_destino, tag_destino);

    char* file_tag_destino = concatenar_path(file_destino, tag_destino);

    if (file_tag_existe(path_destino))
        return -1;

    crear_file(file_destino, tag_destino);

    char *comando = string_from_format("cp -r '%s/.' '%s'", path_origen, path_destino);

    system(comando);

    free(comando);
    
    lock_metadata(file_tag_destino);
    cambiar_estado_metadata(file_tag_destino, "WORK_IN_PROGRESS");
    unlock_metadata(file_tag_destino);

    free(file_tag_destino);
    free(path_origen);
    free(path_destino);

    return 1;
}

int eliminar_tag(int query_id, char *file, char* tag)
{
    char* file_tag = concatenar_path(file, tag);

    if(!file_tag_existe(file_tag)) 
    {
        free(file_tag);
        return -2; 
    }

    char *path_config = path_config_meta(file_tag);
    t_config *meta = config_create(path_config);

    lock_metadata(file_tag);
    char **blocks = config_get_array_value(meta, "BLOCKS");
    unlock_metadata(file_tag);

    int cantidad = 0;

    while (blocks && blocks[cantidad]) cantidad++;

    for(int i = 0; i < cantidad; i++)
    {
        char* bloque_logico = obtener_bloque_logico(file_tag, i);
        eliminar_bloque_logico(query_id, file, tag, i);
        free(bloque_logico);
    }
    

    char* comando = string_from_format("rm -rf '%s/files/%s'", punto_montaje, file_tag);

    system(comando);
    
    for (int i = 0; blocks[i] != NULL; i++) free(blocks[i]);

    free(blocks);
    config_destroy(meta);
    free(comando);
    free(file_tag);

    return 1;
}

// CASOS ERROR

bool file_tag_existe(char *file_tag)
{
    char* path = string_from_format("%s/files/%s", punto_montaje, file_tag);

    if (access(path, F_OK) == 0)
    {
        free(path);
        return 1;
    }
    else
    {
        free(path);
        return 0;
    }
        
}
