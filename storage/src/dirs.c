#include <dirs.h>

void crear_directorio(const char *path)
{
    mkdirat(fd, path, 0777);
}

int crear_file(char *file, char *tag)
{
    char* file_tag = concatenar_path(file, tag);


    if (file_tag_existe(file_tag))
    {
        log_error(logger, "File/Tag <%s> preexistente", file_tag);

        free(file_tag);

        return -1;
    }    
    
    char *path = concatenar_path("files", file);
    char *path_tag = string_from_format("files/%s/%s", file, tag);
    char *chequeo = string_from_format("%s/files/%s", punto_montaje, file);
        
    crear_directorio(path);
    crear_tag(path_tag);

    free(path);
    free(path_tag);
    free(chequeo);
    free(file_tag);

    return 1;
}

void crear_tag(char *path_tag)
{
    crear_directorio(path_tag);
    log_debug(logger, "(%s)", path_tag);

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
    char* file_tag_destino = concatenar_path(file_destino, tag_destino);
    char* file_tag_origen = concatenar_path(file_origen, tag_origen);

    if (!file_tag_existe(file_tag_origen)){
        log_debug(logger, "FILE:TAG <%s> INEXISTENTE", file_tag_origen);

        free(file_tag_destino);
        free(file_tag_origen);

        return -2;
    }

    if (file_tag_existe(file_tag_destino)){
        log_error(logger, "FILE:TAG <%s> PREEXISTENTE", file_tag_destino);

        free(file_tag_destino);
        free(file_tag_origen);

        return -1;
    }

    char *path_origen = string_from_format("%s/files/%s/%s", punto_montaje, file_origen, tag_origen);
    char *path_destino = string_from_format("%s/files/%s/%s", punto_montaje, file_destino, tag_destino);

    crear_file(file_destino, tag_destino);

    dupear_hard_links(query_id, file_origen, tag_origen, file_destino, tag_destino);

    //char *comando = string_from_format("cp -r '%s/.' '%s'", path_origen, path_destino);

    //system(comando);

    //free(comando);
    
    lock_metadata(file_tag_destino);
    cambiar_estado_metadata(file_tag_destino, "WORK_IN_PROGRESS");
    unlock_metadata(file_tag_destino);

    free(file_tag_destino);
    free(path_origen);
    free(path_destino);
    free(file_tag_origen);

    return 1;
}

int eliminar_tag(int query_id, char *file, char* tag)    //// Falta eliminar hash de block hash index si tiene 1 solo HL
{
    char* file_tag = concatenar_path(file, tag);

    if(!file_tag_existe(file_tag)) 
    {
        log_error(logger, "File:Tag <%s> inexistente", file_tag);
        free(file_tag);
        return -2; 
    }

    char *path_config = path_config_meta(file_tag);
    t_config *meta = config_create(path_config);

    lock_metadata(file_tag);
    char **blocks = config_get_array_value(meta, "BLOCKS");
    char* estado_metadata = config_get_string_value(meta, "ESTADO");
    unlock_metadata(file_tag);

    int cantidad = 0;

    while (blocks && blocks[cantidad]) cantidad++;

    for(int i = 0; i < cantidad; i++)
    {
        char* bloque_logico = obtener_bloque_logico(file_tag, i);

        if(strcmp(estado_metadata, "commited") == 0)
        {
            eliminar_de_block_hash_index(bloque_logico);
        }


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
    free(path_config);

    return 1;
}

// CASOS ERROR

bool file_tag_existe(char *file_tag)
{
    char* path = string_from_format("%s/files/%s", punto_montaje, file_tag);

    bool existe = (access(path, F_OK) == 0);

    free(path);

    return existe;
}

void dupear_hard_links(int query_id, char *file_origen, char *tag_origen, char *file_destino, char *tag_destino)
{
    char* file_tag_origen = concatenar_path(file_origen, tag_origen);
    int cant_bloques = cant_bloques_logicos(file_tag_origen);

    char* file_tag_destino = concatenar_path(file_destino, tag_destino);

    for(int i = 0; i < cant_bloques; i++)
    {
        char* bloque_logico = obtener_bloque_logico(file_tag_origen, i);
        char* bloque_fisico = obtener_bloque_fisico_asociado(bloque_logico);

        int nro_block_f = obtener_numero_bloque(bloque_fisico);

        crear_bloque_logico(file_tag_destino, i);

        char* nuevo_bloque_logico = string_from_format("%s/files/%s/%s/logical_blocks/%06d.dat", punto_montaje, file_destino, tag_destino, i);

        cambiar_hard_link(nuevo_bloque_logico, bloque_fisico);

        log_info(logger, "##<%i> - <%s>:<%s> Se agregó el hard link del bloque lógico <%d> al bloque físico <%d>", query_id, file_destino, tag_destino, i, nro_block_f);

        free(bloque_fisico);
        free(nuevo_bloque_logico);
        free(bloque_logico);
    }

    lock_metadata(file_tag_origen);
    int tamanio = tamanio_metadata(file_tag_origen);
    char* tamanio_str = string_itoa(tamanio);
    cambiar_tamanio_metadata(file_tag_destino, tamanio_str);
    unlock_metadata(file_tag_origen);

    free(file_tag_destino);
    free(file_tag_origen);
    free(tamanio_str);
}

void eliminar_de_block_hash_index(char* bloque_logico)
{
    char* bloque_fisico = obtener_bloque_fisico_asociado(bloque_logico);
    char *md5 = obtener_hash_block(bloque_fisico);
    int bloque_fisico_nro = obtener_numero_bloque(bloque_fisico);

    if(obtener_referencias_bloque(bloque_fisico_nro) == 1)
    {
        config_remove_key(hash, md5);
        config_save(hash);
    }

    free(md5);
    free(bloque_fisico);
}
