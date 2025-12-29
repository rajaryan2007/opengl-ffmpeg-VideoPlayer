#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "include/video-rendering.hh"   


void* aligned_malloc(size_t size, size_t alignment) {
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#else
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0)
        return nullptr;
    return ptr;
#endif
}

void aligned_free(void* ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}


int main(int argc, const char** argv) {
    GLFWwindow* window;

    if (!glfwInit()) {
        printf("Couldn't init GLFW\n");
        return 1;
    }

  
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    VideoReaderState vr_state{};

    if (!video_reader_open(
        &vr_state,
        "kanemi3.mov"
    )) {
        printf("Couldn't open video file\n");
        return 1;
    }

    window = glfwCreateWindow(
        vr_state.width,
        vr_state.height,
        "Video Player",
        NULL,
        NULL
    );

    if (!window) {
        printf("Couldn't open window\n");
        return 1;
    }

    glfwMakeContextCurrent(window);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return 1;
    }

    GLuint tex_handle;
    glGenTextures(1, &tex_handle);
    glBindTexture(GL_TEXTURE_2D, tex_handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    constexpr int ALIGNMENT = 128;
    size_t frame_size = vr_state.width * vr_state.height * 4;

    uint8_t* frame_data = (uint8_t*)aligned_malloc(frame_size, ALIGNMENT);
    if (!frame_data) {
        printf("Failed to allocate frame buffer\n");
        return 1;
    }

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        int w, h;
        glfwGetWindowSize(window, &w, &h);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, h, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        int64_t pts;
        if (!video_reader_read_frame(&vr_state, frame_data, &pts)) {
            break;
        }

        static bool first = true;
        if (first) {
            glfwSetTime(0.0);
            first = false;
        }

        double t =
            pts *
            (double)vr_state.time_base.num /
            (double)vr_state.time_base.den;

        while (t > glfwGetTime()) {
            glfwWaitEventsTimeout(t - glfwGetTime());
        }

        glBindTexture(GL_TEXTURE_2D, tex_handle);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            vr_state.width,
            vr_state.height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            frame_data
        );

        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(0, 0);
        glTexCoord2f(1, 0); glVertex2f(vr_state.width, 0);
        glTexCoord2f(1, 1); glVertex2f(vr_state.width, vr_state.height);
        glTexCoord2f(0, 1); glVertex2f(0, vr_state.height);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    aligned_free(frame_data);
    video_reader_close(&vr_state);
    glfwTerminate();
    return 0;
}
