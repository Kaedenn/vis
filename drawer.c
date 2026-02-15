
#define _DEFAULT_SOURCE /* for setenv */

#include "defines.h"

#include "drawer.h"
#include "blender.h"
#include "emitter.h"
#include "genlua.h"
#include "helper.h"
#include "kstring.h"
#include "shader.h"
#include "types.h"
#include <math.h>
#include <time.h>

#include "3rdparty/stb_image_write.h"
/*#include "3rdparty/stb_easy_font.h"*/

static double calculate_blend(particle_t p);
static int render_to_file(drawer_t drawer, const char *path);

static void drawer_ensure_fps_linear(drawer_t drawer);
static void drawer_ensure_fps_absolute(drawer_t drawer);

/* Vertex structure for GPU (TODO: replace with geometry shader) */
typedef struct {
    GLfloat x, y;
    GLfloat radius;
    GLfloat r, g, b, a;
    GLuint depth;
} vertex_t;

/* used, obviously, for fps tracking and limiting */
struct fps {
    void (*limiter)(drawer_t drawer);
    uint32_t framecount;
    double start_time;
    double framestart_time;
    double last_fps;
};

struct drawer {
    GLFWwindow* window;
    GLuint vao;
    GLuint vbo;
    shader_t* shader;
    unsigned int wsize[2];
    int frames_per_second;

    vertex_t* vertex_array;
    size_t vertex_curr;
    size_t vertex_count;

    float bgcolor[3];
    uint32_t frame_skip;
    struct fps fps;
    BOOL tracing;
    emit_desc* trace_emit;
    BOOL verbose_trace;
    char* dump_file_fmt;
};

const GLchar* GEOM_SOURCE = "glsl/geom.glsl";
const GLchar* VERT_SOURCE = "glsl/vert.glsl";
const GLchar* FRAG_SOURCE = "glsl/frag.glsl";
const GLchar* COMPUTE_SOURCE = "glsl/compute.glsl";

void glfw_error_callback(int error, const char* description) {
    EPRINTF("GLFW Error (%d): %s\n", error, description);
}

drawer_t drawer_new(const clargs* args) {
    drawer_t drawer = DBMALLOC(sizeof(struct drawer));
    drawer_config(drawer, args);

    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        EPRINTF("%s\n", "GLFW initialization failed");
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    int winwidth = (int)drawer->wsize[0];
    int winheight = (int)drawer->wsize[1];
    drawer->window = glfwCreateWindow(winwidth, winheight, "Vis", NULL, NULL);
    if (!drawer->window) {
        EPRINTF("%s\n", "Failed to create GLFW window");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(drawer->window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        EPRINTF("%s\n", "Failed to initialize GLEW");
        return NULL;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Initialize shader */
    drawer->shader = shader_create(GEOM_SOURCE, VERT_SOURCE,
                                   FRAG_SOURCE, COMPUTE_SOURCE);
    if (!drawer->shader) {
        EPRINTF("%s\n", "Failed to compile shaders");
        return NULL;
    }

    /* Initialize VAO/VBO */
    glGenVertexArrays(1, &drawer->vao);
    glBindVertexArray(drawer->vao);

    glGenBuffers(1, &drawer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, drawer->vbo);

    /* Define VAO attributes */
    /* 0: position (vec2) */
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
        (GLvoid*)offsetof(vertex_t, x));
    glEnableVertexAttribArray(0);
    /* 1: radius (float) */
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
        (GLvoid*)offsetof(vertex_t, radius));
    glEnableVertexAttribArray(1);
    /* 2: color (vec4) */
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
        (GLvoid*)offsetof(vertex_t, r));
    glEnableVertexAttribArray(2);
    /* 3: depth (uint) */
    glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(vertex_t),
        (GLvoid*)offsetof(vertex_t, depth));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    const GLulong vbuffer_size = VIS_PLIST_INITIAL_SIZE * sizeof(vertex_t);

    /* initialize the vertex storage */
    drawer->vertex_curr = 0;
    drawer->vertex_count = VIS_PLIST_INITIAL_SIZE;
    drawer->vertex_array = DBMALLOC(vbuffer_size);

    /* Buffer initial data (empty) */
    glBindBuffer(GL_ARRAY_BUFFER, drawer->vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vbuffer_size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* initialize default values */
    drawer_bgcolor(drawer, 0, 0, 0);
    /* initialize fps analysis */
    drawer->fps.start_time = glfwGetTime();
    drawer->fps.framestart_time = drawer->fps.start_time;
    drawer->fps.limiter = drawer_ensure_fps_linear;

    /* Store drawer in window user pointer for callbacks if needed */
    glfwSetWindowUserPointer(drawer->window, drawer);

    return drawer;
}

void drawer_free(drawer_t drawer) {
    double runtime = glfwGetTime() - drawer->fps.start_time;
    double fc_want = runtime * drawer->frames_per_second;
    double fc_have = (double)drawer->fps.framecount;
    DBPRINTF("%s", "fps analysis:");
    DBPRINTF("S=%g, F=%g, F/S=%g", runtime, fc_have, fc_have / runtime);
    DBPRINTF("frame error:   (S*FPS-F) %g frames (%g seconds)", fc_want - fc_have,
        (fc_want - fc_have) / drawer->frames_per_second);
    DBPRINTF("error ratio: 1-(S*FPS/F) %g", 1 - fc_want / fc_have);

    DZFREE(drawer->vertex_array);
    if (drawer->dump_file_fmt) {
        DZFREE(drawer->dump_file_fmt);
    }

    glDeleteVertexArrays(1, &drawer->vao);
    glDeleteBuffers(1, &drawer->vbo);
    shader_free(drawer->shader);

    glfwDestroyWindow(drawer->window);
    glfwTerminate();

    DZFREE(drawer->trace_emit);
    DZFREE(drawer);
}

GLFWwindow* drawer_get_window(drawer_t drawer) {
    return drawer->window;
}

void drawer_bgcolor(drawer_t drawer, float r, float g, float b) {
    if (r >= 0) drawer->bgcolor[0] = r;
    if (g >= 0) drawer->bgcolor[1] = g;
    if (b >= 0) drawer->bgcolor[2] = b;
}

int drawer_add_particle(drawer_t drawer, particle_t p) {
    if (drawer->vertex_curr < drawer->vertex_count) {
        vertex_t* v = &drawer->vertex_array[drawer->vertex_curr];

        v->x = 2 * ((GLfloat)p->x / (GLfloat)drawer->wsize[0] - 0.5f);
        v->y = 2 * (0.5f - (GLfloat)p->y / (GLfloat)drawer->wsize[1]);
        v->radius = (GLfloat)p->radius;

        v->r = (GLfloat)p->r;
        v->g = (GLfloat)p->g;
        v->b = (GLfloat)p->b;
        v->a = (GLfloat)sqrt(calculate_blend(p));
        v->depth = (GLuint)p->depth;

        drawer->vertex_curr += 1;
        return 0;
    } else {
        EPRINTF("can't add more than %lu particles, did you call "
                "drawer_draw_to_screen?",
            (unsigned long)drawer->vertex_count);
        return 1;
    }
}

int drawer_draw_to_screen(drawer_t drawer) {
    /*static char text_buffer[0x100000] = {0};*/

    glClearColor(drawer->bgcolor[0], drawer->bgcolor[1], drawer->bgcolor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(drawer->shader);
    glBindVertexArray(drawer->vao);

    /* Update VBO */
    glBindBuffer(GL_ARRAY_BUFFER, drawer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        (GLsizeiptr)(drawer->vertex_curr * sizeof(vertex_t)), drawer->vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_POINTS, 0, (GLsizei)drawer->vertex_curr);

    glBindVertexArray(0);

    if (drawer->dump_file_fmt) {
        if (drawer->fps.framecount >= drawer->frame_skip) {
            kstr s = kstring_newfromvf("%s_%04d.png", drawer->dump_file_fmt,
                drawer->fps.framecount - drawer->frame_skip);
            if (!render_to_file(drawer, kstring_content(s))) {
                EPRINTF("Failed to dump frame %s", kstring_content(s));
            }
            DZFREE(s);
        }
    }

    /*
    char text[] = "Test";
    int num_quads = stb_easy_font_print(0, 0, text, NULL, text_buffer, sizeof(text_buffer));
    DBPRINTF("Printing %d quads for %s", num_quads, text);
    for (int i = 0; i < 4*num_quads; i += 4) {
        DBPRINTF("quad[%d] = %d %d %d %d", i, text_buffer[i], text_buffer[i+1], text_buffer[i+2], text_buffer[i+3]);
    }
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, text_buffer);
    glDrawArrays(GL_QUADS, 0, num_quads*4);
    glDisableClientState(GL_VERTEX_ARRAY);
    */

    glfwSwapBuffers(drawer->window);
    /* glfwPollEvents is called in driver.c */

    drawer->vertex_curr = 0;

    if (drawer->fps.framecount >= drawer->frame_skip) {
        drawer->fps.limiter(drawer);
    }
    drawer->fps.framecount += 1;
    drawer->fps.framestart_time = glfwGetTime();
    return 0;
}

void drawer_preserve_screen(drawer_t drawer) {
    glfwSwapBuffers(drawer->window);
    if (!drawer->dump_file_fmt) {
        if (drawer->fps.framecount >= drawer->frame_skip) {
            drawer->fps.limiter(drawer);
        }
    }
    drawer->fps.framecount += 1;
    drawer->fps.framestart_time = glfwGetTime();
}

void drawer_ensure_fps_linear(drawer_t drawer) {
    double frameend = glfwGetTime();
    double framedelay = frameend - drawer->fps.framestart_time;
    double target_delay = 1.0 / drawer->frames_per_second;

    if (framedelay < target_delay) {
        double sleep_sec = target_delay - framedelay;
        if (sleep_sec > 0) {
            struct timespec req;
            req.tv_sec = (time_t)sleep_sec;
            req.tv_nsec = (long)((sleep_sec - (double)req.tv_sec) * 1e9);
            nanosleep(&req, NULL);
        }
    }
}

void drawer_ensure_fps_absolute(drawer_t drawer) {
    double frametime =
        drawer->fps.start_time + (double)drawer->fps.framecount / drawer->frames_per_second;
    double now = glfwGetTime();
    if (now < frametime) {
        double sleep_sec = frametime - now;
        if (sleep_sec > 0) {
            struct timespec req;
            req.tv_sec = (time_t)sleep_sec;
            req.tv_nsec = (long)((sleep_sec - (double)req.tv_sec) * 1e9);
            nanosleep(&req, NULL);
        }
    }
}

int drawer_get_configured_fps(drawer_t drawer) {
    return drawer->frames_per_second;
}

float drawer_get_fps(drawer_t drawer) {
    return (float)((double)(drawer->fps.framecount - 1) /
                   (glfwGetTime() - drawer->fps.start_time));
}

void drawer_config(drawer_t drawer, const clargs* args) {
    drawer->frames_per_second = args->frames_per_second;
    drawer->frame_skip = (uint32_t)args->frameskip;
    drawer->verbose_trace = args->dumptrace ? TRUE : FALSE;
    if (args->dumpfile) {
        drawer_set_dumpfile_template(drawer, args->dumpfile);
    }
    if (args->absolute_fps) {
        drawer->fps.limiter = drawer_ensure_fps_absolute;
    }
    if (drawer->frame_skip > 0) {
        DBPRINTF("\tdrawer->frame_skip = %d", drawer->frame_skip);
    }
    if (drawer->verbose_trace) {
        DBPRINTF("\t%s", "drawer->verbose_trace = TRUE");
    }
    drawer->wsize[0] = args->wsize[0];
    drawer->wsize[1] = args->wsize[1];
}

void drawer_set_dumpfile_template(drawer_t drawer, const char* path) {
    if (drawer->dump_file_fmt) {
        DZFREE(drawer->dump_file_fmt);
    }
    drawer->dump_file_fmt = strdup(path);
}

void drawer_set_trace(drawer_t drawer, emit_desc* emit) {
    drawer->trace_emit = emit;
}

emit_desc* drawer_get_trace(drawer_t drawer) {
    return drawer->trace_emit;
}

void drawer_begin_trace(drawer_t drawer) {
    drawer->tracing = TRUE;
}

void drawer_trace(drawer_t drawer, float x, float y) {
    if (!drawer->tracing || !drawer->trace_emit)
        return;

    drawer->trace_emit->x = (double)x;
    drawer->trace_emit->y = (double)y;
    emit_frame(drawer->trace_emit);
    if (drawer->verbose_trace) {
        char* line = genlua_emit(drawer->trace_emit, drawer->fps.framecount);
        printf("%s\n", line);
        DZFREE(line);
    }
}

void drawer_end_trace(drawer_t drawer) {
    drawer->tracing = FALSE;
}

void drawer_trace_scroll(drawer_t drawer, UNUSED_PARAM(float xoffset), float yoffset) {
    if (yoffset > 0) {
        drawer->trace_emit->rad += 1;
        drawer->trace_emit->ds *= 1.5f;
    } else if (yoffset < 0) {
        drawer->trace_emit->rad -= 1;
        if (drawer->trace_emit->rad <= 0) {
            drawer->trace_emit->rad = 1;
        }
        drawer->trace_emit->ds /= 1.5f;
        if (drawer->trace_emit->ds < 0.2f) {
            drawer->trace_emit->ds = 0.2f;
        }
    }
}

double calculate_blend(particle_t p) {
    double alpha = p->a;
    double life = particle_get_life(p);
    double lifetime = particle_get_lifetime(p);
    if (p->blender >= VIS_BLEND_NONE && p->blender < VIS_NBLENDS) {
        alpha *= blend_fns[p->blender](life, lifetime);
    }
    return alpha;
}

BOOL render_to_file(drawer_t drawer, const char *path) {
    int width = (int)drawer->wsize[0];
    int height = (int)drawer->wsize[1];
    int stride = width * 4;
    /* allocate buffer for RGBA */
    unsigned char* buffer = DBMALLOC((size_t)stride * (size_t)height);

    if (!buffer) {
        EPRINTF("Failed to allocate buffer to write %s", path);
        return FALSE;
    }

    /* glReadPixels reads starting from bottom-left */
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    /* flip vertically because OpenGL origin is bottom-left, but image origin is top-left */
    stbi_flip_vertically_on_write(1);
    
    if (!stbi_write_png(path, width, height, 4, buffer, stride)) {
        EPRINTF("Failed to write image to %s", path);
        DZFREE(buffer);
        return FALSE;
    }

    DZFREE(buffer);
    return TRUE;
}
