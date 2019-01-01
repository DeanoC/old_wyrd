#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(constant_id = 0) const int c_drawWire = 0;

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
	float t = (c_drawWire !=0) ? edgeFactor() : 0.0;
	vec3 c = vec3(inColour);
	vec3 ic = vec3(1) - c;
	vec3 oc = mix(c, ic, t);
	outColour = vec4(oc, inColour[3]);
}