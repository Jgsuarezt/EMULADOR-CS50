# Emulador CG50 (para Windows)

Emulador de hardware, escrito desde cero, inspirado en la calculadora
grafica **Casio fx-CG50**: implementa un nucleo de CPU compatible con un
subconjunto de la arquitectura **SH-4** (la familia usada en la serie
Prizm/CG de Casio), un bus de memoria y perifericos simplificados (LCD a
color, teclado, timer), con una interfaz grafica en **SDL2**.

**No incluye el firmware/OS real de Casio** (es software propietario). Lee
[`docs/ROM.md`](docs/ROM.md) para el porque y para como usar tus propios
binarios SH-4 o un volcado legalmente obtenido de tu propia calculadora.

## Estado del proyecto

Version inicial / base de trabajo. Funciona de caja: trae un demo interno
en ensamblador SH-4 (escrito a mano, sin toolchain externo) que enciende el
LCD emulado y pinta pixeles, para demostrar que CPU + bus + LCD + GUI estan
correctamente conectados. Ver limitaciones conocidas en
[`docs/ROM.md`](docs/ROM.md) (FPU, MMU, interrupciones reales, etc. aun no
implementadas).

## Compilar en Windows

Requisitos: CMake >= 3.16, un compilador C++17 (MSVC o MinGW) y **SDL2**.

Con [vcpkg](https://github.com/microsoft/vcpkg) (recomendado):

```powershell
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install sdl2:x64-windows

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

El ejecutable queda en `build/Release/cg50emu.exe` (o `build/cg50emu.exe`
con MinGW).

Si CMake no encuentra SDL2, igualmente compila el nucleo (`cg50core`) y las
pruebas (`cpu_tests`); solo se omite el ejecutable grafico hasta que SDL2
este disponible.

## Compilar y probar en Linux/macOS (desarrollo del nucleo)

```bash
sudo apt install libsdl2-dev   # opcional, solo para el ejecutable grafico
cmake -B build -S .
cmake --build build
ctest --test-dir build --output-on-failure
```

## Uso

```powershell
# Con un binario SH-4 propio:
cg50emu.exe mi_programa.bin

# Sin argumentos: corre el demo interno (no requiere ningun archivo)
cg50emu.exe
```

Controles por defecto (ver `src/gui/window.cpp` para el mapeo completo):
flechas = cursor, F1-F6 = teclas de funcion, teclado numerico = digitos,
Enter = EXE, Esc = EXIT.

## Estructura del codigo

```
src/cpu/      Nucleo del interprete SH-4 (sh4.h/.cpp)
src/mem/      Bus de memoria: ROM, RAM y perifericos (bus.h/.cpp)
src/devices/  LCD, teclado y timer (simplificados)
src/gui/      Ventana SDL2: dibuja el framebuffer y traduce el teclado
src/demo/     Programa SH-4 de demostracion, sin ROM externa
tests/        Pruebas basicas del nucleo de CPU (sin dependencias)
docs/         Notas sobre el mapa de hardware y sobre la ROM/firmware
```

## Licencia

MIT (ver [`LICENSE`](LICENSE)). No cubre, obviamente, ningun firmware de
terceros que decidas cargar en el emulador.
