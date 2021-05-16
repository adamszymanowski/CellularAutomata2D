@echo off

mkdir ..\Build
pushd ..\Build

cl ..\Source\%1 /Zi /EHsc User32.lib Gdi32.lib

popd