#ifndef OPENDEMO_RENDER_GL_H
#define OPENDEMO_RENDER_GL_H

#include "render/render.h"

#ifdef OS_WIN
    #include <gl/GL.h>
    #include <gl/glext.h>
#endif

namespace Render {

#if defined(OS_WIN) || defined(OS_LINUX) || defined(OS_ANDROID)

    #ifdef OS_ANDROID
        #define GetProc(x) dlsym(libGL, x);
    #else
        void* GetProc(const char *name) {
            #ifdef OS_WIN
                return (void*)wglGetProcAddress(name);
            #elif OS_LINUX
                return (void*)glXGetProcAddress((GLubyte*)name);
            #else // EGL
                return (void*)eglGetProcAddress(name);
            #endif
        }
    #endif

    #define GetProcOGL(x) x=(decltype(x))GetProc(#x);
    
    // Texture
    #ifdef OS_WIN
        PFNGLACTIVETEXTUREPROC              glActiveTexture;
    #endif
    // VSync
    #ifdef OS_WIN
        typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
    #elif OS_LINUX
        typedef int (*PFNGLXSWAPINTERVALSGIPROC) (int interval);
        PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI;
    #endif

    #if defined(OS_WIN) || defined(OS_LINUX)
        PFNGLGENERATEMIPMAPPROC             glGenerateMipmap;
    // Profiling
        #ifdef PROFILE
            PFNGLOBJECTLABELPROC                glObjectLabel;
            PFNGLPUSHDEBUGGROUPPROC             glPushDebugGroup;
            PFNGLPOPDEBUGGROUPPROC              glPopDebugGroup;
            PFNGLGENQUERIESPROC                 glGenQueries;
            PFNGLDELETEQUERIESPROC              glDeleteQueries;
            PFNGLGETQUERYOBJECTIVPROC           glGetQueryObjectiv;
            PFNGLBEGINQUERYPROC                 glBeginQuery;
            PFNGLENDQUERYPROC                   glEndQuery;
        #endif
    // Shader
        PFNGLCREATEPROGRAMPROC              glCreateProgram;
        PFNGLDELETEPROGRAMPROC              glDeleteProgram;
        PFNGLLINKPROGRAMPROC                glLinkProgram;
        PFNGLUSEPROGRAMPROC                 glUseProgram;
        PFNGLGETPROGRAMINFOLOGPROC          glGetProgramInfoLog;
        PFNGLCREATESHADERPROC               glCreateShader;
        PFNGLDELETESHADERPROC               glDeleteShader;
        PFNGLSHADERSOURCEPROC               glShaderSource;
        PFNGLATTACHSHADERPROC               glAttachShader;
        PFNGLCOMPILESHADERPROC              glCompileShader;
        PFNGLGETSHADERINFOLOGPROC           glGetShaderInfoLog;
        PFNGLGETUNIFORMLOCATIONPROC         glGetUniformLocation;
        PFNGLUNIFORM1IVPROC                 glUniform1iv;
        PFNGLUNIFORM1FVPROC                 glUniform1fv;
        PFNGLUNIFORM2FVPROC                 glUniform2fv;
        PFNGLUNIFORM3FVPROC                 glUniform3fv;
        PFNGLUNIFORM4FVPROC                 glUniform4fv;
        PFNGLUNIFORMMATRIX4FVPROC           glUniformMatrix4fv;
        PFNGLBINDATTRIBLOCATIONPROC         glBindAttribLocation;
        PFNGLENABLEVERTEXATTRIBARRAYPROC    glEnableVertexAttribArray;
        PFNGLDISABLEVERTEXATTRIBARRAYPROC   glDisableVertexAttribArray;
        PFNGLVERTEXATTRIBPOINTERPROC        glVertexAttribPointer;
        PFNGLGETPROGRAMIVPROC               glGetProgramiv;
    // Render to texture
        PFNGLGENFRAMEBUFFERSPROC            glGenFramebuffers;
        PFNGLBINDFRAMEBUFFERPROC            glBindFramebuffer;
        PFNGLGENRENDERBUFFERSPROC           glGenRenderbuffers;
        PFNGLBINDRENDERBUFFERPROC           glBindRenderbuffer;
        PFNGLFRAMEBUFFERTEXTURE2DPROC       glFramebufferTexture2D;
        PFNGLFRAMEBUFFERRENDERBUFFERPROC    glFramebufferRenderbuffer;
        PFNGLRENDERBUFFERSTORAGEPROC        glRenderbufferStorage;
        PFNGLCHECKFRAMEBUFFERSTATUSPROC     glCheckFramebufferStatus;
        PFNGLDELETEFRAMEBUFFERSPROC         glDeleteFramebuffers;
        PFNGLDELETERENDERBUFFERSPROC        glDeleteRenderbuffers;
    // Mesh
        PFNGLGENBUFFERSARBPROC              glGenBuffers;
        PFNGLDELETEBUFFERSARBPROC           glDeleteBuffers;
        PFNGLBINDBUFFERARBPROC              glBindBuffer;
        PFNGLBUFFERDATAARBPROC              glBufferData;
        PFNGLBUFFERSUBDATAARBPROC           glBufferSubData;
    #endif

    PFNGLGENVERTEXARRAYSPROC            glGenVertexArrays;
    PFNGLDELETEVERTEXARRAYSPROC         glDeleteVertexArrays;
    PFNGLBINDVERTEXARRAYPROC            glBindVertexArray;
    PFNGLGETPROGRAMBINARYPROC           glGetProgramBinary;
    PFNGLPROGRAMBINARYPROC              glProgramBinary;
#endif

#if defined(_RENDER_GLES) && !defined(_OS_RPI) && !defined(_OS_CLOVER) && !defined(_OS_IOS)
    PFNGLDISCARDFRAMEBUFFEREXTPROC      glDiscardFramebufferEXT;
#endif


    GLuint FBO, defaultFBO;
    struct RenderTargetCache {
        int count;
        struct Item {
            GLuint  ID;
            int     width;
            int     height;
        } items[MAX_RENDER_BUFFERS];
    } rtCache[2];

    struct Texture {
        uint32_t ID;
        int width, height, origWidth, origHeight;
        TexFormat fmt;
        uint32_t opt;

        Texture(int width, int height, uint32_t opt) : ID(0), width(width), height(height), origWidth(width),
                                                     origHeight(height), fmt(FMT_RGBA), opt(opt) {}

        void init(void *data) {
            ASSERT((opt & OPT_PROXY) == 0);

            bool filter = (opt & OPT_NEAREST) == 0;
            bool mipmaps = (opt & OPT_MIPMAPS) != 0;
            bool cube = (opt & OPT_CUBEMAP) != 0;
            bool isShadow = fmt == FMT_SHADOW;

            glGenTextures(1, &ID);
            bind(0);

            GLenum target = cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;

            if (fmt == FMT_SHADOW) {
                glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
            }

            bool border = isShadow && support.texBorder;
            glTexParameteri(target, GL_TEXTURE_WRAP_S,
                            (opt & OPT_REPEAT) ? GL_REPEAT : (border ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE));
            glTexParameteri(target, GL_TEXTURE_WRAP_T,
                            (opt & OPT_REPEAT) ? GL_REPEAT : (border ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE));
            if (border) {
                float color[] = {1.0f, 1.0f, 1.0f, 1.0f};
                glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);
            }

            glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
                            filter ? (mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR) : (mipmaps
                                                                                        ? GL_NEAREST_MIPMAP_NEAREST
                                                                                        : GL_NEAREST));
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter ? GL_LINEAR : GL_NEAREST);

            static const struct FormatDesc {
                GLuint ifmt, fmt;
                GLenum type;
            } formats[FMT_MAX] = {
                    {GL_LUMINANCE,       GL_LUMINANCE,       GL_UNSIGNED_BYTE}, // LUMINANCE
                    {GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE}, // RGBA
                    {GL_RGB,             GL_RGB,             GL_UNSIGNED_SHORT_5_6_5}, // RGB16
                    {GL_RGBA,            GL_RGBA,            GL_UNSIGNED_SHORT_5_5_5_1}, // RGBA16
                    {GL_RGBA32F,         GL_RGBA,            GL_FLOAT}, // RGBA_FLOAT
                    {GL_RGBA16F,         GL_RGBA,            GL_HALF_FLOAT}, // RGBA_HALF
                    {GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT}, // DEPTH
                    {GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT}, // SHADOW
            };

            FormatDesc desc = formats[fmt];

#ifdef _OS_WEB // fucking firefox!
            if (fmt == FMT_RGBA_FLOAT) {
                        if (Core::support.texFloat) {
                            desc.ifmt = GL_RGBA;
                            desc.type = GL_FLOAT;
                        }
                    }

                    if (fmt == FMT_RGBA_HALF) {
                        if (Core::support.texHalf) {
                            desc.ifmt = GL_RGBA;
                            desc.type = GL_HALF_FLOAT_OES;
                        }
                    }
#else
            if ((fmt == FMT_RGBA_FLOAT && !support.colorFloat) ||
                (fmt == FMT_RGBA_HALF && !support.colorHalf)) {
                desc.ifmt = GL_RGBA;
#ifdef _RENDER_GLES
                if (fmt == FMT_RGBA_HALF)
                                desc.type = GL_HALF_FLOAT_OES;
#endif
            }
#endif

            void *pix = data;
            if (data && !support.texNPOT && (width != origWidth || height != origHeight))
                pix = NULL;

            for (int i = 0; i < 6; i++) {
                glTexImage2D(cube ? (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i) : GL_TEXTURE_2D, 0, desc.ifmt, width, height,
                             0, desc.fmt, desc.type, pix);
                if (!cube) break;
            }

            if (pix != data)
                update(data);
        }

        void deinit() {
            if (ID)
                glDeleteTextures(1, &ID);
        }

        void generateMipMap() {
            bind(0);
            GLenum target = (opt & OPT_CUBEMAP) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;

            glGenerateMipmap(target);
            if (!(opt & OPT_CUBEMAP) && !(opt & OPT_NEAREST) && (support.maxAniso > 0))
                glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, min(int(support.maxAniso), 8));
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
        }

        void update(void *data) {
            bind(0);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, origWidth, origHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }

        void bind(int sampler) {
            if (opt & OPT_PROXY) return;
            ASSERT(ID);

            if (active.textures[sampler] != this) {
                active.textures[sampler] = this;
                glActiveTexture(GL_TEXTURE0 + sampler);
                glBindTexture((opt & OPT_CUBEMAP) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, ID);
            }
        }

        void unbind(int sampler) {
            if (active.textures[sampler]) {
                active.textures[sampler] = NULL;
                glActiveTexture(GL_TEXTURE0 + sampler);
                glBindTexture((opt & OPT_CUBEMAP) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, 0);
            }
        }

        void setFilterQuality(int value) {
            bool filter = (opt & OPT_NEAREST) == 0 && (value > Settings::LOW);
            bool mipmaps = (opt & OPT_MIPMAPS) != 0;

            active.textures[0] = NULL;
            bind(0);
            if (support.maxAniso > 0)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                                value > Settings::MEDIUM ? min(int(support.maxAniso), 8) : 1);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            filter ? (mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR) : (mipmaps
                                                                                        ? GL_NEAREST_MIPMAP_NEAREST
                                                                                        : GL_NEAREST));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter ? GL_LINEAR : GL_NEAREST);
        }
    };

    bool extSupport(const char *str, const char *ext) {
        if (!str) return false;
        return strstr(str, ext) != NULL;
    }

    void init() {
        memset(rtCache, 0, sizeof(rtCache));

        #ifdef OS_ANDROID
            void *libGL = dlopen("libGLESv2.so", RTLD_LAZY);
        #endif

        #if defined(OS_WIN) || defined(OS_LINUX) || defined(OS_ANDROID)
            #ifdef OS_WIN
                GetProcOGL(glActiveTexture);
            #endif

            #ifdef OS_WIN
                GetProcOGL(wglSwapIntervalEXT);
            #elif OS_LINUX
                GetProcOGL(glXSwapIntervalSGI);
            #endif

            #if defined(OS_WIN) || defined(OS_LINUX)
                GetProcOGL(glGenerateMipmap);

                #ifdef PROFILE
                    GetProcOGL(glObjectLabel);
                    GetProcOGL(glPushDebugGroup);
                    GetProcOGL(glPopDebugGroup);
                    GetProcOGL(glGenQueries);
                    GetProcOGL(glDeleteQueries);
                    GetProcOGL(glGetQueryObjectiv);
                    GetProcOGL(glBeginQuery);
                    GetProcOGL(glEndQuery);
                #endif

                GetProcOGL(glCreateProgram);
                GetProcOGL(glDeleteProgram);
                GetProcOGL(glLinkProgram);
                GetProcOGL(glUseProgram);
                GetProcOGL(glGetProgramInfoLog);
                GetProcOGL(glCreateShader);
                GetProcOGL(glDeleteShader);
                GetProcOGL(glShaderSource);
                GetProcOGL(glAttachShader);
                GetProcOGL(glCompileShader);
                GetProcOGL(glGetShaderInfoLog);
                GetProcOGL(glGetUniformLocation);
                GetProcOGL(glUniform1iv);
                GetProcOGL(glUniform1fv);
                GetProcOGL(glUniform2fv);
                GetProcOGL(glUniform3fv);
                GetProcOGL(glUniform4fv);
                GetProcOGL(glUniformMatrix4fv);
                GetProcOGL(glBindAttribLocation);
                GetProcOGL(glEnableVertexAttribArray);
                GetProcOGL(glDisableVertexAttribArray);
                GetProcOGL(glVertexAttribPointer);
                GetProcOGL(glGetProgramiv);

                GetProcOGL(glGenFramebuffers);
                GetProcOGL(glBindFramebuffer);
                GetProcOGL(glGenRenderbuffers);
                GetProcOGL(glBindRenderbuffer);
                GetProcOGL(glFramebufferTexture2D);
                GetProcOGL(glFramebufferRenderbuffer);
                GetProcOGL(glRenderbufferStorage);
                GetProcOGL(glCheckFramebufferStatus);
                GetProcOGL(glDeleteFramebuffers);
                GetProcOGL(glDeleteRenderbuffers);

                GetProcOGL(glGenBuffers);
                GetProcOGL(glDeleteBuffers);
                GetProcOGL(glBindBuffer);
                GetProcOGL(glBufferData);
                GetProcOGL(glBufferSubData);
            #endif

            #ifdef _RENDER_GLES
                GetProcOGL(glDiscardFramebufferEXT);
            #endif

            GetProcOGL(glGenVertexArrays);
            GetProcOGL(glDeleteVertexArrays);
            GetProcOGL(glBindVertexArray);
            GetProcOGL(glGetProgramBinary);
            GetProcOGL(glProgramBinary);
        #endif

        LOG("Vendor   : %s\n", (char*)glGetString(GL_VENDOR));
        LOG("Renderer : %s\n", (char*)glGetString(GL_RENDERER));
        LOG("Version  : %s\n", (char*)glGetString(GL_VERSION));

        char *ext = (char*)glGetString(GL_EXTENSIONS);
    /*
        if (ext != NULL) {
            char buf[255];
            int len = strlen(ext);
            int start = 0;
            for (int i = 0; i < len; i++)
                if (ext[i] == ' ' || (i == len - 1)) {
                    memcpy(buf, &ext[start], i - start);
                    buf[i - start] = 0;
                    LOG("%s\n", buf);
                    start = i + 1;
                }
        }
    */

        support.shaderBinary   = extSupport(ext, "_program_binary");
        support.VAO            = extSupport(ext, "_vertex_array_object");
        support.depthTexture   = extSupport(ext, "_depth_texture");
        support.shadowSampler  = support.depthTexture && (extSupport(ext, "_shadow_samplers") || extSupport(ext, "GL_ARB_shadow"));
        support.discardFrame   = extSupport(ext, "_discard_framebuffer");
        support.texNPOT        = extSupport(ext, "_texture_npot") || extSupport(ext, "_texture_non_power_of_two");
        support.texRG          = extSupport(ext, "_texture_rg ");   // hope that isn't last extension in string ;)
        support.texBorder      = extSupport(ext, "_texture_border_clamp");
        support.maxAniso       = extSupport(ext, "_texture_filter_anisotropic");
        support.colorFloat     = extSupport(ext, "_color_buffer_float");
        support.colorHalf      = extSupport(ext, "_color_buffer_half_float") || extSupport(ext, "GL_ARB_half_float_pixel");
        support.texFloatLinear = support.colorFloat || extSupport(ext, "GL_ARB_texture_float") || extSupport(ext, "_texture_float_linear");
        support.texFloat       = support.texFloatLinear || extSupport(ext, "_texture_float");
        support.texHalfLinear  = support.colorHalf || extSupport(ext, "GL_ARB_texture_float") || extSupport(ext, "_texture_half_float_linear") || extSupport(ext, "_color_buffer_half_float");
        support.texHalf        = support.texHalfLinear || extSupport(ext, "_texture_half_float");

        #ifdef PROFILE
            support.profMarker = extSupport(ext, "_KHR_debug");
            support.profTiming = extSupport(ext, "_timer_query");
        #endif

        if (support.maxAniso)
            glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &support.maxAniso);
        #ifdef _RENDER_GLES
            glGetIntegerv(GL_MAX_VARYING_VECTORS, &support.maxVectors);
        #else
            support.maxVectors = 16;
        #endif

        glEnable(GL_SCISSOR_TEST);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&defaultFBO);
        glGenFramebuffers(1, &FBO);
        glDepthFunc(GL_LEQUAL);
    }

    void deinit() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &FBO);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        for (int b = 0; b < 2; b++)
            for (int i = 0; i < rtCache[b].count; i++)
                glDeleteRenderbuffers(1, &rtCache[b].items[i].ID);
    }
}
#endif //OPENDEMO_RENDER_GL_H