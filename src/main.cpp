#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/sphere_camera.h>
#include <learnopengl//model.h>
#include <rg/Error.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
//Camera camera(glm::vec3(0.0f, 10.0f, 10.0f));
SphereCamera camera = SphereCamera(10.0);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//figure position
glm::vec3 robot_position = glm::vec3(0.0f);
float robot_speed = 2.0f;
float robot_rotate = 0;

// pozicija do koje figura sm da ide po x i z
float floor_size = 5.0;

struct PointLight {
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

bool RenderImGuiEnabled = false;
void DrawImGui(glm::vec4& clearColor);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
//    Shader robotShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader robotShader("resources/shaders/robot_shader.vs", "resources/shaders/robot_shader.fs");
    Shader floorShader ("resources/shaders/floor.vs", "resources/shaders/floor.fs");

    //koordinate podloge
    float floorVertices[] = {
            //positions           //Normal Coords //Tex Coords
            5.0f, -0.5f,  5.0f,   0.0, 0.0, 0.1,   1.0f, 0.0f,
            -5.0f, -0.5f,  5.0f,  0.0, 0.0, 0.1,   0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0, 0.0, 0.1,   0.0f, 1.0f,

            5.0f, -0.5f,  5.0f,   0.0, 0.0, 0.1,   1.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0, 0.0, 0.1,   0.0f, 1.0f,
            5.0f, -0.5f, -5.0f,   0.0, 0.0, 0.1,   1.0f, 1.0f
    };

    //VAO i VBO za podlogu
    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glBindVertexArray(floorVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT,GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT,GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    //teksture za podlogu
    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/metalgrill.jpg").c_str());
    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/metalgrill1.jpg").c_str());

//    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/metal_pattern.jpg").c_str());
//    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/metalpattern.png").c_str());


    floorShader.use();
    floorShader.setInt("material.diffuse", 0);
    floorShader.setInt("material.specular", 1);

    // load models
    // -----------
//    Model robotModel(FileSystem::getPath("resources/objects/backpack/backpack.obj"));
    Model robotModel(FileSystem::getPath("resources/objects/robot_figure/flying-robot.obj"));

    robotModel.SetShaderTextureNamePrefix("material.");
    PointLight pointLight;
    pointLight.ambient = glm::vec3(0.4, 0.4, 0.2);
    pointLight.diffuse = glm::vec3(0.6, 0.5, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);
    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;
    pointLight.position = glm::vec3(4.0, 4.0, 4.0);


    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glm::vec4 clearColor = glm::vec4(0.8f, 0.4f, 0.5f, 1.0f);
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
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms


        //floor
        floorShader.use();
        floorShader.setVec3("light.position", glm::vec3(4.0, 4.0, 4.0));
        floorShader.setVec3("light.ambient", glm::vec3(0.4, 0.4, 0.2));
        floorShader.setVec3("light.diffuse", glm::vec3(0.6, 0.5, 0.6));
        floorShader.setVec3("light.specular", glm::vec3(1.0, 1.0, 1.0));
        floorShader.setVec3("viewPos", camera.Position);
        floorShader.setFloat("material.shininess", 64.0f);

        // view/projection transformations
        glm::mat4 projection1 = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view1 = camera.GetViewMatrix();
        floorShader.setMat4("projection", projection1);
        floorShader.setMat4("view", view1);

        // world transformation
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(0.0, 0.88, 0.0));
//        model1 = glm::scale(model1, glm::vec3(2.0));
        floorShader.setMat4("model", model1);
        //difuzna mapa
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        //spekularna mapa
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //Drawing figure
        robotShader.use();

        pointLight.position = glm::vec3(1.0, 2.0, 5.0);
        robotShader.setVec3("pointLight.position", pointLight.position);
        robotShader.setVec3("pointLight.ambient", pointLight.ambient);
        robotShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        robotShader.setVec3("pointLight.specular", pointLight.specular);
        robotShader.setFloat("pointLight.constant", pointLight.constant);
        robotShader.setFloat("pointLight.linear", pointLight.linear);
        robotShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        robotShader.setVec3("viewPosition", camera.Position);
        robotShader.setFloat("material.shininess", 64.0f);

        //pogled?
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        robotShader.setMat4("projection", projection);
        robotShader.setMat4("view", view);

        // pozicioniranje robota
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, robot_position); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.4f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(robot_rotate), glm::vec3(0, 1, 0));
        robotShader.setMat4("model", model);
        robotModel.Draw(robotShader);


        // Draw Imgui
        if (RenderImGuiEnabled) {
            DrawImGui(clearColor);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        robot_position.z -= robot_speed*deltaTime;
        robot_rotate = 180.0f;
        if(robot_position.z < -floor_size)
            robot_position.z = -floor_size;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        robot_position.z += robot_speed*deltaTime;
        robot_rotate = 0.0f;
        if(robot_position.z > floor_size)
            robot_position.z = floor_size;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
        robot_position.x -= robot_speed*deltaTime;
        robot_rotate = -90.0f;
        if(robot_position.x < -floor_size)
            robot_position.x = -floor_size;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        robot_position.x += robot_speed*deltaTime;
        robot_rotate = 90.0f;
        if(robot_position.x > floor_size)
            robot_position.x = floor_size;
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

    // Don't Update camera rotation ImGui is being rendered
    if (!RenderImGuiEnabled)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(glm::vec4& clearColor) {
    static bool show_demo_window = true;
    static bool show_another_window = false;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);



    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clearColor); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        RenderImGuiEnabled = !RenderImGuiEnabled;
        if (RenderImGuiEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}