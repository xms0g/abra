#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"

Camera::Camera(const glm::vec3& position, const glm::vec3& up, const float yaw, const float pitch) : mFront(glm::vec3(
		0.0f, 0.0f, -1.0f)),
	mMovementSpeed(SPEED),
	mMouseSensitivity(SENSITIVITY),
	mZoom(ZOOM) {
	mPosition = position;
	mWorldUp = up;
	mYaw = yaw;
	mPitch = pitch;

	update();
}

Camera::Camera(const float posX, const float posY, const float posZ, const float upX, const float upY, const float upZ, const float yaw,
               const float pitch) : mFront(glm::vec3(0.0f, 0.0f, -1.0f)),
                              mMovementSpeed(SPEED),
                              mMouseSensitivity(SENSITIVITY),
                              mZoom(ZOOM) {
	mPosition = glm::vec3(posX, posY, posZ);
	mWorldUp = glm::vec3(upX, upY, upZ);
	mYaw = yaw;
	mPitch = pitch;

	update();
}

glm::mat4 Camera::viewMatrix() const {
	return glm::lookAt(mPosition, mPosition + mFront, mUp);
}

float Camera::zoom() const {
	return mZoom;
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

void Camera::update() {
	// calculate the new Front vector
	glm::vec3 _front;
	_front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	_front.y = sin(glm::radians(mPitch));
	_front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	mFront = glm::normalize(_front);
	// also re-calculate the Right and Up vector
	// normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	mRight = glm::normalize(glm::cross(mFront, mWorldUp));
	mUp = glm::normalize(glm::cross(mRight, mFront));

	generateFrustum();
}

void Camera::processKeyboard(const CameraMovement direction, const float deltaTime) {
	const float velocity = mMovementSpeed * deltaTime;
	if (direction == FORWARD)
		mPosition += mFront * velocity;
	if (direction == BACKWARD)
		mPosition -= mFront * velocity;
	if (direction == LEFT)
		mPosition -= mRight * velocity;
	if (direction == RIGHT)
		mPosition += mRight * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset) {
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
