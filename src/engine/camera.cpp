#include "camera.h"

Camera::Camera() {
	_position = glm::vec3(0.0f, 2.0f, -5.0f);
	_target = glm::vec3(0.0f, 0.0f, 0.0f);
	_roll = glm::vec3(0.0f, 0.0f, 0.0f);
	_fov = 0.0f;
	_yaw = 0.0f;
	_pitch = 0.0f;
	_rollAngle = 0.0f;
	_speed = 1.0f;
	_sensitivity = 100.0f;
}

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 roll, float fov) {
	_position = position;
	_target = target;
	_roll = roll;
	_fov = fov;
	_yaw = 0.0f;
	_pitch = 0.0f;
	_rollAngle = 0.0f;
	_speed = 1.0f;
	_sensitivity = 100.0f;
}

Camera::~Camera() {
	
}

void Camera::Update() {
	_pitch = std::fmax(-89.0f, std::fmin(89.0f, _pitch));
	// calculate the new forward vector
	_forward.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	_forward.y = sin(glm::radians(_pitch));
	_forward.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	_forward = glm::normalize(_forward);

	// also re-calculate the right and up vector
	_right = glm::normalize(glm::cross(_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	_up = glm::normalize(glm::cross(_right, _forward));

}

void Camera::Reset() {
	_position = glm::vec3(0.0f, 2.0f, -5.0f);
	_target = glm::vec3(0.0f, 0.0f, 0.0f);
	_roll = glm::vec3(0.0f, 0.0f, 0.0f);
	_fov = 0.0f;
	_yaw = 0.0f;
	_pitch = 0.0f;
	_rollAngle = 0.0f;
	_speed = 0.1f;
	_sensitivity = 0.1f;
}

void Camera::MoveForward(float deltaTime) {
	// print 'out'
	_position += _forward * _speed * deltaTime;
}

void Camera::MoveBackward(float deltaTime) {
	_position -= _forward * _speed * deltaTime;;
}

void Camera::MoveLeft(float deltaTime) {
	_position -= glm::normalize(glm::cross(_forward, _up)) * _speed * deltaTime;;
}

void Camera::MoveRight(float deltaTime) {
	_position += glm::normalize(glm::cross(_forward, _up)) * _speed * deltaTime;;
}

void Camera::MoveUp(float deltaTime) {
	_position += _up * _speed * deltaTime;;
}

void Camera::MoveDown(float deltaTime) {
	_position -= _up * _speed * deltaTime;;
}

void Camera::RotateLeft(float deltaTime) {
	// Rotate around target
	_yaw -= _sensitivity * deltaTime;
}

void Camera::RotateRight(float deltaTime) {
	_yaw += _sensitivity * deltaTime;
	//_target.x = _position.x + cos(glm::radians(_yaw));
	//_target.z = _position.z + sin(glm::radians(_yaw));
}

void Camera::RotateUp(float deltaTime) {
	_pitch -= _sensitivity * deltaTime;
	//_target.y = _position.y + sin(glm::radians(_pitch));
}

void Camera::RotateDown(float deltaTime) {
	_pitch += _sensitivity * deltaTime;
	//_target.y = _position.y + sin(glm::radians(_pitch));
}

glm::vec3 Camera::getPosition() {
	return _position;
}

glm::vec3 Camera::getTarget() {
	return _target;
}

glm::vec3 Camera::getRoll() {
	return _roll;
}

void Camera::LookAt(glm::vec3 target) {
	_target = target;
}

glm::vec3 Camera::getForward() {
	return _forward;
}

glm::vec3 Camera::getUp() {
	return _up;
}

glm::vec3 Camera::getRight() {
	return _right;
}

