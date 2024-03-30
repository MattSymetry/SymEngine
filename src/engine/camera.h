#pragma once
#include "../common/config.h"

class Camera {
private: 
	glm::vec3 m_position;
	glm::vec3 m_target;
	float m_roll;
	glm::vec3 m_up;
	float m_fov;
	float m_speed;
	float m_sensitivity;
	float m_scrollSpeed;
public:
	Camera();
	Camera(glm::vec3 position, glm::vec3 target, float roll, float fov);
	~Camera();
	void Reset();
	void Move(glm::vec3 dir, float deltaTime);
	void LookAt(glm::vec3 target);
	void Orbit(glm::vec2 dir, float deltaTime);
	void RollLeft(float deltaTime);
	void RollRight(float deltaTime);
	glm::vec3 getPosition();
	glm::vec3 getTarget();
	float getRoll();
	float getFov();
	void setFov(float fov);
	void setSpeed(float speed);
	void setSensitivity(float sensitivity);
	float getSpeed();
	float getSensitivity();
	float getScrollSpeed();
	void setScrollSpeed(float scrollSpeed);
	};
