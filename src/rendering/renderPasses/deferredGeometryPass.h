#pragma once
#include <memory>
#include "IRenderPass.hpp"

class Shader;
class FrameBuffer;

class DeferredGeometryPass final : public IRenderPass {
public:
	~DeferredGeometryPass() override;

	const FrameBuffer* getGBuffer() const { return mGBuffer.get(); }

	const Shader& getShader() const { return *mShader; }

	void configure(const RenderContext& ctx) override;

	void execute(const RenderContext& ctx) override;

private:
	std::unique_ptr<FrameBuffer> mGBuffer;
	std::unique_ptr<Shader> mShader;
};
