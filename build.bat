@echo off
if not exist build mkdir build
pushd build
cl D:/Coding/dsp-project-refactor/main.cpp
popd
