#ifndef VIS_SHADER_HEADER_INCLUDED_
#define VIS_SHADER_HEADER_INCLUDED_ 1

#include "helper.h"
#include <GL/glew.h>

typedef struct shader {
    GLchar* geom_path;
    GLchar* vertex_path;
    GLchar* fragment_path;

    GLuint shader_id;
} shader_t;

#endif

shader_t* shader_new(const GLchar* geom_path, const GLchar* vertex_path,
                     const GLchar* fragment_path);
BOOL shader_load(shader_t* shader);
void shader_free(shader_t* shader);

void shader_use(const shader_t* shader);

GLint shader_get_uniform(const shader_t* shader, const GLchar* variable_name);

