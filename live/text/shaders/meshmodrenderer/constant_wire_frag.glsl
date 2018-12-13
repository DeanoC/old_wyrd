#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform pushConstants_t 
{ 
	layout(offset = 64) vec4 pushColour; 
} pushConstants;
layout (location = 0) in vec4 inColour;
layout (location = 1) in vec3 barycentric;
layout (location = 0) out vec4 outColour;

float edgeFactor(){
    vec3 d = fwidth(barycentric);
    vec3 a3 = smoothstep(vec3(0.0), d*1.5, barycentric);
    return min(min(a3.x, a3.y), a3.z);
}

void main() 
{
	float t = edgeFactor();
	vec3 c = vec3(pushConstants.pushColour);
	vec3 ic = vec3(1) - c;
	vec3 oc = mix(c, ic, t);
	outColour = vec4(oc, inColour[3]);
} 