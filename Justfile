compile_shaders:
    cd scripts && bun run compile_shaders.ts
build:
    cmake -S . -B build/ && cmake --build build/ -j $(nproc)
