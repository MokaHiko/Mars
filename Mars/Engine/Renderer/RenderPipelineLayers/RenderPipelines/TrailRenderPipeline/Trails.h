#ifndef TRAILS_H
#define TRAILS_H

#pragma once

#include "Math/Math.h"
#include "Renderer/Vulkan/VulkanMesh.h"

namespace mrs
{
    struct TrailRenderer
    {
        static const int max_points = 12;

        // From: https://docs.unity3d.com/Manual/class-TrailRenderer.html
        // Min Vertex Distance value determines how far in world units the GameObject to which the trail applies must travel before a new segment is added to the trail.
        float min_vertex_distance = 1.0f;
        float width = 1.0f;
    private:
        friend class TrailRenderPipeline;

        int insert_index = 0;

        Vector3 last_position = {};
        std::vector<Vertex> points;

        size_t vertex_offset = 0; // Offset into trails vertex buffer

        bool dirty = true;
    };
}

#endif