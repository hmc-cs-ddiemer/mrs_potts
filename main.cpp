// Example command line input to run teapot with basic c_scale music file:
// ./final_project kernels/bailey.vert kernels/bailey.frag data/volume_ideal_output.csv data/pitches_cScale.csv data/beat_times_cScale.csv

/*
 * Author: Ahhyun (Bailey) Ahn, HMC '17, ahhyunahn@gmail.com
 * Date: December 14th 2017
 * Project: Mrs. Potts, audio visualizer
 */

#include "main.h"
#ifdef WIN32
#define ssize_t SSIZE_T
#endif

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <ctime>

// variables used for zoom and rotation from keyboard input.
float scale = 1.f; 
float delta_scale = 0.01f; 
float rotate = 0.f; 
float delta_rotate = 1.f; 
float rotate_x = 0.f;
float rotate_y = 1.f;
float rotate_z = 0.f;
int default_num = -999;

// variables used for specifics of teapot (color, shininess)
float rgb[3] = {0.2, 0.2, 0.2};
const float new_rgb[3] = {0.2, 0.2, 0.2};
const float default_shine = 128.0;

// variables used for loading audio data csv files.
int max_beat_index;
int current_beat_index = 0;
float pitch_time_unit = 1.0/60.0;
float volume_time_unit = 1.0/10.0;
bool already_loaded_data_file = false;

std::vector<float> beat_times;
std::vector<float> volume_values;
std::vector<float> pitch_values;

// variables used to measure cpu run time
clock_t begin;
int current_pitch_index = 0;
int current_volume_index = 0;
int past_pitch_index = 0;

// variables used for Uniform (transport data to shader)
const std::string BeatTimeDiff = "BeatTimeDiff";
const std::string Pitch = "Pitch";
const std::string PastPitch = "PastPitch";
const std::string Octave = "Octave";
const std::string PitchTimeDiff = "PitchTimeDiff";
const std::string Volume = "Volume";

// shader file names
std::string vertexShader;
std::string fragmentShader;

// input file names
std::string volume_file;
std::string pitch_file;
std::string beat_file;

SimpleShaderProgram *shader;

void KeyCallback(unsigned char key, int x, int y);
void PrintImages();

bool load_music_data(std::string file_name){
    // load csv file data into vectors
    std::ifstream ifs(file_name.c_str());
    float time;
    float value;
    if(ifs.is_open()){
        char type [5];
        if (file_name == volume_file) {
            while (!ifs.eof()) {
                ifs >> time;
                ifs >> value;
                volume_values.push_back(value); 
                already_loaded_data_file = true;
            }
        }
        else if (file_name == beat_file) {
            while (!ifs.eof()) {
                ifs >> time;
                beat_times.push_back(time);
            }
        }
        else if (file_name == pitch_file){
            while (!ifs.eof()) {
                ifs >> value;
                pitch_values.push_back(value);
            }
        }
    }

    ifs.close();
    max_beat_index = sizeof(beat_times) -  1;
    return true;
}

bool load_music_data_file(std::string file_name){
    // load csv files from path given as command line input
    if (already_loaded_data_file) {
        return false;
    }

    beat_times.clear();
    pitch_values.clear();
    volume_values.clear();

    std::string::size_type idx;
    idx = file_name.rfind('.');
    if(idx != std::string::npos)
    {
        std::string extension = file_name.substr(idx + 1);

        // check extension 
        if (extension == "csv"){ 
            return load_music_data(file_name);
        }

        else {
            std::cerr << "ERROR: unable to load music file " 
                << file_name << "  -- unknown extension." << std::endl;
            std::cerr << "Input only csv files" << std::endl;
        }
    }
    return false;
}

float get_time_diff_to_next_beat(
        clock_t end, 
        double elapsed_secs
    ) {
    float current_music_time = beat_times[current_beat_index];
    return std::abs(elapsed_secs - current_music_time);
}

float get_time_diff_to_next_pitch(clock_t end, double elapsed_secs) {
    float remainder = fmod(elapsed_secs, pitch_time_unit);
    past_pitch_index = current_pitch_index;
    current_pitch_index = (int)((elapsed_secs - remainder) / pitch_time_unit);
    return remainder;
}

float get_time_diff_to_next_volume(clock_t end, double elapsed_secs) {
    float remainder = fmod(elapsed_secs, volume_time_unit);
    current_volume_index = (int)((elapsed_secs - remainder) / volume_time_unit);
    return remainder;
}

void DrawWithShader(
        float time_diff_to_next_beat,
        float time_diff_to_next_pitch
    ){
    float pitch = pitch_values[current_pitch_index];
    float octave = 0.0;

    if (pitch >= 11.0) {
        octave = 1.0;
    }

    shader->Bind();
    shader->SetUniform(BeatTimeDiff, time_diff_to_next_beat);
    shader->SetUniform(Pitch, fmod(pitch_values[current_pitch_index], 11.0));
    shader->SetUniform(Octave, octave);
    shader->SetUniform(PastPitch, (fmod(pitch_values[past_pitch_index], 11.0)));
    shader->SetUniform(PitchTimeDiff, time_diff_to_next_pitch);
    shader->SetUniform(Volume, volume_values[current_volume_index]/100.0);

    // color the forthcoming verts - use rgb array!
    glColor3f(rgb[0], rgb[1], rgb[2]);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, default_shine);
    glutSolidTeapot(1.0);
    shader->UnBind();
}

void DisplayCallback(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // eye, center, up!
    glScalef(scale, scale, scale);
    glRotatef(rotate, rotate_x, rotate_y, rotate_z);
    glTranslatef(0.f, 0.f, 0.f);

    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    float time_diff_to_next_beat = get_time_diff_to_next_beat(end, elapsed_secs);
    float time_diff_to_next_pitch = get_time_diff_to_next_pitch(end, elapsed_secs);
    float time_diff_to_next_volume = get_time_diff_to_next_volume(end, elapsed_secs);

    if (time_diff_to_next_beat < 0.05) {
        if (current_beat_index < max_beat_index){
            current_beat_index += 1;
        }
    }
    
    DrawWithShader(time_diff_to_next_beat, time_diff_to_next_pitch);
    glutSwapBuffers();
    glFlush();
}


void ReshapeCallback(int w, int h){
    glViewport(0, 0, w, h);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective(30.0f, (float)w/(float)h, 0.1f, 100000.f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Setup()
{
    shader = new SimpleShaderProgram();
    shader->LoadVertexShader(vertexShader);
    shader->LoadFragmentShader(fragmentShader);
    load_music_data_file(beat_file);
    load_music_data_file(pitch_file);
    load_music_data_file(volume_file);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void KeyCallback(unsigned char key, int x, int y)
{
    // press the "Q" key to quit and close the window
    // press the "X" key to zoom out
    // press the "Z" key to zoom in 
    // press the "A" key to rotate clockwise on y axis
    // press the "S" key to rotate counter clockwise on y axis
    // press the "D" key to rotate clockwise on y axis
    // press the "F" key to rotate counter clockwise on y axis
    // press the "R" key to reset both rotate and zoom
    switch(key) {
        case 'x': // Zoom out (incremental)
            scale -= delta_scale;
            // std::cout << "\nzoomed out " << scale << std::endl;
            break;

        case 'z': // Zoom in (incremental)
            scale += delta_scale;
            // std::cout << "\nzoomed in " << scale << std::endl;
            break;

      case 'a': // rotate clockwise on y axis
            rotate_x = 0.f; // turn on x rotate
            rotate_y = 1.f; // turn off y rotate
            rotate_z = 0.f; // turn on z rotate
            rotate += delta_rotate;
            // std::cout << "\nrotate cw on y" << rotate << std::endl;
            break;

      case 's': // rotate clockwise on y axis
            rotate_x = 0.f; // turn on x rotate
            rotate_y = 1.f; // turn off y rotate
            rotate_z = 0.f; // turn on z rotate
            rotate -= delta_rotate;
            // std::cout << "\nrotate ccw on y" << rotate << std::endl;
            break;

      case 'd': // rotate clockwise on x axis
            rotate_x = 1.f; // turn on x rotate
            rotate_y = 0.f; // turn off y rotate
            rotate_z = 0.f; // turn on z rotate
            rotate += delta_rotate;
            // std::cout << "\nrotate cw on x" << rotate << std::endl;
            break;

      case 'f': // rotate clockwise on x axis
            rotate_x = 1.f; // turn on x rotate
            rotate_y = 0.f; // turn off y rotate
            rotate_z = 0.f; // turn on z rotate
            rotate -= delta_rotate;
            // std::cout << "\nrotate ccw on x" << rotate << std::endl;
            break;

      case 'g': // rotate clockwise on z axis
            rotate_x = 0.f; // turn on x rotate
            rotate_y = 0.f; // turn off y rotate
            rotate_z = 1.f; // turn on z rotate
            rotate -= delta_rotate;
            // std::cout << "\nrotate ccw on x" << rotate << std::endl;
            break;

      case 'r': // reset rotate and zoom
            scale = 1.f; 
            delta_scale = 0.01f; 
            rotate = 0.f; 
            delta_rotate = 1.f; 
            rotate_x = 0.f;
            rotate_y = 1.f;
            // reset color and shine
            std::copy(new_rgb, new_rgb + 5, rgb);
            break;

        case 'q': // Exit
          exit(0);

        default:
            break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv){
    // make clock to count time
    begin = clock();

    // Check for the right number of arguments!
    if(!(argc == 6)){
        printf("\n*******************************"
            "\nusage: ./final_project <vertex shader> <fragment shader> <volume_file> "
            "<pitch_file> <beat_file>\nExample running code will be \n./final_project "
            "kernels/bailey.vert kernels/bailey.frag data/volume.csv data/pitch.csv data/beat.csv"
            "*******************************\n"
        );
        return 0;
    }

    // get the kernel file names from the command line
    vertexShader   = std::string(argv[1]);
    fragmentShader = std::string(argv[2]);
    volume_file = std::string(argv[3]);
    pitch_file = std::string(argv[4]);
    beat_file = std::string(argv[5]);

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(20, 20);
    glutInitWindowSize(640, 480);
    glutCreateWindow("CS155 Amelia & Dan & Bailey's Final Project");

    // Enable lighting
    glEnable(GL_LIGHTING);  // turn on lighting (state variable)
    glEnable(GL_LIGHT0);    // enable the first light (# 0 ) 

    // set up the parameters of light 0:

    // position
    GLfloat light0_pos[]={1.0, 2.0, 3, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);

    // diffuse color
    GLfloat diffuse0[]={0.5, 0.2, 0.1};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);

    // specular color
    GLfloat specular0[]={1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);

    // ambient color
    GLfloat ambient0[]={.6, 0.5, 1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);

    //
    // Initialize GLEW
    //
    #if !defined(__APPLE__) && !defined(__linux__)
    glewInit();
    if(!GLEW_VERSION_2_0) {
        printf("Your graphics card or graphics driver does\n"
               "\tnot support OpenGL 2.0, trying ARB extensions\n");

        if(!GLEW_ARB_vertex_shader || !GLEW_ARB_fragment_shader) {
            printf("ARB extensions don't work either.\n");
            printf("\tYou can try updating your graphics drivers.\n"
                   "\tIf that does not work, you will have to find\n");
            printf("\ta machine with a newer graphics card.\n");
            exit(1);
        }
    }
    #endif

    Setup();

    glutDisplayFunc(DisplayCallback);
    glutReshapeFunc(ReshapeCallback);
    glutIdleFunc(DisplayCallback);
    glutKeyboardFunc(KeyCallback);
    glutMainLoop();
    return 0;
}
