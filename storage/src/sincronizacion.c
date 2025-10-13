#include <sincronizacion.h>

pthread_mutex_t mutex_bitmap;
pthread_mutex_t mutex_hash_index;

t_dictionary* mutex_por_metadata;
t_dictionary* mutex_por_bloque_fisico;

void inicializar_mutexes(void) {
    pthread_mutex_init(&mutex_bitmap, NULL);
    pthread_mutex_init(&mutex_hash_index, NULL);
    mutex_por_metadata = dictionary_create();
    mutex_por_bloque_fisico = dictionary_create();
}

static pthread_mutex_t* obtener_mutex(t_dictionary* dict, char* path) {
    pthread_mutex_t* mutex = dictionary_get(dict, path);
    if (!mutex) {
        mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(mutex, NULL);
        dictionary_put(dict, string_duplicate(path), mutex);
    }
    return mutex;
}

pthread_mutex_t* obtener_mutex_metadata(char* path) {
    return obtener_mutex(mutex_por_metadata, path);
}

pthread_mutex_t* obtener_mutex_bloque_fisico(char* path) {
    return obtener_mutex(mutex_por_bloque_fisico, path);
}


void lock_metadata(char* path) {
    char *path_meta = path_config_meta(path);
    pthread_mutex_t* m = obtener_mutex_metadata(path_meta);
    pthread_mutex_lock(m);
}

void unlock_metadata(char* path) {
    char *path_meta = path_config_meta(path);
    pthread_mutex_t* m = obtener_mutex_metadata(path_meta);
    pthread_mutex_unlock(m);
}

void lock_bloque_fisico(char* path) {
    pthread_mutex_t* m = obtener_mutex_bloque_fisico(path);
    pthread_mutex_lock(m);
}

void unlock_bloque_fisico(char* path) {
    pthread_mutex_t* m = obtener_mutex_bloque_fisico(path);
    pthread_mutex_unlock(m);
}