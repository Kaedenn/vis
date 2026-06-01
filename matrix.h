#ifndef VIS_MATRIX_HEADER_INCLUDED_
#define VIS_MATRIX_HEADER_INCLUDED_ 1

void mat4_identity(float* out);
void mat4_multiply(float* out,
                   const float* a,
                   const float* b);
void mat4_ortho(float* out,
                float left, float right,
                float bottom, float top,
                float near, float far);
void mat4_look_at(float* out,
                  float ex, float ey, float ez,
                  float cx, float cy, float cz,
                  float ux, float uy, float uz);

#endif
