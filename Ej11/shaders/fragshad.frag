#version 330 core

in vec3 fcolor;
uniform float real_light;
out vec4 FragColor;

void main()
{
	if (real_light == 0) {
		if ((int(gl_FragCoord[1]/10) % 2) == 0) {
			FragColor = vec4(1, 1, 1, 1);
		} else {
			FragColor = vec4(0, 0, 0, 0);
		}
	} else {
		FragColor = vec4(fcolor,1);
	}
}
