: script to build the image using WSL, then 
: launch qemu
: %1 is forwarded to the make command
@echo off
: build image
wsl make %1

: start qemu
if "%~1" == "" (
    qemu-system-i386 -drive format=raw,file=bad-apple_boot.img -display sdl
)