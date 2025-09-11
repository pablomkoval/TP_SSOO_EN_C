#include <blocks.h>

void crear_bloque_logico(char* path, int numero)
{
    char* aux = string_from_format("%06d.dat", numero);

    char* bloque_logico = string_from_format("%s/%s/%s",punto_montaje, path, aux);

    char* bloque_fisico = concatenar_path(punto_montaje,"physical_blocks/bloque0000.dat");

    link(bloque_fisico, bloque_logico);
}

void crear_bloques_fisicos()
{
    for (int i = 0; i < cant_blocks; i++) {
        
        char* nombre_bloque = string_from_format("%s/physical_blocks/bloque%04d.dat", punto_montaje, i); //el %04d hace que tenga 4 digitos

        FILE* archivo = fopen(nombre_bloque, "w+");
        if (archivo == NULL) {
            perror("fopen");
            free(nombre_bloque);
            exit(EXIT_FAILURE);
        }
        fclose(archivo);

        free(nombre_bloque);
    }
}

void asociar_hash_block(char* bloque_fisico)
{
    char* contenido = leer_archivo(bloque_fisico);
    int largo= strlen(contenido);
    char* md5 = crypto_md5(contenido, largo);

    free(contenido);

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
    nombre++;                            
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

void eliminar_bloque_logico(char* path, int nro)
{
    char* aux = string_from_format("%06d.dat", nro);
    char* bloque_logico = string_from_format("%s/%s/%s",punto_montaje, path, aux);

    remove(bloque_logico);

    //falta hacer logica de bitmap y chequeos 
}



void truncar_archivo(int nuevo_tamanio, char* file_tag)
{
    char* config_path = concatenar_path(file_tag, "metadata.config");

    int tamanio_previo = cambiar_tamanio_metadata(config_path, nuevo_tamanio);

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
    
}