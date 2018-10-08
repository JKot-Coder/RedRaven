#ifndef OPENDEMO_RENDER_H
#define OPENDEMO_RENDER_H

#include <cstdint>

#define MAX_LIGHTS           4
#define MAX_RENDER_BUFFERS   32
#define MAX_CONTACTS         15

#define SETTINGS_VERSION 2
#define SETTINGS_READING 0xFF


namespace Render {

    struct Texture;
    struct Shader;

    // Texture image format
    enum TexFormat {
        FMT_LUMINANCE,
        FMT_RGBA,
        FMT_RGB16,
        FMT_RGBA16,
        FMT_RGBA_FLOAT,
        FMT_RGBA_HALF,
        FMT_DEPTH,
        FMT_SHADOW,
        FMT_MAX,
    };

    // Texture options
    enum TexOption {
        OPT_REPEAT  = 1,
        OPT_CUBEMAP = 2,
        OPT_MIPMAPS = 4,
        OPT_NEAREST = 8,
        OPT_TARGET  = 16,
        OPT_VERTEX  = 32,
        OPT_PROXY   = 64,
    };

    struct Support {
        int maxVectors;
        int maxAniso;
        bool shaderBinary;
        bool VAO;
        bool depthTexture;
        bool shadowSampler;
        bool discardFrame;
        bool texNPOT;
        bool texRG;
        bool texBorder;
        bool colorFloat, texFloat, texFloatLinear;
        bool colorHalf, texHalf, texHalfLinear;
#ifdef PROFILE
        bool profMarker;
        bool profTiming;
#endif
    } support;

    // Pipeline State Object
    struct PSO {
        void       *data;
        void       *shader;
        vec4       clearColor;
        TexFormat  colorFormat;
        TexFormat  depthFormat;
        uint32_t   renderState;
    };

    struct Viewport {
        int x, y, width, height;

        Viewport() : x(0), y(0), width(0), height(0) {}
        Viewport(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}

        inline bool operator == (const Viewport &vp) const { return x == vp.x && y == vp.y && width == vp.width && height == vp.height; }
        inline bool operator != (const Viewport &vp) const { return !(*this == vp); }
    };

    struct Active {
        const PSO *pso;
        Shader    *shader;
        Texture   *textures[8];
        Texture   *target;
        int32_t   renderState;
        uint32_t  targetFace;
        uint32_t  targetOp;
        Viewport  viewport; // TODO: ivec4
        vec4      material;

        #ifdef _RENDER_GL
            uint32      VAO;
            uint32      iBuffer;
            uint32      vBuffer;
        #endif

        int32_t      basisCount;
        Basis       *basis;
    } active;

     struct Settings {
        enum Quality  { LOW, MEDIUM, HIGH };
        enum Stereo   { STEREO_OFF, STEREO_ON, STEREO_SPLIT, STEREO_VR };

        uint8_t version;

        struct {
            union {
                struct {
                    uint8_t filter;
                    uint8_t lighting;
                    uint8_t shadows;
                    uint8_t water;
                };
                uint8_t quality[4];
            };
            uint8_t vsync;
            uint8_t stereo;
            void setFilter(Quality value) {
                if (value > MEDIUM && !(support.maxAniso > 1))
                    value = MEDIUM;
                filter = value;
            }

            void setLighting(Quality value) {
            #ifdef _OS_PSP
                lighting = LOW;
            #else
                lighting = value;
            #endif
            }

            void setShadows(Quality value) {
            #ifdef _OS_PSP
                shadows = LOW;
            #else
                shadows = value;
            #endif
            }

            void setWater(Quality value) {
            #ifdef _OS_PSP
                water = LOW;
            #else
                if (value > LOW && !(support.texFloat || support.texHalf))
                    value = LOW;
                water = value;
            #endif
            }
        } detail;

        struct {
            uint8_t music;
            uint8_t sound;
            uint8_t reverb;
        } audio;

        struct Controls {
            uint8_t  joyIndex;
            uint8_t  vibration;
            uint8_t  retarget;
            uint8_t  multiaim;
          //  KeySet keys[cMAX];
        } controls[2];

        // temporary, used only for setting controls
        uint8_t playerIndex;
        uint8_t ctrlIndex;
    } settings;
}
#endif //OPENDEMO_RENDER_H