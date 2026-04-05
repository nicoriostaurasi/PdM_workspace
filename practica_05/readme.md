# Práctica 05 – UART

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
[Enunciado Práctica 5](https://docs.google.com/document/d/13_3swkdCwSAWZ8qNzTYDihHd3oLF9torA4u83Hwvsgk/edit?tab=t.0)


## Descripción

Implementar un módulo de software para utilizar la UART y una MEF para parsear comandos recibidos por UART en modo polling.

## Archivos del repositorio

- `main.c` → Implementación de la aplicación principal.
- `main.h` → Definiciones de tipos.
- `API_uart.h` → Prototipos de funciones públicas para manejar la UART
- `API_uart.c` → Código de funciones de uart driver
- `API_cmdparser.h` → Prototipos de funciones públicas
- `API_cmdparser.c` → Código de funciones de parseo, en particular la MEF de parseo
- `README.md` → Descripción de la práctica.

---