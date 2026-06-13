#pragma once
#include <unordered_map>
#include <vector>

struct Material;
class Mesh;
class FrameBuffer;

using MaterialMap = std::unordered_map<uint32_t, Material>;
using MeshMap = std::unordered_map<uint32_t, std::vector<Mesh> >;
using PingPongBuffer = std::array<std::unique_ptr<FrameBuffer>, 2>;
