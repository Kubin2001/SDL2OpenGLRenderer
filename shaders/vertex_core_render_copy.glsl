#version 330 core
layout (location = 2) in vec3 aPos;
layout (location = 3) in vec2 aTexCord;



out vec3 ourColor;
out vec2 texCord;


void main(){
	gl_Position = vec4(aPos ,1.0);

	texCord = aTexCord;
}

//Ten shader odpowiada za pozycje