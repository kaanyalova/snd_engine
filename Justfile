compile_shaders:
    cd scripts && bun run compile_shaders.ts
build:
    cmake -S . -DCMAKE_BUILD_TYPE=Debug -B build/ && cmake --build build/ -j $(nproc)
    cp build/compile_commands.json .
run:
    cmake -S . -DCMAKE_BUILD_TYPE=Debug -B build/ && cmake --build build/ -j $(nproc) 
    cp build/compile_commands.json .
    build/sneed
add_vulkan_sdk_to_path:
    cd scripts && eval $(bun run print_sdk_bin_export.ts)
