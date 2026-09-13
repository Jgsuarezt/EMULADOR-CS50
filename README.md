# CAGIO CG 50

Una calculadora cientifica para Windows, con una interfaz inspirada en el
diseno fisico de las calculadoras graficadoras de Casio (pantalla + teclado
con SHIFT, funciones cientificas, cruceta, etc.), pero con su propia marca
("CAGIO CG 50") y un motor de calculo propio escrito desde cero -- no es
una copia ni contiene ningun software de Casio.

Ademas, el repositorio incluye un segundo proyecto tecnico separado: un
emulador de **hardware** (nucleo de CPU SH-4, la familia usada en las
calculadoras graficadoras reales) para quien quiera experimentar a bajo
nivel. Ver la seccion [Nucleo de hardware SH-4](#nucleo-de-hardware-sh-4-avanzado)
mas abajo.

## Descargar el ejecutable ya compilado

Si solo quieres probarla sin compilar nada, en la carpeta
[`dist/`](dist/) de este repositorio estan los .exe ya compilados:

- [`dist/CAGIO_CG50.exe`](dist/CAGIO_CG50.exe) -- la calculadora.
- [`dist/EmuladorHardwareSH4.exe`](dist/EmuladorHardwareSH4.exe) -- el
  demo del nucleo de hardware SH-4 (ver mas abajo).

Para descargarlos desde GitHub: entra a la carpeta `dist/`, haz clic en el
archivo `.exe` que quieras, y en la pagina del archivo pulsa el boton
**Download raw file** (el icono de flecha hacia abajo, arriba a la
derecha del visor). Ambos estan enlazados de forma estatica -- no
necesitan instalar nada, solo doble clic.

## La calculadora (`cg50calc`)

- Interfaz dibujada a mano (Win32/GDI puro, sin librerias externas) que
  imita la distribucion fisica tipica de una calculadora graficadora:
  pantalla, teclas F1-F6, SHIFT, cruceta de navegacion/historial, teclado
  cientifico y numerico.
- Motor de expresiones propio (`src/calc/engine.h`/`.cpp`): suma, resta,
  multiplicacion, division, potencias, parentesis, multiplicacion
  implicita (`2sin(30)`, `2(3+4)`), funciones trigonometricas (con su
  inversa via SHIFT), logaritmos, raiz cuadrada, `Ans`, `pi`, `e`,
  porcentaje.
- Escribe con el mouse (clic en las teclas) o con el teclado del PC
  (numeros, operadores, Enter = EXE, Backspace = DEL, Esc = AC, flechas
  izquierda/derecha mueven el cursor, arriba/abajo navegan el historial de
  calculos anteriores).
- Modo grados/radianes conmutable con la tecla MENU.

Compilar (Windows, MSVC o MinGW; no requiere SDL2 ni vcpkg):

```powershell
cmake -B build -S .
cmake --build build --config Release
```

El ejecutable queda en `build/Release/cg50calc.exe` (o `build/cg50calc.exe`
con MinGW).

Cross-compilar desde Linux/macOS con MinGW:

```bash
sudo apt install g++-mingw-w64-x86-64
cmake -B build-win -S . -DCMAKE_TOOLCHAIN_FILE=scripts/toolchain-mingw64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-win
```

Genera `build-win/cg50calc.exe`, sin dependencias extra aparte de las DLL
de sistema de Windows (GDI32, USER32, KERNEL32).

## Nucleo de hardware SH-4 (avanzado)

Este repositorio tambien contiene, como proyecto tecnico separado, un
emulador de **hardware** de bajo nivel: un interprete de un subconjunto de
la arquitectura SH-4 (`src/cpu`), un bus de memoria con LCD/teclado/timer
simplificados (`src/mem`, `src/devices`) y un ejecutable de demostracion
(`cg50emu`) que corre un programa SH-4 de prueba (o binarios propios) sin
requerir ningun firmware de Casio -- ver [`docs/ROM.md`](docs/ROM.md) y
[`docs/HARDWARE.md`](docs/HARDWARE.md) para el detalle. No tiene relacion
con la calculadora de arriba (esa usa un motor de calculo nativo en C++,
no pasa por esta CPU emulada).

```powershell
# Con un binario SH-4 propio:
cg50emu.exe mi_programa.bin
# Sin argumentos: corre el demo interno
cg50emu.exe
```

`cg50dump` genera una captura del framebuffer sin abrir ninguna ventana
(util para verificar sin pantalla ni Windows):

```bash
cg50dump salida.bmp [rom.bin] [instrucciones]
```

## Compilar y probar en Linux/macOS (sin las GUI, que son especificas de Windows)

```bash
cmake -B build -S .
cmake --build build
ctest --test-dir build --output-on-failure
```

Compila el nucleo SH-4 (`cg50core`), el motor de calculo (`cg50calclib`),
`cg50dump` y las pruebas (`cpu_tests`, `calc_engine_tests`).

## Estructura del codigo

```
src/calc/       Motor de expresiones de la calculadora (multiplataforma)
src/calc_gui/   Interfaz Win32/GDI de la calculadora (CAGIO CG 50)
src/main_calc.cpp   Punto de entrada de cg50calc

src/cpu/        Nucleo del interprete SH-4 (proyecto tecnico separado)
src/mem/        Bus de memoria: ROM, RAM y perifericos
src/devices/    LCD, teclado y timer (simplificados)
src/gui/        Ventana Win32/GDI del emulador de hardware (cg50emu)
src/demo/       Programa SH-4 de demostracion, sin ROM externa
src/main.cpp    Punto de entrada de cg50emu

src/tools/      Herramienta cg50dump (captura headless, multiplataforma)
src/util/       Utilidades compartidas (escritura de BMP)
tests/          Pruebas del motor de calculo y del nucleo de CPU
docs/           Notas sobre el mapa de hardware y sobre la ROM/firmware
scripts/        Toolchain de CMake para cross-compilar a Windows con MinGW
```

## Licencia

MIT (ver [`LICENSE`](LICENSE)). No cubre, obviamente, ningun firmware de
terceros que decidas cargar en el emulador de hardware.
