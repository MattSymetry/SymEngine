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
        updateWorldSpace(m_parentTransform);
    }

    void setWorldPosition(const glm::vec3& pos) {
        glm::mat4 tmp = *m_parentTransform;
        setPosition(pos - glm::vec3(tmp[2][0], tmp[2][1], tmp[2][2]));
	}

    void setWorldRotation(const glm::vec3& rot) {
        glm::mat4 tmp = *m_parentTransform;
		setRotation(rot - glm::vec3(tmp[1][0], tmp[1][1], tmp[1][2]));
	}

    void setRotation(const glm::vec3& rot) {
        m_transform.rotation = rot;
        updateWorldSpace(m_parentTransform);
    }

    void setScale(const glm::vec3& scl) {
        m_transform.scale = scl;
        updateWorldSpace(m_parentTransform);
    }

    void move(const glm::vec3& pos) {
		m_transform.position += pos;
        updateWorldSpace(m_parentTransform);
	}

    void rotate(const glm::vec3& rot) {
        m_transform.rotation += rot;
        updateWorldSpace(m_parentTransform);
    }

    void scale(const glm::vec3& scl) {
		m_transform.scale += scl;
        updateWorldSpace(m_parentTransform);
	}

    void updateWorldSpace(glm::mat4* parentTransform) {
        if (m_parentTransform != parentTransform) {
            glm::mat4 diff = *m_parentTransform - *parentTransform;
            m_transform.position += glm::vec3(diff[2][0], diff[2][1], diff[2][2]);
            m_transform.rotation += glm::vec3(diff[1][0], diff[1][1], diff[1][2]);
        	m_parentTransform = parentTransform;
        }
        m_worldTransform = *m_parentTransform;
        // Set rotation
        m_worldTransform[1][0] += m_transform.rotation.x;
        m_worldTransform[1][1] += m_transform.rotation.y;
        m_worldTransform[1][2] += m_transform.rotation.z;
        // Set translation
        m_worldTransform[2][0] += m_transform.position.x;
        m_worldTransform[2][1] += m_transform.position.y;
        m_worldTransform[2][2] += m_transform.position.z;
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
        updateWorldSpace(m_parentTransform);
		return m_worldTransform;
	}

    glm::mat4* getTransform() {
		return &m_worldTransform;
	}

    glm::mat4* getParentTransform() {
        updateWorldSpace(m_parentTransform);
		return m_parentTransform;
	}
};
