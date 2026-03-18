#pragma once
#include <unordered_map>
#include <vector>

struct Material;
class Mesh;
using MaterialMap = std::unordered_map<uint32_t, Material>;
using MeshMap = std::unordered_map<uint32_t, std::vector<Mesh> >;
