import { $ } from "bun";
import { basename, dirname } from "path";

const config = await Bun.file("config.json").json();
const slangc_binary = config.slangc_path;

for (const shader_dir of config.shader_paths) {
  const glob = new Bun.Glob(shader_dir).scanSync();

  process.env.LD_LIBRARY_PATH = `${config.vulkan_sdk_libs_path}:${process.env.LD_LIBRARY_PATH}`;

  for (const file of glob) {
    const parentDir = dirname(file);
    const fileName = basename(file, ".slang");

    const outputPath = `${parentDir}/${fileName}.spv`;

    const command = `${slangc_binary} ${file} ${config.slangc_options} -o ${outputPath}`;
    console.log(command);
    $`${command}`;
  }
}
