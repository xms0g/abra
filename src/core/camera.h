#pragma once
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "../math/frustum.h"
#include "../config/config.hpp"

enum CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
};

class Camera {
public:
	explicit Camera(
		const glm::vec3& position = glm::vec3{0.0f, 0.0f, 0.0f},
		const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
		float yaw = YAW,
		float pitch = PITCH);

	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	[[nodiscard]]
	glm::mat4 viewMatrix() const;

	[[nodiscard]]
	const glm::vec3& position() const;

	[[nodiscard]]
	const glm::vec3& front() const;

	[[nodiscard]]
	const math::Frustum& frustum() const;

	void update();

	void processKeyboard(CameraMovement direction, float deltaTime);

	void processMouseMovement(float xoffset, float yoffset);

private:
	void generateFrustum();

	math::Frustum mFrustum;
	// camera Attributes
	glm::vec3 mPosition{};
	glm::vec3 mFront;
	glm::vec3 mUp{};
	glm::vec3 mRight{};
	glm::vec3 mWorldUp{};
	// euler Angles
	float mYaw;
	float mPitch;
	// camera options
	float mMovementSpeed;
	float mMouseSensitivity;

	const float halfVSide = ZFAR * tanf(glm::radians(ZOOM) * 0.5f);
	const float halfHSide = halfVSide * (static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT));
};
