#version 330 core

out vec4 FragColor;

in vec2 texCord;

uniform sampler2D texture1;

uniform float alpha;

void main(){
	vec4 texcolor = texture(texture1,texCord);
	texcolor.a *= alpha;
	FragColor = texcolor;
}

//Ten shader odpowiada za kolory