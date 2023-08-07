#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#pragma once

namespace mrs
{
    class HeightMap
    {
    public:
        //HeightMap(std::shared_ptr<Texture> texture);
        HeightMap(const std::string& path);
        ~HeightMap();

    private:
        std::vector<char> _data;
    };
}

#endif