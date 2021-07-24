//SDL Libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <stack>

//OpenGL Libraries
#include <GL/glew.h>
#include <GL/gl.h>

//GML libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include "logger.h"

#include "Cube.h"
#include "Sphere.h"
#include "Cylinder.h"
#include "Cone.h"

#include <SDL2/SDL_image.h>

//STD Libs
#include<iostream>
#include<string>
#include<fstream> 
#include<vector>
#include<sstream>
#include<algorithm>
#define SDL_timer_h_

//Draw files
#include<Obj_loader.h>
#include<Draw.h>

using namespace std;

#define WIDTH     800
#define HEIGHT    800 //600
#define FRAMERATE 60
#define TIME_PER_FRAME_MS  (1.0f/FRAMERATE * 1e4)
#define INDICE_TO_PTR(x) ((void*)(x))

//generer une texture a partir d'un chemin de fichier image
int generateTexture(const char* path) {
    //Texture sol:
    SDL_Surface* img = IMG_Load(path);
    //Convert to an RGBA8888 surface
    SDL_Surface* rgbImg = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);
    //Delete the old surface
    SDL_FreeSurface(img);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, rgbImg->w, rgbImg->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)rgbImg->pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

int main(int argc, char* argv[])
{
    ////////////////////////////////////////
    //SDL2 / OpenGL Context initialization : 
    ////////////////////////////////////////

    //Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0)
    {
        ERROR("The initialization of the SDL failed : %s\n", SDL_GetError());
        return 0;
    }


    //Init the IMG component
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        std::cerr << "Could?not?load?SDL2_image?with?PNG?files\n";
        return EXIT_FAILURE;
    }

    //Create a Window
    SDL_Window* window = SDL_CreateWindow("VR Camera",                           //Titre
        SDL_WINDOWPOS_UNDEFINED,               //X Position
        SDL_WINDOWPOS_UNDEFINED,               //Y Position
        WIDTH, HEIGHT,                         //Resolution
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); //Flags (OpenGL + Show)

//Initialize OpenGL Version (version 3.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    //Initialize the OpenGL Context (where OpenGL resources (Graphics card resources) lives)
    SDL_GLContext context = SDL_GL_CreateContext(window);

    //Tells GLEW to initialize the OpenGL function with this version
    glewExperimental = GL_TRUE;
    glewInit();

    GLint64 timer;
    glGetInteger64v(GL_TIMESTAMP, &timer);

    //Start using OpenGL to draw something on screen
    glViewport(0, 0, WIDTH, HEIGHT); //Draw on ALL the screen

    //The OpenGL background color (RGBA, each component between 0.0f and 1.0f)
    glClearColor(0., 0.61, 0.88, 1.0); //Background color

    glEnable(GL_DEPTH_TEST); //Active the depth test

    //From here you can load your OpenGL objects, like VBO, Shaders, etc.

    FILE* vertFile = fopen("Shaders/color.vert", "r");
    FILE* fragFile = fopen("Shaders/color.frag", "r");

    Shader* shader = Shader::loadFromFiles(vertFile, fragFile);

    FILE* vertFile_sun = fopen("Shaders/color_sun.vert", "r");
    FILE* fragFile_sun = fopen("Shaders/color_sun.frag", "r");

    Shader* shader_sun = Shader::loadFromFiles(vertFile_sun, fragFile_sun);

    fclose(vertFile);
    fclose(fragFile);

    if (shader == NULL)
    {
        return EXIT_FAILURE;
    }

    if (shader_sun == NULL)
    {
        return EXIT_FAILURE;
    }
    //Les matrices
   
    glm::mat4 viewMatrix(1.0f);
    glm::mat4 projectionMatrix(1.0f);
    glm::vec4 temp = glm::inverse(projectionMatrix * glm::inverse(viewMatrix)) * glm::vec4(0, 0, -1, 1);
    glm::vec3 cameraPos = glm::vec3(temp) / temp.w;

    //Création du sol:
    Cube cube;
    GLuint VBO = generateVBO(cube);

    //Texture sol:
    GLuint textureSol = generateTexture("texture/texture_herbe.jpg");

    //Position du sol:
    glm::mat4 cubematrix(1.0f);
    cubematrix = glm::scale(cubematrix, glm::vec3(10, 1, 10));

    //ARBRES:
    Cylinder tronc(3);
    GLuint VBOtronc = generateVBO(tronc);

    //Position du tronc 1:
    glm::mat4 troncmatrix1(1.0f);
    troncmatrix1 = glm::translate(troncmatrix1, glm::vec3(4, 2, 4)); //tronc en bas à droite
    troncmatrix1 = glm::scale(troncmatrix1, glm::vec3(1, 4, 1));
    troncmatrix1 = glm::rotate(troncmatrix1, glm::radians(90.f), glm::vec3(1.0, 0.0, 0.0));

    //Position du tronc 2:
    glm::mat4 troncmatrix2(1.0f);
    troncmatrix2 = glm::translate(troncmatrix2, glm::vec3(-4, 2, 4)); //tronc en bas à gauche
    troncmatrix2 = glm::scale(troncmatrix2, glm::vec3(1, 4, 1));
    troncmatrix2 = glm::rotate(troncmatrix2, glm::radians(90.f), glm::vec3(1.0, 0.0, 0.0));

    //Position du tronc 3:
    glm::mat4 troncmatrix3(1.0f);
    troncmatrix3 = glm::translate(troncmatrix3, glm::vec3(-4, 2, -4)); //tronc en haut à gauche
    troncmatrix3 = glm::scale(troncmatrix3, glm::vec3(1, 4, 1));
    troncmatrix3 = glm::rotate(troncmatrix3, glm::radians(90.f), glm::vec3(1.0, 0.0, 0.0));

    //Position du tronc 4:
    glm::mat4 troncmatrix4(1.0f);
    troncmatrix4 = glm::translate(troncmatrix4, glm::vec3(4, 2, -4)); //tronc en haut à droite
    troncmatrix4 = glm::scale(troncmatrix4, glm::vec3(1, 4, 1));
    troncmatrix4 = glm::rotate(troncmatrix4, glm::radians(90.f), glm::vec3(1.0, 0.0, 0.0));

    //Position du tronc 2:
    glm::mat4 troncmatrix5(1.0f);
    troncmatrix5 = glm::translate(troncmatrix5, glm::vec3(0.35, 2, 0.35)); //tronc au milieu (taille sol/2*racine(2))
    troncmatrix5 = glm::scale(troncmatrix5, glm::vec3(1, 4, 1));
    troncmatrix5 = glm::rotate(troncmatrix5, glm::radians(90.f), glm::vec3(1.0, 0.0, 0.0));

    //Texture tronc:
    GLuint textureTronc = generateTexture("texture/texture_tronc.jpg");

    //FEUILLES ARBRES:
    Sphere feuille(10, 10);
    GLuint VBOfeuille = generateVBO(feuille);

    //Position feuilles arbres BD ET BG:
    glm::mat4 feuillematrix_haut_BD(1.0f);
    feuillematrix_haut_BD = glm::translate(feuillematrix_haut_BD, glm::vec3(4, 4, -4)); //feuilles du haut de l'arbre en bas à droite
    feuillematrix_haut_BD = glm::scale(feuillematrix_haut_BD, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_D_BD(1.0f);
    feuillematrix_D_BD = glm::translate(feuillematrix_D_BD, glm::vec3(3.9, 3, -4)); //feuilles à droite de l'arbre en bas à droite
    feuillematrix_D_BD = glm::scale(feuillematrix_D_BD, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_G_BD(1.0f);
    feuillematrix_G_BD = glm::translate(feuillematrix_G_BD, glm::vec3(4.1, 3, -4)); //feuilles à gauche de l'arbre en bas à droite
    feuillematrix_G_BD = glm::scale(feuillematrix_G_BD, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_haut_BG(1.0f);
    feuillematrix_haut_BG = glm::translate(feuillematrix_haut_BG, glm::vec3(-4, 4, -4)); //feuilles du haut de l'arbre en bas à gauche
    feuillematrix_haut_BG = glm::scale(feuillematrix_haut_BG, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_D_BG(1.0f);
    feuillematrix_D_BG = glm::translate(feuillematrix_D_BG, glm::vec3(-3.9, 3, -4)); //feuilles du haut de l'arbre en bas à gauche
    feuillematrix_D_BG = glm::scale(feuillematrix_D_BG, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_G_BG(1.0f);
    feuillematrix_G_BG = glm::translate(feuillematrix_G_BG, glm::vec3(-4.1, 3, -4)); //feuilles du haut de l'arbre en bas à gauche
    feuillematrix_G_BG = glm::scale(feuillematrix_G_BG, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_haut_HD(1.0f);
    feuillematrix_haut_HD = glm::translate(feuillematrix_haut_HD, glm::vec3(4, 4, 4)); //feuilles du haut de l'arbre en haut à droite
    feuillematrix_haut_HD = glm::scale(feuillematrix_haut_HD, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_D_HD(1.0f);
    feuillematrix_D_HD = glm::translate(feuillematrix_D_HD, glm::vec3(3.9, 3, 4)); //feuilles à droite de l'arbre en haut à droite
    feuillematrix_D_HD = glm::scale(feuillematrix_D_HD, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_G_HD(1.0f);
    feuillematrix_G_HD = glm::translate(feuillematrix_G_HD, glm::vec3(4.1, 3, 4)); //feuilles à gauche de l'arbre en haut à droite
    feuillematrix_G_HD = glm::scale(feuillematrix_G_HD, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_haut_HG(1.0f);
    feuillematrix_haut_HG = glm::translate(feuillematrix_haut_HG, glm::vec3(-4, 4, 4)); //feuilles du haut de l'arbre en haut à gauche
    feuillematrix_haut_HG = glm::scale(feuillematrix_haut_HG, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_D_HG(1.0f);
    feuillematrix_D_HG = glm::translate(feuillematrix_D_HG, glm::vec3(-3.9, 3, 4)); //feuilles de droite de l'arbre en haut à gauche
    feuillematrix_D_HG = glm::scale(feuillematrix_D_HG, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_G_HG(1.0f);
    feuillematrix_G_HG = glm::translate(feuillematrix_G_HG, glm::vec3(-4.1, 3, 4)); //feuilles de gauche de l'arbre en haut à gauche
    feuillematrix_G_HG = glm::scale(feuillematrix_G_HG, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_haut_milieu(1.0f);
    feuillematrix_haut_milieu = glm::translate(feuillematrix_haut_milieu, glm::vec3(0.35, 4, 0.35)); //feuilles du haut de l'arbre au milieu
    feuillematrix_haut_milieu = glm::scale(feuillematrix_haut_milieu, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_D_milieu(1.0f);
    feuillematrix_D_milieu = glm::translate(feuillematrix_D_HG, glm::vec3(0.34, 3, 0.35)); //feuilles du haut de l'arbre au milieu
    feuillematrix_D_milieu = glm::scale(feuillematrix_D_HG, glm::vec3(1.6, 1.6, 1.6));

    glm::mat4 feuillematrix_G_milieu(1.0f);
    feuillematrix_G_milieu = glm::translate(feuillematrix_G_milieu, glm::vec3(0.36, 3, 4)); //feuilles du haut de l'arbre au milieu
    feuillematrix_G_milieu = glm::scale(feuillematrix_G_milieu, glm::vec3(1.6, 1.6, 1.6));

    //Texture feuille
    GLuint textureFeuille = generateTexture("texture/texture_feuille.jpg");
 
    viewMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.5, 1.5, 1.5)) * viewMatrix;
    viewMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.5, 1.5, 1.5)) * viewMatrix;
    viewMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.5, 1.5, 1.5)) * viewMatrix;
    viewMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.5, 1.5, 1.5)) * viewMatrix;
    viewMatrix = glm::rotate(viewMatrix, glm::radians(45.0f), glm::vec3(0.0, 0.1, 0.0));
    viewMatrix = glm::rotate(viewMatrix, glm::radians(45.0f), glm::vec3(0.1, 0.0, 0.0));

    //Matrice camera
    glm::mat4 modelMatrix(1.0f);
    glm::mat4 cameraMatrix(1.0f);
    cameraMatrix = glm::rotate(glm::mat4(1.0), glm::radians(45.0f), glm::vec3(0.0, 0.1, 0.0)) * cameraMatrix;
    cameraMatrix = glm::rotate(cameraMatrix, glm::radians(45.0f), glm::vec3(0.0, 0.1, 0.0));
    cameraMatrix = glm::rotate(cameraMatrix, glm::radians(45.0f), glm::vec3(0.1, 0.0, 0.0));

    //Matrice mvp
    glm::mat4 mvp = glm::scale(glm::mat4(1.0), glm::vec3(0.03, 0.03, 0.03));
    mvp = cameraMatrix * mvp;

    mvp = glm::translate(mvp, glm::vec3(0., 3.2, 0.0));

    //Matrice tete
    glm::mat4 matrice_head(1.0f);
    matrice_head = glm::translate(matrice_head, glm::vec3(0.5, 22.5, -1.15));

    //Matrice oeil gauche
    glm::mat4 matrice_eye(1.0f);
    matrice_eye = glm::translate(matrice_eye, glm::vec3(1.0, 1.0, 0.0));
    matrice_eye = glm::rotate(matrice_eye, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
    matrice_eye = glm::rotate(matrice_eye, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
    matrice_eye = glm::scale(matrice_eye, glm::vec3(1.0, 2.0, 1.0));
    matrice_eye = glm::translate(matrice_eye, glm::vec3(0.25, 10.75, -0.6));

    //Matrice oeil droit
    glm::mat4 matrice_eye_d(1.0f);
    matrice_eye_d = glm::translate(matrice_eye, glm::vec3(1.22, 0.0, 0.0));

    ////// Read our .obj file
    std::vector<glm::vec3> vertices2;
    std::vector<glm::vec2> uvs2;
    std::vector<glm::vec3> normals2;
    size_t nb_vertices2;

    std::vector<glm::vec3> vertices1;
    std::vector<glm::vec2> uvs1;
    std::vector<glm::vec3> normals1;
    size_t nb_vertices1;

    std::vector<glm::vec3> vertices3;
    std::vector<glm::vec2> uvs3;
    std::vector<glm::vec3> normals3;
    size_t nb_vertices3;

    std::vector<glm::vec3> vertices4;
    std::vector<glm::vec2> uvs4;
    std::vector<glm::vec3> normals4;
    size_t nb_vertices4;

    //Load images of the character
    bool image1 = loadOBJ("Objets/Naruto_1.obj", vertices1, uvs1, normals1, nb_vertices1);
    GLuint vertex1 = generateObjectVBO(nb_vertices1, vertices1, uvs1, normals1);

    bool image2 = loadOBJ("Objets/Naruto_2.obj", vertices2, uvs2, normals2, nb_vertices2);
    GLuint vertex2 = generateObjectVBO(nb_vertices2, vertices2, uvs2, normals2);

    bool image4 = loadOBJ("Objets/Naruto_3.obj", vertices3, uvs3, normals3, nb_vertices3);
    GLuint vertex3 = generateObjectVBO(nb_vertices3, vertices3, uvs3, normals3);

    bool head = loadOBJ("Objets/Head.obj", vertices4, uvs4, normals4, nb_vertices4);
    GLuint vertex4 = generateObjectVBO(nb_vertices4, vertices4, uvs4, normals4);

    //Creation de ses yeux
    Sphere oeil(32, 32);
    GLuint VBO_oeil = generateVBO(oeil);

    Material sphereMtl = { glm::vec3(1.0, 0.5, 0.0), 0.2 , 0.5, 0.4, 40.0f };
    Light sphereLight;
    sphereLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    sphereLight.position = glm::vec3(10.0f, 10.0f, -10.0f);

    //Texture corps
    GLuint textureBody = generateTexture("Objets/Body.png");

    //Texture tete
    GLuint textureHead = generateTexture("Objets/Head.png");

    //Texture yeux
    GLuint textureEye = generateTexture("Objets/Eye.png");

    //ViewMatrix qui est la matrice de la vue (camera)
    viewMatrix = glm::rotate(glm::mat4(1.0), glm::radians(45.0f), glm::vec3(0.0, 0.5, 0.0)) * viewMatrix;
 
    //Création du soleil
    Sphere soleil(32, 32);
    GLuint VBO_s = generateVBO(soleil);

    Material soleilMtl = { glm::vec3(1.0, 1.0, 0.0), 1,0.5, 0, 40.0f };
    Light soeleilLight;
    soeleilLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    soeleilLight.position = glm::vec3(10.0f, 10.0f, -10.0f);

    //Position du soleil:
    glm::mat4 sunmatrix(1.0f);
    sunmatrix = glm::translate(sunmatrix, glm::vec3(0, 6, 0));
    sunmatrix = glm::scale(sunmatrix, glm::vec3(1, 1, 1));

    //Matrice du soleil
    glm::mat4 mvpsun = projectionMatrix * glm::inverse(viewMatrix) * sunmatrix;
    mvpsun = glm::rotate(mvpsun, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
    mvpsun = glm::scale(mvpsun, glm::vec3(5.0, 5., 5.0));
    mvpsun = glm::translate(mvpsun, glm::vec3(0.4, 0.4, 5.5));

    //Matrice mvp qui va permettre de deplacer Naruto
    mvp = glm::scale(mvp, glm::vec3(3.5, 3.5, 3.5));
    mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0., 25., 0.));
    mvp = glm::rotate(glm::mat4(1.0), glm::radians(45.0f), glm::vec3(0.0, 0.0, 1.1)) * mvp;
    mvp = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.4, 0.0)) * mvp;

    //Ajustement de la matrice view
    viewMatrix = glm::rotate(glm::mat4(1.0), glm::radians(10.0f), glm::vec3(0.0, 0.0, 0.5))* viewMatrix;
    viewMatrix = glm::scale(glm::mat4(1.0),  glm::vec3(0.9, 0.9, 0.9)) * viewMatrix;
    viewMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0., 0.4, 0.)) * viewMatrix;

    int count = 0;
    string positionCourante = "up";
    bool isOpened = true;

    //Main application loop
    while (isOpened)
    {
        //Time in ms telling us when this frame started. Useful for keeping a fix framerate
        uint32_t timeBegin = SDL_GetTicks();

        //Fetch the SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) //recupere l'evenement
        {
            switch (event.type) //analyse le type d'evenement
            {
            //Evenements claviers
            case SDL_KEYDOWN:
                
                switch (event.key.keysym.sym)
                {
                    //Evenement pour bouger le personnage
                case SDLK_LEFT:  //aller a gauche
                    if (positionCourante == "right") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                    }
                    else if (positionCourante == "left") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                    }
                    else if (positionCourante == "up") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                    }
                    mvp = glm::translate(mvp, glm::vec3(0, 0.0, 0.5));
                    count++;
                    positionCourante = "down";
                    break;
                case SDLK_RIGHT:  //aller a droite
                    if (positionCourante == "right") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                    }
                    else if (positionCourante == "left") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));

                    }
                    else if (positionCourante == "down") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                    }
                    mvp = glm::translate(mvp, glm::vec3(0.0, 0.0, 0.5));
                    count++;
                    positionCourante = "up";
                    break;
                case SDLK_UP:  //aller en haut
                    if (positionCourante == "right") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                    }
                    else if (positionCourante == "up") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                    }
                    else if (positionCourante == "down") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                    }
                    mvp = glm::translate(mvp, glm::vec3(0.0, 0.0, 0.5));
                    count++;
                    positionCourante = "left";
                    break;
                case SDLK_DOWN: // //aller en bas
                    if (positionCourante == "left") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                    }
                    else if (positionCourante == "down") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                    }
                    else if (positionCourante == "up") {
                        mvp = glm::rotate(mvp, glm::radians(90.0f), glm::vec3(0.0, 0.1, 0.0));
                    }
                    mvp = glm::translate(mvp, glm::vec3(0.0, 0.0, 0.5));

                    count++;
                    positionCourante = "right";
                    break;
                default:
                    break;
                }
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    isOpened = false;
                    break;
                default:
                    break;
                }
                break;

            default:
                break;
                //We can add more event, like listening for the keyboard or the mouse. See SDL_Event documentation for more details
            }
        }

        //Clear the screen : the d epth buffer and the color buffer
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        //Dessine le soleil:
        Draw(projectionMatrix* glm::inverse(viewMatrix)* mvpsun, shader_sun, VBO_s, soleil, 0, soleilMtl, soeleilLight, modelMatrix, cameraPos);

        //Dessine le sol:
        Draw(projectionMatrix* glm::inverse(viewMatrix)* cubematrix, shader, VBO, cube, textureSol, sphereMtl, sphereLight, modelMatrix, cameraPos);
        
        //Dessine le tronc 1:
        Draw(projectionMatrix* glm::inverse(viewMatrix)* troncmatrix1, shader, VBOtronc, tronc, textureTronc, sphereMtl, sphereLight, modelMatrix, cameraPos);

        //Dessine le tronc 2:
        Draw(projectionMatrix* glm::inverse(viewMatrix)* troncmatrix2, shader, VBOtronc, tronc, textureTronc, sphereMtl, sphereLight, modelMatrix, cameraPos);

        //Dessine le tronc 3:
        Draw(projectionMatrix* glm::inverse(viewMatrix)* troncmatrix3, shader, VBOtronc, tronc, textureTronc, sphereMtl, sphereLight, modelMatrix, cameraPos);

        //Dessine le tronc 4:
        Draw(projectionMatrix* glm::inverse(viewMatrix)* troncmatrix4, shader, VBOtronc, tronc, textureTronc, sphereMtl, sphereLight, modelMatrix, cameraPos);

        //Dessine le tronc 5:
        Draw(projectionMatrix* glm::inverse(viewMatrix)* troncmatrix5, shader, VBOtronc, tronc, textureTronc, sphereMtl, sphereLight, modelMatrix, cameraPos);

        //Dessine les feuilles des differents arbres:
        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_haut_BD, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);

        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_D_BD, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);

        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_G_BD, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);

        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_haut_BG, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);

        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_D_BG, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);

        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_G_BG, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);

        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_haut_HG, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);

        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_D_HG, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);

        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_G_HG, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);

        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_haut_milieu, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);

        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_D_milieu, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);

        Draw(projectionMatrix* glm::inverse(viewMatrix)* feuillematrix_G_HG, shader, VBOfeuille, feuille, textureFeuille, sphereMtl, sphereLight, modelMatrix, cameraPos);


        //Dessine les evenements clés de naruto
        if (count == 0 || count == 4) {
            DrawObject(projectionMatrix* glm::inverse(viewMatrix)* mvp, shader, vertex1, modelMatrix, nb_vertices1, textureBody);
        }
        else if (count == 1) {
            DrawObject(projectionMatrix* glm::inverse(viewMatrix)* mvp, shader, vertex2, modelMatrix, nb_vertices2, textureBody);
        }
        else if (count == 2) {
            DrawObject(projectionMatrix* glm::inverse(viewMatrix)* mvp, shader, vertex1, modelMatrix, nb_vertices1, textureBody);
        }
        else if (count == 3) {
            DrawObject(projectionMatrix* glm::inverse(viewMatrix)* mvp, shader, vertex3, modelMatrix, nb_vertices3, textureBody);
        }
        if (count >= 4) {
            count = 0;
        }

        //Dessine la tete
        DrawObject(projectionMatrix* glm::inverse(viewMatrix)* mvp * matrice_head, shader, vertex4, modelMatrix, nb_vertices4, textureHead);

        //Dessine les yeux
        Draw(projectionMatrix* glm::inverse(viewMatrix)* mvp * matrice_eye, shader, VBO_oeil, oeil, textureEye, sphereMtl, sphereLight, modelMatrix, glm::vec3(0.0, 0.0, -1.0));
        Draw(projectionMatrix* glm::inverse(viewMatrix)* mvp * matrice_eye_d, shader, VBO_oeil, oeil, textureEye, sphereMtl, sphereLight, modelMatrix, glm::vec3(0.0, 0.0, -1.0));

        //Display on screen (swap the buffer on screen and the buffer you are drawing on)
        SDL_GL_SwapWindow(window);

        //Time in ms telling us when this frame ended. Useful for keeping a fix framerate
        uint32_t timeEnd = SDL_GetTicks();

        //We want FRAMERATE FPS
        if (timeEnd - timeBegin < TIME_PER_FRAME_MS)
            SDL_Delay(TIME_PER_FRAME_MS - (timeEnd - timeBegin));
    }
    //Free everything
    if (context != NULL)
        SDL_GL_DeleteContext(context);
    if (window != NULL)
        SDL_DestroyWindow(window);

    //SDL_FreeSurface(rgbImg); //Delete the surface at the end of the program

    return 0;
}
