#ifdef VERTEX

layout(location = 0)in vec4 vert;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_Position = projection * view * model * vert;
}

#endif

#ifdef FRAGMENT

out vec4 fragColor;

void main()
{
	fragColor = vec4(0.4, 0.4, 0.8, 1.0);
}

#endif