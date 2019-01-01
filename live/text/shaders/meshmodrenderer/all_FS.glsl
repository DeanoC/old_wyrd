#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(constant_id = 0) const int c_drawWire = 0;
layout(constant_id = 1) const int c_shaderType = 0;

layout(push_constant) uniform pushConstants_t 
{ 
	layout(offset = 64) vec4 pushColour; 
} pushConstants;

layout (location = 0) in vec3 normal;
layout (location = 1) in vec3 barycentric;

layout (location = 0) out vec4 outColour;

float edgeFactor(){
    vec3 d = fwidth(barycentric);
    vec3 a3 = smoothstep(vec3(0.0), d*1.5, barycentric);
    return min(min(a3.x, a3.y), a3.z);
}

void main() 
{
	float t = (c_drawWire !=0) ? 1.0 - edgeFactor() : 0.0;
	vec3 c = {1,1,0};
	if(c_shaderType == 0)
	{
		// constant shader type
		c = vec3(1,1,1);
	} else if(c_shaderType == 1)
	{
		// normals shader tpe
		c = (normal + vec3(1)) * 0.5;
	} else if(c_shaderType == 2)
	{
		// dot shader type
		c = vec3(max(dot(normal, normalize(vec3(-0.707, -0.707, 1))),0) + 0.2);
	}

	c = c * pushConstants.pushColour.xyz;

	vec3 ic = vec3(1) - c;
	vec3 oc = mix(c, ic, t);
	outColour = vec4(oc, pushConstants.pushColour.w);
}