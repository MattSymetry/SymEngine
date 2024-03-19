#pragma once

struct TransformStruct {
    glm::vec4 position = glm::vec4(0.0f);
    glm::vec4 rotation = glm::vec4(0.0f);
    glm::vec4 scale = glm::vec4(1.0f);
};

class Transform : public Component {
public:
    TransformStruct m_transform;

    Transform(const glm::vec3& pos = glm::vec3(0.0f), const glm::vec3& rot = glm::vec3(0.0f), const glm::vec3& scl = glm::vec3(1.0f)) {
        m_transform.position = glm::vec4(pos,0.0f);
        m_transform.rotation = glm::vec4(rot,0.0f);
        m_transform.scale = glm::vec4(scl,0.0f);
    }

    void setPosition(const glm::vec3& pos) {
        m_transform.position = glm::vec4(pos, 0.0f);
    }

    void setRotation(const glm::vec3& rot) {
        m_transform.rotation = glm::vec4(rot, 0.0f);
    }

    void setScale(const glm::vec3& scl) {
        m_transform.scale = glm::vec4(scl, 0.0f);
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
};
