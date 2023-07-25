@echo OFF
cls
SET mypath=%~dp0
pushd %mypath%\TEMP\
start prime_factor.exe
popd