#include "camera.h"

Camera::Camera() {
	Reset();
}

Camera::Camera(glm::vec3 position, glm::vec3 target, float roll, float fov, float aspectRatio) {
	m_position = position;
	m_target = target;
	m_roll = roll;
	m_fov = fov;
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	m_speed = 1.0f;
	m_sensitivity = 1.0f;
	m_scrollSpeed = 20.0f;
	m_aspectRatio = aspectRatio;
	updateViewMatrix();
	updateProjectionMatrix(aspectRatio);
}

Camera::~Camera() {
	
}

void Camera::Reset() {
	m_position = glm::vec3(0.0f, 2.0f, -5.0f);
	m_target = glm::vec3(0.0f, 0.0f, 0.0f);
	m_roll = 0.0f;
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	m_fov = 0.0f;
	m_speed = 1.0f;
	m_sensitivity = 1.0f;
	m_scrollSpeed = 20.0f;
}

glm::vec3 Camera::getPosition() {
	return m_position;
}

void Camera::setPosition(glm::vec3 position) {
	m_position = position;
	updateViewMatrix();
}

glm::vec3 Camera::getTarget() {
	return m_target;
}

float Camera::getRoll() {
	return m_roll;
}

void Camera::LookAt(glm::vec3 target) {
	m_target = target;
	updateViewMatrix();
}

void Camera::Move(glm::vec3 dir, float deltaTime, bool moveTarget) {
	m_position += dir * m_speed * deltaTime;
	if (moveTarget) m_target += dir * m_speed * deltaTime;
	updateViewMatrix();
	updateProjectionMatrix(16.0f / 9.0f);
}

void Camera::Orbit(glm::vec2 dir, float deltaTime) {
	glm::vec3 direction = m_position - m_target;

	// Convert spherical (polar) coordinates to Cartesian coordinates
	float radius = glm::length(direction);
	float theta = atan2(direction.z, direction.x); // Azimuth
	float phi = acos(direction.y / radius); // Inclination

	// Adjust theta and phi with the mouse input
	theta += dir.x * deltaTime * m_sensitivity;
	phi -= dir.y * deltaTime * m_sensitivity;

	// Clamp the phi (inclination) to prevent the camera from flipping over at the poles
	phi = glm::clamp(phi, 0.1f, 3.14f - 0.1f);

	// Convert back to Cartesian coordinates
	direction.x = radius * sin(phi) * cos(theta);
	direction.y = radius * cos(phi);
	direction.z = radius * sin(phi) * sin(theta);

	// Update camera position
	m_position = m_target + direction;
	updateViewMatrix();
	updateProjectionMatrix(16.0f / 9.0f);
}

void Camera::RollLeft(float deltaTime) {
	m_roll += 1.0f * deltaTime * m_sensitivity;
}

void Camera::RollRight(float deltaTime) {
	m_roll -= 1.0f * deltaTime * m_sensitivity;
}

void Camera::setFov(float fov) {
	m_fov = fov;
}

float Camera::getFov() {
	return m_fov;
}

void Camera::setSpeed(float speed) {
	m_speed = speed;
}

float Camera::getSpeed() {
	return m_speed;
}

void Camera::setSensitivity(float sensitivity) {
	m_sensitivity = sensitivity;
}

float Camera::getSensitivity() {
	return m_sensitivity;
}

void Camera::setScrollSpeed(float scrollSpeed) {
	m_scrollSpeed = scrollSpeed;
}

float Camera::getScrollSpeed() {
	return m_scrollSpeed;
}

void Camera::updateViewMatrix() {
	viewMatrix = glm::lookAt(m_position, m_target, m_up);
}

glm::mat4 Camera::getViewMatrix() {
	return viewMatrix;
}

void Camera::updateProjectionMatrix(float aspectRatio) {
	projectionMatrix = glm::perspective(glm::radians(m_fov), aspectRatio, 0.1f, 1000.0f);
}

glm::mat4 Camera::getProjectionMatrix() {
	return projectionMatrix;
} 

void Camera::setAspectRatio(float aspectRatio) {
	m_aspectRatio = aspectRatio;
	updateProjectionMatrix(aspectRatio);
}

float Camera::getAspectRatio() {
	return m_aspectRatio;
}

