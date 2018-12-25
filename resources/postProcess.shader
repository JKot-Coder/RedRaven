#extension GL_ARB_explicit_attrib_location : require

struct VertexData {
    vec2 TextureCoord;
};

uniform sampler2D AlbedoTex;

#ifdef VERTEX

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TextureCoord;

out VertexData Vertex;

void main()
{
    Vertex.TextureCoord = TextureCoord;
	gl_Position = vec4(Position, 1.0);
}

#endif

#ifdef FRAGMENT

in VertexData Vertex;

out vec4 fragColor;

void main()
{
	fragColor = vec4(texture(AlbedoTex, Vertex.TextureCoord).rgb, 1.0);
}

#endif