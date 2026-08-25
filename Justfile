compile_shaders:
    cd scripts && bun run compile_shaders.ts

build:
    cmake -S . -DCMAKE_BUILD_TYPE=Debug -B build/debug/ && cmake --build build/debug/ -j $(nproc)
    cp build/debug/compile_commands.json .

run:
    cmake -S . -DCMAKE_BUILD_TYPE=Debug -B build/debug/ && cmake --build build/debug/ -j $(nproc) 
    cp build/debug/compile_commands.json .
    build/debug/sneed

build_release:
    cmake -S . -DCMAKE_BUILD_TYPE=Release -B build/release/ && cmake --build build/release/ -j $(nproc)
    cp build/release/compile_commands.json .

run_release:
    cmake -S . -DCMAKE_BUILD_TYPE=Release -B build/release/ && cmake --build build/release/ -j $(nproc)
    cp build/release/compile_commands.json .
    build/release/sneed

add_vulkan_sdk_to_path:
    cd scripts && eval $(bun run print_sdk_bin_export.ts)
