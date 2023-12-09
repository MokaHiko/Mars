cd /D "%~dp0"
"%VULKAN_SDK%/bin/glslc.exe" default_shader.vert -o default_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" default_shader.frag -o default_shader.frag.spv

"%VULKAN_SDK%/bin/glslc.exe" offscreen_shader.vert -o offscreen_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" offscreen_shader.frag -o offscreen_shader.frag.spv

"%VULKAN_SDK%/bin/glslc.exe" sprite_shader.vert -o sprite_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" sprite_shader.frag -o sprite_shader.frag.spv

"%VULKAN_SDK%/bin/glslc.exe" particle_graphics_shader.vert -o particle_graphics_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" particle_graphics_shader.frag -o particle_graphics_shader.frag.spv
"%VULKAN_SDK%/bin/glslc.exe" particle_compute_shader.comp -o particle_compute_shader.comp.spv

"%VULKAN_SDK%/bin/glslc.exe" default_line_shader.vert -o default_line_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" default_line_shader.frag -o default_line_shader.frag.spv

"%VULKAN_SDK%/bin/glslc.exe" post_process_shader.vert -o post_process_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" post_process_shader.frag -o post_process_shader.frag.spv

"%VULKAN_SDK%/bin/glslc.exe" terrain_shader.vert -o terrain_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" terrain_shader.frag -o terrain_shader.frag.spv
"%VULKAN_SDK%/bin/glslc.exe" terrain_shader.tesc -o terrain_shader.tesc.spv
"%VULKAN_SDK%/bin/glslc.exe" terrain_shader.tese -o terrain_shader.tese.spv

"%VULKAN_SDK%/bin/glslc.exe" cb_shader.vert -o cb_shader.vert.spv
"%VULKAN_SDK%/bin/glslc.exe" cb_shader.frag -o cb_shader.frag.spv
"%VULKAN_SDK%/bin/glslc.exe" cb_shader.tesc -o cb_shader.tesc.spv
"%VULKAN_SDK%/bin/glslc.exe" cb_shader.tese -o cb_shader.tese.spv

