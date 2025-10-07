#include <blocks.h>


void crear_bloque_logico(char* path, int numero)
{
    char* bloque_logico = obtener_bloque_logico(path, numero);

    char* bloque_fisico = concatenar_path(punto_montaje,"physical_blocks/bloque0000.dat");

    link(bloque_fisico, bloque_logico);

    free(bloque_logico);
    free(bloque_fisico);
}

char* obtener_bloque_logico(char* path, int numero)
{
    char* aux = string_from_format("%06d.dat", numero);
    char* bloque_logico = string_from_format("%s/%s/%s",punto_montaje, path, aux);
    free(aux);
    return bloque_logico;
}

char* bloque_fisico_por_nro(int nro)
{
    return string_from_format("%s/physical_blocks/bloque%04d.dat", punto_montaje, nro);
}

void crear_bloques_fisicos()
{
    for (int i = 0; i < cant_blocks; i++) {
        
        char* nombre_bloque = bloque_fisico_por_nro(i);

        int fd = open(nombre_bloque, O_RDWR | O_CREAT | O_TRUNC, 0666);

        if (fd == -1) {
            perror("fopen");
            free(nombre_bloque);
            exit(EXIT_FAILURE);
        }

        if (ftruncate(fd, block_size) == -1) 
        { 
            perror("ftruncate"); 
        }

        close(fd);
        free(nombre_bloque);
    }
}

char* obtener_hash_block(char* bloque)
{
    char* contenido = leer_archivo(bloque);
    int largo= strlen(contenido);
    char* md5 = crypto_md5(contenido, largo);
    free(contenido);
    return md5;
}

void asociar_hash_block(char* bloque_fisico)
{
    char* md5 = obtener_hash_block(bloque_fisico);
    
    int numero = obtener_numero_bloque(bloque_fisico);

    char* bloque = string_from_format("BLOCK_%04d", numero);

    if(config_has_property(hash, md5))
    {

    }else
    {
        config_set_value(hash, md5, bloque);
        config_save(hash);
    }

    free(md5);
    free(bloque);
}

int obtener_numero_bloque(char* path) {
    const char* nombre = strrchr(path, '/');  

    if (!nombre) 
    {
        nombre = path;   
    }

    else nombre++;                               
    int numero;
    sscanf(nombre, "bloque%d.dat", &numero);
    return numero;
}

char* leer_archivo(char* path)
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

int escribir_archivo(char* path, char* contenido)
{
    FILE* f = fopen(path, "w");

    size_t escritos = fwrite(contenido, 1, strlen(contenido), f);
    fclose(f);

    if (escritos < strlen(contenido)) {
        fprintf(stderr, "Error: no se escribieron todos los bytes\n");
        return -1;
    }
    return 0;
}

char* obtener_bloque_fisico_asociado(char* bloque_logico)
{
    struct stat st_logico;
    struct stat st_fisico;

    for(int i = 0; i < cant_blocks; i++)
    {
        char* bloque_fisico = string_from_format("%s/physical_blocks/bloque%04d.dat", punto_montaje, i);
        
        if(stat(bloque_fisico, &st_fisico) == -1) 
        {
            continue; 
        }

        if (st_fisico.st_ino == st_logico.st_ino && st_fisico.st_dev == st_logico.st_dev) 
        {
            return bloque_fisico;
        }
    }
    return NULL;
}

void eliminar_bloque_logico(char* path, int nro)
{
    char* aux = string_from_format("%06d.dat", nro);
    char* bloque_logico = string_from_format("%s/%s/%s",punto_montaje, path, aux);

    char* md5 = obtener_hash_block(bloque_logico);

    int nro_bloque = obtener_bloque_por_hash(md5);

    remove(bloque_logico);

    if(obtener_referencias_bloque(nro_bloque) <= 1)
    {
        bitarray_clean_bit(bitmap, nro_bloque);
    }
}

int obtener_referencias_bloque(int nro_bloque)
{
    char* path = string_from_format("%s/physical_blocks/bloque%04d.dat",punto_montaje, nro_bloque);

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

int obtener_bloque_por_hash(char* md5)
{
    if(config_has_property (hash, md5))
    {
        char* bloque = config_get_string_value (hash, md5);
        return obtener_numero_bloque(bloque);
    }else return -1;

}

void truncar_archivo(int nuevo_tamanio, char* file_tag)
{
    char* config_path = concatenar_path(file_tag, "metadata.config");

    char* nuevo_tamanio_str = string_itoa(nuevo_tamanio);

    int tamanio_previo = cambiar_tamanio_metadata(config_path, nuevo_tamanio_str);

    int bloque_maximo = tamanio_previo / block_size;

    char* logical_blocks_path = concatenar_path(file_tag, "logical_blocks");

    if(nuevo_tamanio > tamanio_previo)
    {
        int bloques_necesarios = nuevo_tamanio / block_size;

        for(int i = bloque_maximo; i < bloques_necesarios; i++)
        {
            crear_bloque_logico(logical_blocks_path, i);
        }
    }else
    {
        int bloques_necesarios = (tamanio_previo - nuevo_tamanio) / block_size;

        for(int i = bloque_maximo - 1; i > bloques_necesarios - 1; i--)
        {
            eliminar_bloque_logico(logical_blocks_path, i);
        }
    }

    free(nuevo_tamanio_str);
    
}

void cambiar_hard_link(char* bloque_logico, char* bloque_fisico)
{
    unlink(bloque_logico);
    link(bloque_fisico, bloque_logico);
}
