#pragma once

struct TransformStruct {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

class Transform : public Component {
public:
    TransformStruct m_transform;
    glm::mat4 m_worldTransform;
    glm::mat4* m_parentTransform;

    Transform(const glm::vec3& pos = glm::vec3(0.0f), const glm::vec3& rot = glm::vec3(0.0f), const glm::vec3& scl = glm::vec3(1.0f), glm::mat4* parentTransform = new glm::mat4(0.0f)) {
        m_transform.position = pos;
        m_transform.rotation = rot;
        m_transform.scale = scl;
        m_parentTransform = parentTransform;
        updateWorldSpace(parentTransform);
    }

    void setPosition(const glm::vec3& pos) {
        m_transform.position = pos;
        updateWorldSpace();
    }

    void setWorldPosition(const glm::vec3& pos) {
        glm::mat4 tmp = *m_parentTransform;
        glm::vec3 ppos = glm::vec3(tmp[2][0], tmp[2][1], tmp[2][2]);
        setPosition(inverseRotateChild(ppos, glm::vec3(tmp[1][0], tmp[1][1], tmp[1][2]), pos));
	}

    void setWorldRotation(const glm::vec3& rot) {
        glm::mat4 tmp = *m_parentTransform;
		setRotation(rot - glm::vec3(tmp[1][0], tmp[1][1], tmp[1][2]));
	}

    void setRotation(const glm::vec3& rot) {
        m_transform.rotation = rot;
        updateWorldSpace();
    }

    void setScale(const glm::vec3& scl) {
        m_transform.scale = scl;
        updateWorldSpace();
    }

    void move(const glm::vec3& pos) {
		m_transform.position += pos;
        updateWorldSpace();
	}

    void rotate(const glm::vec3& rot) {
        m_transform.rotation += rot;
        updateWorldSpace();
    }

    void scale(const glm::vec3& scl) {
		m_transform.scale += scl;
        updateWorldSpace();
	}

    void updateWorldSpace(glm::mat4* parentTransform = nullptr) {
        if (parentTransform == nullptr) parentTransform = m_parentTransform;
        if (m_parentTransform != parentTransform) {
            glm::mat4 diff = *m_parentTransform - *parentTransform;
            m_transform.position += glm::vec3(diff[2][0], diff[2][1], diff[2][2]);
            m_transform.rotation += glm::vec3(diff[1][0], diff[1][1], diff[1][2]);
        	m_parentTransform = parentTransform;
        }
        m_worldTransform = *m_parentTransform;
        glm::vec3 rotChildPos = rotateChild(m_worldTransform[2], m_worldTransform[1], m_transform.position);
        // Set rotation
        m_worldTransform[1][0] += m_transform.rotation.x;
        m_worldTransform[1][1] += m_transform.rotation.y;
        m_worldTransform[1][2] += m_transform.rotation.z;
        // Set translation
        //std::cout << "Rotated child position: " << rotChildPos.x << " " << rotChildPos.y << " " << rotChildPos.z << std::endl;
        m_worldTransform[2][0] = rotChildPos.x;
        m_worldTransform[2][1] = rotChildPos.y;
        m_worldTransform[2][2] = rotChildPos.z;
    }

    const glm::vec3 getPosition() const {
        return m_transform.position;
    }

    const glm::vec3 getRotation() const {
        return m_transform.rotation;
    }

    const glm::vec3 getScale() const {
        return m_transform.scale;
    }

    const TransformStruct getStruct() const {
        return m_transform;
    }

    const glm::mat4 getWorldTransform() {
        updateWorldSpace();
		return m_worldTransform;
	}

    glm::mat4* getTransform() {
		return &m_worldTransform;
	}

    glm::mat4* getParentTransform() {
        updateWorldSpace();
		return m_parentTransform;
	}

    glm::vec3 rotateChild(const glm::vec3 parentPos, const glm::vec3 parentRot, const glm::vec3 childLocalPos) {
        glm::vec3 parentWorldRotationRadians = glm::radians(parentRot);
        glm::quat parentWorldRotationQuat = glm::quat(parentWorldRotationRadians);
        glm::vec3 rotatedPosition = parentWorldRotationQuat * childLocalPos;
        glm::vec3 worldPosition = parentPos + rotatedPosition;

        return worldPosition;
    }

    glm::vec3 inverseRotateChild(const glm::vec3 parentPos, const glm::vec3 parentRot, const glm::vec3 childWorldPos) {
        // Convert rotation angles from degrees to radians
        glm::vec3 radianRot = glm::radians(parentRot);

        // Create inverse rotation matrices
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -radianRot.x, glm::vec3(1, 0, 0))
            * glm::rotate(glm::mat4(1.0f), -radianRot.y, glm::vec3(0, 1, 0))
            * glm::rotate(glm::mat4(1.0f), -radianRot.z, glm::vec3(0, 0, 1));

        // Apply inverse translation and rotation
        glm::vec4 childLocalPos = rotation * (glm::vec4(childWorldPos, 1.0f) - glm::vec4(parentPos, 1.0f));

        return glm::vec3(childLocalPos);
    }

};
