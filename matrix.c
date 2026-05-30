#include "defines.h"
#include "matrix.h"
#include <math.h>
#include <string.h>

void mat4_identity(float* out) {
    memset(out, 0, 16 * sizeof(float));
    out[0] = 1.0f;
    out[5] = 1.0f;
    out[10] = 1.0f;
    out[15] = 1.0f;
}

void mat4_multiply(float* out, const float* a, const float* b) {
    float temp[16];
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            temp[col * 4 + row] = a[0 * 4 + row] * b[col * 4 + 0] +
                                  a[1 * 4 + row] * b[col * 4 + 1] +
                                  a[2 * 4 + row] * b[col * 4 + 2] +
                                  a[3 * 4 + row] * b[col * 4 + 3];
        }
    }
    memcpy(out, temp, 16 * sizeof(float));
}

void mat4_ortho(float* out, float left, float right, float bottom, float top, float near, float far) {
    mat4_identity(out);
    out[0] = 2.0f / (right - left);
    out[5] = 2.0f / (top - bottom);
    out[10] = -2.0f / (far - near);
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[14] = -(far + near) / (far - near);
}

static void vec3_normalize(float* v) {
    float length = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (length > 0.0f) {
        v[0] /= length;
        v[1] /= length;
        v[2] /= length;
    }
}

static void vec3_cross(float* out, const float* a, const float* b) {
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

void mat4_look_at(float* out, float ex, float ey, float ez, float cx, float cy, float cz, float ux, float uy, float uz) {
    float f[3] = { cx - ex, cy - ey, cz - ez };
    vec3_normalize(f);

    float u[3] = { ux, uy, uz };
    vec3_normalize(u);

    float s[3];
    vec3_cross(s, f, u);
    vec3_normalize(s);

    float u_calc[3];
    vec3_cross(u_calc, s, f);

    mat4_identity(out);
    out[0] = s[0];
    out[4] = s[1];
    out[8] = s[2];
    
    out[1] = u_calc[0];
    out[5] = u_calc[1];
    out[9] = u_calc[2];
    
    out[2] = -f[0];
    out[6] = -f[1];
    out[10] = -f[2];

    out[12] = -(s[0] * ex + s[1] * ey + s[2] * ez);
    out[13] = -(u_calc[0] * ex + u_calc[1] * ey + u_calc[2] * ez);
    out[14] = f[0] * ex + f[1] * ey + f[2] * ez;
}
