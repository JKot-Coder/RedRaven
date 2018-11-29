#extension GL_ARB_explicit_attrib_location : require

#ifdef VERTEX

layout(location = 0) in vec3 Vert;

uniform mat4 ViewProjection;
uniform mat4 Model;

void main()
{
	gl_Position = ViewProjection * vec4(Vert, 1.0);
}

#endif

#ifdef FRAGMENT

out vec4 fragColor;

void main()
{
	fragColor = vec4(0.9, 0.4, 0.8, 1.0);
}

#endif