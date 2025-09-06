#include <storage_config.h>

t_log* iniciar_logger();

char* puerto_escucha;
bool fresh_start;
char* punto_montaje;
int retardo_operacion;
int retardo_acceso_bloque;
char* log_level;





t_log* iniciar_logger(void){
    t_log_level nivel = log_level_from_string (log_level);
    t_log* nuevo_logger;
    nuevo_logger = log_create("storage.log","LogStorage",true, nivel);
    log_trace(nuevo_logger, "Funciona logger storage :)");
    return nuevo_logger;
}

t_config* iniciar_config(void){
    t_config* nueva_config;
    nueva_config = config_create("storage.config");
    if(config_has_property(nueva_config, "PUERTO_ESCUCHA"))
    {
        puerto_escucha = config_get_string_value(nueva_config, "PUERTO_ESCUCHA");
        punto_montaje = config_get_string_value(nueva_config, "PUNTO_MONTAJE");
        fresh_start = config_get_int_value(nueva_config, "FRESH_START");
        retardo_acceso_bloque = config_get_int_value(nueva_config, "RETARDO_ACCESO_BLOQUE");
        retardo_operacion = config_get_int_value(nueva_config, "RETARDO_OPERACION");
        log_level = config_get_string_value(nueva_config, "LOG_LEVEL");
    }
    return nueva_config;
}