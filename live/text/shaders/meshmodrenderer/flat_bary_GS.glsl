#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec4 inColour[];

layout( triangles ) in;
layout( triangle_strip, max_vertices = 3 ) out;

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec3 baryCentric;
layout(location = 2) out vec3 normal;

void main()
{
	vec3 e0 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec3 e1 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	normal = cross(normalize(e0), normalize(e1));
	
	gl_Position = gl_in[0].gl_Position;
	outColour = inColour[0];
	baryCentric = vec3(1,0,0);
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;    
	outColour = inColour[1];
	baryCentric = vec3(0,1,0);
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	outColour = inColour[2];
	baryCentric = vec3(0,0,1);
	EmitVertex();

	EndPrimitive();
} 