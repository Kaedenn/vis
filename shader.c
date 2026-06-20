
#include "defines.h"
#include "shader.h"
#include "helper.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glew.h>

static char* read_file_full(const char* file_path) {
    FILE* fobj = fopen(file_path, "r");
    if (!fobj) {
        EPRINTF("Error %d opening \"%s\": %s\n", errno, strerror(errno), file_path);
        return NULL;
    }

    size_t bufsize = 4096;
    char* buffer = stralloc(bufsize);
    char line[1024] = {0};
    while (fgets(line, sizeof(line), fobj) != NULL) {
        allocat(buffer, line, &bufsize);
    }
    if (ferror(fobj)) {
        EPRINTF("read(\"%s\") error %d %s\n", file_path, errno, strerror(errno));
        fclose(fobj);
        free(buffer);
        return NULL;
    }
    fclose(fobj);
    return buffer;
}

GLuint compile_shader(const GLchar* shader_path, GLenum shader_type) {
    char* buffer = read_file_full(shader_path);
    if (!buffer) {
        return (GLuint)-1;
    }

    GLuint shader_program = glCreateShader(shader_type);
    glShaderSource(shader_program, 1, (const GLchar* const*)&buffer, NULL);
    glCompileShader(shader_program);
    free(buffer);

    GLint success;
    glGetShaderiv(shader_program, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[1024] = {0};
        glGetShaderInfoLog(shader_program, sizeof(info_log), NULL, info_log);
        EPRINTF("Compiling \"%s\" failed: %s\n", shader_path, info_log);
        glDeleteShader(shader_program);
        return (GLuint)-1;
    }

    return shader_program;
}

shader_t* shader_new(const GLchar* geom_path, const GLchar* vertex_path,
                     const GLchar* fragment_path) {
    shader_t* shader = DBMALLOC(sizeof(shader_t));
    shader->geom_path = dupstr(geom_path);
    shader->vertex_path = dupstr(vertex_path);
    shader->fragment_path = dupstr(fragment_path);
    shader->shader_id = 0;
    return shader;
}

BOOL shader_load(shader_t* shader) {
    GLuint geom_shader_id = compile_shader(shader->geom_path, GL_GEOMETRY_SHADER);
    if (geom_shader_id == (GLuint)-1) {
        return FALSE;
    }
    GLuint vertex_shader_id = compile_shader(shader->vertex_path, GL_VERTEX_SHADER);
    if (vertex_shader_id == (GLuint)-1) {
        glDeleteShader(geom_shader_id);
        return FALSE;
    }
    GLuint fragment_shader_id = compile_shader(shader->fragment_path, GL_FRAGMENT_SHADER);
    if (fragment_shader_id == (GLuint)-1) {
        glDeleteShader(geom_shader_id);
        glDeleteShader(vertex_shader_id);
        return FALSE;
    }
    GLuint vertex_program = glCreateProgram();
    glAttachShader(vertex_program, geom_shader_id);
    glAttachShader(vertex_program, vertex_shader_id);
    glAttachShader(vertex_program, fragment_shader_id);
    glLinkProgram(vertex_program);

    shader->shader_id = vertex_program;

    glDeleteShader(geom_shader_id);
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    GLint success;
    glGetProgramiv(vertex_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[1024] = {0};
        glGetProgramInfoLog(vertex_program, sizeof(info_log), NULL, info_log);
        EPRINTF("Linking shaders failed: %s\n", info_log);
        shader->shader_id = 0;
        glDeleteProgram(vertex_program);
        return FALSE;
    }

    return TRUE;
}

void shader_free(shader_t* shader) {
    free(shader->geom_path);
    free(shader->vertex_path);
    free(shader->fragment_path);
    if (shader->shader_id != 0) {
        glDeleteProgram(shader->shader_id);
    }
    free(shader);
}

void shader_use(const shader_t* shader) {
    glUseProgram(shader->shader_id);
}

GLint shader_get_uniform(const shader_t* shader, const GLchar* variable_name) {
    return glGetUniformLocation(shader->shader_id, variable_name);
}