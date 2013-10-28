varying vec3 norm;
varying vec3 ePos;
void main()
{
	norm = gl_NormalMatrix * gl_Normal;
	
	vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;
	
	ePos = normalize(ecPosition.xyz/ecPosition.w);
	
	gl_Position = gl_ProjectionMatrix * ecPosition;
}

