#version 330 core
layout (location = 2) in vec3 aPos;
layout (location = 3) in vec2 aTexCord;



out vec3 ourColor;
out vec2 texCord;

uniform mat4 transform;


void main(){
	gl_Position = transform * vec4(aPos ,1.0);
	texCord = aTexCord;
}

//Ten shader odpowiada za pozycje