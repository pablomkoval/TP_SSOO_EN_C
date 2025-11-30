#include <metadata.h>

t_config *metadata;

void crear_metadata_config(char *path)
{
    char *config = concatenar_path(punto_montaje, path);
    FILE *archivo = fopen(config, "w+");
    fclose(archivo);

    metadata = config_create(config);

    config_set_value(metadata, "TAMAÑO", "0");
    config_set_value(metadata, "BLOCKS", "[]");
    config_set_value(metadata, "ESTADO", "WORK_IN_PROGRESS");

    pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    dictionary_put(mutex_por_metadata, config, mutex);

    config_save(metadata);
    config_destroy(metadata);

    free(config);
}

void cambiar_estado_metadata(char *path, char *nuevo_estado)
{
    char *path_config = path_config_meta(path);
    t_config *meta = config_create(path_config);                      
    config_set_value(meta, "ESTADO", nuevo_estado);
    config_save(meta);
    config_destroy(meta);
    free(path_config);
}

int cambiar_tamanio_metadata(char *path, char *nuevo_tamanio)
{
    char *path_config = path_config_meta(path);
    t_config *meta = config_create(path_config);
    int tamanio_viejo = config_get_int_value(meta, "TAMAÑO");
    config_set_value(meta, "TAMAÑO", nuevo_tamanio);
    config_save(meta);
    config_destroy(meta);
    free(path_config);
    return tamanio_viejo;
}

int tamanio_metadata(char *path)
{
    char *path_config = path_config_meta(path);
    t_config *meta = config_create(path_config);
    int tamanio_viejo = config_get_int_value(meta, "TAMAÑO");
    config_destroy(meta);
    free(path_config);
    return tamanio_viejo;
}

char *estado_metadata(char *path)
{
    char *path_config = path_config_meta(path);
    t_config *meta = config_create(path_config);
    char *estado_original = config_get_string_value(meta, "ESTADO");
    char *estado = string_duplicate(estado_original);
    
    config_destroy(meta);
    free(path_config);

    return estado;
}

void cambiar_bloque_metadata(char *path, int bloque, int pos)
{
    char *path_config = path_config_meta(path);
    t_config *meta = config_create(path_config);
    char **blocks = config_get_array_value(meta, "BLOCKS");

    char *bloque_str = string_itoa(bloque);

    int contador = 0;

    while (blocks && blocks[contador])
        contador++;

    if (blocks && blocks[pos] != NULL) {
        free(blocks[pos]); 
    }

    blocks[pos] = bloque_str;

    char *blocks_str = string_new();

    string_append(&blocks_str, "[");

    for (int i = 0; i < contador; i++)
    {
        string_append(&blocks_str, blocks[i]);
        if (i < contador - 1)
            string_append(&blocks_str, ",");
    }
    string_append(&blocks_str, "]");

    config_set_value(meta, "BLOCKS", blocks_str);
    config_save(meta);

    for (int i = 0; blocks[i] != NULL; i++)
        free(blocks[i]);

    free(blocks);
    free(blocks_str);
    config_destroy(meta);
    free(path_config);
}

void agregar_bloque_metadata(char *path, int bloque, int pos)
{
    char *path_config = path_config_meta(path);
    t_config *meta = config_create(path_config);
    char **blocks = config_get_array_value(meta, "BLOCKS");

    char *bloque_str = string_itoa(bloque);

    char *blocks_str = string_new();

    int cantidad = 0;
    while (blocks && blocks[cantidad])
        cantidad++;

    char **nuevos = malloc(sizeof(char *) * (cantidad + 2));

    for (int i = 0; i < cantidad; i++)
        nuevos[i] = string_duplicate(blocks[i]);

    nuevos[cantidad] = string_duplicate(bloque_str);
    nuevos[cantidad + 1] = NULL;

    string_append(&blocks_str, "[");

    for (int i = 0; i < cantidad + 1; i++)
    {
        string_append(&blocks_str, nuevos[i]);
        if (i < cantidad)
            string_append(&blocks_str, ",");
    }
    string_append(&blocks_str, "]");

    config_set_value(meta, "BLOCKS", blocks_str);
    config_save(meta);

    for (int i = 0; i < cantidad + 1; i++)
        free(nuevos[i]);
    free(nuevos);

    for (int i = 0; blocks[i] != NULL; i++) free(blocks[i]);

    free(blocks);
    free(bloque_str);
    free(blocks_str);
    config_destroy(meta);
    free(path_config);
}

void quitar_ultimo_bloque_metadata(char *path)
{
    char *path_config = path_config_meta(path);
    t_config *meta = config_create(path_config);
    char **blocks = config_get_array_value(meta, "BLOCKS");

    int cantidad = 0;
    while (blocks && blocks[cantidad])
        cantidad++;

    if (cantidad == 0)
    {
        log_warning(logger, "No hay bloques para eliminar en %s", path);
        config_destroy(meta);
        free(path_config);
        string_array_destroy(blocks);
        return;
    }

    char **nuevos = malloc(sizeof(char *) * cantidad);
    for (int i = 0; i < cantidad - 1; i++)
        nuevos[i] = string_duplicate(blocks[i]);
    nuevos[cantidad - 1] = NULL;

    char *blocks_str = string_new();
    string_append(&blocks_str, "[");

    for (int i = 0; i < cantidad - 1; i++)
    {
        string_append(&blocks_str, nuevos[i]);
        if (i < cantidad - 2)
            string_append(&blocks_str, ",");
    }
    string_append(&blocks_str, "]");


    config_set_value(meta, "BLOCKS", blocks_str);
    config_save(meta);

    for (int i = 0; i < cantidad - 1; i++)
        free(nuevos[i]);
    free(nuevos);
    string_array_destroy(blocks);
    free(blocks_str);
    config_destroy(meta);
    free(path_config);

}

int cant_bloques_logicos(char *path)   //si aparece hay que lockear metadata
{
    char *path_config = path_config_meta(path);
    t_config *meta = config_create(path_config);
    char **bloques = config_get_array_value(meta, "BLOCKS");
    

    int cant = 0;
    while (bloques[cant] != NULL)
    {
        cant++;
    }

    config_destroy(meta);
    free(path_config); 
    string_array_destroy(bloques);
    
    return cant;
}

int commmit_file(int query_id, char *file, char *tag)  //sincronizada
{
    char *file_tag = concatenar_path(file, tag);
    

    if (!file_tag_existe(file_tag))
    {
        log_error(logger, "FILE_TAG_INEXISTENTE");
        return -2;
    }
    
    lock_metadata(file_tag);

    char* estado = estado_metadata(file_tag);
    if (strcmp(estado, "COMMITED") == 0)
    {
        log_error(logger, "el estado del tag ya estaba en COMMITED");
        return 1;
    }
    cambiar_estado_metadata(file_tag, "COMMITED");
    int cant = cant_bloques_logicos(file_tag);

    unlock_metadata(file_tag);

    for (int i = 0; i < cant; i++)   //por cada bloque logico
    {
        char *bloque_logico = obtener_bloque_logico(file_tag, i);    //path bloque logico
        char *md5 = obtener_hash_block(bloque_logico);  // veo su contenido md5
                                   
        char *bloque_fisico = obtener_bloque_fisico_asociado(bloque_logico);  // consigo su bloque fisico
        int nro_block_f = obtener_numero_bloque(bloque_fisico);    

        int nro_bloque = obtener_bloque_por_hash(md5);                              // me fijo si hay otro bloque fisico con el mismo contenido
        char *bloque_fisico_nuevo = bloque_fisico_por_nro(nro_bloque);
        
        

        if (nro_bloque != -1 && (nro_bloque != nro_block_f)) // si hay algún bloque fisico con el mismo contenido...
        {
            lock_metadata(file_tag);
            cambiar_hard_link(bloque_logico, bloque_fisico_nuevo);
            cambiar_bloque_metadata(file_tag, nro_bloque, i);
            unlock_metadata(file_tag);

            log_info(logger, "##<%i> - <%s>:<%s> Se eliminó el hard link del bloque lógico <%d> al bloque físico <%d>", query_id, file, tag, i, nro_block_f);
            log_info(logger, "##<%i> - <%s>:<%s> Se agregó el hard link del bloque lógico <%d> al bloque físico <%d>", query_id, file, tag, i, nro_bloque);
            

            log_info(logger, "##<%d> - Bloque Lógico <%i> se reasigna de <%i> a <%i>", query_id, i, nro_block_f, nro_bloque);

            if (obtener_referencias_bloque(nro_block_f) <= 1)
            {
                pthread_mutex_lock(&mutex_bitmap);
                bitarray_clean_bit(bitmap, nro_block_f);
                pthread_mutex_unlock(&mutex_bitmap);
            }
        }
        else if(nro_bloque == nro_block_f)
        {
            break;
        }
        else   //si no hay 
        {
            pthread_mutex_lock(&mutex_hash_index);
            asociar_hash_block(bloque_fisico);            // creo la entrada de hash en el archivo 
            pthread_mutex_unlock(&mutex_hash_index);
        }

        free(bloque_logico);
        free(md5);
        free(bloque_fisico);
        free(bloque_fisico_nuevo);
    }

    free(file_tag);
    free(estado);

    return 1;
}

bool escritura_no_permitida(char *file_tag)
{
    char *estado = estado_metadata(file_tag);
    bool bloqueado = (strcmp(estado, "COMMITED") == 0);
    free(estado);
    return bloqueado;
}

char *path_config_meta(char *file_tag)
{
    return string_from_format("%s/files/%s/metadata.config", punto_montaje, file_tag);
}