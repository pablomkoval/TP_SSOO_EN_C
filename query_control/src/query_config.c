#include <query_config.h>

char* ip_master;
char* puerto_master;
char* log_level;


t_log* iniciar_logger(void){
    t_log_level nivel = log_level_from_string (log_level);
    t_log* nuevo_logger;
    nuevo_logger = log_create("query_control.log","LogQueryControl",true, nivel);
    log_trace(nuevo_logger, "Funciona logger query control :)");
    return nuevo_logger;
}

t_config* iniciar_config(char* nombre_archivo){
    t_config* nueva_config;
    nueva_config = config_create(nombre_archivo);
    if(config_has_property(nueva_config, "IP_MASTER")){
        ip_master = config_get_string_value(nueva_config, "IP_MASTER");
        puerto_master = config_get_string_value(nueva_config, "PUERTO_MASTER");
        log_level = config_get_string_value(nueva_config, "LOG_LEVEL");
    }
    return nueva_config;
}
