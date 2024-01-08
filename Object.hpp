#ifndef OBJECT_H
#define OBJECT_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // for glm::vec3
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "graphics.hpp"

class RenderableObject {
public:
    RenderableObject(std::vector<Vec3> v, std::vector<glm::vec3> n);
    ~RenderableObject();

    void initialize(); // Set up VAO, VBO, etc.
    void render(const GLuint& shaderProgram); // Render the object
    void setPosition(const glm::vec3& position);
    void setRotation(const glm::vec3& axis, float angle);
    void setScale(const glm::vec3& scale);

    glm::mat4 getModelMatrix() const;

private:
    GLuint VAO, VBO; // Vertex Array Object, Vertex Buffer Object
    glm::mat4 modelMatrix;
    std::vector<Vec3> vertices; // Vertex data
    std::vector<glm::vec3> normals; // Normal data

    void updateModelMatrix(); // Recalculate the model matrix if transformations change
    // Other private methods and properties as needed
};
RenderableObject::RenderableObject(std::vector<Vec3> v, std::vector<glm::vec3> n) : vertices(v), normals(n), VAO(0), VBO(0) {
    initialize();
}

RenderableObject::~RenderableObject() {
    // Clean up resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void RenderableObject::initialize() {
    // Generate and bind VAO and VBO, upload vertex data, etc.
    GLuint normalAttributeIndex = 1;

    VBO = createVBO(vertices);
    VAO = createVAO(VBO);
    GLuint normalVbo = createNormalsVBO(normals);
    bindNormalsToVAO(VAO, normalVbo, normalAttributeIndex);
}

void RenderableObject::render(const GLuint& shaderProgram) {
    //glUseProgram(shaderProgram);
    
    // Set uniforms like model matrix
    //GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(getModelMatrix()));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
}

void RenderableObject::setPosition(const glm::vec3& position) {
    modelMatrix = glm::translate(glm::mat4(1.0f), position);
    updateModelMatrix();
}

void RenderableObject::setRotation(const glm::vec3& axis, float angle) {
    modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    updateModelMatrix();
}

void RenderableObject::setScale(const glm::vec3& scale) {
    modelMatrix = glm::scale(glm::mat4(1.0f), scale);
    updateModelMatrix();
}

glm::mat4 RenderableObject::getModelMatrix() const {
    return modelMatrix;
}

void RenderableObject::updateModelMatrix() {
    // Recalculate modelMatrix based on position, rotation, and scale
}
#endif // OBJECT_H