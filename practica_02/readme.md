# Práctica 02 – Retardos no bloqueantes

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
[Enunciado Práctica 2](https://docs.google.com/document/d/173_tBdN7rIAfT5S-5lSsJWCGIJHKG1ay3VvMh4yfM58/edit?usp=sharing)


## Descripción

En esta práctica se implementa un módulo de **retardos no bloqueantes** utilizando una base de tiempo obtenida mediante `HAL_GetTick()`.

Se define la estructura `delay_t` y las funciones:

- `delayInit()`
- `delayRead()`
- `delayWrite()`

El programa principal utiliza estas funciones para generar el **parpadeo del LED de la placa de desarrollo** con distintos períodos definidos en un arreglo de tiempos.

## Archivos del repositorio

- `main.c` → Implementación del módulo de delay y programa principal.
- `main.h` → Definiciones de tipos, estructura `delay_t` y prototipos de funciones.
- `README.md` → Descripción de la práctica.

## Preguntas de análisis

Las siguientes preguntas se proponen para reflexionar luego de resolver el ejercicio.

### 1. ¿Se pueden cambiar los tiempos de encendido fácilmente o están hardcodeados?

Si, desde el uso de macro para los periódos del 50% es cuestión de agregar una nueva macro o cambiar el valor a una existente, ya estan referenciadas para mS y se puede referenciar a segundos incluso (tener cuidado en utilizar el sistick para tiempos grandes ya que existen otros tipo de timers diseñados para estas funciones).

---

### 2. ¿Qué bibliotecas estándar se debieron agregar para que el código compile? Si las funcionalidades crecieran, habría que pensar cuál sería el mejor lugar para incluir esas bibliotecas y algunos typedefs que se usan en el ejercicio.

Se tuvo que agregar las bibliotecas de "stdint.h" y "stdbool.h" para los tipos de datos y los booleanos respectivamente. Si el codigo creciera, es conveniente modularizar los diferentes módulos en capas de software para que cada uno contenga sus bibliotecas a utilizar sin necesidad de que estas se compilen cuando no es necesario (por ejemplo si la exportamos a otro proyecto).

---

### 3. ¿Es adecuado el control de los parámetros pasados por el usuario que se hace en las funciones implementadas? ¿Se controla que sean valores válidos? ¿Se controla que estén dentro de los rangos correctos?

Se podría definir un máximo a implementar de delay por el módulo de systick, para que no pueda ser configurado con valores erroneos que el sistick va a desbordar. Por otro lado al manejar estructuras por referencia, es decir, por punteros. Se debe validar que las mismas no esten vacías (null pointer) para no generar errores en ejecución del programa principal.

---

### 4. ¿Cuán reutilizable es el código implementado?

El único limitante que contiene es que la obtención de los ticks se hace directamente con la HAL, para ampliar la portabilidad se podría crear una interfaz en la que este paso se haga dedicado al micro importado. Por otro lado, al usar tipos de dato standard no debería haber problemas en moverse de arquitectura.

---

### 5. ¿Cuán sencillo resulta en su implementación cambiar el patrón de tiempos de parpadeo?

Se pensó para que el patron de tiempos se pueda modificar cambiando el orden del vector en el que fue declarado.

---


