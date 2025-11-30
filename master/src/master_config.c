#include <master_config.h>

char* puerto_escucha;
char* algoritmo_planificacion;
int tiempo_aging;
char* log_level;

t_config* iniciar_config(char* nombre_config){
    t_config* nueva_config = config_create(nombre_config);

    if(nueva_config == NULL){
        printf("No se encontro el archivo de config '%s'\n", nombre_config);
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