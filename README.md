# Emulador CG50 (para Windows)

Emulador de hardware, escrito desde cero, inspirado en la calculadora
grafica **Casio fx-CG50**: implementa un nucleo de CPU compatible con un
subconjunto de la arquitectura **SH-4** (la familia usada en la serie
Prizm/CG de Casio), un bus de memoria y perifericos simplificados (LCD a
color, teclado, timer), con una interfaz grafica **nativa de Windows**
(GDI puro, sin librerias externas como SDL2).

**No incluye el firmware/OS real de Casio** (es software propietario). Lee
[`docs/ROM.md`](docs/ROM.md) para el porque y para como usar tus propios
binarios SH-4 o un volcado legalmente obtenido de tu propia calculadora.

## Descargar el ejecutable ya compilado

Si solo quieres probarlo sin compilar nada: en la seccion
[Releases](../../releases) de este repositorio (o el ultimo build subido)
esta `cg50emu.exe`, listo para ejecutar en Windows con doble clic. No
necesita instalar nada (SDL2, vcpkg, etc.): esta enlazado de forma
estatica.

## Estado del proyecto

Version inicial / base de trabajo. Funciona de caja: trae un demo interno
en ensamblador SH-4 (escrito a mano, sin toolchain externo) que enciende el
LCD emulado y pinta pixeles, para demostrar que CPU + bus + LCD + GUI estan
correctamente conectados. Ver limitaciones conocidas en
[`docs/ROM.md`](docs/ROM.md) (FPU, MMU, interrupciones reales, etc. aun no
implementadas).

## Compilar en Windows

Requisitos: CMake >= 3.16 y un compilador C++17 (MSVC o MinGW). **No hace
falta SDL2 ni vcpkg** -- la interfaz grafica usa unicamente la API de
Windows (GDI), que ya viene con el sistema.

```powershell
cmake -B build -S .
cmake --build build --config Release
```

El ejecutable queda en `build/Release/cg50emu.exe` (o `build/cg50emu.exe`
con MinGW).

## Compilar en Linux/macOS (desarrollo del nucleo)

La ventana grafica (`cg50emu`) es especifica de Windows (usa `<windows.h>`
directamente), asi que en Linux/macOS solo se compilan el nucleo
(`cg50core`), las pruebas (`cpu_tests`) y la herramienta de volcado
headless (`cg50dump`, ver mas abajo):

```bash
cmake -B build -S .
cmake --build build
ctest --test-dir build --output-on-failure
```

### Cross-compilar el .exe de Windows desde Linux

```bash
sudo apt install g++-mingw-w64-x86-64
cmake -B build-win -S . -DCMAKE_TOOLCHAIN_FILE=scripts/toolchain-mingw64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-win
```

Genera `build-win/cg50emu.exe`, enlazado de forma estatica (sin
dependencias extra, solo las DLL de sistema de Windows: GDI32, USER32,
KERNEL32).

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

### cg50dump: capturas sin abrir ninguna ventana

Sirve para verificar el emulador (o generar una captura) sin necesitar
pantalla ni Windows:

```bash
cg50dump salida.bmp [rom.bin] [instrucciones]
```

## Estructura del codigo

```
src/cpu/      Nucleo del interprete SH-4 (sh4.h/.cpp)
src/mem/      Bus de memoria: ROM, RAM y perifericos (bus.h/.cpp)
src/devices/  LCD, teclado y timer (simplificados)
src/gui/      Ventana nativa de Windows (GDI): dibuja el framebuffer y traduce el teclado
src/demo/     Programa SH-4 de demostracion, sin ROM externa
src/tools/    Herramienta cg50dump (captura headless, multiplataforma)
src/util/     Utilidades compartidas (escritura de BMP)
tests/        Pruebas basicas del nucleo de CPU (sin dependencias)
docs/         Notas sobre el mapa de hardware y sobre la ROM/firmware
scripts/      Toolchain de CMake para cross-compilar a Windows con MinGW
```

## Licencia

MIT (ver [`LICENSE`](LICENSE)). No cubre, obviamente, ningun firmware de
terceros que decidas cargar en el emulador.
