# Trabajo Práctico - Sistemas Operativos (UTN FRBA)

## Descripción
Desarrollo de un sistema distribuido en **C** bajo el entorno **Linux/GNU**. Este proyecto fue realizado para la materia Sistemas Operativos de la Universidad Tecnológica Nacional (UTN FRBA), correspondiente al 2do cuatrimestre de 2025. 

El sistema simula un entorno distribuido de procesamiento, planificando tareas, administrando memoria y gestionando la comunicación entre distintos nodos a través de la red.

## Arquitectura del Sistema
El ecosistema del proyecto se divide en módulos independientes y concurrentes:

* **Master:** Orquestador principal del sistema. Se encarga de la planificación, gestión de conexiones y administración de los bloques de control.
* **Worker:** Nodo de procesamiento encargado de ejecutar las tareas. Administra su propia memoria interna y procesa las peticiones mediante un intérprete de comandos.
* **Storage:** Módulo responsable de la persistencia de datos.
* **Query Control:** Módulo encargado del control y ruteo de las consultas.

## Mis Contribuciones Principales
Mi rol en el equipo se centró en la arquitectura, lógica y desarrollo de los procesos **Worker** y **Master**:

**Desarrollo del Módulo Worker:**
* Implementación de la gestión de memoria interna.
* Desarrollo del procesamiento e interpretación de consultas.
* Programación de la interfaz de red para las conexiones del worker.

**Desarrollo del Módulo Master:**
* Diseño e implementación del algoritmo del **Planificador** de tareas para la correcta orquestación del trabajo.
* Implementación y manejo de los Query Control Blocks.
* Parseo de configuraciones y gestión de sockets del maestro.

## Tecnologías y Conceptos Aplicados
* **Lenguaje:** C .
* **Sistemas Distribuidos:** Arquitectura de múltiples procesos comunicados por Sockets TCP.
* **Concurrencia:** Manejo avanzado de hilos (`pthreads`), semáforos y exclusión mutua (`mutex`) para sincronización de procesos.
* **Gestión de Memoria:** Manejo de punteros, serialización/deserialización de estructuras dinámicas y control de fugas de memoria con `Valgrind`.
* **Herramientas de Desarrollo:** `gcc`, `make`.
