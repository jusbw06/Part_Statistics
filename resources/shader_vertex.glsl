#version 450 core
layout(location = 0) in vec4 vertPos;


uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main(){
	
	vec4 tpos =  M * vec4( vertPos.xyz/1000 , 1.0);
	gl_Position = P * V * tpos;

}
