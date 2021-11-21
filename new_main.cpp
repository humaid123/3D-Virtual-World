

int main() {
    GLFWwindow* window;
    glewExperimental = true; // for core profile

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

    // Open a window and create its OpenGL context

    int window_width = 1200, window_height = 1200; std::string window_name = "Terrain";
    window = glfwCreateWindow(window_width, window_height, window_name, NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Initialize GLEW

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    Terrain terrain(hybridfbm, blinnPhong, width, heigth, n_width, n_height, {
	"snow.png", "grass.png", ...
    });
    Snow snow(snowVert, snowFrag, camera.position, num_particles);
    Skybox skybox(skyboxVert, skyboxFrag, {
        "right", "left", "top", "bottom", "front", "back"
    });


    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    do {
        // Clear the screen. 
        glClear(GL_COLOR_BUFFER_BIT);
        
	camera.inputs(window); // 
	camera.updateMatrix();

        terrain.draw(camera); // need to look for translation
	snow.draw(camera);    // needed for offset
	skybox.draw(camera); 

        // FBO stuff for water
        water.draw();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0); // Check if the ESC key was pressed or the window was closed

   
    return EXIT_SUCCESS;

}