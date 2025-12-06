
#include "shader.h"
#include "helper.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

GLuint compile_shader(const GLchar *shader_path, GLenum shader_type) {
  FILE *fobj = fopen(shader_path, "r");
  if (!fobj) {
    EPRINTF("Error %d opening \"%s\": %s\n", errno, strerror(errno),
            shader_path);
    return (GLuint)-1;
  }

  size_t bufsize = 4096;
  char *buffer = stralloc(bufsize);
  char line[1024] = {0};
  while (!feof(fobj)) {
    fgets(line, sizeof(line), fobj);
    if (ferror(fobj)) {
      EPRINTF("read(\"%s\") error %d %s\n", shader_path, errno,
              strerror(errno));
      fclose(fobj);
      free(buffer);
      return (GLuint)-1;
    }
    allocat(buffer, line, &bufsize);
  }
  fclose(fobj);

  GLuint shader_program = glCreateShader(shader_type);
  glShaderSource(shader_program, 1, (const GLchar *const *)&buffer, NULL);
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

shader_t *shader_create(const GLchar *vertex_path, const GLchar *fragment_path,
                        const GLchar *compute_path) {
  shader_t *shader = (shader_t *)malloc(sizeof(shader_t));
  shader->vertex_path = stralloc(strlen(vertex_path) + 1);
  shader->fragment_path = stralloc(strlen(fragment_path) + 1);
  shader->compute_path = stralloc(strlen(compute_path) + 1);
  shader->shader_id = 0;
  strcpy(shader->vertex_path, vertex_path);
  strcpy(shader->fragment_path, fragment_path);
  strcpy(shader->compute_path, compute_path);

  GLuint vertex_shader_id = compile_shader(vertex_path, GL_VERTEX_SHADER);
  if (vertex_shader_id == (GLuint)-1) {
    shader_free(shader);
    return NULL;
  }
  GLuint fragment_shader_id = compile_shader(fragment_path, GL_FRAGMENT_SHADER);
  if (fragment_shader_id == (GLuint)-1) {
    glDeleteShader(vertex_shader_id);
    shader_free(shader);
    return NULL;
  }
  GLuint compute_shader_id = compile_shader(compute_path, GL_COMPUTE_SHADER);
  if (compute_shader_id == (GLuint)-1) {
    glDeleteShader(vertex_shader_id);
    shader_free(shader);
    return NULL;
  }

  GLuint vert_frag_program = glCreateProgram();
  glAttachShader(vert_frag_program, vertex_shader_id);
  glAttachShader(vert_frag_program, fragment_shader_id);
  glLinkProgram(vert_frag_program);

  GLuint compute_program = glCreateProgram();
  glAttachShader(compute_program, compute_shader_id);
  glLinkProgram(compute_program);

  shader->shader_id = vert_frag_program;
  shader->compute_shader_id = compute_program;

  glDeleteShader(vertex_shader_id);
  glDeleteShader(fragment_shader_id);
  glDeleteShader(compute_shader_id);

  GLint success;
  glGetProgramiv(vert_frag_program, GL_LINK_STATUS, &success);
  if (!success) {
    char info_log[1024] = {0};
    glGetProgramInfoLog(vert_frag_program, sizeof(info_log), NULL, info_log);
    EPRINTF("Linking shaders failed: %s\n", info_log);
    shader_free(shader);
    return NULL;
  }

  glGetProgramiv(compute_program, GL_LINK_STATUS, &success);
  if (!success) {
    char info_log[1024] = {0};
    glGetProgramInfoLog(vert_frag_program, sizeof(info_log), NULL, info_log);
    EPRINTF("Linking shaders failed: %s\n", info_log);
    shader_free(shader);
    return NULL;
  }

  return shader;
}

void shader_free(shader_t *shader) {
  free(shader->vertex_path);
  free(shader->fragment_path);
  free(shader->compute_path);
  if (shader->shader_id != 0) {
    glDeleteProgram(shader->shader_id);
  }
  if (shader->compute_shader_id != 0) {
    glDeleteProgram(shader->compute_shader_id);
  }
  free(shader);
}

void shader_use(const shader_t *shader) { glUseProgram(shader->shader_id); }

void shader_compute_use(const shader_t *shader) {
  glUseProgram(shader->compute_shader_id);
}

GLint shader_get_uniform(const shader_t *shader, const GLchar *variable_name) {
  return glGetUniformLocation(shader->shader_id, variable_name);
}

GLint shader_compute_get_uniform(const shader_t *shader,
                                 const GLchar *variable_name) {
  return glGetUniformLocation(shader->compute_shader_id, variable_name);
}
