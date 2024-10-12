/**
* Author: Pingchuan Dong
* Assignment: Pong Clone
* Date due: 2024-10-12, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <cstdlib>

enum AppStatus { RUNNING, TERMINATED };

constexpr float WINDOW_SIZE_MULT = 2.0f;

constexpr int WINDOW_WIDTH = 640 * WINDOW_SIZE_MULT,
WINDOW_HEIGHT = 480 * WINDOW_SIZE_MULT;


constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr GLint NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr char PADDLE_SPRITE_FILEPATH[] = "paddle.png",
BALL_SPRITE_FILEPATH[] = "ball.png",
MIDLINE_SPRITE_FILEPATH[] = "mid_line.png";



constexpr glm::vec3 INIT_SCALE_BALL = glm::vec3(0.25f, 0.25f, 0.0f),
INIT_POS_BALL= glm::vec3(0.0f, 1.0f, 0.0f),
INIT_SCALE_PADDLE = glm::vec3(0.2f, 1.325f, 0.0f),
INIT_POS_PADDLE1 = glm::vec3(-4.5f, 0.0f, 0.0f),
INIT_POS_PADDLE2 = glm::vec3(4.5f, 0.0f, 0.0f),
INIT_SCALE_MIDLINE = glm::vec3(0.2f, 10.5f, 0.0f),
INIT_POS_MIDLINE = glm::vec3(0.0f, 0.0f, 0.0f);

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix, g_paddle1_matrix, g_paddle2_matrix, g_projection_matrix, g_ball_matrix, g_midline_matrix;

float g_previous_ticks = 0.0f;

GLuint g_paddle1_texture_id;
GLuint g_paddle2_texture_id;
GLuint g_ball_texture_id;
GLuint g_midline_texture_id;


glm::vec3 g_paddle1_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_paddle1_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_paddle2_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_paddle2_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_movement = glm::vec3(-1.0f, 0.4f, 0.0f);


float ball_speed = 2.8f;
float paddle_speed = 4.0f;
bool singlePlayer = false;
bool gameOver = false;

float distance_1x = 0.0f;
float distance_1y = 0.0f;
float distance_2x = 0.0f;
float distance_2y = 0.0f;


void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("PONG GAME",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);


    if (g_display_window == nullptr) shutdown();

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_paddle1_matrix = glm::mat4(1.0f);
    g_paddle2_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::mat4(1.0f);
    g_midline_matrix = glm::mat4(1.0f);
    g_midline_matrix = glm::translate(g_midline_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
    g_ball_matrix = glm::translate(g_ball_matrix, glm::vec3(1.0f, 1.0f, 0.0f));

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    g_paddle1_texture_id = load_texture(PADDLE_SPRITE_FILEPATH);
    g_paddle2_texture_id = load_texture(PADDLE_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    g_midline_texture_id = load_texture(MIDLINE_SPRITE_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_q: g_app_status = TERMINATED; break;
            case SDLK_t:  singlePlayer = true; break;
            default: break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    g_paddle1_movement = glm::vec3(0.0f);
    g_paddle2_movement = glm::vec3(0.0f);
    if (key_state[SDL_SCANCODE_W]) {
        g_paddle1_movement.y = 1.0f;
    }

    if (key_state[SDL_SCANCODE_S]) {
        g_paddle1_movement.y = -1.0f;
    }
    if (not(singlePlayer)) {
        if (key_state[SDL_SCANCODE_DOWN]) {
            g_paddle2_movement.y = -1.0f;
        }
        if (key_state[SDL_SCANCODE_UP]) {
            g_paddle2_movement.y = 1.0f;
        }
    }
    else {
            // paddle 2 moving up and down
        static bool moving_up = true;

            //currently moving up
        if (moving_up) {
            g_paddle2_movement.y = 1;
            if (g_paddle2_position.y > 3.75f - INIT_SCALE_PADDLE.y / 2) {   //border detection
                moving_up = false;
            }
        }
            //currently going down
        else {
            g_paddle2_movement.y = -1;
            if (g_paddle2_position.y < -3.75f + INIT_SCALE_PADDLE.y / 2) {
                moving_up = true;
            }
        }
    }


}


void update()
{
    // --- DELTA TIME CALCULATIONS --- //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    // --- ACCUMULATOR LOGIC --- //
    g_paddle1_position += g_paddle1_movement * paddle_speed * delta_time;
    g_paddle2_position += g_paddle2_movement * paddle_speed * delta_time;
    g_ball_position += g_ball_movement * ball_speed * delta_time;

    // --- COLLISION LOGIC --- //
    float ball_left = g_ball_position.x + INIT_POS_BALL.x - INIT_SCALE_BALL.x / 2;
    float ball_right = g_ball_position.x + INIT_POS_BALL.x + INIT_SCALE_BALL.x / 2;
    float ball_top = g_ball_position.y + INIT_POS_BALL.y + INIT_SCALE_BALL.y / 2;
    float ball_bot = g_ball_position.y + INIT_POS_BALL.y - INIT_SCALE_BALL.y / 2;

    // Paddle 1 boundaries
    float paddle1_right = INIT_POS_PADDLE1.x + INIT_SCALE_PADDLE.x / 2;
    float paddle1_top = g_paddle1_position.y + INIT_SCALE_PADDLE.y / 2;
    float paddle1_bot = g_paddle1_position.y - INIT_SCALE_PADDLE.y / 2;

    // Paddle 2 boundaries
    float paddle2_left = INIT_POS_PADDLE2.x - INIT_SCALE_PADDLE.x / 2;
    float paddle2_top = g_paddle2_position.y + INIT_SCALE_PADDLE.y / 2;
    float paddle2_bot = g_paddle2_position.y - INIT_SCALE_PADDLE.y / 2;

//ball collision with paddle1
    if (ball_left <= paddle1_right && ball_top >= paddle1_bot && ball_bot <= paddle1_top) {
        float overlap_1x = paddle1_right - ball_left;
        float overlap_1y = fmin(paddle1_top - ball_bot, ball_top - paddle1_bot);

        //if there's an x-axis overlap, reverse the ball's x velocity and adjust its position to avoid clipping
        if (overlap_1x > 0) {
            g_ball_movement.x = -g_ball_movement.x;
            g_ball_position.x += overlap_1x;
        }
    }

    //ball collision with paddle2
    if (ball_right >= paddle2_left && ball_top >= paddle2_bot && ball_bot <= paddle2_top) {
        float overlap_2x = ball_right - paddle2_left;
        float overlap_2y = fmin(paddle2_top - ball_bot, ball_top - paddle2_bot);

        if (overlap_2x > 0) {
            g_ball_movement.x = -g_ball_movement.x;
            g_ball_position.x -= overlap_2x;
        }
    }


    if (g_paddle1_position.y + INIT_SCALE_PADDLE.y / 2 > 3.75) {
        g_paddle1_position.y = 3.75 - (INIT_SCALE_PADDLE.y / 2);
    }
    if (g_paddle1_position.y - INIT_SCALE_PADDLE.y / 2 < -3.75) {
        g_paddle1_position.y = -3.75 + (INIT_SCALE_PADDLE.y / 2);
    }
    if (not singlePlayer) {
        if (g_paddle2_position.y + INIT_SCALE_PADDLE.y / 2 > 3.75) {
            g_paddle2_position.y = 3.75 - (INIT_SCALE_PADDLE.y / 2);
        }
        if (g_paddle2_position.y - INIT_SCALE_PADDLE.y / 2 < -3.75) {
            g_paddle2_position.y = -3.75 + (INIT_SCALE_PADDLE.y / 2);
        }
    }

    if (g_ball_position.y > 2.65 || g_ball_position.y < -4.65f) {
        g_ball_movement.y = -g_ball_movement.y;
    }



    // --- TRANSLATION --- //
    g_paddle1_matrix = glm::mat4(1.0f);
    g_paddle1_matrix = glm::translate(g_paddle1_matrix, INIT_POS_PADDLE1);
    g_paddle1_matrix = glm::translate(g_paddle1_matrix, g_paddle1_position);
    g_paddle2_matrix = glm::mat4(1.0f);
    g_paddle2_matrix = glm::translate(g_paddle2_matrix, INIT_POS_PADDLE2);
    g_paddle2_matrix = glm::translate(g_paddle2_matrix, g_paddle2_position);
    g_ball_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::translate(g_ball_matrix, INIT_POS_BALL);
    g_ball_matrix = glm::translate(g_ball_matrix, g_ball_position);
    // --- SCALING --- //
    g_midline_matrix = glm::mat4(1.0f);
    g_paddle1_matrix = glm::scale(g_paddle1_matrix, INIT_SCALE_PADDLE);
    g_paddle2_matrix = glm::scale(g_paddle2_matrix, INIT_SCALE_PADDLE);
    g_ball_matrix = glm::scale(g_ball_matrix, INIT_SCALE_BALL);
    g_midline_matrix = glm::scale(g_midline_matrix, INIT_SCALE_MIDLINE);


    // --- TERMINATE GAME IF BALL MISSED --- //
    if (fabs(g_ball_position.x)-INIT_SCALE_BALL.x/2 > 5.0f){
        g_app_status = TERMINATED;
    }
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_paddle1_matrix, g_paddle1_texture_id);
    draw_object(g_paddle2_matrix, g_paddle2_texture_id);
    draw_object(g_midline_matrix, g_midline_texture_id);
    draw_object(g_ball_matrix, g_ball_texture_id);
    


    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}