#pragma once
#include "../common/config.h"

class Camera {
private: 
	glm::vec3 _position;
	glm::vec3 _target;
	glm::vec3 _roll;
	glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 _right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 _forward = glm::vec3(0.0f, 0.0f, -1.0f);
	float _fov;
	float _yaw;
	float _pitch;
	float _rollAngle;
	float _speed;
	float _sensitivity;
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
