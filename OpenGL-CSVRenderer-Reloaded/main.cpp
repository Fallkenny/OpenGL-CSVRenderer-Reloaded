#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include "lib/shader_m.h"
#include "lib/camera.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 6.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// Reflexo especular
float specularStrength = 0.5;

typedef struct
{
    unsigned int VAO;
    unsigned int VBO;
    unsigned int texture;
    float *vertexes;
    int pointsCount;
    bool loadedTexture;

} RenderableObj;

std::pair<std::string, std::vector<float>> read_csv(std::string filename)
{
    std::vector<float> resultVector;
    std::string texture;
    std::ifstream myFile(filename);
    if (!myFile.is_open())
        throw std::runtime_error("Could not open file");

    std::string line, colname;
    float val;

    if (myFile.good())
    {
        std::stringstream ss(line);
    }
    std::getline(myFile, line);
    std::stringstream ss(line);
    ss >> texture;

    while (std::getline(myFile, line))
    {
        std::stringstream ss(line);

        int colIdx = 0;

        while (ss >> val)
        {
            resultVector.push_back(val);

            if (ss.peek() == ';')
                ss.ignore();

            colIdx++;
        }
    }

    myFile.close();

    return std::make_pair(texture, resultVector);
}

RenderableObj load_renderableObj(std::string file)
{
    RenderableObj obj;

    std::pair<std::string, std::vector<float>> content = read_csv(file);
    std::string textureIMG = content.first;
    std::vector<float> vertexes = content.second;

    int vectorSize = vertexes.size();
    float *vertices = new float[vectorSize];
    for (size_t i = 0; i < vectorSize; i++)
    {
        vertices[i] = vertexes[i];
    }
    std::cout << vectorSize / 11 << std::endl;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int VBO, objVAO;
    glGenVertexArrays(1, &objVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(objVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vectorSize, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    if (textureIMG != "")
    {
        obj.loadedTexture =true;
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        std::string tmp = "textures/" + textureIMG;
        unsigned char *data = stbi_load(tmp.c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        obj.texture = texture;
    }
    else
        obj.loadedTexture = false;

    obj.pointsCount = vectorSize / 11;
    obj.vertexes = vertices;
    obj.VAO = objVAO;
    obj.VBO = VBO;

    return obj;
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Modelo de Iluminação Phong", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glew: load all OpenGL function pointers
    // ---------------------------------------
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Ocorreu um erro iniciando GLEW!" << std::endl;
    }
    else
    {
        std::cout << "GLEW OK!" << std::endl;
        std::cout << glGetString(GL_VERSION) << std::endl;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("shader/phong_lighting.vs", "shader/phong_lighting.fs");
    Shader lightCubeShader("shader/light_cube.vs", "shader/light_cube.fs");

    std::string models[] =
    {
        "chao.csv",
        "paredes.csv",
        "teto.csv",
        "porta.csv",
        "janela_d.csv",
        "janela_e.csv",
        "chamine.csv",
        "caule.csv",
        "copa.csv",
        "cerca.csv"
    };

    int modelscount = sizeof(models) / sizeof(models[0]);
    RenderableObj *objects = new RenderableObj[modelscount];

    for (int i = 0; i < modelscount; i++)
    {
        objects[i] = load_renderableObj("csv/"+models[i]);
    }
    RenderableObj sun = load_renderableObj("csv/sun.csv");

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        lightPos.x = cos(glfwGetTime()) * 2.5f;
        lightPos.y = sin(glfwGetTime()) * 5.0f;
        lightPos.z = cos(glfwGetTime()) * 5.0f;


        lightingShader.use();
        lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("lightPos", lightPos);
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("specularStrength", specularStrength);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);


        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        for (int i = 0; i < modelscount; i++)
        {
            glBindTexture(GL_TEXTURE_2D, objects[i].texture);
            lightingShader.setBool("drawTexture", objects[i].loadedTexture);
            glBindVertexArray(objects[i].VAO);
            glDrawArrays(GL_TRIANGLES, 0, objects[i].pointsCount);
        }

        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        lightCubeShader.setMat4("model", model);

        glBindVertexArray(sun.VAO);
        glDrawArrays(GL_TRIANGLES, 0, sun.pointsCount);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:

    for (int i = 0; i < modelscount; i++)
    {
        glDeleteVertexArrays(1, &objects[i].VAO);
        glDeleteBuffers(1, &objects[i].VBO);
    }
    // ------------------------------------------------------------------------

    glDeleteVertexArrays(1, &sun.VAO);
    glDeleteBuffers(1, &sun.VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        specularStrength += 0.01f;
        if (specularStrength > 5.0f)
            specularStrength = 5.0f;
        std::cout << "Especular = " << specularStrength << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        specularStrength -= 0.01f;
        if (specularStrength < 0.0f)
            specularStrength = 0.0f;
        std::cout << "Especular = " << specularStrength << std::endl;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
