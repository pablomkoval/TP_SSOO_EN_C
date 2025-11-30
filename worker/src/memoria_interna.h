#ifndef MEMORIA_INTERNA_H_
#define MEMORIA_INTERNA_H_

#include <utils/utils.h>
#include <worker.h>

typedef struct 
{
    bool bit_presencia;
    bool bit_uso;
    bool bit_modificado;
    int timestamp;//usamos un int incremental en vez de un temporal porque es mas facil de manejar
    int nro_pagina;
    int frame;
    char* file_tag;
} pagina_t;

typedef struct
{
    t_list* paginas;
} tabla_paginas_t;


extern int contador_lru;
extern t_dictionary* tablas_de_paginas;
extern void* memoria_interna;
extern t_list* paginas_en_memoria;
extern pthread_mutex_t mutex_paginas_en_memoria;

void inicializar_memoria_interna();
tabla_paginas_t* obtener_tabla(char* file_tag);
int buscar_frame_libre();
pagina_t* buscar_victima_reemplazo();
int cargar_pagina_de_storage(char* file_tag, char* file, char* tag, int nro_pagina, int frame, int qid);
int liberar_frame(pagina_t* victima, int qid);
pagina_t* obtener_pagina(char* file_tag, int nro_pagina, int qid);
int hacer_flush_de_pagina(char* file, char* tag, int nro_pagina, int frame, int qid);

int obtener_pagina_logica(int direccion_logica);
int obtener_offset_pagina(int direccion_logica);

#endif