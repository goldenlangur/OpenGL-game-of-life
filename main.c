#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct
{
    GLFWwindow *glfw_window;
    int closed;
} Window;

#define SCR_SIZE 800

#define ALIVE 1
#define DEAD  0
#define GRID_SIZE 128

typedef struct Cell
{
    int state;
    float r, g, b;
}Cell;

int main(void)
{
    GLFWwindow* glfw_window;
    if (!glfwInit())
        printf("[ERROR]: Failed to init GLFW!\n");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfw_window = glfwCreateWindow(SCR_SIZE, SCR_SIZE, "Game of Life", NULL, NULL);

    if (!glfw_window)
    {
        glfwTerminate();
        printf("[ERROR]: Failed to create window!\n");
    }
    glfwMakeContextCurrent(glfw_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("[ERROR]: Failed to init glad!\n");
    }

    glfwSwapInterval(1);

    float vertices[] = 
    {
         0.9f,  0.9f, 0.0f,  1.0f, 1.0f,
         0.9f, -0.9f, 0.0f,  1.0f, 0.0f,
        -0.9f, -0.9f, 0.0f,  0.0f, 0.0f,
        -0.9f,  0.9f, 0.0f,  0.0f, 1.0f
    };

    unsigned int indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };

    const char *v_shader_src = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 v_pos;\n"
    "layout (location = 1) in vec2 v_uv;\n"
    "out vec2 o_uv;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(v_pos, 1.0);\n"
    "   o_uv = vec2(v_uv.x, 1.0 - v_uv.y);\n;"
    "}\0";

    const char *f_shader_src = 
    "#version 330 core\n"
    "in vec2 o_uv;\n"
    "out vec4 f_color;\n"
    "uniform sampler2D tex;\n"
    "void main()\n"
    "{\n"
    "f_color = texture(tex, o_uv);\n"
    //"f_color = vec4(o_uv, 0.0f,  1.0f);\n"
    "}\0";

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    unsigned int v_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v_shader, 1, &v_shader_src, NULL);
    glCompileShader(v_shader);

    int success;
    char infoLog[512];
    glGetShaderiv(v_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(v_shader, 512, NULL, infoLog);
        printf("%s %s\n", "Failed to compile VERTEX shader!:", infoLog);
    }

    unsigned int f_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f_shader, 1, &f_shader_src, NULL);
    glCompileShader(f_shader);

    glGetShaderiv(f_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(f_shader, 512, NULL, infoLog);
        printf("%s %s\n", "Failed to compile FRAGMENT shader!:", infoLog);
    }

    unsigned int shader = glCreateProgram();
    glAttachShader(shader, v_shader);
    glAttachShader(shader, f_shader);
    glLinkProgram(shader);

    glDeleteShader(v_shader);
    glDeleteShader(f_shader);
    glUseProgram(shader);

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,  sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    static Cell cells_state[GRID_SIZE][GRID_SIZE];
    static Cell cells_output[GRID_SIZE][GRID_SIZE];
    
    for (int x = 0; x < GRID_SIZE; x++)
    {
        for (int y = 0; y < GRID_SIZE; y++)
        {
            int random  =  (rand() % 2);
            if(random > 8)
                cells_state[x][y].state = 1;
            else
                cells_state[x][y].state = 0;

            cells_state[x][y].r = 0;
            cells_state[x][y].g = 220;
            cells_state[x][y].b = 0;
            cells_state[x][y].state = random;

            cells_output[x][y] = cells_state[x][y];
        }
    }
    
    unsigned char texture_data[GRID_SIZE * GRID_SIZE * 4];
    for (int i = 0; i < GRID_SIZE * GRID_SIZE * 4; i+=4)
    {
        texture_data[i + 0] = 255 ;
        texture_data[i + 1] = 255 ;
        texture_data[i + 2] = 255 ;
        texture_data[i + 3] = 255;
    }

    unsigned int texture;
    glGenTextures(1, &texture);  
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture); 

    //FPS
    double previous_time = glfwGetTime();
    int frame_count = 0;

    while (!glfwWindowShouldClose(glfw_window))
    {
        double current_time = glfwGetTime();
        frame_count+=1;

        if ( current_time - previous_time >= 1.0 )
        {
            char text[20]; 
            sprintf(text, "%d", frame_count);   
            glfwSetWindowTitle(glfw_window, text);

            frame_count = 0;
            previous_time = current_time;
        }

        glfwSwapBuffers(glfw_window);
        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        for (int x = 0; x < GRID_SIZE; x++)
        {
            for (int y = 0; y < GRID_SIZE; y++)
            {
                int live_neighbours;
                if(x == 0)
                    live_neighbours =     cells_state[GRID_SIZE - 1][y + 1].state + cells_state[x][y + 1].state + cells_state[1][y + 1].state +
                                          cells_state[GRID_SIZE - 1][y + 0].state +                               cells_state[1][y + 0].state +
                                          cells_state[GRID_SIZE - 1][y - 1].state + cells_state[x][y - 1].state + cells_state[1][y - 1].state;
                else if(x == GRID_SIZE - 1)
                    live_neighbours =     cells_state[x - 1][y + 1].state + cells_state[x][y + 1].state + cells_state[0][y + 1].state +
                                          cells_state[x - 1][y + 0].state +                               cells_state[0][y + 0].state +
                                          cells_state[x - 1][y - 1].state + cells_state[x][y - 1].state + cells_state[0][y - 1].state;
                else
                    live_neighbours =     cells_state[x - 1][y + 1].state + cells_state[x][y + 1].state + cells_state[x + 1][y + 1].state +
                                          cells_state[x - 1][y + 0].state +                               cells_state[x + 1][y + 0].state +
                                          cells_state[x - 1][y - 1].state + cells_state[x][y - 1].state + cells_state[x + 1][y - 1].state;
                
                if(cells_state[x][y].state == ALIVE)
                {
                    if(live_neighbours > 3 || live_neighbours < 2)
                        cells_output[x][y].state = DEAD;
                    else
                        cells_output[x][y].state = ALIVE;
                }
                if(cells_state[x][y].state == DEAD && live_neighbours == 3)
                {
                    cells_output[x][y].state = ALIVE;
                }  
            }
        } 
        for (int x = 0; x < GRID_SIZE; x++)
        {
            for (int y = 0; y < GRID_SIZE; y++)
            {
                cells_state[x][y].state = cells_output[x][y].state;
                texture_data[(x * 4) + (y * GRID_SIZE * 4) + 0] = cells_output[x][y].state * cells_output[x][y].r;
                texture_data[(x * 4) + (y * GRID_SIZE * 4) + 1] = cells_output[x][y].state * cells_output[x][y].g;
                texture_data[(x * 4) + (y * GRID_SIZE * 4) + 2] = cells_output[x][y].state * cells_output[x][y].b;
                texture_data[(x * 4) + (y * GRID_SIZE * 4) + 3] = cells_output[x][y].state *  255;
            }
        } 
        
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GRID_SIZE, GRID_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
        glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(unsigned int), GL_UNSIGNED_INT, 0);
    }
    
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteProgram(shader);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(glfw_window);
    glfwTerminate();
}
