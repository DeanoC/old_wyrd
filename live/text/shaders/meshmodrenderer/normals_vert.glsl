#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec4 inColour;

layout(push_constant) uniform pushConstants_t 
{ 
	layout(offset = 0) mat4 world; 
} pushConstants;

layout(binding = 0) uniform globals_t
{
	mat4 view;
	mat4 projection;	
	mat4 viewprojection;
} globals;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec4 outColour;

void main() 
{
	outColour = inColour;
	gl_Position = globals.viewprojection * pushConstants.world * inPos;
} 