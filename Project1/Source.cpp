#include <cstdlib>
#include <GL/glut.h>
#include <soil2.h>
#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <tuple>

// Add these at the top with other global variables
float bodyBob = 0.0f;
float playerRotation = 0.0f;  // For facing direction
float targetRotation = 0.0f;  // For smooth rotation
bool isWalking = false;
bool isAttackAnimating = false;

//animation timers
int attack_animation_timer = 0;
int smoke_animation_timer = 0;
int ground_animation_timer = 0;
int frameDelayCounter = 1; //frame delay for smoke animation
int anime_time = 0;
int attackAnimationTimer = 0;

//structure for restricted areas
struct RestrictedArea {
    float xMin, xMax;
    float zMin, zMax;
};

// Struct for player and enemy entities
struct Entity {
    float x, y, z;
    float health;
    bool isAlive;
    bool isRanged;
};

// Global variables for player
float playerX = 0.0f, playerY = 0.5f, playerZ = 0.0f;
float playerHealth = 100.0f;
bool isAttacking = false;
bool gameOver = false;
float attackRange = 2.0f;
float enemyRespawnTime = 3.0f;
int score = 0;

//restricted areas
std::vector<RestrictedArea> restrictedAreas = {
    {8.0f, 20.0f, -20.0f, -9.0f},// Example: Area between (1,1) and (3,3)
   };


// Enemies and game settings
std::vector<Entity> enemies;
float enemySpeed = 0.02f;
int maxEnemies = 3;
float rangedAttackCooldown = 2.0f;

//save normals
std::tuple<float, float, float> normal;

//function for calculate normals
std::tuple<float, float, float> calculateNormal(
    const std::tuple<float, float, float>& v1,
    const std::tuple<float, float, float>& v2,
    const std::tuple<float, float, float>& v3) {

    float x1 = std::get<0>(v1), y1 = std::get<1>(v1), z1 = std::get<2>(v1);
    float x2 = std::get<0>(v2), y2 = std::get<1>(v2), z2 = std::get<2>(v2);
    float x3 = std::get<0>(v3), y3 = std::get<1>(v3), z3 = std::get<2>(v3);

    float ux = x2 - x1, uy = y2 - y1, uz = z2 - z1;
    float vx = x3 - x1, vy = y3 - y1, vz = z3 - z1;

    float nx = uy * vz - uz * vy;
    float ny = uz * vx - ux * vz;
    float nz = ux * vy - uy * vx;

    float length = std::sqrt(nx * nx + ny * ny + nz * nz);
    if (length != 0) {
        nx /= length;
        ny /= length;
        nz /= length;
    }

    return std::make_tuple(nx, ny, nz);
}

//function to check the player in restricted area
bool isInRestrictedArea(float x, float z) {
    for (const auto& area : restrictedAreas) {
        if (x >= area.xMin && x <= area.xMax && z >= area.zMin && z <= area.zMax) {
            return true;
        }
    }
    return false;
}


//drawGrid
void drawGrid()
{
    GLfloat step = 1.0f;
    GLint line;

    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f); // Translate to the origin

    glColor3f(0.7f, 0.7f, 0.7f); // Light gray color for the grid lines
    glBegin(GL_LINES);
    for (line = -20; line <= 20; line += step)
    {
        glVertex3f(line, 0.0, 20);
        glVertex3f(line, 0.0, -20);

        glVertex3f(20, 0.0, line);
        glVertex3f(-20, 0.0, line);
    }
    glEnd();

    // Draw measurements
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the measurements
    for (line = -20; line <= 20; line += step)
    {
        glRasterPos3f(line, 0.0, 0.0);
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "%d", line);
        for (char* c = buffer; *c != '\0'; c++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
        }

        glRasterPos3f(0.0, 0.0, line);
        snprintf(buffer, sizeof(buffer), "%d", line);
        for (char* c = buffer; *c != '\0'; c++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
        }
    }

    glPopMatrix();
}

//set lighting
void setLightingAndShading() {
    glEnable(GL_LIGHTING);

    GLfloat lightColor[] = { 0.9568f, 0.9137f, 0.6078f, 1.0f };
    //GLfloat lightColor[] = { 1.0f, 0.5f, 0.0f, 0.9f };

    // Light 0
    GLfloat l0_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat l0_diffuse[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat l0_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat l0_position[] = { -15.0f, 15.0f, 10.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, l0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, l0_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, l0_position);

    // //Light 1
    //GLfloat l1_ambient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    //GLfloat l1_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    //GLfloat l1_specular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    //GLfloat l1_position[] = { 0.0f + playerX, 1.0f + playerZ, -0.5f, 1.0f };

    //glLightfv(GL_LIGHT1, GL_AMBIENT, l1_ambient);
    //glLightfv(GL_LIGHT1, GL_DIFFUSE, l1_diffuse);
    //glLightfv(GL_LIGHT1, GL_SPECULAR, l1_specular);
    //glLightfv(GL_LIGHT1, GL_POSITION, l1_position);


    // Material properties
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    GLfloat specRef[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, specRef);
    glMateriali(GL_FRONT, GL_SHININESS, 128);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHT1);
}

//variable for load textures
GLuint texture_smoke[7];
GLuint texture_attack[7];
GLuint texture_ground[5];

// loeading textures
void loadTexture() {

    const char* smoke_file[7] = {
        "textures/smoke/frame0000.png",
        "textures/smoke/frame0001.png",
        "textures/smoke/frame0002.png",
        "textures/smoke/frame0003.png",
        "textures/smoke/frame0004.png",
        "textures/smoke/frame0005.png",
        "textures/smoke/frame0006.png",
    };

    const char* attack_file[7] = {
        "textures/attackVFX/frame0000.png",
        "textures/attackVFX/frame0001.png",
        "textures/attackVFX/frame0002.png",
        "textures/attackVFX/frame0003.png",
        "textures/attackVFX/frame0004.png",
        "textures/attackVFX/frame0005.png",
        "textures/attackVFX/frame0006.png",
    };
    const char* ground_file[7] = {
    "textures/ground/frame0000.png",
    "textures/ground/frame0001.png",
    "textures/ground/frame0002.png",
    "textures/ground/frame0003.png",
    "textures/ground/frame0004.png"
    };


    //smoke texture
    for (int i = 0; i < 7; i++) {
        texture_smoke[i] = SOIL_load_OGL_texture(smoke_file[i], SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, SOIL_FLAG_GL_MIPMAPS | SOIL_FLAG_INVERT_Y);
        if (!texture_smoke[i]) {
            printf("Texture loading failed: %s\n", SOIL_last_result());
        }
    }
    //attack texture
    for (int i = 0; i < 7; i++) {
        texture_attack[i] = SOIL_load_OGL_texture(attack_file[i], SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID, SOIL_FLAG_GL_MIPMAPS | SOIL_FLAG_INVERT_Y);
        if (!texture_attack[i]) {
            printf("Texture loading failed: %s\n", SOIL_last_result());
        }
    }
    //ground texture
    for (int i = 0; i < 5; i++) {
        texture_ground[i] = SOIL_load_OGL_texture(ground_file[i], SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_GL_MIPMAPS);
        if (!texture_ground[i]) {
            printf("Texture loading failed: %s\n", SOIL_last_result());
        }
    }

}

//basic shapes
void truncatedPyramid(float height, float half_top_width, float half_bottom_width) {
    glPushMatrix();
    glBegin(GL_QUADS);
    // Front Face
    normal = calculateNormal(
        { -half_bottom_width, 0.0f, half_bottom_width },
        { half_bottom_width, 0.0f, half_bottom_width },
        { half_top_width, height, half_top_width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_bottom_width, 0.0f, half_bottom_width);
    glVertex3f(half_bottom_width, 0.0f, half_bottom_width);
    glVertex3f(half_top_width, height, half_top_width);
    glVertex3f(-half_top_width, height, half_top_width);

    // Back Face
    normal = calculateNormal(
        { -half_bottom_width, 0.0f, -half_bottom_width },
        { half_bottom_width, 0.0f, -half_bottom_width },
        { half_top_width, height, -half_top_width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_bottom_width, 0.0f, -half_bottom_width);
    glVertex3f(half_bottom_width, 0.0f, -half_bottom_width);
    glVertex3f(half_top_width, height, -half_top_width);
    glVertex3f(-half_top_width, height, -half_top_width);

    // Top Face
    normal = calculateNormal(
        { -half_top_width, height, -half_top_width },
        { -half_top_width, height, half_top_width },
        { half_top_width, height, half_top_width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_top_width, height, -half_top_width);
    glVertex3f(-half_top_width, height, half_top_width);
    glVertex3f(half_top_width, height, half_top_width);
    glVertex3f(half_top_width, height, -half_top_width);

    // Bottom Face
    normal = calculateNormal(
        { -half_bottom_width, 0.0f, -half_bottom_width },
        { half_bottom_width, 0.0f, -half_bottom_width },
        { half_bottom_width, 0.0f, half_bottom_width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_bottom_width, 0.0f, -half_bottom_width);
    glVertex3f(half_bottom_width, 0.0f, -half_bottom_width);
    glVertex3f(half_bottom_width, 0.0f, half_bottom_width);
    glVertex3f(-half_bottom_width, 0.0f, half_bottom_width);

    // Right Face
    normal = calculateNormal(
        { half_bottom_width, 0.0f, -half_bottom_width },
        { half_top_width, height, -half_top_width },
        { half_top_width, height, half_top_width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(half_bottom_width, 0.0f, -half_bottom_width);
    glVertex3f(half_top_width, height, -half_top_width);
    glVertex3f(half_top_width, height, half_top_width);
    glVertex3f(half_bottom_width, 0.0f, half_bottom_width);

    // Left Face
    normal = calculateNormal(
        { -half_bottom_width, 0.0f, -half_bottom_width },
        { -half_bottom_width, 0.0f, half_bottom_width },
        { -half_top_width, height, half_top_width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_bottom_width, 0.0f, -half_bottom_width);
    glVertex3f(-half_bottom_width, 0.0f, half_bottom_width);
    glVertex3f(-half_top_width, height, half_top_width);
    glVertex3f(-half_top_width, height, -half_top_width);

    glEnd();
    glPopMatrix();
}
void truncatedPrism(float height, float thickness, float width_top, float width_bottom) {



    glPushMatrix();
    glBegin(GL_QUADS);


    // Front Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, thickness },
        { width_bottom, 0.0f, thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, thickness);
    glVertex3f(width_bottom, 0.0f, thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(-width_top, height, thickness);

    // Back Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, -thickness },
        { width_bottom, 0.0f, -thickness },
        { width_top, height, -thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, -thickness);
    glVertex3f(width_bottom, 0.0f, -thickness);
    glVertex3f(width_top, height, -thickness);
    glVertex3f(-width_top, height, -thickness);

    // Top Face
    normal = calculateNormal(
        { -width_top, height, -thickness },
        { -width_top, height, thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_top, height, -thickness);
    glVertex3f(-width_top, height, thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(width_top, height, -thickness);

    // Bottom Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, -thickness },
        { width_bottom, 0.0f, -thickness },
        { width_bottom, 0.0f, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, -thickness);
    glVertex3f(width_bottom, 0.0f, -thickness);
    glVertex3f(width_bottom, 0.0f, thickness);
    glVertex3f(-width_bottom, 0.0f, thickness);

    // Right Face
    normal = calculateNormal(
        { width_bottom, 0.0f, -thickness },
        { width_top, height, -thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(width_bottom, 0.0f, -thickness);
    glVertex3f(width_top, height, -thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(width_bottom, 0.0f, thickness);

    // Left Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, -thickness },
        { -width_bottom, 0.0f, thickness },
        { -width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, -thickness);
    glVertex3f(-width_bottom, 0.0f, thickness);
    glVertex3f(-width_top, height, thickness);
    glVertex3f(-width_top, height, -thickness);

    glEnd();

    glPopMatrix();

}
void prism(float width, float height, float thickness) {

    float half_thickness = thickness / 2;


    glPushMatrix();

    glBegin(GL_TRIANGLES);
    normal = calculateNormal(
        { -width, 0, half_thickness },
        { width, 0, half_thickness },
        { 0, height, half_thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width, 0, half_thickness);
    glVertex3f(width, 0, half_thickness);
    glVertex3f(0, height, half_thickness);
    glEnd();

    glBegin(GL_TRIANGLES);
    normal = calculateNormal(
        { -width, 0, -half_thickness },
        { width, 0,-half_thickness },
        { 0, height, -half_thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width, 0, -half_thickness);
    glVertex3f(width, 0, -half_thickness);
    glVertex3f(0, height, -half_thickness);
    glEnd();


    glBegin(GL_QUADS);
    normal = calculateNormal(
        { width, 0.0f, half_thickness },
        { width, 0, -half_thickness },
        { 0, height, -half_thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(width, 0.0f, half_thickness);
    glVertex3f(width, 0, -half_thickness);
    glVertex3f(0, height, -half_thickness);
    glVertex3f(0, height, half_thickness);
    glEnd();

    glBegin(GL_QUADS);
    normal = calculateNormal(
        { -width, 0.0f, -half_thickness },
        { -width, 0, half_thickness },
        { 0, height, half_thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width, 0.0f, -half_thickness);
    glVertex3f(-width, 0, half_thickness);
    glVertex3f(0, height, half_thickness);
    glVertex3f(0, height, -half_thickness);
    glEnd();

    glPopMatrix();

}
void base(float height, float thickness, float width_top, float width_bottom) {

    glPushMatrix();
    glBegin(GL_QUADS);


    // Front Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, thickness },
        { width_bottom, 0.0f, thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, thickness);
    glVertex3f(width_bottom, 0.0f, thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(-width_top, height, thickness);

    // Back Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, -thickness },
        { width_bottom, 0.0f, -thickness },
        { width_top, height, -thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, -thickness);
    glVertex3f(width_bottom, 0.0f, -thickness);
    glVertex3f(width_top, height, -thickness);
    glVertex3f(-width_top, height, -thickness);

    // Top Face
    normal = calculateNormal(
        { -width_top, height, -thickness },
        { -width_top, height, thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_top, height, -thickness);
    glVertex3f(-width_top, height, thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(width_top, height, -thickness);

    // Bottom Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, -thickness },
        { width_bottom, 0.0f, -thickness },
        { width_bottom, 0.0f, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, -thickness);
    glVertex3f(width_bottom, 0.0f, -thickness);
    glVertex3f(width_bottom, 0.0f, thickness);
    glVertex3f(-width_bottom, 0.0f, thickness);

    // Right Face
    normal = calculateNormal(
        { width_bottom, 0.0f, -thickness },
        { width_top, height, -thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(width_bottom, 0.0f, -thickness);
    glVertex3f(width_top, height, -thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(width_bottom, 0.0f, thickness);

    // Left Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, -thickness },
        { -width_bottom, 0.0f, thickness },
        { -width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, -thickness);
    glVertex3f(-width_bottom, 0.0f, thickness);
    glVertex3f(-width_top, height, thickness);
    glVertex3f(-width_top, height, -thickness);

    glEnd();

    glPopMatrix();

}
void boxwWithBevels(float height, float width, float bevel) {

    float half_Width = width / 2;
    float half_Width_n_bevel = half_Width + bevel;

    glPushMatrix();


    //bottom face
    glBegin(GL_POLYGON);
    normal = calculateNormal(
        { -half_Width, 0, half_Width_n_bevel },
        { -half_Width_n_bevel, 0, half_Width },
        { -half_Width_n_bevel, 0, -half_Width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, 0, half_Width_n_bevel);
    glVertex3f(half_Width, 0, half_Width_n_bevel);
    glVertex3f(half_Width_n_bevel, 0, half_Width);
    glVertex3f(half_Width_n_bevel, 0, -half_Width);
    glVertex3f(half_Width, 0, -half_Width_n_bevel);
    glVertex3f(-half_Width, 0, -half_Width_n_bevel);
    glVertex3f(-half_Width_n_bevel, 0, -half_Width);
    glVertex3f(-half_Width_n_bevel, 0, half_Width);
    glEnd();

    //top face
    glBegin(GL_POLYGON);
    normal = calculateNormal(
        { -half_Width, height, half_Width_n_bevel },
        { half_Width, height, half_Width_n_bevel },
        { half_Width_n_bevel, height, half_Width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, height, half_Width_n_bevel);
    glVertex3f(half_Width, height, half_Width_n_bevel);
    glVertex3f(half_Width_n_bevel, height, half_Width);
    glVertex3f(half_Width_n_bevel, height, -half_Width);
    glVertex3f(half_Width, height, -half_Width_n_bevel);
    glVertex3f(-half_Width, height, -half_Width_n_bevel);
    glVertex3f(-half_Width_n_bevel, height, -half_Width);
    glVertex3f(-half_Width_n_bevel, height, half_Width);
    glEnd();

    glBegin(GL_QUADS);

    //front face
    normal = calculateNormal(
        { -half_Width, 0.0f, -half_Width_n_bevel },
        { half_Width, 0, half_Width_n_bevel },
        { half_Width, height, half_Width_n_bevel });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, 0.0f, half_Width_n_bevel);
    glVertex3f(half_Width, 0, half_Width_n_bevel);
    glVertex3f(half_Width, height, half_Width_n_bevel);
    glVertex3f(-half_Width, height, half_Width_n_bevel);

    //front mini face right
    normal = calculateNormal(
        { half_Width, 0.0f, half_Width_n_bevel },
        { half_Width_n_bevel, 0.0f, half_Width },
        { half_Width_n_bevel, height, half_Width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(half_Width, 0.0f, half_Width_n_bevel);
    glVertex3f(half_Width_n_bevel, 0.0f, half_Width);
    glVertex3f(half_Width_n_bevel, height, half_Width);
    glVertex3f(half_Width, height, half_Width_n_bevel);

    //front mini face left
    normal = calculateNormal(
        { -half_Width, 0.0f, half_Width_n_bevel },
        { -half_Width, height, half_Width_n_bevel },
        { -half_Width_n_bevel, height, half_Width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, 0.0f, half_Width_n_bevel);
    glVertex3f(-half_Width_n_bevel, 0.0f, half_Width);
    glVertex3f(-half_Width_n_bevel, height, half_Width);
    glVertex3f(-half_Width, height, half_Width_n_bevel);

    //left face
    normal = calculateNormal(
        { half_Width_n_bevel, 0.0f, half_Width },
        { half_Width_n_bevel, 0, -half_Width },
        { half_Width_n_bevel, height, half_Width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(half_Width_n_bevel, 0.0f, half_Width);
    glVertex3f(half_Width_n_bevel, 0, -half_Width);
    glVertex3f(half_Width_n_bevel, height, -half_Width);
    glVertex3f(half_Width_n_bevel, height, half_Width);

    //right face
    normal = calculateNormal(
        { -half_Width_n_bevel, 0.0f, half_Width },
        { -half_Width_n_bevel, height, half_Width },
        { -half_Width_n_bevel, height, -half_Width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width_n_bevel, 0.0f, half_Width);
    glVertex3f(-half_Width_n_bevel, height, half_Width);
    glVertex3f(-half_Width_n_bevel, height, -half_Width);
    glVertex3f(-half_Width_n_bevel, 0, -half_Width);

    //back face
    normal = calculateNormal(
        { -half_Width, 0.0f, -half_Width_n_bevel },
        { -half_Width, height, -half_Width_n_bevel },
        { half_Width, height, -half_Width_n_bevel });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, 0.0f, -half_Width_n_bevel);
    glVertex3f(-half_Width, height, -half_Width_n_bevel);
    glVertex3f(half_Width, height, -half_Width_n_bevel);
    glVertex3f(half_Width, 0, -half_Width_n_bevel);

    //back mini face right
    normal = calculateNormal(
        { half_Width, 0.0f, -half_Width_n_bevel },
        { half_Width, height, -half_Width_n_bevel },
        { half_Width_n_bevel, height, -half_Width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(half_Width, 0.0f, -half_Width_n_bevel);
    glVertex3f(half_Width, height, -half_Width_n_bevel);
    glVertex3f(half_Width_n_bevel, height, -half_Width);
    glVertex3f(half_Width_n_bevel, 0.0f, -half_Width);

    normal = calculateNormal(
        { -half_Width, 0.0f, -half_Width_n_bevel },
        { -half_Width_n_bevel, 0.0f, -half_Width },
        { -half_Width_n_bevel, height, -half_Width });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, 0.0f, -half_Width_n_bevel);
    glVertex3f(-half_Width_n_bevel, 0.0f, -half_Width);
    glVertex3f(-half_Width_n_bevel, height, -half_Width);
    glVertex3f(-half_Width, height, -half_Width_n_bevel);

    glEnd();

    glPopMatrix();
}


//house
void smokePlane(float x, float y, float z) {

    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(x, y, z);
    glBindTexture(GL_TEXTURE_2D, texture_smoke[smoke_animation_timer]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1, 0.0f, 0);
    glTexCoord2f(1, 0.0f); glVertex3f(1, 0.0f, 0);
    glTexCoord2f(1, 1.0f); glVertex3f(1, 1, 0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0);

    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}
void singleTrangPart(float height, float width, float thickness) {

    //float thickness = 0.25;
    float halfThickness = thickness / 2;
    //float width = 2.0;
    float halfWidth = width / 2;
    float innerWidth = halfWidth - 0.2;
    //float height = 2.0;
    float innerHeight = height - 0.2;

    //right side

    glPushMatrix();

    glBegin(GL_QUADS);
    normal = calculateNormal(
        { 0,innerHeight,halfThickness },
        { innerWidth, 0.0, halfThickness },
        { halfWidth, 0.0, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(0, innerHeight, halfThickness);
    glVertex3f(innerWidth, 0.0, halfThickness);
    glVertex3f(halfWidth, 0.0, halfThickness);
    glVertex3f(0, height, halfThickness);



    normal = calculateNormal(
        { halfWidth, 0.0, -halfThickness },
        { 0, height, -halfThickness },
        { 0, height, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(halfWidth, 0.0, -halfThickness);
    glVertex3f(0, height, -halfThickness);
    glVertex3f(0, height, halfThickness);
    glVertex3f(halfWidth, 0.0, halfThickness);

    normal = calculateNormal(
        { innerWidth, 0.0, -halfThickness },
        { 0.0, innerHeight, -halfThickness },
        { 0.0, innerHeight, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(innerWidth, 0.0, -halfThickness);
    glVertex3f(0.0, innerHeight, -halfThickness);
    glVertex3f(0.0, innerHeight, halfThickness);
    glVertex3f(innerWidth, 0.0, halfThickness);


    //LEFT side

    normal = calculateNormal(
        { 0,innerHeight,halfThickness },
        { 0, height, halfThickness },
        { -halfWidth, 0.0, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(0, innerHeight, halfThickness);
    glVertex3f(0, height, halfThickness);
    glVertex3f(-halfWidth, 0.0, halfThickness);
    glVertex3f(-innerWidth, 0.0, halfThickness);


    normal = calculateNormal(
        { -halfWidth, 0.0, -halfThickness },
        { -halfWidth, 0.0, halfThickness },
        { 0, height, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-halfWidth, 0.0, -halfThickness);
    glVertex3f(-halfWidth, 0.0, halfThickness);
    glVertex3f(0, height, halfThickness);
    glVertex3f(0, height, -halfThickness);


    normal = calculateNormal(
        { -innerWidth, 0.0, -halfThickness },
        { 0.0, innerHeight, -halfThickness },
        { 0.0, innerHeight, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-innerWidth, 0.0, -halfThickness);
    glVertex3f(0.0, innerHeight, -halfThickness);
    glVertex3f(0.0, innerHeight, halfThickness);
    glVertex3f(-innerWidth, 0.0, halfThickness);


    //CLOSING

    normal = calculateNormal(
        { innerWidth, 0.0, -halfThickness },
        { halfWidth, 0.0, -halfThickness },
        { halfWidth, 0.0, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(innerWidth, 0.0, -halfThickness);
    glVertex3f(innerWidth, 0.0, halfThickness);
    glVertex3f(halfWidth, 0.0, halfThickness);
    glVertex3f(halfWidth, 0.0, -halfThickness);

    normal = calculateNormal(
        { -innerWidth, 0.0, -halfThickness },
        { -innerWidth, 0.0, halfThickness },
        { -halfWidth, 0.0, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-innerWidth, 0.0, -halfThickness);
    glVertex3f(-innerWidth, 0.0, halfThickness);
    glVertex3f(-halfWidth, 0.0, halfThickness);
    glVertex3f(-halfWidth, 0.0, -halfThickness);



    glEnd();

    glPopMatrix();

}
void houseBase() {
    float height = 3.0f;
    float thickness = 2.0f;
    float width_top = 1.5f;
    float width_bottom = 1.0f;

    glPushMatrix();
    glBegin(GL_QUADS);

    glColor3f(1, 0.9333, 0.7568);//250, 238, 193
    // Front Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, thickness },
        { width_bottom, 0.0f, thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, thickness);
    glVertex3f(width_bottom, 0.0f, thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(-width_top, height, thickness);

    // Back Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, -thickness },
        { width_bottom, 0.0f, -thickness },
        { width_top, height, -thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, -thickness);
    glVertex3f(width_bottom, 0.0f, -thickness);
    glVertex3f(width_top, height, -thickness);
    glVertex3f(-width_top, height, -thickness);

    // Top Face
    normal = calculateNormal(
        { -width_top, height, -thickness },
        { -width_top, height, thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_top, height, -thickness);
    glVertex3f(-width_top, height, thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(width_top, height, -thickness);

    // Bottom Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, -thickness },
        { width_bottom, 0.0f, -thickness },
        { width_bottom, 0.0f, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, -thickness);
    glVertex3f(width_bottom, 0.0f, -thickness);
    glVertex3f(width_bottom, 0.0f, thickness);
    glVertex3f(-width_bottom, 0.0f, thickness);

    // Right Face
    normal = calculateNormal(
        { width_bottom, 0.0f, -thickness },
        { width_top, height, -thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(width_bottom, 0.0f, -thickness);
    glVertex3f(width_top, height, -thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(width_bottom, 0.0f, thickness);

    // Left Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, -thickness },
        { -width_bottom, 0.0f, thickness },
        { -width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, -thickness);
    glVertex3f(-width_bottom, 0.0f, thickness);
    glVertex3f(-width_top, height, thickness);
    glVertex3f(-width_top, height, -thickness);

    glEnd();

    glPopMatrix();
}
void roofBase(float height, float thickness, float width_top, float width_bottom) {



    glPushMatrix();
    glBegin(GL_QUADS);

    glColor3f(1, 0.9333, 0.7568);//250, 238, 193
    // Front Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, thickness },
        { width_bottom, 0.0f, thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, thickness);
    glVertex3f(width_bottom, 0.0f, thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(-width_top, height, thickness);

    glColor3f(1, 0.9333, 0.7568);//250, 238, 193
    // Back Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, -thickness },
        { width_bottom, 0.0f, -thickness },
        { width_top, height, -thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom, 0.0f, -thickness);
    glVertex3f(width_bottom, 0.0f, -thickness);
    glVertex3f(width_top, height, -thickness);
    glVertex3f(-width_top, height, -thickness);

    glColor3f(0.4078f, 0.3529f, 0.4196f); //104, 90, 107
    // Top Face
    normal = calculateNormal(
        { -width_top, height, -thickness },
        { -width_top, height, thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_top, height, -thickness);
    glVertex3f(-width_top, height, thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(width_top, height, -thickness);

    // Right Face
    normal = calculateNormal(
        { width_bottom, 0.0f, -thickness },
        { width_top, height, -thickness },
        { width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(width_bottom + 0.15, -0.2f, -thickness);
    glVertex3f(width_top, height, -thickness);
    glVertex3f(width_top, height, thickness);
    glVertex3f(width_bottom + 0.15, -0.2f, thickness);

    //right extra panel
    normal = calculateNormal(
        { width_bottom + 0.15, -0.2f, -thickness },
        { width_bottom + 0.15, -0.2f, thickness },
        { 2.1, -0.5, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(width_bottom + 0.15, -0.2f, -thickness);
    glVertex3f(width_bottom + 0.15, -0.2f, thickness);
    glVertex3f(2.1, -0.5, thickness);
    glVertex3f(2.1, -0.5, -thickness);



    // Left Face
    normal = calculateNormal(
        { -width_bottom, 0.0f, -thickness },
        { -width_bottom, 0.0f, thickness },
        { -width_top, height, thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom - 0.15, -0.2f, -thickness);
    glVertex3f(-width_bottom - 0.15, -0.2f, thickness);
    glVertex3f(-width_top, height, thickness);
    glVertex3f(-width_top, height, -thickness);


    // Left extra panel
    normal = calculateNormal(
        { -width_bottom - 0.15, -0.2f, -thickness },  // v1
        { -2.1, -0.5, -thickness },                  // v2
        { -2.1, -0.5, thickness }                    // v3
    );
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width_bottom - 0.15, -0.2f, -thickness);  // v1
    glVertex3f(-2.1, -0.5, -thickness);                  // v2
    glVertex3f(-2.1, -0.5, thickness);                   // v3
    glVertex3f(-width_bottom - 0.15, -0.2f, thickness);  // v4



    glEnd();

    glPopMatrix();

}
void roofFrameFront(float x, float y, float z) {
    glColor3f(0.4078f, 0.3529f, 0.4196f); //104, 90, 107
    float thickness = 0.25;
    float halfThickness = thickness / 2;
    //right side

    glPushMatrix();
    glTranslatef(x, y, z);
    glBegin(GL_QUADS);
    normal = calculateNormal(
        { 0,1.8,halfThickness },
        { 1.45, -0.2, halfThickness },
        { 1.65, -0.2, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(0, 1.8, halfThickness);
    glVertex3f(1.45, -0.2, halfThickness);
    glVertex3f(1.65, -0.2, halfThickness);
    glVertex3f(0, 2.0, halfThickness);




    normal = calculateNormal(
        { 0,1.8,halfThickness },
        { 0, 2.0, halfThickness },
        { 1.65, -0.2, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(0, 1.8, -halfThickness);
    glVertex3f(0, 2.0, -halfThickness);
    glVertex3f(1.65, -0.2, -halfThickness);
    glVertex3f(1.45, -0.2, -halfThickness);



    normal = calculateNormal(
        { 1.65, -0.2, -halfThickness },
        { 0, 2.0, -halfThickness },
        { 0, 2.0, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(1.65, -0.2, -halfThickness);
    glVertex3f(0, 2.0, -halfThickness);
    glVertex3f(0, 2.0, halfThickness);
    glVertex3f(1.65, -0.2, halfThickness);

    normal = calculateNormal(
        { 1.45, -0.2, -halfThickness },
        { 0.0, 1.8, -halfThickness },
        { 0.0, 1.8, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(1.45, -0.2, -halfThickness);
    glVertex3f(0.0, 1.8, -halfThickness);
    glVertex3f(0.0, 1.8, halfThickness);
    glVertex3f(1.45, -0.2, halfThickness);

    //extra
    normal = calculateNormal(
        { 1.45, -0.2, halfThickness },
        { 2.1, -0.7, halfThickness },
        { 2.1, -0.5, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(1.45, -0.2, halfThickness);
    glVertex3f(2.1, -0.7, halfThickness);
    glVertex3f(2.1, -0.5, halfThickness);
    glVertex3f(1.65, -0.2, halfThickness);

    normal = calculateNormal(
        { 1.45, -0.2, -halfThickness },
        { 1.65, -0.2, -halfThickness },
        { 2.1, -0.5, -halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(1.45, -0.2, -halfThickness);
    glVertex3f(1.65, -0.2, -halfThickness);
    glVertex3f(2.1, -0.5, -halfThickness);
    glVertex3f(2.1, -0.7, -halfThickness);

    normal = calculateNormal(
        { 1.65, -0.2, halfThickness },
        { 2.1, -0.5, halfThickness },
        { 2.1, -0.5, -halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(1.65, -0.2, halfThickness);
    glVertex3f(2.1, -0.5, halfThickness);
    glVertex3f(2.1, -0.5, -halfThickness);
    glVertex3f(1.65, -0.2, -halfThickness);

    normal = calculateNormal(
        { 1.45, -0.2, halfThickness },
        { 2.1, -0.7, halfThickness },
        { 2.1, -0.7, -halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(1.45, -0.2, halfThickness);
    glVertex3f(2.1, -0.7, halfThickness);
    glVertex3f(2.1, -0.7, -halfThickness);
    glVertex3f(1.45, -0.2, -halfThickness);

    normal = calculateNormal(
        { 2.1, -0.7, halfThickness },
        { 2.1, -0.7, -halfThickness },
        { 2.1, -0.5, -halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(2.1, -0.7, halfThickness);
    glVertex3f(2.1, -0.7, -halfThickness);
    glVertex3f(2.1, -0.5, -halfThickness);
    glVertex3f(2.1, -0.5, halfThickness);


    //LEFT side

    normal = calculateNormal(
        { 0,1.8,halfThickness },
        { 0, 2.0, halfThickness },
        { -1.65, -0.2, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(0, 1.8, halfThickness);
    glVertex3f(0, 2.0, halfThickness);
    glVertex3f(-1.65, -0.2, halfThickness);
    glVertex3f(-1.45, -0.2, halfThickness);

    normal = calculateNormal(
        { 0, 1.8, -halfThickness },
        { -1.45, -0.2, -halfThickness },
        { -1.65, -0.2, -halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(0, 1.8, -halfThickness);
    glVertex3f(-1.45, -0.2, -halfThickness);
    glVertex3f(-1.65, -0.2, -halfThickness);
    glVertex3f(0, 2.0, -halfThickness);



    normal = calculateNormal(
        { -1.65, -0.2, -halfThickness },
        { -1.65, -0.2, halfThickness },
        { 0, 2.0, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-1.65, -0.2, -halfThickness);
    glVertex3f(-1.65, -0.2, halfThickness);
    glVertex3f(0, 2.0, halfThickness);
    glVertex3f(0, 2.0, -halfThickness);


    normal = calculateNormal(
        { -1.45, -0.2, -halfThickness },
        { 0.0, 1.8, -halfThickness },
        { 0.0, 1.8, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-1.45, -0.2, -halfThickness);
    glVertex3f(0.0, 1.8, -halfThickness);
    glVertex3f(0.0, 1.8, halfThickness);
    glVertex3f(-1.45, -0.2, halfThickness);

    //extra
    normal = calculateNormal(
        { -1.45, -0.2, halfThickness },
        { -1.65, -0.2, halfThickness },
        { -2.1, -0.5, halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-1.45, -0.2, halfThickness);
    glVertex3f(-1.65, -0.2, halfThickness);
    glVertex3f(-2.1, -0.5, halfThickness);
    glVertex3f(-2.1, -0.7, halfThickness);



    normal = calculateNormal(
        { -1.45, -0.2, -halfThickness },
        { -2.1, -0.7, -halfThickness },
        { -2.1, -0.5, -halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-1.45, -0.2, -halfThickness);
    glVertex3f(-2.1, -0.7, -halfThickness);
    glVertex3f(-2.1, -0.5, -halfThickness);
    glVertex3f(-1.65, -0.2, -halfThickness);



    normal = calculateNormal(
        { -1.65, -0.2, halfThickness },
        { -1.65, -0.2, -halfThickness },
        { -2.1, -0.5, -halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-1.65, -0.2, halfThickness);
    glVertex3f(-1.65, -0.2, -halfThickness);
    glVertex3f(-2.1, -0.5, -halfThickness);
    glVertex3f(-2.1, -0.5, halfThickness);



    normal = calculateNormal(
        { -1.45, -0.2, halfThickness },
        { -2.1, -0.7, halfThickness },
        { 2.1, -0.7, -halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-1.45, -0.2, halfThickness);
    glVertex3f(-1.45, -0.2, -halfThickness);
    glVertex3f(-2.1, -0.7, -halfThickness);
    glVertex3f(-2.1, -0.7, halfThickness);



    normal = calculateNormal(
        { -2.1, -0.7, halfThickness },
        { -2.1, -0.5, halfThickness },
        { -2.1, -0.5, -halfThickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-2.1, -0.7, halfThickness);
    glVertex3f(-2.1, -0.5, halfThickness);
    glVertex3f(-2.1, -0.5, -halfThickness);
    glVertex3f(-2.1, -0.7, -halfThickness);

    glEnd();

    glPopMatrix();
}
void roofFrontTrang(float x, float y, float z) {

    float width = 1.49;
    float height = 2;
    float thickness = 0.51f;
    float half_thickness = thickness / 2;


    glPushMatrix();

    glColor3f(1, 0.9333, 0.7568);//250, 238, 193
    glBegin(GL_TRIANGLES);
    normal = calculateNormal(
        { -width, 0, half_thickness },
        { width, 0, half_thickness },
        { 0, height, half_thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width, 0, half_thickness);
    glVertex3f(width, 0, half_thickness);
    glVertex3f(0, height, half_thickness);
    glEnd();

    glColor3f(0.4078f, 0.3529f, 0.4196f); //104, 90, 107
    glBegin(GL_TRIANGLES);
    normal = calculateNormal(
        { -width, 0, -half_thickness },
        { width, 0,-half_thickness },
        { 0, height, -half_thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width, 0, -half_thickness);
    glVertex3f(width, 0, -half_thickness);
    glVertex3f(0, height, -half_thickness);
    glEnd();


    glBegin(GL_QUADS);
    normal = calculateNormal(
        { width, 0.0f, half_thickness },
        { width, 0, -half_thickness },
        { 0, height, -half_thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(width, 0.0f, half_thickness);
    glVertex3f(width, 0, -half_thickness);
    glVertex3f(0, height, -half_thickness);
    glVertex3f(0, height, half_thickness);
    glEnd();

    glBegin(GL_QUADS);
    normal = calculateNormal(
        { -width, 0.0f, -half_thickness },
        { -width, 0, half_thickness },
        { 0, height, half_thickness });
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-width, 0.0f, -half_thickness);
    glVertex3f(-width, 0, half_thickness);
    glVertex3f(0, height, half_thickness);
    glVertex3f(0, height, -half_thickness);
    glEnd();

    glPopMatrix();
}
void chimney(float x, float y, float z) {

    glColor3f(0.96078f, 0.8353f, 0.6314f); //245, 213, 161
    glPushMatrix();
    glTranslatef(x, y, z);

    glPushMatrix();
    glTranslatef(0, 0.1, 0);
    truncatedPyramid(0.2f, 0.15f, 0.185f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -0.4, 0);
    truncatedPyramid(0.5f, 0.15f, 0.12f);
    glPopMatrix();


    smokePlane(-0.8, 0.45, 0);
    glPopMatrix();

}
void roofTop(float x, float y, float z) {
    glColor3f(0.4078f, 0.3529f, 0.4196f); //104, 90, 107
    glPushMatrix();
    glTranslatef(x, y, z);
    base(0.04, 2, 0.15, 0.17);
    chimney(-0.05, 0.2, -1.2);
    chimney(-0.05, 0.2, 1);


    glPopMatrix();
}
void roofBaseStructure(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    roofBase(1.8, 2, 0.15, 1.5);
    glPopMatrix();

}
void roofAssemble() {

    roofFrameFront(0, 3, 2.125);
    roofFrameFront(0, 3, -2.125);
    roofBaseStructure(0, 3, 0);
    roofTop(0, 4.8, 0);
}
void houseFoundation(float x, float y, float z) {

    float thickness = 2.7;
    float halfWidth = 2.5;
    //float 

    glColor3f(0.4078f, 0.3529f, 0.4196f); //104, 90, 107

    glPushMatrix();
    glTranslatef(x, y, z);
    base(0.5, thickness, halfWidth, 2.2);

    glPushMatrix();
    glTranslatef(halfWidth - 0.3, 0.5, 0);
    base(0.55, thickness, 0.28, 0.3);
    glTranslatef(-2 * halfWidth + 0.6, 0, 0);
    base(0.55, thickness, 0.28, 0.3);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 0.5, -thickness + 0.279);
    //glRotatef(90,0,1,0);
    glScalef(8.5, 1, 1);
    base(0.55, 0.28, 0.28, 0.28);

    glPopMatrix();


    glPopMatrix();

}
void windowType1(float x, float y, float z) {

    float halfWidth = 1.5;
    float height = halfWidth / 2;

    glColor3f(0.4078f, 0.3529f, 0.4196f); //104, 90, 107
    glPushMatrix();
    glTranslatef(x, y, z);
    singleTrangPart(height, halfWidth, 0.25);
    glRotatef(180, 0, 0, 1);
    singleTrangPart(height, halfWidth, 0.25);
    glPopMatrix();

}
void WindowType2(float x, float y, float z) {
    glColor3f(0.4078f, 0.3529f, 0.4196f); //dark purple


    glPushMatrix();
    glTranslatef(x, y, z);
    truncatedPrism(0.125, 0.125, 1, 1);
    glPushMatrix();
    glRotatef(95, 0, 0, 1);
    glTranslatef(1, 0.75, 0);
    truncatedPrism(0.125, 0.125, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glRotatef(-95, 0, 0, 1);
    glTranslatef(-1, 0.75, 0);
    truncatedPrism(0.125, 0.125, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glRotatef(15, 0, 0, 1);
    glTranslatef(0, 2, 0);
    truncatedPrism(0.125, 0.125, 0.6, 0.6);
    glPopMatrix();

    glPushMatrix();
    glRotatef(-15, 0, 0, 1);
    glTranslatef(0, 2, 0);
    truncatedPrism(0.125, 0.125, 0.6, 0.6);
    glPopMatrix();

    glPopMatrix();

}
void door(float x, float y, float z) {

    float thickness = 0.0625;
    float trangThickness = 2 * thickness;
    glColor3f(0.9608, 0.6, 0.1137); //245, 153, 29

    glPushMatrix();
    glTranslatef(x, y, z);
    glPushMatrix();
    glTranslatef(0, 1.5, 0);
    prism(0.65, 0.65, trangThickness);
    glPopMatrix();
    base(1.5, thickness, 0.65, 0.65);
    glPopMatrix();
}
void doorFrame(float x, float y, float z) {

    float height = 1.5;
    float width = 1.5;
    float halfWidth = width / 2;

    glColor3f(0.4078f, 0.3529f, 0.4196f); //104, 90, 107

    glPushMatrix();
    glTranslatef(x, y, z);
    glPushMatrix();
    glTranslatef(0, height, 0);
    singleTrangPart(0.75, 1.5, 0.25);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(halfWidth - 0.15, 0, -0.125);
    base(height, 0.125, 0.1, 0.1);
    glTranslatef(-width + 0.3, 0, 0);
    base(height, 0.125, 0.1, 0.1);
    glPopMatrix();

    glPushMatrix();
    base(0.15, 0.25, 1, 1);
    glPopMatrix();

    glPopMatrix();


}
void doorAssemble() {
    door(0, 0, 1.975);
    doorFrame(0, 0, 2.1);
}
void staircase(float x, float y, float z) {

    float height = 0.5;
    float width = 0.5;
    float thickness = 0.5;

    float stepHeight = height / 3;

    glPushMatrix();
    glTranslatef(x, y, z);
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(0, i * stepHeight, -i * 0.1);
        base(stepHeight, thickness, width, width);
        glPopMatrix();
    }
    glPopMatrix();

}
void drawGroundBlock(float x, float y, float z) {

    float height = 0.5f;
    float width = 10;
    float bevel = 0.6f;

    glColor3f(0.361, 0.38, 0.18); //grass green

    glPushMatrix();
    glTranslatef(x, y, z);

    boxwWithBevels(height, width, bevel);
    staircase(0, 0, 5.5);
    glPopMatrix();
}
void house(float x, float y, float z) {

    glPushMatrix();
    glTranslatef(x, y, z);

    houseBase();
    roofAssemble();
    windowType1(0, 3.5, 2);
    houseFoundation(0, -0.5, 0);
    staircase(0, -0.5, 2.75);
    doorAssemble();

    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glRotatef(15, 1, 0, 0);
    glScalef(0.5, 0.5, 1);
    WindowType2(0, 2.5, 0.8);
    glPopMatrix();

    drawGroundBlock(0, -1, 0);

    glPopMatrix();
}

//tree
void drawTreeTrunk() {

    glColor3f(0.49019f, 0.36078f, 0.23921f);
    glPushMatrix();
    boxwWithBevels(1, 0.125, 0.075);
    glTranslatef(0, 0.5, 0);
    glRotatef(-60, 0, 0, 1);
    glScalef(0.5, 0.5, 0.5);
    boxwWithBevels(1, 0.125, 0.075);
    glPopMatrix();
}
void drawTreeTop() {

    glColor3f(0.49411f, 0.5255f, 0.2235f);

    glPushMatrix();
    truncatedPyramid(1, 0.1, 0.5);
    glTranslatef(0, 0.5, 0);
    truncatedPyramid(1, 0.1, 0.5);
    glPopMatrix();
}
void drawTree(float x, float y, float z) {

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(2, 2, 2);
    drawTreeTrunk();
    glTranslatef(0, 1, 0);
    drawTreeTop();
    glPopMatrix();
}

// Initialize enemies with different types and positions
void initializeEnemies() {
    for (int i = 0; i < maxEnemies; ++i) {
        Entity enemy = { static_cast<float>(i * 5), 0.5f, -10.0f, 50.0f, true, i % 2 == 0 };
        enemies.push_back(enemy);
    }
}

// Check collision between player attack and enemy
bool checkCollision(Entity& enemy) {
    float dx = playerX - enemy.x;
    float dz = playerZ - enemy.z;
    float distance = std::sqrt(dx * dx + dz * dz);
    return distance <= attackRange;
}

// Update player state, dash, and energy
void updatePlayer() {

    // Attack animation timer management
    if (isAttacking) {
        if (attackAnimationTimer < 6.0f) {
            attackAnimationTimer += 1.0f; // Increment timer
        }
        else {
            isAttacking = false;  // End attack after timer reaches 6
            attackAnimationTimer = 0.0f;
        }
    }
    else {
        isAttacking = false;  // End attack
        attackAnimationTimer = 0.0f;
    }
}

// Update enemy behavior and check for interactions with player
void updateEnemies() {
    for (auto& enemy : enemies) {
        if (!enemy.isAlive) continue;

        // Move enemy towards player
        float dx = playerX - enemy.x;
        float dz = playerZ - enemy.z;
        float distance = std::sqrt(dx * dx + dz * dz);

        // Movement towards player
        if (distance > 0.5f) {
            enemy.x += enemySpeed * (dx / distance);
            enemy.z += enemySpeed * (dz / distance);
        }

        // Enemy attack logic (ranged or melee)
        if (enemy.isRanged && distance < 5.0f) {
            if (rangedAttackCooldown <= 0.0f) {
                playerHealth -= 5.0f;  // Range attack damage
                rangedAttackCooldown = 2.0f;  // Reset cooldown
            }
            else {
                rangedAttackCooldown -= 0.1f;  // Cooldown management
            }
        }
        else if (distance < 1.0f) {
            playerHealth -= 0.05f;  // Melee damage
        }

        // Player attack: reduce enemy health if in range
        if (isAttacking && checkCollision(enemy)) {
            enemy.health -= 10.0f;
            if (enemy.health <= 0.0f) {
                enemy.isAlive = false;
                score += 10; // Increment score for defeating enemy
            }
        }

        // Respawn dead enemies after a delay
        if (!enemy.isAlive) {
            enemyRespawnTime -= 0.1f;
            if (enemyRespawnTime <= 0.0f) {
                enemy.isAlive = true;
                enemy.health = 50.0f;
                enemyRespawnTime = 3.0f;
            }
        }
    }
}

// Draw health and energy bars
void drawHUD() {
    // Health Bar
    glColor3f(1.0f, 0.0f, 0.0f);  // Red for health
    glRectf(-0.9f, 0.9f, -0.9f + (playerHealth / 100.0f) * 0.5f, 1.0f);

    // Score Display
    char scoreText[32];
    sprintf_s(scoreText, sizeof(scoreText), "Score: %d", score);
    glRasterPos2f(0.7f, 0.9f);
    for (char* c = scoreText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Game Over message if player health is zero
    if (gameOver) {
        const char* gameOverText = "Game Over!";
        glRasterPos2f(-0.1f, 0.0f);
        for (const char* c = gameOverText; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
}

//gorund
void ground() {
    //glBegin(GL_QUADS);
    //glColor3f(0.8, 0.7, 0.3);
    //glVertex3f(-20, 0, -20);
    //glVertex3f(20, 0, -20);
    //glVertex3f(20, 0, 20);
    //glVertex3f(-20, 0, 20);
    //glEnd();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture_ground[ground_animation_timer]);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-20, 0.0f, -20);
    glTexCoord2f(1, 0.0f); glVertex3f(20, 0.0f, -20);
    glTexCoord2f(1, 1.0f); glVertex3f(20, 0, 20);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-20, 0, 20);

    glEnd();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

}

//fox
void drawEar() {
    float ear_heights[] = { 0.060754,0.046858, 0.061077, 0.0614 };
    float ear_widths[] = { 0.132819,0.148319,0.140216,0.113106,0.049443 };

    glColor3f(0.972, 0.501, 0.223);
    glPushMatrix();
    glScalef(0.5, 1.5, 0.25);
    for (int i = 0; i < 4; i++) {
        if (i == 0) {

            truncatedPyramid(ear_heights[i], ear_widths[i + 1], ear_widths[i]);
            continue;
        }
        glTranslatef(0, ear_heights[i - 1], 0);
        truncatedPyramid(ear_heights[i], ear_widths[i + 1], ear_widths[i]);

    }
    glPopMatrix();

}
void handAndLegBase() {
    float heights[] = { 0.041494, 0.043045, 0.021329, 0, 0.069027 };
    float radius[] = { 0.019777, 0.028309, 0.029472, 0.029472, 0.036836, 0.036836 };

    glPushMatrix();

    GLUquadric* quad = gluNewQuadric();

    gluSphere(quad, 0.055393, 32, 32);

    glTranslatef(0, -0.2, 0);
    for (int i = 0; i < 5; i++) {

        if (i == 0) {
            glPushMatrix();
            glRotatef(-90, 1, 0, 0);
            gluCylinder(quad, radius[i], radius[i + 1], heights[i], 64, 100);
            glPopMatrix();
            continue;
        }

        glTranslatef(0, heights[i - 1], 0);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);

        // Ensure we don't access out of bounds
        float topRadius = (i + 1 < 5) ? radius[i + 1] : radius[i];  // Use current radius if out-of-bounds
        gluCylinder(quad, radius[i], topRadius, heights[i], 64, 100);
        glPopMatrix();

    }
    gluDeleteQuadric(quad);

    glPopMatrix();
}
void drawHand() {

    float armSwing = isWalking ? sin(anime_time * 0.3) * 20.0f : 0.0f;

    glColor3f(0.972, 0.501, 0.223);
    glPushMatrix();
    // glScalef(1.5, 1.5, 1.5);
    glRotatef(armSwing, 0, 1, 0);
    glRotatef(60, 0, 0, 1);
    handAndLegBase();
    glPopMatrix();

}
void drawLeg() {

    float legSwing = isWalking ? sin(anime_time * 0.3f) * 30.0f : 0.0f;
    glColor3f(0.972, 0.501, 0.223);
    glPushMatrix();
    // glScalef(1.5, 1.5, 1.5);
    glRotatef(legSwing, 1, 0, 0);
    handAndLegBase();
    glPopMatrix();

}
void drawBody() {

    float body_seg_heights[] = { 0.09794,0.09794, 0.096668, 0.095418 };
    float body_seg_widths[] = { 0.141328,0.166982,0.157522,0.134948,0.104278 };

    glPushMatrix();
    glColor3f(0.972, 0.501, 0.223);
    glScalef(1, 1, 0.75);
    for (int i = 0; i < 4; i++) {

        if (i == 0) {

            truncatedPyramid(body_seg_heights[i], body_seg_widths[i + 1], body_seg_widths[i]);
            continue;
        }
        glTranslatef(0, body_seg_heights[i - 1], 0);
        truncatedPyramid(body_seg_heights[i], body_seg_widths[i + 1], body_seg_widths[i]);

    }
    glPopMatrix();

    glPushMatrix();

    GLUquadric* quad = gluNewQuadric();
    glColor3f(0.850, 0.584, 0.427);
    glTranslatef(0, 0.15, 0.12);
    glScalef(1, 1.5, 0.1);
    gluSphere(quad, 0.1f, 32, 32);
    gluDeleteQuadric(quad);

    glPopMatrix();

}
void drawHead() {


    glPushMatrix();



    GLUquadric* quad = gluNewQuadric();
    glColor3f(0.972, 0.501, 0.223);
    gluSphere(quad, 0.335507f, 32, 32);

    glPushMatrix();
    glColor3f(0.850, 0.584, 0.427);
    glTranslated(0, -0.15, 0.2);
    glScalef(1.5, 1, 2);
    gluSphere(quad, 0.157, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.2, 0.2, 0.1);
    glTranslated(0, -0.15, 0.54);
    gluSphere(quad, 0.05, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0, 0, 0.1);
    glTranslated(0.12, 0.1, 0.29);
    gluSphere(quad, 0.05, 32, 32);
    glTranslated(-0.24, 0, 0);
    gluSphere(quad, 0.05, 32, 32);
    glPopMatrix();

    gluDeleteQuadric(quad);

    glPopMatrix();

}
void drawTail() {

    float armSwing = isWalking ? sin(glutGet(GLUT_ELAPSED_TIME) * 0.01f) * 20.0f : 0.0f;

    glPushMatrix();

    GLUquadric* quad = gluNewQuadric();
    glColor3f(0.972, 0.501, 0.223);

    glRotatef(armSwing, 0, 1, 0);
    glTranslatef(0, 0, -0.1);
    glScalef(1, 1, 2);
    gluSphere(quad, 0.1f, 32, 32);
    gluDeleteQuadric(quad);

    glPopMatrix();

}
void attackPlane(float x, float y, float z) {


    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(x, y, z);
    glBindTexture(GL_TEXTURE_2D, texture_attack[attack_animation_timer]);
    //glDisable(GL_DEPTH_TEST);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1, 0.0f, 0);
    glTexCoord2f(1, 0.0f); glVertex3f(1, 0.0f, 0);
    glTexCoord2f(1, 1.0f); glVertex3f(1, 1, 0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0);

    glEnd();
    //glEnable(GL_DEPTH_TEST);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

}

void player() {

    // Update animations

    bodyBob = sin(anime_time * 0.1) * 0.05f;

    float rotationDiff = targetRotation - playerRotation;
    if (fabs(rotationDiff) > 1.0f) {
        playerRotation += rotationDiff * 0.1f;
    }
    glPushMatrix(); // whole body
    glTranslatef(0, -0.5, 0);
    glRotatef(playerRotation, 0.0f, 1.0f, 0.0f);

    glPushMatrix();

    glTranslatef(0, bodyBob, 0);

    //head
    glPushMatrix();
    glTranslated(0, 1, 0);
    drawHead();
    glPopMatrix();

    //ears
    glPushMatrix();
    glTranslatef(-0.16, 1.225507f, -0.02);
    drawEar();
    glTranslatef(0.32, 0, 0);
    drawEar();
    glPopMatrix();

    glPopMatrix();

    //body
    glPushMatrix();
    glScalef(1 + bodyBob, 1 + bodyBob, 1 + bodyBob);
    glTranslatef(0, 0.27, 0);
    drawBody();
    glPopMatrix();

    ////hands
    glPushMatrix();
    glScalef(1 + bodyBob, 1 + bodyBob, 1 + bodyBob);

    glPushMatrix();
    glTranslatef(0.16, 0.6, 0);
    drawHand();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.16, 0.6, 0);
    glRotatef(180, 0, 1, 0);
    drawHand();
    glPopMatrix();

    glPopMatrix();

    //legs

    glPushMatrix();
    glTranslatef(0.1, 0.25, 0);
    drawLeg();
    glTranslatef(-0.2, 0, 0);
    glRotatef(180, 0, 1, 0);
    drawLeg();
    glPopMatrix();

    //tail 
    glPushMatrix();
    glTranslatef(0, 0.3, -0.2);
    drawTail();
    glPopMatrix();

    //attack animation plane
    if (isAttackAnimating) {
        glColor3f(1, 1, 1);
        glPushMatrix();
        glTranslatef(0, 0.5, 0);
        glRotatef(90, 1, 0, 0);
        attackPlane(0, 0, 0);
        glPopMatrix();
    }

    glPopMatrix(); // whole body
}


// Display callback function
void displayEntities() {
    glPushMatrix();
    glTranslatef(playerX, playerY, playerZ);
    drawHUD();
    glPopMatrix();

    ground();

    // Draw enemies
    for (const auto& enemy : enemies) {
        if (enemy.isAlive) {
            glColor3f(enemy.isRanged ? 1.0f : 0.0f, 0.0f, 1.0f);
        }
        else {
            glColor3f(0.5f, 0.5f, 0.5f);
        }
        glPushMatrix();
        glTranslatef(enemy.x, enemy.y, enemy.z);
        glutSolidCube(1.0);// enemy model gose here
        glPopMatrix();
    }

    //draw enviroment
    glPushMatrix();

    //trees
    glPushMatrix();
    drawTree(5, 0, -13);

    glPushMatrix();
    glTranslatef(6, 0, -5);
    glRotatef(-60, 0, 1, 0);
    glScalef(2, 2, 2);
    drawTreeTrunk();
    glPopMatrix();

    glPopMatrix();

    //house
    house(14, 1, -15);
    glPopMatrix();

    // Draw player
    glPushMatrix();
    glTranslatef(playerX, playerY, playerZ);
    //drawPlayer
    player();
    glPopMatrix();
}
void display() {
    if (gameOver) {
        printf("Game Over! Player died. Final Score: %d\n", score);
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    setLightingAndShading();

    gluLookAt(playerX, playerY + 15.0f, playerZ + 15.0f, playerX, playerY, playerZ, 0.0f, 1.0f, 0.0f);

    drawGrid();
    displayEntities();
    updateEnemies();
    updatePlayer();


    glutSwapBuffers();
}

// Timer function 
void timer(int) {
    if (!gameOver) {
        if (playerHealth <= 0.0f) {
            gameOver = true;  // End game if health is zero
        }
        glutPostRedisplay();
    }
    anime_time += 1;

    if (frameDelayCounter % 8 == 0) {  // Change every second timer call
        smoke_animation_timer = (smoke_animation_timer + 1) % 4;
    }
    if (frameDelayCounter % 8 == 0) {  // Change every second timer call
        ground_animation_timer = (ground_animation_timer + 1) % 4;
    }


    if (isAttackAnimating) {
        if (frameDelayCounter % 2 == 0) {
            attack_animation_timer++;
            if (attack_animation_timer >= 6) {
                isAttackAnimating = false;
                isAttacking = false;
                attack_animation_timer = 0;
            }
        }
    }
    frameDelayCounter++;

    glutTimerFunc(16, timer, 0);  // ~60 FPS
}

// Keyboard input for player movement, attack, and dash
void keyboard(unsigned char key, int x, int y) {
    if (gameOver) return;

    float moveSpeed = 0.5f;
    isWalking = true;

    float newX = playerX;
    float newZ = playerZ;

    switch (key) {
    case 'w':
        newZ -= moveSpeed;
        targetRotation = 180.0f;
        break;
    case 's':
        newZ += moveSpeed;
        targetRotation = 0.0f;
        break;
    case 'a':
        newX -= moveSpeed;
        targetRotation = -90.0f;
        break;
    case 'd':
        newX += moveSpeed;
        targetRotation = 90.0f;
        break;
    case ' ':
        if (!isAttackAnimating) {
            isAttacking = true;
            isAttackAnimating = true;
            attack_animation_timer = 0;
        }
        break;
    default:
        isWalking = false;
    }

    // Restrict movement to boundaries
    if (newX >= -20.0f && newX <= 20.0f && !isInRestrictedArea(newX, playerZ)) {
        playerX = newX;
    }
    if (newZ >= -20.0f && newZ <= 20.0f && !isInRestrictedArea(playerX, newZ)) {
        playerZ = newZ;
    }
}


// Keyboard release callback to stop attack and dash
void keyboardUp(unsigned char key, int x, int y) {
    if (key == ' ') isAttacking = false;
    if (key == 'w' || key == 'a' || key == 's' || key == 'd') {
        isWalking = false;
    }
}

// Initialization function for OpenGL settings
void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    loadTexture();
}

// Window reshape callback function
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Advanced Game Mechanics");

    init();
    initializeEnemies();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
