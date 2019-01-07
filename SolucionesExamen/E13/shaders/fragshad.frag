#version 330 core

in vec3 fcolor;
out vec4 FragColor;

uniform float paintingZebra;

void main()
{	if(paintingZebra == 0.0){
        FragColor = vec4(fcolor,1);	
	}
	else{ // ***** SI QUEREMOS EL PATRON CEBRADO HEMOS DE HACER ESTO *****
        if(int(gl_FragCoord.y) % 20 >= 10){
            FragColor = vec4(0.0,0.0,0.0,1);
        }
        else {
            FragColor = vec4(1.0,1.0,1.0,1);
        }
	}
}
