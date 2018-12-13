#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform pushConstants_t 
{ 
	layout(offset = 64) vec4 colour; 
} pushConstants;

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = pushConstants.colour;
} 