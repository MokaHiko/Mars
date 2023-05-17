"%VULKAN_SDK%/bin/glslc.exe" default_shader.vert -o default_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" default_shader.frag -o default_shader.frag.spv

"%VULKAN_SDK%/bin/glslc.exe" offscreen_shader.vert -o offscreen_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" offscreen_shader.frag -o offscreen_shader.frag.spv
PAUSE