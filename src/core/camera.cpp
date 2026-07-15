#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "../config/configManager.h"
#include "../event/eventBus.hpp"
#include "../event/events/keyPressedEvent.hpp"
#include "../event/events/mouseMovementEvent.hpp"

Camera::Camera(
	const glm::vec3& position,
	const glm::vec3& up)
	: mPosition(position),
	  mFront(0.0f, 0.0f, -1.0f),
	  mWorldUp(up) {
}

glm::mat4 Camera::viewMatrix() const {
	return glm::lookAt(mPosition, mPosition + mFront, mUp);
}

const glm::vec3& Camera::position() const {
	return mPosition;
}

const glm::vec3& Camera::front() const {
	return mFront;
}

float Camera::zfar() const {
	return mZFar;
}

float Camera::znear() const {
	return mZNear;
}

float Camera::zoom() const {
	return mZoom;
}

math::Frustum Camera::generateFrustum() const {
	math::Frustum frustum;

	const glm::vec3 frontMultFar = mZFar * mFront;
	frustum.nearFace = {mPosition + mZNear * mFront, mFront};
	frustum.farFace = {mPosition + frontMultFar, -mFront};
	frustum.rightFace = {mPosition, glm::cross(frontMultFar - mRight * mHalfHSide, mUp)};
	frustum.leftFace = {mPosition, glm::cross(mUp, frontMultFar + mRight * mHalfHSide)};
	frustum.topFace = {mPosition, glm::cross(mRight, frontMultFar - mUp * mHalfVSide)};
	frustum.bottomFace = {mPosition, glm::cross(frontMultFar + mUp * mHalfVSide, mRight)};

	return frustum;
}

void Camera::configure(EventBus& eventBus) {
	mYaw = CONFIG_MANAGER_INSTANCE.get<float>("camera.yaw");
	mPitch = CONFIG_MANAGER_INSTANCE.get<float>("camera.pitch");
	mMovementSpeed = CONFIG_MANAGER_INSTANCE.get<float>("camera.speed");
	mMouseSensitivity = CONFIG_MANAGER_INSTANCE.get<float>("camera.sensitivity");
	mZNear = CONFIG_MANAGER_INSTANCE.get<float>("camera.znear");
	mZFar = CONFIG_MANAGER_INSTANCE.get<float>("camera.zfar");
	mZoom = CONFIG_MANAGER_INSTANCE.get<float>("camera.zoom");
	mHalfVSide = mZFar * tanf(glm::radians(mZoom) * 0.5f);

	const float width = static_cast<float>(CONFIG_MANAGER_INSTANCE.get<int32_t>("window.width"));
	const float height = static_cast<float>(CONFIG_MANAGER_INSTANCE.get<int32_t>("window.height"));
	mHalfHSide = mHalfVSide * (width / height);

	update();

	eventBus.subscribeToEvent<Camera, KeyPressedEvent>(this, &Camera::processKeyboard);
	eventBus.subscribeToEvent<Camera, MouseMovementEvent>(this, &Camera::processMouseMovement);
}

void Camera::update() {
	// calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	front.y = sin(glm::radians(mPitch));
	front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	mFront = glm::normalize(front);
	// also re-calculate the Right and Up vector
	// normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	mRight = glm::normalize(glm::cross(mFront, mWorldUp));
	mUp = glm::normalize(glm::cross(mRight, mFront));
}

void Camera::processKeyboard(const KeyPressedEvent& event) {
	const float velocity = mMovementSpeed * event.deltaTime;
	if (event.direction == FORWARD)
		mPosition += mFront * velocity;
	if (event.direction == BACKWARD)
		mPosition -= mFront * velocity;
	if (event.direction == LEFT)
		mPosition -= mRight * velocity;
	if (event.direction == RIGHT)
		mPosition += mRight * velocity;
}

void Camera::processMouseMovement(const MouseMovementEvent& event) {
	float xoffset = event.x;
	float yoffset = event.y;

	xoffset *= mMouseSensitivity;
	yoffset *= mMouseSensitivity;

	mYaw += xoffset;
	mPitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (mPitch > 89.0f)
		mPitch = 89.0f;
	if (mPitch < -89.0f)
		mPitch = -89.0f;
}
