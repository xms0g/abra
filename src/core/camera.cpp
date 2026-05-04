#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "../event/eventBus.hpp"
#include "../event/events/keyPressedEvent.hpp"
#include "../event/events/mouseMovementEvent.hpp"

Camera::Camera(
	const glm::vec3& position,
	const glm::vec3& up,
	const float yaw,
	const float pitch)
	: mPosition(position),
	  mFront(0.0f, 0.0f, -1.0f),
	  mWorldUp(up),
	  mYaw(yaw),
	  mPitch(pitch),
	  mMovementSpeed(SPEED),
	  mMouseSensitivity(SENSITIVITY) {
}

Camera::Camera(
	const float posX, const float posY, const float posZ,
	const float upX, const float upY, const float upZ,
	const float yaw, const float pitch)
	: mPosition(posX, posY, posZ),
	  mFront(0.0f, 0.0f, -1.0f),
	  mWorldUp(upX, upY, upZ),
	  mYaw(yaw),
	  mPitch(pitch),
	  mMovementSpeed(SPEED),
	  mMouseSensitivity(SENSITIVITY) {
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

const math::Frustum& Camera::frustum() const {
	return mFrustum;
}

void Camera::configure(EventBus& eventBus) {
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

	generateFrustum();
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

void Camera::generateFrustum() {
	const glm::vec3 frontMultFar = ZFAR * mFront;

	mFrustum.nearFace = {mPosition + ZNEAR * mFront, mFront};
	mFrustum.farFace = {mPosition + frontMultFar, -mFront};
	mFrustum.rightFace = {mPosition, glm::cross(frontMultFar - mRight * halfHSide, mUp)};
	mFrustum.leftFace = {mPosition, glm::cross(mUp, frontMultFar + mRight * halfHSide)};
	mFrustum.topFace = {mPosition, glm::cross(mRight, frontMultFar - mUp * halfVSide)};
	mFrustum.bottomFace = {mPosition, glm::cross(frontMultFar + mUp * halfVSide, mRight)};
}
