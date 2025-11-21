#!/bin/bash

cd ..

if [ -f "build-wasm/Makefile" ]; then
    cd build-wasm
    make
    cp -v AlengWasm.js ../aleng-ide/public
    cp -v AlengWasm.wasm ../aleng-ide/public
else
  rm -r build-wasm
  mkdir build-wasm
  cd build-wasm

  emcmake cmake ..
  make

  cp -v AlengWasm.js ../aleng-ide/public
  cp -v AlengWasm.wasm ../aleng-ide/public
fi