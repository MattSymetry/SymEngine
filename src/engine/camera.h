#pragma once
#include "../common/config.h"

class Camera {
private: 
	glm::vec3 m_position;
	glm::vec3 m_target;
	glm::vec3 m_roll;
	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
	float m_fov;
	float m_yaw;
	float m_pitch;
	float m_rollAngle;
	float m_speed;
	float m_sensitivity;
public:
	Camera();
	Camera(glm::vec3 position, glm::vec3 target, glm::vec3 roll, float fov);
	~Camera();
	void Update();
	void Reset();
	void MoveForward(float deltaTime);
	void MoveBackward(float deltaTime);
	void MoveLeft(float deltaTime);
	void MoveRight(float deltaTime);
	void MoveUp(float deltaTime);
	void MoveDown(float deltaTime);
	void RotateLeft(float deltaTime);
	void RotateRight(float deltaTime);
	void RotateUp(float deltaTime);
	void RotateDown(float deltaTime);
	void LookAt(glm::vec3 target);
	glm::vec3 getPosition();
	glm::vec3 getTarget();
	glm::vec3 getRoll();
	glm::vec3 getForward();
	glm::vec3 getUp();
	glm::vec3 getRight();
	};
