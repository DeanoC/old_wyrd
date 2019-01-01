#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(constant_id = 0) const int c_drawWire = 0;
layout(constant_id = 1) const int c_phongNormals = 0;

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inNormal;

layout(push_constant) uniform pushConstants_t 
{ 
	layout(offset = 0) mat4 world; 
} pushConstants;

layout(binding = 0) uniform globals_t
{
	mat4 view;	
	mat4 projection;	
	mat4 viewProjection;
	mat4 inverseViewProjection;
} globals;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec3 normal;

void main() 
{
	if(c_phongNormals != 0) normal = mat3(pushConstants.world) * inNormal;
	
	gl_Position = globals.viewProjection * pushConstants.world * inPos;
} 