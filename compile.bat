@echo off
REM Caminhos da Allegro — personalizados pra você
set INCLUDE_PATH="C:\Users\pedro\OneDrive\Documentos\Allegro\allegro-i686-w64-mingw32-gcc-14.2.0-posix-dwarf-static-5.2.10.0\allegro\include"
set LIB_PATH="C:\Users\pedro\OneDrive\Documentos\Allegro\allegro-i686-w64-mingw32-gcc-14.2.0-posix-dwarf-static-5.2.10.0\allegro\lib"
set DLL_PATH="C:\Users\pedro\OneDrive\Documentos\Allegro\allegro-i686-w64-mingw32-gcc-14.2.0-posix-dwarf-static-5.2.10.0\allegro\bin"

REM Compilar o main.c
gcc main.c -o jogo.exe -I%INCLUDE_PATH% -L%LIB_PATH% -lallegro_monolith -static-libgcc -static-libstdc++

REM Verifica se houve erro de compilação
if %errorlevel% neq 0 (
    echo [ERRO] A compilação falhou.
    pause
    exit /b
)

REM Copia as DLLs da Allegro para a pasta do jogo
copy %DLL_PATH%\*.dll . >nul

REM Roda o jogo
echo [OK] Compilado com sucesso! Rodando o jogo...
jogo.exe

pause
