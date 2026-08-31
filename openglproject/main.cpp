#include <glad/glad.h>   // MUST come before GLFW
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <vector>
class samay{
private:
    float lastTime = (float)glfwGetTime();
    float delta=0.0f;
public:
    float deltaTime() {
        delta = (float)glfwGetTime() - lastTime;
        lastTime += delta;
        return delta;
    }
};
class shape {
public:
    unsigned int VAO, VBO, EBO;
    float radius;
    int segments;
    float posY=0.0f;
    float posX = 0.0f;
    float centerY;
    float centerX;
    float velocity_Y=0.0f;
    float velocity_X=0.0f;
    float friction = 2.0f;
    float vMax = 0.8f;
    int mass = 10;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    shape(float radius, int segments, float centerX = 0.0f,float centerY = 0.0f) {
        this->segments = segments;
        this->radius = radius;
        this->centerY = centerY;
        this->centerX = centerX;
        vertices.push_back(0.0f); // x
        vertices.push_back(0.0f); // y
        vertices.push_back(0.0f); // z

        for (int i = 1;i <= segments;i++) {
            float angle = 2.0f * 3.14159265f * i / segments;
            float x = radius * cos(angle);  // 0.5f = radius
            float y = radius * sin(angle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.0f);
        }
        for (int i = 1; i <= segments; i++)
        {
            indices.push_back(0);       // center
            indices.push_back(i);       // current edge point
            indices.push_back(i == segments ? 1 : i + 1); // next edge point
        }
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

    }
    void draw(unsigned int shaderProgram)
    {
        int offsetLoc = glGetUniformLocation(shaderProgram, "uOffset");
        glUniform2f(offsetLoc, posX, posY);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    }
    void update(float deltaTime, GLFWwindow* window) {
        //posY += velocity_Y * deltaTime;
        //posX += velocity_X * deltaTime;
        //if ((posY <= radius - 1) && velocity_Y < 0.0f)
        //    velocity_Y = (-1) * velocity_Y;
        //else if ((posY >= 1 - radius) && velocity_Y > 0.0f)
        //    velocity_Y = (-1) * velocity_Y;
        //if ((posX <= radius - 1) && velocity_X < 0.0f)
        //    velocity_X = (-1) * velocity_X;
        //else if ((posX >= 1 - radius) && velocity_X > 0.0f)
        //    velocity_X = (-1) * velocity_X;
        float inputX=0, inputY=0;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            inputY += 8;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            inputY -= 8;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            inputX -= 8;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            inputX += 8;
            addForce(inputX, inputY, deltaTime);
    }
    int dirn(float x) {
        return (x != 0) ? (int)(x / (sqrt(x * x))) : 0;
    }
    void addForce(float x,float y,float dt) {
        float acclnX = (x-dirn(velocity_X)*friction) / mass;
        float acclnY = (y-dirn(velocity_Y)*friction) / mass;
        //x *= (1 - friction);
        //y *= (1 - friction);
        //float acclnX = x / mass;
        //float acclnY = y / mass;
        velocity_X += acclnX * dt;
        velocity_Y += acclnY * dt;

        if (velocity_X > vMax) velocity_X = vMax;
        if (velocity_X < -vMax) velocity_X = -vMax;
        if (velocity_Y > vMax) velocity_Y = vMax;
        if (velocity_Y < -vMax) velocity_Y = -vMax;

        posX += velocity_X * dt;
        posY += velocity_Y * dt;
        checkBorder();
    }
    void checkBorder() {
        if (posX > 1 - radius) {
            posX = 1 - (0.01f +1)*radius;
            velocity_X = 0.0f;
        }
        if (posX < radius -1) {
            posX = radius*(1 + 0.01f) - 1;
            velocity_X = 0.0f;
        }
        if (posY > 1 - radius) {
            posY = 1 - (0.01f + 1)*radius;
            velocity_Y = 0.0f;
        }
        if (posY < radius - 1) {
            posY = radius*(1 + 0.01f) - 1;
            velocity_Y = 0.0f;
        }
    }
};
// --- Shaders (as plain strings for now — files come later) ---
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform vec2 uOffset;
void main()
{
    gl_Position = vec4(aPos.x + uOffset.x, aPos.y + uOffset.y, aPos.z, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(0.2f, 0.6f, 1.0f, 1.0f); // light blue
}
)";

int main(void)
{
    std::vector<shape> objects;
    samay t;
    float dt = t.deltaTime();
    float speed = 0.002f;
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    // Request OpenGL 3.3 Core profile (matches what we generated GLAD for)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 800, "Hello Square", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Load all OpenGL function pointers via GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    objects.push_back(shape(0.07f, 50, 0.0f, 0.0f));

    // --- Compile vertex shader ---
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // --- Compile fragment shader ---
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // --- Link into a shader program ---
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Shaders are compiled into the program now, don't need the pieces anymore
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        for (auto& s : objects)
        {
            s.update(dt,window);
            s.draw(shaderProgram);
        }
        dt = t.deltaTime();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    for (auto& s : objects) {
        glDeleteVertexArrays(1, &s.VAO);
        glDeleteBuffers(1, &s.VBO);
        glDeleteBuffers(1, &s.EBO);
    }
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}