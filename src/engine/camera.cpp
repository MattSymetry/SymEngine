#include "camera.h"

Camera::Camera() {
	m_position = glm::vec3(0.0f, 2.0f, -5.0f);
	m_target = glm::vec3(0.0f, 0.0f, 0.0f);
	m_roll = glm::vec3(0.0f, 0.0f, 0.0f);
	m_fov = 0.0f;
	m_yaw = 0.0f;
	m_pitch = 0.0f;
	m_rollAngle = 0.0f;
	m_speed = 1.0f;
	m_sensitivity = 100.0f;
}

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 roll, float fov) {
	m_position = position;
	m_target = target;
	m_roll = roll;
	m_fov = fov;
	m_yaw = 0.0f;
	m_pitch = 0.0f;
	m_rollAngle = 0.0f;
	m_speed = 1.0f;
	m_sensitivity = 100.0f;
}

Camera::~Camera() {
	
}

void Camera::Update() {
	m_pitch = std::fmax(-89.0f, std::fmin(89.0f, m_pitch));
	// calculate the new forward vector
	m_forward.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_forward.y = sin(glm::radians(m_pitch));
	m_forward.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_forward = glm::normalize(m_forward);

	// also re-calculate the right and up vector
	m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	m_up = glm::normalize(glm::cross(m_right, m_forward));

}

void Camera::Reset() {
	m_position = glm::vec3(0.0f, 2.0f, -5.0f);
	m_target = glm::vec3(0.0f, 0.0f, 0.0f);
	m_roll = glm::vec3(0.0f, 0.0f, 0.0f);
	m_fov = 0.0f;
	m_yaw = 0.0f;
	m_pitch = 0.0f;
	m_rollAngle = 0.0f;
	m_speed = 0.1f;
	m_sensitivity = 0.1f;
}

void Camera::MoveForward(float deltaTime) {
	// print 'out'
	m_position += m_forward * m_speed * deltaTime;
}

void Camera::MoveBackward(float deltaTime) {
	m_position -= m_forward * m_speed * deltaTime;;
}

void Camera::MoveLeft(float deltaTime) {
	m_position -= glm::normalize(glm::cross(m_forward, m_up)) * m_speed * deltaTime;;
}

void Camera::MoveRight(float deltaTime) {
	m_position += glm::normalize(glm::cross(m_forward, m_up)) * m_speed * deltaTime;;
}

void Camera::MoveUp(float deltaTime) {
	m_position += m_up * m_speed * deltaTime;;
}

void Camera::MoveDown(float deltaTime) {
	m_position -= m_up * m_speed * deltaTime;;
}

void Camera::RotateLeft(float deltaTime) {
	// Rotate around target
	m_yaw -= m_sensitivity * deltaTime;
}

void Camera::RotateRight(float deltaTime) {
	m_yaw += m_sensitivity * deltaTime;
	//_target.x = m_position.x + cos(glm::radians(_yaw));
	//_target.z = m_position.z + sin(glm::radians(_yaw));
}

void Camera::RotateUp(float deltaTime) {
	m_pitch -= m_sensitivity * deltaTime;
	//_target.y = m_position.y + sin(glm::radians(_pitch));
}

void Camera::RotateDown(float deltaTime) {
	m_pitch += m_sensitivity * deltaTime;
	//_target.y = m_position.y + sin(glm::radians(_pitch));
}

glm::vec3 Camera::getPosition() {
	return m_position;
}

glm::vec3 Camera::getTarget() {
	return m_target;
}

glm::vec3 Camera::getRoll() {
	return m_roll;
}

void Camera::LookAt(glm::vec3 target) {
	m_target = target;
}

glm::vec3 Camera::getForward() {
	return m_forward;
}

glm::vec3 Camera::getUp() {
	return m_up;
}

glm::vec3 Camera::getRight() {
	return m_right;
}

