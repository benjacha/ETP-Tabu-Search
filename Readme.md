# Solución ETP - Tabu Search

Este proyecto implementa una metaheurística de Búsqueda Tabú para resolver el *Examination Timetabling Problem* en su variante *Room-Capacitated*.

**Autor:** Benjamin araos

## Requisitos

- Compilador C++ (G++ con soporte C++11 o superior)

- Utilidad `make`

## Estructura del Proyecto

- `etp_ts.cpp`: Código fuente principal.

- `instancias/`: Carpeta que debe contener los archivos de entrada (`i1.in` a `i8.in`).

- `Makefile`: Archivo de automatización de compilación.



## Instrucciones de Compilación y Ejecución



Para compilar el proyecto, abre una terminal en la carpeta raíz y ejecuta:



``` make ```



Esto generará un ejecutable llamado solver.



Para ejecutar los experimentos completos (que prueban las 3 inicializaciones con diferentes iteraciones), utiliza el comando:



``` make run ```



Salidas Generadas

El programa ejecutará automáticamente el algoritmo con las siguientes configuraciones:



Estrategias: Greedy, Naive (Secuencial Simple) y Aleatoria.



Iteraciones: 60, 500 y 1000.



Instancias: 8 archivos de prueba estandarizados.



Los resultados se guardarán en los siguientes archivos de texto para su análisis:



Greedy.txt: Reporte de métricas usando inicialización voraz.



Naive.txt: Reporte de métricas usando inicialización secuencial.



Aleatorio.txt: Reporte de métricas usando inicialización aleatoria.



Limpieza

Para eliminar el ejecutable y los archivos de resultados generados, ejecute:



``` make clean ```
