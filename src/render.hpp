#pragma once
#include "ui.hpp"
namespace moss {
class Renderer {
public:
    Renderer();
    ~Renderer();
    Renderer(const Renderer&)=delete;
    Renderer& operator=(const Renderer&)=delete;
    void draw(const Game& game,float dt);
    void capture(const std::filesystem::path& path) const;
private:
    RenderTexture2D target{};
    Vec camera{};
    RegionId lastRegion=RegionId::Count;
    void world(const Game& game,float dt);
    void title(const Game& game);
};
}
