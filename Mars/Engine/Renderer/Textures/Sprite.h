#ifndef SPRITE_H
#define SPRITE_H

#pragma once


#include "Renderer/Vulkan/VulkanTexture.h"
#include "Core/Memory.h"

#include "Math/Math.h"
#include "Core/ResourceManager.h"
namespace mrs {
    struct Rectangle
    {
        float x = 0;
        float y = 0;

        float width = 0;
        float height = 0;
    };

    class Sprite
    {
    public:
        enum class Mode
        {
            Single,
            Multiple
        };

        Sprite();
        ~Sprite();

        // Create a sprite from a texture
		static Ref<Sprite> Create(Ref<Texture> texture, const std::string& yaml_meta_data, const std::string& alias = "");
		static Ref<Sprite> Create(Ref<Texture> texture, const std::string& alias = "");
		static Ref<Sprite> Get(const std::string& alias);

        const Ref<Texture> Atlas() const {return _atlas;}
        Rectangle& Rect(int index = 0);

        const VkDescriptorSet DescriptorSet() const {return _descriptor_set;}
        VkDescriptorSet& DescriptorSet() {return _descriptor_set;}

        const Mode SpriteMode () const;
        void SetMode(Sprite::Mode mode);

        const int SpriteIndex() const;
        int& SpriteIndex();

        const int SpriteCount() const;
    private:
        // For single mode
        Rectangle _current_rect;
        Ref<Texture> _atlas;

        // For multiple mode
        int _sprite_index = 0;
        std::vector<Rectangle> _multi_sprites;
    private:
        VkDescriptorSet _descriptor_set;
        Mode _mode = Mode::Single;
    };
}

#endif