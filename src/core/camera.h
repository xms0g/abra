#pragma once
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "../math/frustum.h"

struct MouseMovementEvent;
struct KeyPressedEvent;
class EventBus;

enum CameraMovement: uint8_t {
	FORWARD = 26,
	BACKWARD = 22,
	LEFT = 4,
	RIGHT = 7,
};

class Camera {
public:
	explicit Camera(
		const glm::vec3& position = glm::vec3{0.0f, 0.0f, 0.0f},
		const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

	[[nodiscard]]
	glm::mat4 viewMatrix() const;

	[[nodiscard]]
	const glm::vec3& position() const;

	[[nodiscard]]
	const glm::vec3& front() const;

	[[nodiscard]]
	math::Frustum generateFrustum() const;

	void configure(EventBus& eventBus);

	void update();

	void processKeyboard(const KeyPressedEvent& event);

	void processMouseMovement(const MouseMovementEvent& event);

private:
	// camera Attributes
	glm::vec3 mPosition{};
	glm::vec3 mFront;
	glm::vec3 mUp{};
	glm::vec3 mRight{};
	glm::vec3 mWorldUp{};
	// euler Angles
	float mYaw{};
	float mPitch{};
	// camera options
	float mZFar{};
	float mZNear{};
	float mMovementSpeed{};
	float mMouseSensitivity{};

	float mHalfVSide{};
	float mHalfHSide{};
};
