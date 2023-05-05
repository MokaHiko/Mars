#include "VulkanInitializers.h"

VkCommandPoolCreateInfo vkinit::command_pool_create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags create_flags)
{
    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;

    info.queueFamilyIndex = queue_family_index;
    info.flags = create_flags;

    return info;
}

VkCommandBufferAllocateInfo vkinit::command_buffer_alloc_info(VkCommandPool cmd_pool, VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc.pNext = nullptr;

    alloc.commandBufferCount = 1;
    alloc.commandPool = cmd_pool;
    alloc.level = level;

    return alloc;
}

VkCommandBufferBeginInfo vkinit::command_buffer_begin_info(VkCommandBufferUsageFlags usage)
{
    VkCommandBufferBeginInfo begin = {};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.pNext = nullptr;
    begin.pInheritanceInfo = nullptr;
    begin.flags = usage;

    return begin;
}

VkRenderPassBeginInfo vkinit::render_pass_begin_info(VkFramebuffer frame_buffer, VkRenderPass render_pass, VkRect2D& render_area, VkClearValue* clear_value, uint32_t clear_value_count)
{
    VkRenderPassBeginInfo begin = {};
    begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin.pNext = nullptr;

    begin.clearValueCount = clear_value_count;
    begin.pClearValues = clear_value;

    begin.framebuffer = frame_buffer;
    begin.renderPass = render_pass;
    begin.renderArea = render_area;

    return begin;
}
