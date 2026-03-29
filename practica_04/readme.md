# Práctica 04 – Debounce

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
[Enunciado Práctica 4](https://docs.google.com/document/d/17ozxLRF3_898eoATfY_PE-KQy71lubf5OqtGzNxQ5-I/edit?tab=t.0)


## Descripción

Se busca la implementación de una MEF para trabajar con anti-rebotes por software. 

## Archivos del repositorio

- `main.c` → Implementación de la aplicación principal.
- `main.h` → Definiciones de tipos.
- `API_delay.h` → Estructura `delay_t` y prototipos de funciones
- `API_delay.c` → Código de funciones
- `API_debounce.h` → Prototipos de funciones públicas
- `API_debounce.c` → Código de funciones, en particular la MEF
- `README.md` → Descripción de la práctica.


## Preguntas de análisis

Las siguientes preguntas se proponen para reflexionar luego de resolver el ejercicio.

### 1. ¿Es adecuado el control de los parámetros pasados por el usuario que se hace en las funciones implementadas? ¿Se controla que sean valores válidos? ¿Se controla que estén dentro de los rangos correctos?
 
Si, el control es adecuado ya que se evalúa que los mismos no generen excepciónes o errores en ejecución. Se controla que los mismos esten en el rango adecuado.

---

### 2. ¿Se nota una mejora en la detección de las pulsaciones respecto a la práctica 0? ¿Se pierden pulsaciones? ¿Hay falsos positivos?

Con respecto a la práctica 0 se mejora la respuesta al implementar un antirebote. Por otro lado se elimina todo efecto de falsos positivos y ruido eléctrico.

---

### 3. ¿Es adecuada la temporización con la que se llama a debounceFSM_update()? ¿Y a readKey()? ¿Qué pasaría si se llamara con un tiempo mucho más grande? ¿Y mucho más corto?

La temporización es independiente en ambos casos y por otro lado no es bloqueante. Por otro lado, si el tiempo de debounce es mas grande el sistema tardaría mas en detectar los cambios de las teclas, volviendo al sistema mas lento. Si el tiempo es mas corto, la respuesta es mas rápida pero se corre el riesgo de que sucedan falsos positivos o errores de detección del estado de las teclas.

---