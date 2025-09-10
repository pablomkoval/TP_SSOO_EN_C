#include <master.h>

pthread_t hilo_main_escucha;

t_config* config_master;
t_log* logger;
t_config* iniciar_config();
t_log* iniciar_logger();

char* puerto_escucha;
char* algoritmo_planificacion;
int tiempo_aging;
char* log_level;

int main(int argc, char* argv[]) {
    
    config_master = iniciar_config();
    logger = iniciar_logger();

    log_debug(logger, "se iniciaron logger y config");

    int socket_escucha = iniciar_servidor(puerto_escucha, logger);

    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket_escucha;

    pthread_create(&hilo_main_escucha, NULL, funcion_main_escucha, socket_ptr);
    pthread_detach(hilo_main_escucha);

    pause();
    return 0;
}


t_config* iniciar_config(){
    t_config* nueva_config = config_create("master.config");

    if(nueva_config == NULL){
        printf("No se encontro el archivo de config");
        return NULL;
    }
    if(config_has_property(nueva_config, "PUERTO_ESCUCHA")){
        puerto_escucha = config_get_string_value(nueva_config, "PUERTO_ESCUCHA");
        algoritmo_planificacion = config_get_string_value(nueva_config, "ALGORITMO_PLANIFICACION");
        tiempo_aging = config_get_int_value(nueva_config, "TIEMPO_AGING");
        log_level = config_get_string_value(nueva_config, "LOG_LEVEL");
        
    }
    return nueva_config;
}

t_log* iniciar_logger(){
    t_log* nuevo_logger;
    t_log_level log_level_enum = log_level_from_string(log_level);
    nuevo_logger = log_create("master.log", "LogMaster", true, log_level_enum);
    return nuevo_logger;
}
