#ifndef GRAPHICS_H
#define GRAPHICS_H

// Define a simple 3D vector class
struct Vec3 {
    float x, y, z;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 normalize() const;
    Vec3 operator+(const Vec3& other) const;
};

std::vector<Vec3> createIcosahedronVertices();

std::vector<unsigned int> createIcosahedronFaces();

// Function to subdivide a triangle
void subdivide(std::vector<Vec3>& vertices, const Vec3& v1, const Vec3& v2, const Vec3& v3, int depth);

std::vector<Vec3> createIcosphere(int subdivisions);

GLuint createVBO(const std::vector<Vec3>& vertices);

GLuint createVAO(GLuint vbo);

GLuint createEBO(const std::vector<unsigned int>& indices);

GLuint createNormalsVBO(const std::vector<glm::vec3>& normals);

void bindNormalsToVAO(GLuint vaoID, GLuint normalsVBO, GLuint normalAttributeIndex);
GLuint createShader(GLenum type, const GLchar* source);

GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader);
#endif // GRAPHICS_H