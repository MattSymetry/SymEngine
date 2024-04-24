#pragma once

struct ShapeDataStruct {
    int type;
    glm::mat4 parameters;
};

struct Sphere {
    float radius = 0.2f;
    Sphere() {
        radius = 0.2f;
    }
};

struct Box {
    glm::vec3 size = glm::vec3(0.4f);
    float cornerRadius = 0.1f;
    Box() {
		size = glm::vec3(0.4f);
		cornerRadius = 0.1f;
	}
};

struct Cone {
    float height = 0.5f;
    float angle = 70.0f;
    Cone() {
        height = 0.5f;
        angle = 70.0f;
    }
};

enum class Type { Sphere = 0, Box = 1, Cone = 2 };
class Shape : public Component {
public:
    ShapeDataStruct m_shapeData;
    Type type;

    union ShapeUnion {
        Sphere sphere;
        Box box;
        Cone cone;

        ShapeUnion() {}
        ~ShapeUnion() {}
    } shape;

    // Constructors for each shape type
    Shape(const Sphere& s) : type(Type::Sphere) {
        m_shapeData.type = 0;
        new (&shape.sphere) Sphere(s);
    }

    Shape(const Box& b) : type(Type::Box) {
        m_shapeData.type = 1;
        new (&shape.box) Box(b);
    }

    Shape(const Cone& c) : type(Type::Cone) {
        m_shapeData.type = 2;
        new (&shape.cone) Cone(c);
    }

    Shape(const Shape& other) : type(other.type) {
        setNewShape(other);
    }

    void setNewShape(const Shape& other) {
        type = other.type;
        switch (type) {
        case Type::Sphere:
            m_shapeData.type = 0;
            new (&shape.sphere) Sphere(other.shape.sphere);
            break;
        case Type::Box:
            m_shapeData.type = 1;
            new (&shape.box) Box(other.shape.box);
            break;
        case Type::Cone:
            m_shapeData.type = 2;
            new (&shape.cone) Cone(other.shape.cone);
            break;
        }
	}

    Shape& operator=(const Shape& other) {
        if (this != &other) {
            this->~Shape();
            type = other.type;
            switch (type) {
                case Type::Sphere:
                    m_shapeData.type = 0;
                    new (&shape.sphere) Sphere(other.shape.sphere);
                    break;
                case Type::Box:
                    m_shapeData.type = 1;
                    new (&shape.box) Box(other.shape.box);
                    break;
                case Type::Cone:
                    m_shapeData.type = 2;
                    new (&shape.cone) Cone(other.shape.cone);
                    break;
            }
        }
        return *this;
    }

    const ShapeDataStruct getStruct() const {
		return m_shapeData;
	}

    Type getType() {
		return type;
	}

    void setType(Type t) {
        type = t;
        m_shapeData.type = static_cast<int>(t);
        if (t == Type::Sphere) {
			new (&shape.sphere) Sphere();
        }
        else if (t == Type::Box) {
			new (&shape.box) Box();
        }
        else if (t == Type::Cone) {
			new (&shape.cone) Cone();
		}
    }

    glm::mat4 getParams() {
        switch (type) {
			case Type::Sphere:
				m_shapeData.parameters = glm::mat4(0.0f);
				m_shapeData.parameters[0][0] = shape.sphere.radius;
				break;
			case Type::Box:
				m_shapeData.parameters = glm::mat4(0.0f);
				m_shapeData.parameters[0][0] = shape.box.size.x;
				m_shapeData.parameters[0][1] = shape.box.size.y;
				m_shapeData.parameters[0][2] = shape.box.size.z;
				m_shapeData.parameters[0][3] = shape.box.cornerRadius;
				break;
			case Type::Cone:
				m_shapeData.parameters = glm::mat4(0.0f);
				m_shapeData.parameters[0][0] = shape.cone.height;
				m_shapeData.parameters[0][1] = shape.cone.angle;
				break;
		}
        m_shapeData.parameters[3][3] = static_cast<float>(type);
		return m_shapeData.parameters;
	}

    ~Shape() {
        switch (type) {
            case Type::Sphere:
                shape.sphere.~Sphere();
                break;
            case Type::Box:
                shape.box.~Box();
                break;
            case Type::Cone:
                shape.cone.~Cone();
                break;
        }
    }
};
