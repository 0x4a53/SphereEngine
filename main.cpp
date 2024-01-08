#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // for glm::vec3
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.hpp"
#include "Object.hpp"
#include "graphics.hpp"

const GLuint WIDTH = 800, HEIGHT = 600;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

const int KEY_COUNT = 1024;
bool keys[KEY_COUNT] = {false}; // Global array

// Define a simple 3D vector class



Vec3 Vec3::normalize() const {
    float length = sqrt(x * x + y * y + z * z);
    return { x / length, y / length, z / length };
}

Vec3 Vec3::operator+(const Vec3& other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
}

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

GLuint createNormalsVBO(const std::vector<glm::vec3>& normals) {
    GLuint vboID;
    glGenBuffers(1, &vboID); // Generate VBO
    glBindBuffer(GL_ARRAY_BUFFER, vboID); // Bind the VBO
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW); // Upload normals data

    // Unbind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vboID; // Return the VBO ID
}

void bindNormalsToVAO(GLuint vaoID, GLuint normalsVBO, GLuint normalAttributeIndex) {
    glBindVertexArray(vaoID); // Bind the VAO

    // Bind the normals VBO
    glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);

    // Enable the vertex attribute array for normals
    glEnableVertexAttribArray(normalAttributeIndex);

    // Specify how the data is structured in the VBO
    glVertexAttribPointer(normalAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0); // Unbind the VAO
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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key >= 0 && key < KEY_COUNT) {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
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

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    std::fill_n(keys, KEY_COUNT, false);

    // Create icosphere vertices
    std::vector<Vec3> icosphereVertices = createIcosphere(5);
    std::vector<glm::vec3> icosphereNormals;

    for (const auto& vertex : icosphereVertices) {
        // Assuming Vec3 has x, y, z components accessible
        glm::vec3 glmVertex(vertex.x, vertex.y, vertex.z);

        glm::vec3 normal = glm::normalize(glmVertex);
        icosphereNormals.push_back(normal);
    }

    RenderableObject Sphere(icosphereVertices, icosphereNormals);

    
    //shaders
    const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal; // Normal vector

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    out vec3 Normal; // Normal to pass to fragment shader
    out vec3 FragPos; // Fragment position

    void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;

        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)glsl";
    const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;

    in vec3 Normal; // Normal vector
    in vec3 FragPos; // Fragment position

    // Light properties
    uniform vec3 lightPos; // Position of the light source
    uniform vec3 viewPos; // Position of the camera
    uniform vec3 lightColor; // Color of the light
    uniform vec3 objectColor; // Color of the object

    void main() {
        // Ambient
        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * lightColor;
    
        // Diffuse 
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Specular
        float specularStrength = 0.7;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor;  

        vec3 result = (ambient + diffuse + specular) * objectColor;
        vec3 visualizedNormal = normalize(Normal) * 0.5 + 0.5;
        FragColor = vec4(result, 1.0);
    }

)glsl";



    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = createShaderProgram(vertexShader, fragmentShader);
    
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");



    // Check for errors
    if (modelLoc == -1 || viewLoc == -1 || projLoc == -1) {
        std::cerr << "Unable to find matrix uniforms in the shader program" << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(Camera::FORWARD, deltaTime);
        if (keys[GLFW_KEY_S])
            camera.ProcessKeyboard(Camera::BACKWARD, deltaTime);
        if (keys[GLFW_KEY_A])
            camera.ProcessKeyboard(Camera::LEFT, deltaTime);
        if (keys[GLFW_KEY_D])
            camera.ProcessKeyboard(Camera::RIGHT, deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Create transformations
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f); // Start with the identity matrix
        glm::vec3 cameraPosition = camera.Position;


        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glUniform3f(lightPosLoc, 3.0f, 0.5f, 0.0f);
        glUniform3f(viewPosLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
        glUniform3f(objectColorLoc, 1.0f, 0.4f, 0.4f);

        // Render the icosphere
        Sphere.render(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

