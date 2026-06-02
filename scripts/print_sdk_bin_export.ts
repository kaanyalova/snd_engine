import { $ } from "bun";

const config = await Bun.file("./config.json").json();
const vulkan_sdk_bin_path = `${config.vulkan_sdk_path}/bin`;
console.log(`export PATH="${vulkan_sdk_bin_path}:$PATH"`);