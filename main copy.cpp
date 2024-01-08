#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Define a simple 3D vector class
struct Vec3 {
    float x, y, z;

    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 normalize() const {
        float length = sqrt(x * x + y * y + z * z);
        return { x / length, y / length, z / length };
    }

    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
};

std::vector<Vec3> createIcosahedronVertices() {
    const float t = (1.0f + sqrt(5.0f)) / 2.0f; // Golden ratio

    std::vector<Vec3> vertices = {
        {-1,  t,  0}, { 1,  t,  0}, {-1, -t,  0}, { 1, -t,  0},
        { 0, -1,  t}, { 0,  1,  t}, { 0, -1, -t}, { 0,  1, -t},
        { t,  0, -1}, { t,  0,  1}, {-t,  0, -1}, {-t,  0,  1}
    };

    // Normalize each vertex to place it on the sphere
    for (auto& v : vertices) {
        v = v.normalize();
    }

    return vertices;
}

std::vector<unsigned int> createIcosahedronFaces() {
    return {
        // 5 faces around point 0
        0, 11, 5,  0, 5, 1,  0, 1, 7,  0, 7, 10,  0, 10, 11,
        // Adjacent faces 
        1, 5, 9,  5, 11, 4,  11, 10, 2,  10, 7, 6,  7, 1, 8,
        // 5 faces around 3
        3, 9, 4,  3, 4, 2,  3, 2, 6,  3, 6, 8,  3, 8, 9,
        // Adjacent faces
        4, 9, 5,  2, 4, 11,  6, 2, 10,  8, 6, 7,  9, 8, 1
    };
}

// Function to subdivide a triangle
void subdivide(std::vector<Vec3>& vertices, const Vec3& v1, const Vec3& v2, const Vec3& v3, int depth) {
    if (depth == 0) {
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
        return;
    }

    Vec3 mid1 = (v1 + v2).normalize();
    Vec3 mid2 = (v2 + v3).normalize();
    Vec3 mid3 = (v3 + v1).normalize();

    subdivide(vertices, v1, mid1, mid3, depth - 1);
    subdivide(vertices, v2, mid2, mid1, depth - 1);
    subdivide(vertices, v3, mid3, mid2, depth - 1);
    subdivide(vertices, mid1, mid2, mid3, depth - 1);
}

std::vector<Vec3> createIcosphere(int subdivisions) {
    std::vector<Vec3> vertices = createIcosahedronVertices();
    std::vector<unsigned int> faces = createIcosahedronFaces();

    std::vector<Vec3> subdividedVertices;

    // Subdivide each face
    for (int i = 0; i < faces.size(); i += 3) {
        subdivide(subdividedVertices, vertices[faces[i]], vertices[faces[i + 1]], vertices[faces[i + 2]], subdivisions);
    }

    return subdividedVertices;
}

GLuint createVBO(const std::vector<Vec3>& vertices) {
    GLuint vbo;
    glGenBuffers(1, &vbo); // Generate a buffer ID
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind the buffer (VBO)
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), &vertices[0], GL_STATIC_DRAW); // Upload vertex data

    return vbo;
}

GLuint createVAO(GLuint vbo) {
    GLuint vao;
    glGenVertexArrays(1, &vao); // Generate a VAO ID
    glBindVertexArray(vao); // Bind the VAO

    glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind the VBO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0); // Set vertex attributes
    glEnableVertexAttribArray(0); // Enable vertex attribute array

    return vao;
}

GLuint createEBO(const std::vector<unsigned int>& indices) {
    GLuint ebo;
    glGenBuffers(1, &ebo); // Generate buffer ID
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // Bind the buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW); // Upload index data

    return ebo;
}

GLuint createShader(GLenum type, const GLchar* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check for shader compile errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader); // Once linked, we no longer need these
    glDeleteShader(fragmentShader);

    return shaderProgram;
}










class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    float Yaw;
    float Pitch;
    // ... other camera properties and methods ...
};

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.Position += cameraSpeed * camera.Front;
    // ... handle other keys for movement ...
    // ... handle mouse input for looking around ...
}












GLFWwindow* initWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Icosphere", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return nullptr;
    }

    glViewport(0, 0, 800, 600);

    return window;
}


int main() {
    GLFWwindow* window = initWindow();
    if (window == nullptr) {
        return -1;
    }

    // Create icosphere vertices
    std::vector<Vec3> icosphereVertices = createIcosphere(2);

    // Create VBO and VAO
    GLuint vbo = createVBO(icosphereVertices);
    GLuint vao = createVAO(vbo);

    
    //shaders
    const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;   // Vertex position
    layout (location = 1) in vec3 aColor; // Vertex color

    out vec3 ourColor; // Pass color to the fragment shader

    uniform mat4 model;       // Model matrix
    uniform mat4 view;        // View matrix
    uniform mat4 projection;  // Projection matrix

    void main()
    {
        // Apply the model, view, and projection matrices to the vertex position
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        ourColor = aColor; // Pass the color to the fragment shader
    }

)glsl";
    const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;

    void main() {
        FragColor = vec4(1.0, 0.5, 0.2, 1.0); // Orange color
    }
)glsl";

    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = createShaderProgram(vertexShader, fragmentShader);

    glUseProgram(shaderProgram);
    

    glEnable(GL_DEPTH_TEST);
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        processInput(window);

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(front);

        
        glm::mat4 view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        // Render the icosphere
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, icosphereVertices.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

