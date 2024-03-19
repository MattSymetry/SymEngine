#pragma once

struct Sphere {
    float radius;
};

struct Box {
    float height;
    float width;
    float depth;
    float cornerRadius;
};

struct Cone {
    float height;
    glm::vec2 angle;
};

class Shape : public Component {
public:
    enum class Type { Sphere, Box, Cone } type;

    union ShapeUnion {
        Sphere sphere;
        Box box;
        Cone cone;

        ShapeUnion() {}
        ~ShapeUnion() {}
    } shape;

    // Constructors for each shape type
    Shape(const Sphere& s) : type(Type::Sphere) {
        new (&shape.sphere) Sphere(s);
    }

    Shape(const Box& b) : type(Type::Box) {
        new (&shape.box) Box(b);
    }

    Shape(const Cone& c) : type(Type::Cone) {
        new (&shape.cone) Cone(c);
    }

    Shape(const Shape& other) : type(other.type) {
        switch (type) {
            case Type::Sphere:
                new (&shape.sphere) Sphere(other.shape.sphere);
                break;
            case Type::Box:
                new (&shape.box) Box(other.shape.box);
                break;
            case Type::Cone:
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
                    new (&shape.sphere) Sphere(other.shape.sphere);
                    break;
                case Type::Box:
                    new (&shape.box) Box(other.shape.box);
                    break;
                case Type::Cone:
                    new (&shape.cone) Cone(other.shape.cone);
                    break;
            }
        }
        return *this;
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
