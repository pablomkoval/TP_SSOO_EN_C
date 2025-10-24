#include <blocks.h>

void crear_bloque_logico(char *file_tag, int numero)
{

    char *logical_blocks_path = concatenar_path(file_tag, "logical_blocks");

    char *bloque_logico = obtener_bloque_logico(logical_blocks_path, numero);

    char *bloque_fisico = concatenar_path(punto_montaje, "physical_blocks/bloque0000.dat");

    lock_metadata(file_tag);
    unlink(bloque_logico);
    link(bloque_fisico, bloque_logico);
    agregar_bloque_metadata(file_tag, 0, numero);
    unlock_metadata(file_tag);

    free(logical_blocks_path);
    free(bloque_logico);
    free(bloque_fisico);
}

char *obtener_bloque_logico(char *path, int numero)  //no hace falta sincro
{
    char *aux = string_from_format("%06d.dat", numero);
    char *bloque_logico = string_from_format("%s/files/%s/%s", punto_montaje, path, aux);
    free(aux);
    return bloque_logico;
}

char *bloque_fisico_por_nro(int nro) //no hace falta sincro
{
    return string_from_format("%s/physical_blocks/bloque%04d.dat", punto_montaje, nro);
}

void crear_bloques_fisicos() //no hace falta sincro
{
    for (int i = 0; i < cant_blocks; i++)
    {

        char *nombre_bloque = bloque_fisico_por_nro(i);

        int fd = open(nombre_bloque, O_RDWR | O_CREAT | O_TRUNC, 0666);

        if (fd == -1)
        {
            perror("fopen");
            free(nombre_bloque);
            exit(EXIT_FAILURE);
        }

        if (ftruncate(fd, block_size) == -1)
        {
            perror("ftruncate");
        }

        pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(mutex, NULL);
        dictionary_put(mutex_por_bloque_fisico, nombre_bloque, mutex);

        close(fd);
        free(nombre_bloque);
    }
}

char *obtener_hash_block(char *bloque) 
{
    char *contenido = leer_archivo(bloque);
    int largo = strlen(contenido);
    char *md5 = crypto_md5(contenido, largo);
    free(contenido);
    return md5;
}

void asociar_hash_block(char *bloque_fisico)
{
    char *md5 = obtener_hash_block(bloque_fisico);

    int numero = obtener_numero_bloque(bloque_fisico);

    char *bloque = string_from_format("BLOCK_%04d", numero);

    if (config_has_property(hash, md5))
    {
    }
    else
    {
        config_set_value(hash, md5, bloque);
        config_save(hash);
    }

    free(md5);
    free(bloque);
}

int obtener_numero_bloque(char *path) //no hace falta sincro
{
    const char *nombre = strrchr(path, '/');

    if (!nombre)
    {
        nombre = path;
    }

    else
        nombre++;
    int numero;
    sscanf(nombre, "bloque%d.dat", &numero);
    return numero;
}

char *leer_archivo(char *path) //sincro cuando se usa
{
    FILE* f = fopen(path, "r");

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char* buffer = malloc(size + 1);

    size_t leidos = fread(buffer, 1, size, f);
    buffer[leidos] = '\0';  

    fclose(f);
    return buffer;
}

int escribir_archivo(char *path, char *contenido, int offset) //sincro cuando se usa
{
    FILE *f = fopen(path, "r+");

    fseek(f, offset, SEEK_SET);

    fwrite(contenido, 1, strlen(contenido), f);

    fclose(f);

    return 0;
}

char *obtener_bloque_fisico_asociado(char *bloque_logico) //no se si se necesita sincro (?)
{
    struct stat st_logico;
    struct stat st_fisico;

    if (stat(bloque_logico, &st_logico) == -1) {
        perror("Error al acceder al bloque lógico");
        return NULL;
    }

    for (int i = 0; i < cant_blocks; i++)
    {
        char *bloque_fisico = string_from_format("%s/physical_blocks/bloque%04d.dat", punto_montaje, i);

        if (stat(bloque_fisico, &st_fisico) == -1)
        {
            continue;
        }

        if (st_fisico.st_ino == st_logico.st_ino && st_fisico.st_dev == st_logico.st_dev)
        {
            return bloque_fisico;
        }

        free(bloque_fisico);
    }
    return NULL;
}

void eliminar_bloque_logico(int query_id, char *file, char* tag, int nro)   //ya sincro, no se obtener bloque fisico asociado
{
    char* file_tag = concatenar_path(file, tag);
    char *logical_blocks_path = concatenar_path(file_tag, "logical_blocks");

    char *aux = string_from_format("%06d.dat", nro);
    char *bloque_logico = string_from_format("%s/files/%s/%s", punto_montaje, logical_blocks_path, aux);

    char* bloque_fisico = obtener_bloque_fisico_asociado(bloque_logico);

    int nro_bloque = obtener_numero_bloque(bloque_fisico);

    remove(bloque_logico);
    
    lock_metadata(file_tag);
    quitar_ultimo_bloque_metadata(file_tag);
    unlock_metadata(file_tag);

    if (obtener_referencias_bloque(nro_bloque) <= 1)
    {
        pthread_mutex_lock(&mutex_bitmap);
        bitarray_clean_bit(bitmap, nro_bloque);
        pthread_mutex_unlock(&mutex_bitmap);
        log_info(logger, "##<%i> - Bloque Físico Liberado - Número de Bloque: <%i>", query_id, nro_bloque);
    }

    log_info(logger, "##<%i> - <%s>:<%s> Se eliminó el hard link del bloque lógico <%d> al bloque físico <%d>", query_id, file, tag, nro, nro_bloque);

    free(file_tag);
    free(logical_blocks_path);
    free(aux);
    free(bloque_logico);
    free(bloque_fisico);
}

int obtener_referencias_bloque(int nro_bloque) //no se si hace falta sincro (?)
{
    char *path = string_from_format("%s/physical_blocks/bloque%04d.dat", punto_montaje, nro_bloque);

    struct stat st;

    if (stat(path, &st) == -1)
    {
        perror("stat");
        free(path);
        return -1;
    }

    free(path);
    return st.st_nlink;
}

int obtener_bloque_por_hash(char *md5) 
{
    if (config_has_property(hash, md5))
    {
        char *bloque = config_get_string_value(hash, md5);
        return obtener_numero_bloque(bloque);
    }
    else
        return -1;
}

int truncar_archivo(int query_id, int nuevo_tamanio, char *file, char *tag)
{
    char *file_tag = concatenar_path(file, tag);

    if (!file_tag_existe(file_tag))
        return -2;

    char *nuevo_tamanio_str = string_itoa(nuevo_tamanio);

    lock_metadata(file_tag);
    int tamanio_previo = cambiar_tamanio_metadata(file_tag, nuevo_tamanio_str);
    unlock_metadata(file_tag);

    int bloques_previos = (tamanio_previo + block_size - 1) / block_size;
    int bloques_nuevos  = (nuevo_tamanio + block_size - 1) / block_size;

    if (nuevo_tamanio > tamanio_previo)
    {

        for (int i = bloques_previos; i < bloques_nuevos; i++)
        {
            crear_bloque_logico(file_tag, i);
            log_info(logger, "##<%i> - <%s>:<%s> Se agregó el hard link del bloque lógico <%d> al bloque físico <%d>", query_id, file, tag, i, 0);
        }
    }
    else
    {
        for (int i = bloques_previos - 1; i >= bloques_nuevos - 1; i--)
        {
            eliminar_bloque_logico(query_id, file, tag, i);
        }
    }
    
    free(file_tag);
    free(nuevo_tamanio_str);

    return 1;
}

void cambiar_hard_link(char *bloque_logico, char *bloque_fisico)
{
    unlink(bloque_logico);
    link(bloque_fisico, bloque_logico);
}

int escribir_bloque(int query_id, char *file, char* tag, int offset, char *contenido)
{
    char *path = concatenar_path(file, tag);
    int tamanio = strlen(contenido);

    if (!file_tag_existe(path)){
        free(path);
        return -2;
    }

    lock_metadata(path);

    if (escritura_no_permitida(path)){
        free(path);
        return -4;
    }
        
    if (operacion_fuera_de_rango(offset, tamanio, path)){
        free(path);
        return -5;
    }
        

    unlock_metadata(path);

    int nro_bloque = offset / block_size;
    int offset_interno = offset - (nro_bloque * block_size);

    char *aux = string_from_format("%06d.dat", nro_bloque);
    char *bloque_logico = string_from_format("%s/files/%s/logical_blocks/%s", punto_montaje, path, aux);

    char *bloque_fisico = obtener_bloque_fisico_asociado(bloque_logico);

    int nro_block_f = obtener_numero_bloque(bloque_fisico);

    if (obtener_referencias_bloque(nro_block_f) <= 1)
    {
        lock_bloque_fisico(bloque_fisico);
        escribir_archivo(bloque_fisico, contenido, offset_interno);
        unlock_bloque_fisico(bloque_fisico);

        log_info(logger, "##<%d> - Bloque Lógico Escrito <%s>:<%s> - Número de Bloque: <%i>",query_id, file, tag, nro_bloque);
    }
    else
    {
        pthread_mutex_lock(&mutex_bitmap);

        int nro_bloque_f_nuevo = buscar_bloque_libre();
        if (nro_bloque_f_nuevo == -3)
        {
            pthread_mutex_unlock(&mutex_bitmap);
            return -3;
        }
        asignar_bloque(nro_bloque_f_nuevo);

        pthread_mutex_unlock(&mutex_bitmap);

        char *nuevo_bloque_fisico = bloque_fisico_por_nro(nro_bloque_f_nuevo);

        lock_metadata(path);
        lock_bloque_fisico(nuevo_bloque_fisico);

        escribir_archivo(nuevo_bloque_fisico, contenido, offset_interno);
        asociar_hash_block(nuevo_bloque_fisico);
        cambiar_hard_link(bloque_logico, nuevo_bloque_fisico);
        cambiar_bloque_metadata(path, nro_bloque_f_nuevo, nro_bloque);

        unlock_bloque_fisico(nuevo_bloque_fisico);
        unlock_metadata(path);

        free(nuevo_bloque_fisico);

        log_info(logger, "##<%i> - Bloque Físico Reservado - Número de Bloque: <%i>", query_id, nro_bloque_f_nuevo);
        log_info(logger, "##<%i> - <%s>:<%s> Se eliminó el hard link del bloque lógico <%d> al bloque físico <%d>", query_id, file, tag, nro_bloque, nro_block_f);
        log_info(logger, "##<%i> - <%s>:<%s> Se agregó el hard link del bloque lógico <%d> al bloque físico <%d>", query_id, file, tag, nro_bloque, nro_bloque_f_nuevo);
        //log_info(logger, "##<%d> - Bloque Lógico <%i> se reasigna de <%i> a <%i>",query_id, nro_bloque, nro_block_f, nro_bloque_f_nuevo);
        log_info(logger, "##<%d> - Bloque Lógico Escrito <%s>:<$%s> - Número de Bloque: <%i>",query_id, file, tag, nro_bloque);
    }
    free(aux);
    free(bloque_logico);
    free(bloque_fisico);
    free(path);
    return 1;
}

///// MANU ACORDATE DE AGREGAR LOS CASOS DE ERROR GRACIAS ATTE MANU :P
int leer_bloque(int query_id, char* file, char* tag, int nro_bloque, char** buffer )
{
    char *file_tag = concatenar_path(file, tag);

    if (!file_tag_existe(file_tag))
        return -2;
    if (operacion_fuera_de_rango(nro_bloque * block_size, block_size, file_tag))
        return -5;

    //int nro_bloque = offset / block_size;
    //int offset_interno = offset - (nro_bloque * block_size);

    char *aux = string_from_format("%06d.dat", nro_bloque);
    char *bloque_logico = string_from_format("%s/%s/%s", punto_montaje, file_tag, aux);
    char* bloque_fisico = obtener_bloque_fisico_asociado(bloque_logico);

    lock_bloque_fisico(bloque_fisico);
    char *contenido = leer_archivo(bloque_fisico);
    unlock_bloque_fisico(bloque_fisico);

    *buffer = contenido;

    free(file_tag);
    free(aux);
    free(bloque_logico);
    free(bloque_fisico);
    free(contenido);

    return 1;
}

bool operacion_fuera_de_rango(int offset, int tamanio, char *path)
{
    char* path_meta = path_config_meta(path);
    t_config *meta = config_create(path_meta);
    int tamanio_tag = config_get_int_value(meta, "TAMAÑO");

    free(path_meta);
    config_destroy(meta);

    if ((offset + tamanio) > tamanio_tag)
        return true;

    return false;
    
    
}
