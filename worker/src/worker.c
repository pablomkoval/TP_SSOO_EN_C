#include <worker.h>

t_config* config_worker;
t_log* logger;
t_config* iniciar_config(char* archivo_config);
t_log* iniciar_logger(int worker_id);

char* ip_master;
char* puerto_master;
char* ip_storage;
char* puerto_storage;
int tam_memoria;
int retardo_memoria;
char* algoritmo_reemplazo;
char* path_queries;
char* log_level;
int tam_pagina;

int socket_storage;
int socket_master;
int worker_id;

pthread_t thread_query_interpreter;
pthread_t thread_escucha_master;
bool hay_interrupcion = false;
pthread_mutex_t mutex_interrupcion = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char** argv) {
     if(argc < 3){
         printf("Faltaron argumentos para iniciar el worker");
         return EXIT_FAILURE;
     }
     char* nombre_archivo = argv[1];
     worker_id = atoi(argv[2]);
    // char* nombre_archivo = "worker.config";
    // worker_id = 1;
    config_worker = iniciar_config(nombre_archivo);
    logger = iniciar_logger(worker_id);

    log_debug(logger, "se iniciaron logger y config");


    socket_storage = conectar_storage(worker_id);
    socket_master = conectar_master(worker_id);
    inicializar_memoria_interna();

    log_debug(logger, "se iniciaron conexiones");


    pthread_create(&thread_escucha_master, NULL, funcion_escucha_master, NULL);
    pthread_detach(thread_escucha_master);

    pause();

    return 0;
}

t_config* iniciar_config(char* archivo_config){
    t_config* nueva_config = config_create(archivo_config);

    if(nueva_config == NULL){
        printf("No se encontro el archivo de config");
        return NULL;
    }
    if(config_has_property(nueva_config, "IP_MASTER")){
        ip_master = config_get_string_value(nueva_config, "IP_MASTER");
        puerto_master = config_get_string_value(nueva_config, "PUERTO_MASTER");
        ip_storage = config_get_string_value(nueva_config, "IP_STORAGE");
        puerto_storage = config_get_string_value(nueva_config, "PUERTO_STORAGE");
        tam_memoria = config_get_int_value(nueva_config, "TAM_MEMORIA");
        retardo_memoria = config_get_int_value(nueva_config, "RETARDO_MEMORIA");
        algoritmo_reemplazo = config_get_string_value(nueva_config, "ALGORITMO_REEMPLAZO");
        path_queries = config_get_string_value(nueva_config, "PATH_QUERIES");
        log_level = config_get_string_value(nueva_config, "LOG_LEVEL");
        
    }
    return nueva_config;
}

t_log* iniciar_logger(int worker_id){
    t_log* nuevo_logger;
    t_log_level log_level_enum = log_level_from_string(log_level);
    char* nombre_log = string_from_format("worker%d.log", worker_id);
    nuevo_logger = log_create(nombre_log, "LogWorker", true, log_level_enum);
    free(nombre_log);
    return nuevo_logger;
}
