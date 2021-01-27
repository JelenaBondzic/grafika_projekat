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
void new_treat();
unsigned int loadCubeMap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
//Camera camera(glm::vec3(0.0f, 10.0f, 10.0f));
SphereCamera camera = SphereCamera(14.0);
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

int points = 0;

//
bool is_speed_treat= true;
float scale_treat = 1.0f;

//pozicija baterije
glm::vec3 battery_position = glm::vec3(0.5f, 0.3f, 1.0);

// pozicija do koje figura i baterija smeju da idu po x i z
float floor_size = 5.0;

float ambient_light = 0.5;

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
   // stbi_set_flip_vertically_on_load(true);

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
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    // build and compile shaders
    // -------------------------
//    Shader robotShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader robotShader("resources/shaders/robot_shader.vs", "resources/shaders/robot_shader.fs");
    Shader floorShader ("resources/shaders/floor.vs", "resources/shaders/floor.fs");
    Shader cubeShader("resources/shaders/cubeShader.vs", "resources/shaders/cubeShader.fs");
    Shader skyBoxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");

    //koordinate podloge
    float floorVertices[] = {
            //positions           //Normal Coords //Tex Coords
            5.0f, -0.5f,  5.0f,   0.0, 1.0, 0.0,   1.0f, 0.0f,
            -5.0f, -0.5f,  5.0f,  0.0, 1.0, 0.0,   0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0, 1.0, 0.0,   0.0f, 1.0f,

            5.0f, -0.5f,  5.0f,   0.0, 1.0, 0.0,   1.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0, 1.0, 0.0,   0.0f, 1.0f,
            5.0f, -0.5f, -5.0f,   0.0, 1.0, 0.0,   1.0f, 1.0f
    };

    float cubeVertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    // za stencil testing kocku
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glStencilMask(0x00);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof (cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

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

    // skybox VAO I VBO

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);



    //teksture za podlogu
    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/metalgrill.jpg").c_str());
    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/metalgrill1.jpg").c_str());

//    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/metal_pattern.jpg").c_str());
//    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/metalpattern.png").c_str());

    //teksture za skybox
    vector<std::string> faces
            {

                    FileSystem::getPath("resources/textures/skybox/right.jpg"),
                    FileSystem::getPath("resources/textures/skybox/left.jpg"),
                    FileSystem::getPath("resources/textures/skybox/top.jpg"),
                    FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front.jpg"),
                    FileSystem::getPath("resources/textures/skybox/back.jpg")
            };
    unsigned int cubemapTexture = loadCubeMap(faces);

    floorShader.use();
    floorShader.setInt("material.diffuse", 0);
    floorShader.setInt("material.specular", 1);

    skyBoxShader.use();
    skyBoxShader.setInt("skybox", 0);

    // load models
    // -----------
    Model robotModel(FileSystem::getPath("resources/objects/robot_figure/flying-robot.obj"));
    Model batteryModel(FileSystem::getPath("resources/objects/battery2/9v.obj"));

    robotModel.SetShaderTextureNamePrefix("material.");

    PointLight cameraPointLight;
    cameraPointLight.position = camera.getPosition();
    cameraPointLight.ambient = glm::vec3(ambient_light);
    cameraPointLight.diffuse = glm::vec3(0.8);
    cameraPointLight.specular = glm::vec3(2.0);
    cameraPointLight.constant = 1.0f;
    cameraPointLight.linear = 0.01;
    cameraPointLight.quadratic = 0.05;



    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glm::vec4 clearColor = glm::vec4( 0.439216f, 0.858824f,  0.576471f, 1.0f);
    // render loop
    // -----------
    new_treat(); //postavlja prvu bateriju/kocku za ubrzanje
    while (!glfwWindowShouldClose(window))
    {
        cameraPointLight.ambient = glm::vec3(ambient_light);
        cameraPointLight.position = camera.getPosition();

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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0x00);

        //floor
        floorShader.use();
        floorShader.setVec3("viewPos", camera.Position);
        floorShader.setFloat("material.shininess", 64.0f);

        floorShader.setVec3("dirlight.direction", glm::vec3(0.0, 4.0, 0.0));
        floorShader.setVec3("dirlight.ambient", glm::vec3(0.1));
        floorShader.setVec3("dirlight.diffuse", glm::vec3(0.5));
        floorShader.setVec3("dirlight.specular", glm::vec3(1.0));

        floorShader.setVec3("pointlight.position", cameraPointLight.position);
        floorShader.setVec3("pointlight.ambient", cameraPointLight.ambient);
        floorShader.setVec3("pointlight.diffuse", cameraPointLight.diffuse);
        floorShader.setVec3("pointlight.specular", cameraPointLight.specular);
        floorShader.setFloat("pointlight.constant", cameraPointLight.constant);
        floorShader.setFloat("pointlight.linear", cameraPointLight.linear);
        floorShader.setFloat("pointlight.quadratic", cameraPointLight.quadratic);

        floorShader.setVec3("spotlight.position", robot_position + glm::vec3(0.0, 6.0, 0.0));
        floorShader.setVec3("spotlight.direction", glm::vec3(0.0, -1.0, 0.0));
        floorShader.setVec3("spotlight.ambient", glm::vec3(0.5));
        floorShader.setVec3("spotlight.diffuse", glm::vec3(1.0));
        floorShader.setVec3("spotlight.specular", glm::vec3(1.0));
        floorShader.setFloat("spotlight.constant", 1);
        floorShader.setFloat("spotlight.linear", 0.07);
        floorShader.setFloat("spotlight.quadratic", 0.08);
        floorShader.setFloat("spotlight.cutOff", glm::cos(glm::radians(5.0)));
        floorShader.setFloat("spotlight.outerCutOff", glm::cos(glm::radians(10.0)));

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

        //Crtanje robota
        robotShader.use();

        robotShader.setVec3("pointLight.position", cameraPointLight.position);
        robotShader.setVec3("pointLight.ambient", cameraPointLight.ambient);
        robotShader.setVec3("pointLight.diffuse", cameraPointLight.diffuse);
        robotShader.setVec3("pointLight.specular", cameraPointLight.specular);
        robotShader.setFloat("pointLight.constant", cameraPointLight.constant);
        robotShader.setFloat("pointLight.linear", cameraPointLight.linear);
        robotShader.setFloat("pointLight.quadratic", cameraPointLight.quadratic);

        robotShader.setVec3("viewPosition", camera.Position);
        robotShader.setFloat("material.shininess", 64.0f);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        robotShader.setMat4("projection", projection);
        robotShader.setMat4("view", view);

        // pozicioniranje robota
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 model2 = glm::mat4(1.0f);

        model = glm::translate(model, robot_position); // translate it down so it's at the center of the scene
        model2 = glm::translate(model2, glm::vec3(battery_position.x, battery_position.y+sin(glfwGetTime()) / 7, battery_position.z)); // translate it down so it's at the center of the scene

        model = glm::scale(model, glm::vec3(0.4f));	// it's a bit too big for our scene, so scale it down
        model2 = glm::scale(model2, glm::vec3(0.2f));	// it's a bit too big for our scene, so scale it down

        model = glm::rotate(model, glm::radians(robot_rotate), glm::vec3(0, 1, 0));
        robotShader.setMat4("model", model);

        robotModel.Draw(robotShader);

        robotShader.setMat4("model", model2);

        if(!is_speed_treat)
            batteryModel.Draw(robotShader);



        // crtanje skyboxa
        glDepthFunc(GL_LEQUAL);
        skyBoxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // pomeramo translaciju iz view matrice
        skyBoxShader.setMat4("view", view);
        skyBoxShader.setMat4("projection", projection);

        // skybox kocka
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        if(is_speed_treat){
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);

            cubeShader.use();
            model = glm::mat4(1.0);
            model = glm::translate(model,battery_position);
            model = glm::scale(model, glm::vec3(scale_treat));
            scale_treat -= deltaTime/8;
            if(scale_treat<=0)
                new_treat();

            cubeShader.setMat4("view", camera.GetViewMatrix());
            cubeShader.setMat4("model", model);
            cubeShader.setMat4("projection", projection);
            cubeShader.setVec4("color",glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            glDisable(GL_DEPTH_TEST);
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);

            model = glm::scale(model, glm::vec3(1.2f));
            cubeShader.setMat4("model", model);
            cubeShader.setVec4("color",glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            glDrawArrays(GL_TRIANGLES, 0, 36);

            glBindVertexArray(0);
            glStencilMask(0xFF);
            glStencilFunc(GL_ALWAYS, 0, 0xFF);
            glEnable(GL_DEPTH_TEST);
        }


        // Draw Imgui
        if (RenderImGuiEnabled) {
            DrawImGui(clearColor);
        }



        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);


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
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS){
        ambient_light -= 0.1;
        if(ambient_light<0.1)
            ambient_light = 0.1;
    }
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS){
        ambient_light += 0.1;
        if(ambient_light>3)
            ambient_light = 3;
    }


    if(pow(battery_position.x-robot_position.x, 2) + pow(battery_position.z-robot_position.z, 2) < 0.2){
        points ++;
        if(is_speed_treat)
            robot_speed += 0.1;
        new_treat();}
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

void new_treat(){
    srand(glfwGetTime());
    if(rand()%10<3) {
        is_speed_treat = true;
        battery_position.y = 0.8;
        scale_treat =1.0f;
    }
    else {
        is_speed_treat = false;
        battery_position.y = 0.33;
    }

//    scale_treat =1.0f;
//    battery_position.y = 0.8;
//    is_speed_treat = true;

    // bez negativnog dela bi obilazilo samo jedan kvadrant podloge
    int sgn1 = -1;
    int sgn2 = -1;
    if(rand()%2==0)
        sgn1 = 1;
    if(rand()%2==0)
        sgn2 = 1;

    battery_position.x = (float)rand()/RAND_MAX * floor_size * sgn1;
    battery_position.z = (float)rand()/RAND_MAX * floor_size * sgn2;
}


unsigned int loadCubeMap(vector<std::string> faces){

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA , width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;

}