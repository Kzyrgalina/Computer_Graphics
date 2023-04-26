
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "math_3d.h"

GLuint VBO;
GLuint gWorldLocation;


static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
                                                                                    \n\
uniform mat4 gWorld;                                                                \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWorld * vec4(Position, 1.0);                                     \n\
}";

static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);                                           \n\
}";

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT); // Это очистка буфера кадра

    static float Scale = 0.0f;

    Scale += 0.003f; //увелисиваем значение синуса

    Matrix4f World; // Матрица для перемещения

    World.m[0][0] = 1.0f; World.m[0][1] = 0.0f; World.m[0][2] = 0.0f; World.m[0][3] = sinf(Scale);
    World.m[1][0] = 0.0f; World.m[1][1] = 1.0f; World.m[1][2] = 0.0f; World.m[1][3] = 0.0f;
    World.m[2][0] = 0.0f; World.m[2][1] = 0.0f; World.m[2][2] = 1.0f; World.m[2][3] = 0.0f;     
    World.m[3][0] = 0.0f; World.m[3][1] = 0.0f; World.m[3][2] = 0.0f; World.m[3][3] = 1.0f;

    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &World.m[0][0]);

    glEnableVertexAttribArray(0); // Координаты вершин, используемые в буфере, рассматриваются как атрибут вершины с индексом 0
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Параметр GL_ARRAY_BUFFER означает, что буфер будет хранить массив вершин для отрисовки 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // Вызов говорит конвейеру как воспринимать данные внутри буфера

    glDrawArrays(GL_TRIANGLES, 0, 3); // Функция для отрисовки (для тругольника содержит 3 вершины)

    glDisableVertexAttribArray(0); // Отключает каждый атрибут вершины, как только отпадает необходимость в нем

    glutSwapBuffers(); // Меняет местами буферы текущего окна при двойной буферизации
}


static void InitializeGlutCallbacks() //Просит GLUT поменять фоновый буфер и буфер кадра местами
{
    glutDisplayFunc(RenderSceneCB); // Задает отображение обратного вызова для текущего окна
    glutIdleFunc(RenderSceneCB); // создание графических пользовательских интерфейсов и управления окнами и событиями
}

static void CreateVertexBuffer()
{
    // Массив из экземпляров структуры Vector3f заголовка math_3d.h
    Vector3f Vertices[3]; // треугольник
    Vertices[0] = Vector3f(-1.0f, -1.0f, 0.0f);
    Vertices[1] = Vector3f(1.0f, -1.0f, 0.0f);
    Vertices[2] = Vector3f(0.0f, 1.0f, 0.0f);

    glGenBuffers(1, &VBO); // функция для генерации объектов переменных типов
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // функция для генерации объектов переменных типов
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType); // создание шейдера

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    // Исходный код шейдера
    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);

    // Компилируем шейдер
    glCompileShader(ShaderObj); 
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj); // проверяем программу 
}

static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram(); //создание программного объекта

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER); // создаем шейдер
    AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER); // создаем шейдер

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram); // создаем испольняемую программу и связываем шейдеры
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glValidateProgram(ShaderProgram); // проверка корректности шейдерной программы
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram); // устанавливает шейдерную программу для отрисовки

    gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
    assert(gWorldLocation != 0xFFFFFFFF);
}

int main(int argc, char** argv)
{
    system("chcp 1251");

    glutInit(&argc, argv); // инициализируем GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); //(отрисовка будет происходить в фоновый буфер, в то время как другой буфер отображается)
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Tutorial 06");

    InitializeGlutCallbacks();

    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // (красный, зелёный, синий, альфа-канал)

    CreateVertexBuffer(); // Создает буфер вершин

    CompileShaders(); // шейдер

    glutMainLoop(); // Передаёт контроль GLUT'у


    return 0;
}
