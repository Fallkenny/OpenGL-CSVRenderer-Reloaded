//edit
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include "shader_m.h"
#include "camera.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
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

std::vector<float>  read_csv(std::string filename)
{
    std::vector<float> resultVector;
    std::ifstream myFile(filename);
    if(!myFile.is_open()) throw std::runtime_error("Could not open file");

    std::string line, colname;
    float val;

    if(myFile.good())
    {
        std::stringstream ss(line);
    }

    while(std::getline(myFile, line))
    {
        std::stringstream ss(line);

        int colIdx = 0;

        while(ss >> val){

            resultVector.push_back(val);

            if(ss.peek() == ';') ss.ignore();

            colIdx++;
        }
    }

    myFile.close();

    return resultVector;
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Modelo de Iluminação Phong", NULL, NULL);
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
    if(glewInit()!=GLEW_OK) {
        std::cout << "Ocorreu um erro iniciando GLEW!" << std::endl;
    } else {
        std::cout << "GLEW OK!" << std::endl;
        std::cout << glGetString(GL_VERSION) << std::endl;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("phong_lighting.vs", "phong_lighting.fs");
    // Iluminação pelo modelo de Gourad
    //Shader lightingShader("gourad_lighting.vs", "gourad_lighting.fs");
    // Phong no espaço de visualização ao invés de no espaço mundial
    //Shader lightingShader("model_lighting.vs", "model_lighting.fs");

    Shader lightCubeShader("light_cube.vs", "light_cube.fs");
    float factor = 1.0f;
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float verticesCube[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,//factor, factor, // top right
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,//factor, 0.0f, // bottom right
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,//0.0f, 0.0f, // bottom left
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,//0.0f, factor,  // top left
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,//factor, factor, // top right
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,//factor, 0.0f, // bottom right

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,//0.0f, 0.0f, // bottom left
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,//0.0f, factor,  // top left
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,//factor, factor, // top right
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,//factor, 0.0f, // bottom right
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,//0.0f, 0.0f, // bottom left
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,//0.0f, factor,  // top left

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,//factor, factor, // top right
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,//factor, 0.0f, // bottom right
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,//0.0f, 0.0f, // bottom left
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,//0.0f, factor,  // top left
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,//factor, factor, // top right
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,//factor, 0.0f, // bottom right

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,//0.0f, 0.0f, // bottom left
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,//0.0f, factor,  // top left
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,//factor, factor, // top right
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,//factor, 0.0f, // bottom right
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,//0.0f, 0.0f, // bottom left
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,//0.0f, factor,  // top left

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,//factor, factor, // top right
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,//factor, 0.0f, // bottom right
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,//0.0f, 0.0f, // bottom left
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,//0.0f, factor,  // top left
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,//factor, factor, // top right
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,//factor, 0.0f, // bottom right

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,//0.0f, 0.0f, // bottom left
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,//0.0f, factor,  // top left
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,//factor, factor, // top right
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,//factor, 0.0f, // bottom right
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,//0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f//,0.0f, factor  // top left
    };

    std::vector<float> vertexes =  read_csv("test comentado.csv");

    int vectorSize = vertexes.size();
    float* vertices = new float[vectorSize];
    for (size_t i = 0; i < vectorSize; i++) {
        vertices[i] = vertexes[i];
    }
    std::cout << vectorSize/8 << std::endl;

    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)* vectorSize, vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //objColor attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // unsigned int indices[] = {
    //     0, 1, 3, // first triangle
    //     1, 2, 3  // second triangle
    // };
    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO, lightCubeVBO, EBO;
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &lightCubeVBO);
    // glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCube), verticesCube, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(lightCubeVAO);

    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
   // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    // glEnableVertexAttribArray(2);






    // // load and create a texture
    // // -------------------------
    // unsigned int texture;
    // glGenTextures(1, &texture);
    // glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // // set the texture wrapping parameters
    // // Podem ser GL_REPEAT. GL_MIRRORED_REPEAT, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_EDGE

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // // set texture filtering parameters
    // // Podem ser GL_LINEAR, GL_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // // load image, create texture and generate mipmaps
    // int width, height, nrChannels;

    // // Imagens são carregadas de baixo para cima. Precisam ser invertidas
    // stbi_set_flip_vertically_on_load(1);

    // // Corrige o alinhamento da imagem em imagens cujas dimensões não são potências de dois
    // // NPOT (Not Power-of-Two)
    // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // //unsigned char *data = stbi_load("res/images/gremio.jpg", &width, &height, &nrChannels, 0);
    // unsigned char *data = stbi_load("hereComesTheSun_durudurum.jpg", &width, &height, &nrChannels, 0);
    // if (data)
    // {
    //     // Se a imagem for PNG com transparência
    //     //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //     // Se a imagem for JPG, e portanto sem transparência
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    //     glGenerateMipmap(GL_TEXTURE_2D);
    // }
    // else
    // {
    //     std::cout << "Failed to load texture" << std::endl;
    // }
    // stbi_image_free(data);






    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // change the light's position values over time (can be done anywhere in the render loop actually, but try to do it at least before using the light source positions)
        lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
        lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        //lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("lightPos", lightPos);
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("specularStrength",specularStrength);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        // Demonstra o problema da distorção do vetor normal
        //model = glm::scale(model, glm::vec3(0.5f, 0.2f, 3.0f)); // transformação de escala não linear
        lightingShader.setMat4("model", model);

        // render the cube
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, vectorSize/9);




        // glBindTexture(GL_TEXTURE_2D, texture);





        // also draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        //model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        lightCubeShader.setMat4("model", model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &lightCubeVBO);
    glDeleteBuffers(1, &VBO);

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
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        specularStrength += 0.01f;
        if(specularStrength > 5.0f)
            specularStrength = 5.0f;
        std::cout << "Especular = " << specularStrength << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        specularStrength -= 0.01f;
        if(specularStrength < 0.0f)
            specularStrength = 0.0f;
        std::cout << "Especular = " << specularStrength << std::endl;
    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
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
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

