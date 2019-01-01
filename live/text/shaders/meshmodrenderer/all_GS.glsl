#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(constant_id = 0) const int c_drawWire = 0;
layout(constant_id = 1) const int c_phongNormals = 0;

layout(location = 0) in vec3 inNormal[];
layout( triangles ) in;

layout(binding = 0) uniform globals_t
{
	mat4 view;	
	mat4 projection;	
	mat4 viewProjection;
	mat4 inverseViewProjection;
} globals;

layout( triangle_strip, max_vertices = 3 ) out;
layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 baryCentric;

void main()
{
	if(c_phongNormals != 0)
	{
		gl_Position = gl_in[0].gl_Position;
		normal = inNormal[0];
		if(c_drawWire !=0) baryCentric = vec3(1,0,0);
		EmitVertex();

		gl_Position = gl_in[1].gl_Position;    
		normal = inNormal[1];
		if(c_drawWire !=0) baryCentric = vec3(0,1,0);
		EmitVertex();

		gl_Position = gl_in[2].gl_Position;
		normal = inNormal[2];
		if(c_drawWire !=0) baryCentric = vec3(0,0,1);
		EmitVertex();
	} else
	{
		vec3 v0 = vec3(globals.inverseViewProjection * gl_in[0].gl_Position);
		vec3 v1 = vec3(globals.inverseViewProjection * gl_in[1].gl_Position);
		vec3 v2 = vec3(globals.inverseViewProjection * gl_in[2].gl_Position);

		vec3 e0 = v1 - v0;
		vec3 e1 = v2 - v0;
		vec3 n = -normalize(cross(normalize(e0), normalize(e1)));
		
		gl_Position = gl_in[0].gl_Position;
		normal = n;
		if(c_drawWire !=0) baryCentric = vec3(1,0,0);
		EmitVertex();

		gl_Position = gl_in[1].gl_Position;    
		normal = n;
		if(c_drawWire !=0) baryCentric = vec3(0,1,0);
		EmitVertex();

		gl_Position = gl_in[2].gl_Position;
		normal = n;
		if(c_drawWire !=0) baryCentric = vec3(0,0,1);
		EmitVertex();		
	}

	EndPrimitive();
} 