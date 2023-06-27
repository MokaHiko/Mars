cd /D "%~dp0"
"%VULKAN_SDK%/bin/glslc.exe" default_shader.vert -o default_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" default_shader.frag -o default_shader.frag.spv

"%VULKAN_SDK%/bin/glslc.exe" offscreen_shader.vert -o offscreen_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" offscreen_shader.frag -o offscreen_shader.frag.spv

"%VULKAN_SDK%/bin/glslc.exe" particle_graphics_shader.vert -o particle_graphics_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" particle_graphics_shader.frag -o particle_graphics_shader.frag.spv
"%VULKAN_SDK%/bin/glslc.exe" particle_compute_shader.comp -o particle_compute_shader.comp.spv