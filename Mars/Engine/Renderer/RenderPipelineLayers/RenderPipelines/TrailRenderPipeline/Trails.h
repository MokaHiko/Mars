#ifndef TRAILS_H
#define TRAILS_H

#pragma once

#include "Math/Math.h"
#include "Renderer/Vulkan/VulkanMesh.h"

namespace mrs
{
    struct TrailRenderer
    {
        // From: https://docs.unity3d.com/Manual/class-TrailRenderer.html
        // Min Vertex Distance value determines how far in world units the GameObject to which the trail applies must travel before a new segment is added to the trail.
        float min_vertex_distance = 1.0f;
        float width = 1.0f;

        Ref<Mesh> mesh;
    private:
        friend class TrailRenderPipeline;

        int insert_index = 0;
        int max_points = 12;

        std::vector<Vector3> points;

        int start_index = 0;
    };
}

#endif