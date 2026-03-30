#ifndef DRAW_H
#define DRAW_H

/*
    Minimal immediate-style draw API (OpenGL)

    FEATURES:
    - Built-in shader
    - NDC drawing (Line, Rect, etc.)
    - Pixel-space drawing (LineP, RectP, etc.)

    USAGE:
        #define DRAW_IMPLEMENTATION
        #include "draw.h"

        DrawInit(width, height);
*/

#ifdef __cplusplus
extern "C" {
#endif

/* ================= Types ================= */

typedef struct { float x, y; } Vec2;
typedef struct { float x, y, z; } Vec3;

/* ================= API ================= */

void DrawInit(int width, int height);

/* NDC (-1 → 1) */
void Pixel(Vec2 p, Vec3 color);
void Line(Vec2 a, Vec2 b, Vec3 color);
void Triangle(Vec2 a, Vec2 b, Vec2 c, Vec3 color);
void Rect(Vec2 pos, Vec2 size, Vec3 color);

/* Pixel space (0 → width/height) */
void PixelP(Vec2 p, Vec3 color);
void LineP(Vec2 a, Vec2 b, Vec3 color);
void TriangleP(Vec2 a, Vec2 b, Vec2 c, Vec3 color);
void RectP(Vec2 pos, Vec2 size, Vec3 color);

#ifdef __cplusplus
}
#endif

/* ================= IMPLEMENTATION ================= */

#ifdef DRAW_IMPLEMENTATION

#include "glad/glad.h"

/* ================= Internal ================= */

static unsigned int d_vao;
static unsigned int d_vbo;
static unsigned int d_shader;
static int d_colorLoc;

static int d_width;
static int d_height;

/* shaders */
static const char* d_vs =
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"void main(){ gl_Position = vec4(aPos, 0.0, 1.0); }";

static const char* d_fs =
"#version 330 core\n"
"uniform vec3 uColor;\n"
"out vec4 FragColor;\n"
"void main(){ FragColor = vec4(uColor, 1.0); }";

/* compile */
static unsigned int d_compile(const char* vs, const char* fs) {
    unsigned int v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vs, NULL);
    glCompileShader(v);

    unsigned int f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fs, NULL);
    glCompileShader(f);

    unsigned int p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);

    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

/* ndc conversion */
static Vec2 d_toNDC(Vec2 p) {
    return (Vec2){
        (p.x / d_width) * 2.0f - 1.0f,
        1.0f - (p.y / d_height) * 2.0f
    };
}

static void d_draw(float* verts, int count, unsigned int mode) {
    glBindVertexArray(d_vao);
    glBindBuffer(GL_ARRAY_BUFFER, d_vbo);
    glBufferData(GL_ARRAY_BUFFER, count * 2 * sizeof(float), verts, GL_DYNAMIC_DRAW);
    glDrawArrays(mode, 0, count);
}

/* ================= Init ================= */

void DrawInit(int width, int height) {
    d_width = width;
    d_height = height;

    d_shader = d_compile(d_vs, d_fs);

    glGenVertexArrays(1, &d_vao);
    glGenBuffers(1, &d_vbo);

    glBindVertexArray(d_vao);
    glBindBuffer(GL_ARRAY_BUFFER, d_vbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    d_colorLoc = glGetUniformLocation(d_shader, "uColor");
}

/* ================= NDC Drawing ================= */

void PixelO(Vec2 p, Vec3 c) {
    float v[] = { p.x, p.y };
    glUseProgram(d_shader);
    glUniform3f(d_colorLoc, c.x, c.y, c.z);
    glPointSize(1.0f);
    d_draw(v, 1, GL_POINTS);
}

void LineO(Vec2 a, Vec2 b, Vec3 c) {
    float v[] = { a.x, a.y, b.x, b.y };
    glUseProgram(d_shader);
    glUniform3f(d_colorLoc, c.x, c.y, c.z);
    d_draw(v, 2, GL_LINES);
}

void TriangleO(Vec2 a, Vec2 b, Vec2 c, Vec3 col) {
    float v[] = { a.x,a.y, b.x,b.y, c.x,c.y };
    glUseProgram(d_shader);
    glUniform3f(d_colorLoc, col.x, col.y, col.z);
    d_draw(v, 3, GL_TRIANGLES);
}

void RectO(Vec2 pos, Vec2 size, Vec3 col) {
    float x = pos.x, y = pos.y, w = size.x, h = size.y;
    float v[] = {
        x,y, x+w,y, x+w,y+h,
        x,y, x+w,y+h, x,y+h
    };
    glUseProgram(d_shader);
    glUniform3f(d_colorLoc, col.x, col.y, col.z);
    d_draw(v, 6, GL_TRIANGLES);
}

/* ================= Pixel Drawing ================= */

void Pixel(Vec2 p, Vec3 colour) {
    PixelO(d_toNDC(p), colour);
}

void Line(Vec2 a, Vec2 b, Vec3 colour) {
    LineO(d_toNDC(a), d_toNDC(b), colour);
}

void Triangle(Vec2 a, Vec2 b, Vec2 c, Vec3 colour) {
    TriangleO(d_toNDC(a), d_toNDC(b), d_toNDC(c), colour);
}

void Rect(Vec2 pos, Vec2 size, Vec3 colour) { //Size is WIDTH, HEIGHT
    Vec2 p1 = d_toNDC(pos);
    Vec2 p2 = d_toNDC((Vec2){pos.x + size.x, pos.y + size.y});

    Vec2 sizeNDC = { p2.x - p1.x, p2.y - p1.y };
    RectO(p1, sizeNDC, colour);
}

#endif /* DRAW_IMPLEMENTATION */

#endif /* DRAW_H */
