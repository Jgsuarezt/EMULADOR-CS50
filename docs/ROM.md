# Sobre la ROM / firmware

Este proyecto **no incluye, ni incluira, el sistema operativo/firmware real
de la Casio fx-CG50**. Ese firmware es software propietario de CASIO
COMPUTER CO., LTD. y esta protegido por derechos de autor: distribuirlo o
incluirlo en este repositorio seria una infraccion.

Lo que este repositorio si contiene es un **emulador de hardware**: un
nucleo de CPU (subconjunto de la ISA SH-4), un bus de memoria y unos
perifericos simplificados (LCD, teclado, timer). Es exactamente el mismo
modelo legal que usan los emuladores de consolas serios (por ejemplo los de
Nintendo o Sony): el emulador se distribuye sin la BIOS/firmware del
fabricante, y cada usuario aporta su propio volcado, obtenido legalmente de
un dispositivo que posee.

## ¿Que puedo usar entonces?

- El **demo interno** (`demo::BuildFillDemo()`): un programa SH-4 minimo,
  escrito a mano por este proyecto, que enciende el LCD emulado y pinta una
  franja de pixeles. No necesita ningun archivo externo y sirve para
  comprobar que el nucleo de CPU + bus + LCD funciona.
- Binarios propios en SH-4 (por ejemplo compilados con un toolchain
  educativo/de terceros para SH), cargados con:
  ```
  cg50emu.exe mi_programa.bin
  ```
- Si posees fisicamente una fx-CG50 y decides extraer tu propio volcado de
  firmware para uso personal (copia de seguridad de un dispositivo que te
  pertenece), puedes intentar cargarlo aqui. Ten en cuenta que:
  1. El mapa de memoria/perifericos de este proyecto es **simplificado y
     propio** (ver `docs/HARDWARE.md`), no una replica verificada del
     hardware real, asi que no hay garantia de que el OS real arranque tal
     cual; probablemente haga falta documentacion adicional del hardware
     real (registros del controlador de LCD, teclado, interrupciones, MMU,
     FPU, etc.) para lograr compatibilidad completa.
  2. Sigue las leyes de tu pais respecto a copias de seguridad de software
     que posees legalmente.

## Estado del nucleo de CPU

El interprete SH-4 de este proyecto implementa un subconjunto util de la
ISA (aritmetica, logica, saltos, acceso a memoria, algunos registros de
control) pero **no** implementa todavia:

- Unidad de punto flotante (FPU).
- MMU / modos privilegiados completos.
- Controlador de interrupciones real (el timer solo se puede consultar por
  sondeo, no genera interrupciones).
- Banco de registros R0-R7 (SR.RB) usado en el manejo rapido de
  excepciones.

Cualquier opcode no implementado detiene la CPU de forma controlada e
informa por consola el PC y el opcode exacto, para facilitar extender el
nucleo.
