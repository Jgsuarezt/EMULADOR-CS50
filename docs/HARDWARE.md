# Modelo de hardware (simplificado y propio)

Importante: este mapa de memoria y de registros es un **diseno propio y
simplificado** de este proyecto, no una copia verificada del hardware real
de la fx-CG50. Sirve para tener un sistema autocontenido y funcional donde
probar el nucleo de CPU; portar el firmware real de Casio requeriria en
algun momento documentacion de hardware que este proyecto no reproduce.

## Mapa de memoria

| Rango                     | Contenido                                   |
|---------------------------|----------------------------------------------|
| `0x00000000-0x003FFFFF`   | ROM (hasta 4 MB, cargada desde archivo)       |
| `0x08000000-0x087FFFFF`   | RAM (8 MB)                                    |
| `0x18000000`              | Registro de habilitacion de LCD (escribir !=0)|
| `0x18001000-0x18029800`   | Framebuffer LCD, RGB565, 384x216               |
| `0x18030000`               | Teclado: escribir = fila activa, leer = columnas |
| `0x18030004`               | Timer de libre ejecucion (solo lectura)       |
| `0x18030008`               | Control del timer (escribir !=0 = reset)      |

La CPU arranca con `PC = 0x00000000` (no hay tabla de vectores real; es
una simplificacion de arranque).

## Pantalla

384x216 pixeles, formato RGB565 (2 bytes por pixel), igual a la resolucion
fisica documentada de la fx-CG50, aunque el controlador en si es una
implementacion propia y no una replica del chip real.

## Teclado

Matriz de 8 filas x 8 columnas. El mapeo de teclas de PC -> (fila, columna)
usado por la interfaz SDL2 esta definido en `src/gui/window.cpp`
(`kKeyMap`) y no corresponde al escaneo real del teclado de la calculadora.

## CPU

Subconjunto de la ISA SH-4 (familia usada en la serie Prizm/CG de Casio),
implementado en `src/cpu/sh4.cpp`. Ver `docs/ROM.md` para el detalle de que
esta y que no esta implementado todavia.
