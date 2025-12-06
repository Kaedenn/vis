#ifndef VIS_SHADER_HEADER_INCLUDED_
#define VIS_SHADER_HEADER_INCLUDED_ 1

#include <GL/glew.h>
#include <string.h>

#include "memory.h"

typedef struct shader {
    GLchar* vertex_path;
    GLchar* fragment_path;
    GLchar* compute_path;

    GLuint shader_id;
    GLuint compute_shader_id;
} shader_t;

#endif

shader_t* shader_create(
        const GLchar* vertex_path,
        const GLchar* fragment_path,
        const GLchar* compute_path);
void shader_free(shader_t* shader);

void shader_use(const shader_t* shader);
void shader_compute_use(const shader_t* shader);

GLint shader_get_uniform(const shader_t* shader, const GLchar* variable_name);
GLint shader_compute_get_uniform(const shader_t* shader, const GLchar* variable_name);
