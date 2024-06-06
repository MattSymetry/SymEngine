#pragma once
#include <map>

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
		size = glm::vec3(0.2f);
		cornerRadius = 0.1f;
	}
};

struct Cone {
    float height = 0.5f;
    float topRadius = 0.0f;
    float bottomRadius = 0.2f;
    Cone() {
        height = 0.5f;
        topRadius = 0.0f;
        bottomRadius = 0.2f;
    }
};

struct Cylinder {
	float height = 0.5f;
	float radius = 0.2f;
    Cylinder() {
		height = 0.5f;
		radius = 0.2f;
	}
};

struct Pyramid {
	float height = 0.5f;
	float base = 0.2f;
    Pyramid() {
		height = 0.5f;
		base = 0.2f;
	}
};

struct Torus {
	float majorRadius = 0.2f;
	float minorRadius = 0.1f;
    Torus() {
		majorRadius = 0.2f;
		minorRadius = 0.1f;
	}
};

enum class Type { Sphere = 0, Box = 1, Cone = 2, Cylinder = 3, Pyramid = 4, Torus = 5};
static const std::map<std::string, Type> NameToTypeMap = {
    {ICON_LC_CIRCLE " Sphere", Type::Sphere},
    {ICON_LC_CUBOID " Box", Type::Box},
    {ICON_LC_CONE " Cone", Type::Cone},
    {ICON_LC_CYLINDER " Cylinder", Type::Cylinder},
    {ICON_LC_PYRAMID " Pyramid", Type::Pyramid},
    {ICON_LC_TORUS " Torus", Type::Torus}
};

struct ShaderShape {
    std::string name;
    std::string code;
    float id = -1.0f;
    ShaderShape(std::string n, std::string c) : name(n), code(c) {
        id = static_cast<float>(std::hash<std::string>{}(name));
    }
    ShaderShape() : name(""), code("") {}

    bool operator==(const ShaderShape& other) const {
		return id == other.id;
	}

    template <class Archive>
    void serialize(Archive& archive) {
		archive(name, code, id);
	}
};

class Shape : public Component {
public:
    ShapeDataStruct m_shapeData;
    Type type;
    std::string shaderName;

    union ShapeUnion {
        Sphere sphere;
        Box box;
        Cone cone;
        Cylinder cylinder;
        Pyramid pyramid;
        Torus torus;

        ShapeUnion() {}
        ~ShapeUnion() {}
    } shape;

    // Constructors for each shape type
    Shape(const Sphere& s) : type(Type::Sphere) {
        shaderName = "sdSphere";
        m_shapeData.type = 0;
        new (&shape.sphere) Sphere(s);
    }

    Shape(const Box& b) : type(Type::Box) {
        shaderName = "sdRoundBox";
        m_shapeData.type = 1;
        new (&shape.box) Box(b);
    }

    Shape(const Cone& c) : type(Type::Cone) {
        shaderName = "sdCone";
        m_shapeData.type = 2;
        new (&shape.cone) Cone(c);
    }

    Shape(const Cylinder& c) : type(Type::Cylinder) {
		shaderName = "sdCylinder";
		m_shapeData.type = 3;
		new (&shape.cylinder) Cylinder(c);
	}

    Shape(const Pyramid& p) : type(Type::Pyramid) {
        shaderName = "sdPyramid";
        m_shapeData.type = 4;
        new (&shape.pyramid) Pyramid(p);
    }

    Shape(const Torus& t) : type(Type::Torus) {
		shaderName = "sdTorus";
		m_shapeData.type = 5;
		new (&shape.torus) Torus(t);
	}

    Shape(const Shape& other) : type(other.type) {
        setNewShape(other);
    }

    void setNewShape(const Shape& other) {
        type = other.type;
        shaderName = other.shaderName;
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
        case Type::Cylinder:
			m_shapeData.type = 3;
			new (&shape.cylinder) Cylinder(other.shape.cylinder);
			break;
        case Type::Pyramid:
			m_shapeData.type = 4;
			new (&shape.pyramid) Pyramid(other.shape.pyramid);
			break;
        case Type::Torus:
            m_shapeData.type = 5;
            new (&shape.torus) Torus(other.shape.torus);
            break;
        }
	}

    Shape& operator=(const Shape& other) {
        if (this != &other) {
            this->~Shape();
            type = other.type;
            shaderName = other.shaderName;
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
                case Type::Cylinder:
                    m_shapeData.type = 3;
					new (&shape.cylinder) Cylinder(other.shape.cylinder);
					break;
                case Type::Pyramid:
                    m_shapeData.type = 4;
                    new (&shape.pyramid) Pyramid(other.shape.pyramid);
                    break;
                case Type::Torus:
                    m_shapeData.type = 5;
					new (&shape.torus) Torus(other.shape.torus);
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
            shaderName = "sdSphere";
			new (&shape.sphere) Sphere();
        }
        else if (t == Type::Box) {
            shaderName = "sdRoundBox";
			new (&shape.box) Box();
        }
        else if (t == Type::Cone) {
            shaderName = "sdCone";
			new (&shape.cone) Cone();
		}
        else if (t == Type::Cylinder) {
            shaderName = "sdCylinder";
            new (&shape.cylinder) Cylinder();
        }
        else if (t == Type::Pyramid) {
			shaderName = "sdPyramid";
			new (&shape.pyramid) Pyramid();
		}
        else if (t == Type::Torus) {
			shaderName = "sdTorus";
			new (&shape.torus) Torus();
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
				m_shapeData.parameters[0][1] = shape.cone.topRadius;
                m_shapeData.parameters[0][2] = shape.cone.bottomRadius;
				break;
        	case Type::Cylinder:
                m_shapeData.parameters = glm::mat4(0.0f);
                m_shapeData.parameters[0][0] = shape.cylinder.height;
                m_shapeData.parameters[0][1] = shape.cylinder.radius;
                break;
            case Type::Pyramid:
				m_shapeData.parameters = glm::mat4(0.0f);
				m_shapeData.parameters[0][0] = shape.pyramid.height;
				m_shapeData.parameters[0][1] = shape.pyramid.base;
				break;
            case Type::Torus:
				m_shapeData.parameters = glm::mat4(0.0f);
				m_shapeData.parameters[0][0] = shape.torus.majorRadius;
				m_shapeData.parameters[0][1] = shape.torus.minorRadius;
				break;
		}
        m_shapeData.parameters[1][3] = static_cast<float>(type);
        m_shapeData.parameters[1][2] = static_cast<float>(std::hash<std::string>{}(shaderName));
		return m_shapeData.parameters;
	}

    void setShapeData(ShapeDataStruct data, std::string sName = "") {
		m_shapeData = data;
        if (sName != "") {
			shaderName = sName;
		}
		type = static_cast<Type>(data.type);
		switch (type) {
			case Type::Sphere:
				shape.sphere.radius = data.parameters[0][0];
				break;
			case Type::Box:
				shape.box.size.x = data.parameters[0][0];
				shape.box.size.y = data.parameters[0][1];
				shape.box.size.z = data.parameters[0][2];
				shape.box.cornerRadius = data.parameters[0][3];
				break;
			case Type::Cone:
				shape.cone.height = data.parameters[0][0];
				shape.cone.topRadius = data.parameters[0][1];
                shape.cone.bottomRadius = data.parameters[0][2];
				break;
            case Type::Cylinder:
				shape.cylinder.height = data.parameters[0][0];
				shape.cylinder.radius = data.parameters[0][1];
				break;
			case Type::Pyramid:
                shape.pyramid.height = data.parameters[0][0];
				shape.pyramid.base = data.parameters[0][1];
				break;
			case Type::Torus:
                shape.torus.majorRadius = data.parameters[0][0];
				shape.torus.minorRadius = data.parameters[0][1];
				break;
		}
	}

    std::string getShaderName() {
        return shaderName;
    }

    void setShaderName(std::string sName) {
		shaderName = sName;
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
            case Type::Cylinder:
                shape.cylinder.~Cylinder();
                break;
            case Type::Pyramid:
				shape.pyramid.~Pyramid();
				break;
			case Type::Torus:
				shape.torus.~Torus();
				break;
        }
    }

    // static function that creates a shape from a Type
    static Shape createShape(Type t) {
		switch (t) {
			case Type::Sphere:
				return Shape(Sphere());
			case Type::Box:
				return Shape(Box());
			case Type::Cone:
				return Shape(Cone());
			case Type::Cylinder:
				return Shape(Cylinder());
            case Type::Pyramid:
                return Shape(Pyramid());
            case Type::Torus:
				return Shape(Torus());
		}
	}


};
