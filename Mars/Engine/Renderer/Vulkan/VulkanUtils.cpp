#include "VulkanUtils.h"

VkPipeline vkutil::PipelineBuilder::Build(VkDevice device, VkRenderPass renderPass)
{
    VkPipelineViewportStateCreateInfo view_port_state = {};
	view_port_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

	view_port_state.viewportCount = 1;
	view_port_state.pViewports = &_viewport;
	view_port_state.scissorCount = 1;
	view_port_state.pScissors = &_scissor;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &_color_blend_attachment;

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	pipeline_info.stageCount = static_cast<uint32_t>(_shader_stages.size());
	pipeline_info.pStages = _shader_stages.data();
	pipeline_info.pVertexInputState = &_vertex_input_info;
	pipeline_info.pInputAssemblyState = &_input_assembly;
	pipeline_info.pRasterizationState = &_rasterizer;
	pipeline_info.pMultisampleState = &_multisampling;
	pipeline_info.pDepthStencilState = &_depth_stencil;
	pipeline_info.layout = _pipeline_layout;
	pipeline_info.pViewportState = &view_port_state;
	pipeline_info.pColorBlendState = &color_blending;

	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.renderPass = renderPass;
	pipeline_info.subpass = 0;

	VkPipeline new_pipeline;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &new_pipeline) != VK_SUCCESS)
	{
		printf("Failed to create graphics pipeline!\n");
		return VK_NULL_HANDLE;
	}

	return new_pipeline;
}
