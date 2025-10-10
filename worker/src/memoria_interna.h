#ifndef MEMORIA_INTERNA_H_
#define MEMORIA_INTERNA_H_

#include <utils/utils.h>
#include <worker.h>

typedef struct 
{
    bool bit_presencia;
    bool bit_modificado;
    int nro_pagina;
    int frame;
} pagina_t;

typedef struct
{
    t_list* paginas;
} tabla_paginas_t;


extern t_dictionary* tablas_de_paginas;

#endif