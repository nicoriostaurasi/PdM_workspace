# Práctica 03 – Modularización

## Materia
Programación de Microcontroladores  
Especialización en Sistemas Embebidos – FIUBA

## Alumno
Nombre: Ing. Nicolás Gabriel Rios Taurasi  
Email: nicolas.rios.taurasi@gmail.com/nicoriostaurasi@frba.utn.edu.ar  

## Docente
Nombre: Mg. Ing. Patricio Bos  
Email: pbos@fi.uba.ar

## Ciclo lectivo
1er Bimetre - 2026

## Enunciado

El enunciado completo de la práctica puede consultarse en:
[Enunciado Práctica 3](https://docs.google.com/document/d/1o45cu4Y6-IP3PXC4nwgnLuBHyxaYz1LkqilcBYN1GCU/edit?tab=t.0#heading=h.imu48mg4u4w2)


## Descripción

En esta práctica el módulo implementado previamente para la práctica 2 de **retardos no bloqueantes** se modulariza bajo el nombre de API, dentro de la carpeta de Drivers.

Se define un archivo `API_delay.h` el cual define la estructura `delay_t` y los prótotipos de funciones:

- `delayInit()`
- `delayRead()`
- `delayWrite()`
- `delayIsRunning()`

El programa principal utiliza este modulo para generar el **parpadeo del LED de la placa de desarrollo** con distintos períodos definidos en un arreglo de tiempos.

## Archivos del repositorio

- `main.c` → Implementación del módulo de delay y programa principal.
- `main.h` → Definiciones de tipos.
- `API_delay.h` → Estructura `delay_t` y prototipos de funciones
- `API_delay.c` → Código de funciones
- `README.md` → Descripción de la práctica.

## Preguntas de análisis

Las siguientes preguntas se proponen para reflexionar luego de resolver el ejercicio.

### 1. ¿Es suficientemente clara la consigna 2 o da lugar a implementaciones con distinto comportamiento? 

El parpadeo puede interpretarse a que es un solo período o que son varias repeticiones.

---

### 2. ¿Se puede cambiar el tiempo de encendido del led fácilmente en un solo lugar del código o éste está hardcodeado? ¿Hay números “mágicos” en el código?

Se puede modificar el tiempo del define, o definir una nueva constante del preprocesador con el nuevo tiempo.

---

### 3. ¿Qué bibliotecas estándar se debieron agregar a API_delay.h para que el código compile? Si las funcionalidades de una API propia crecieran, habría que pensar cuál sería el mejor lugar para incluir esas bibliotecas y algunos typedefs que se usen en la implementación, ¿Cuál sería el mejor lugar?.

Las bibliotecas que se debieron agregar son `stdbool.h` y `stdint.h` ya que son necesarias para crear los tipos de datos a utilizar en la API de delay. Por otro lado la HAL de STM32, unicamente ha sido importada dentro del .c ya que es interno del módulo. De crecer la biblioteca, se podría armar un header el cual incluya todas las dependencias del módulo ahi y uno incluyendo solamente ese archivo puede utilizar todos los módulos de la librería.

---

### 4. ¿Es adecuado el control de los parámetros pasados por el usuario que se hace en las funciones implementadas? ¿Se controla que sean valores válidos? ¿Se controla que estén dentro de los rangos esperados?

Se realiza un control de nullptr por si el usuario no ha inicializado aun el puntero a la estructura a pasar por referencia, esto podría generar excepciones en ejecución finalizando el programa principal, o llevando el micro a un estado de error.  

---