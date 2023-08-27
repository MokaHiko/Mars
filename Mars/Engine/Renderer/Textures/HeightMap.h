#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#pragma once

#include "Core/Memory.h"

namespace mrs
{
    class HeightMap
    {
    public:
        //HeightMap(Ref<Texture> texture);
        HeightMap(const std::string& path);
        ~HeightMap();

    private:
        std::vector<char> _data;
    };
}

#endif