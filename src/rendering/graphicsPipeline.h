#pragma once
#include <vector>
#include "glad/glad.h"
#include "shader.h"
#include "enumUtils.hpp"

enum class CullMode: uint32_t {
	None = GL_NONE,
	Front = GL_FRONT,
	Back = GL_BACK
};

enum class FrontFace: uint32_t {
	Clockwise = GL_CW,
	CounterClockwise = GL_CCW
};

enum class PolygonMode: uint32_t {
	Fill = GL_FILL,
	Line = GL_LINE,
	Point = GL_POINT
};

enum class PolygonFace: uint32_t {
	Front = GL_FRONT,
	Back = GL_BACK,
	FrontAndBack = GL_FRONT_AND_BACK
};

enum class BlendFactor: uint32_t {
	SrcAlpha = GL_SRC_ALPHA,
	OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
	One = GL_ONE,
	Zero = GL_ZERO,
	DstAlpha = GL_DST_ALPHA,
	OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA,
	SrcColor = GL_SRC_COLOR,
	OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
	DstColor = GL_DST_COLOR,
	OneMinusDstColor = GL_ONE_MINUS_DST_COLOR,
};

enum class BlendOp: uint32_t {
	Add = GL_FUNC_ADD,
	Subtract = GL_FUNC_SUBTRACT,
	ReverseSubtract = GL_FUNC_REVERSE_SUBTRACT,
	Min = GL_MIN,
	Max = GL_MAX
};

enum class ColorComponent: uint32_t {
	Red = GL_RED,
	Green = GL_GREEN,
	Blue = GL_BLUE,
	Alpha = GL_ALPHA
};

enum class PrimitiveTopology: uint32_t {
	Triangles = GL_TRIANGLES,
	Lines = GL_LINES,
	Points = GL_POINTS,
	Patches = GL_PATCHES
};

enum class CompareOp: uint32_t {
	Less = GL_LESS,
	Lequal = GL_LEQUAL,
	Greater = GL_GREATER,
	Gequal = GL_GEQUAL,
	Equal = GL_EQUAL,
	Notequal = GL_NOTEQUAL,
	Always = GL_ALWAYS,
	Never = GL_NEVER
};

struct SamplerInfo {
	std::string name;
	int32_t slot;
};

struct UniformBindingInfo {
	std::string name;
	uint32_t binding;
};

struct PipelinePrimitiveAssemblyState {
	PrimitiveTopology topology;
};

struct PipelineRasterizationState {
	CullMode cullMode;
	FrontFace frontFace;
	PolygonMode polygonMode;
	PolygonFace polygonFace;
};

struct PipelineDepthStencilState {
	bool depthTestEnable{};
	bool depthWriteEnable{};
	bool stencilTestEnable{};
	CompareOp depthCompareOp{};
};

struct PipelineColorBlendState {
	bool blendEnable{};
	BlendFactor srcColorBlendFactor{};
	BlendFactor dstColorBlendFactor{};
	BlendOp colorBlendOp{};
	BlendFactor srcAlphaBlendFactor{};
	BlendFactor dstAlphaBlendFactor{};
	BlendOp alphaBlendOp{};
	ColorComponent colorWriteMask{};
};

struct PipelineTessellationState {
	int32_t patchControlPoints;
};

struct PipelineRenderingInfo {
	PipelinePrimitiveAssemblyState primitiveAssembly;
	PipelineRasterizationState rasterization;
	PipelineDepthStencilState depthStencil;
	PipelineColorBlendState colorBlend;
	PipelineTessellationState tessellation;
	Shader stage;
	std::vector<SamplerInfo> samplers;
	std::vector<UniformBindingInfo> uniforms;
};

struct PipelineState {
	PipelinePrimitiveAssemblyState inputAssembly;
	PipelineRasterizationState rasterization;
	PipelineDepthStencilState depthStencil;
	PipelineColorBlendState colorBlend;
	PipelineTessellationState tessellation;
	Shader stage;
};

constexpr ColorComponent operator|(const ColorComponent lhs, const ColorComponent rhs) {
	return static_cast<ColorComponent>(toUnderlying(lhs) | toUnderlying(rhs));
}

constexpr ColorComponent operator&(const ColorComponent lhs, const ColorComponent rhs) {
	return static_cast<ColorComponent>(toUnderlying(lhs) & toUnderlying(rhs));
}

class GraphicsPipeline {
public:
	GraphicsPipeline() = default;

	explicit GraphicsPipeline(PipelineRenderingInfo& renderingInfo);

	GraphicsPipeline(const GraphicsPipeline& other) = delete;

	GraphicsPipeline& operator=(const GraphicsPipeline& other) = delete;

	GraphicsPipeline(GraphicsPipeline&& other) noexcept
		: mState(std::move(other.mState)) {
	}

	GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept {
		if (this == &other)
			return *this;
		mState = std::move(other.mState);
		return *this;
	}

	[[nodiscard]]
	PipelineState& state();

	static GraphicsPipeline createFullscreenQuadPipeline(Shader& shader, const std::vector<SamplerInfo>& samplers);

private:
	PipelineState mState{};
};
