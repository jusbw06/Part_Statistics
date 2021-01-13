#version 450 core
out vec4 color;

uniform vec3 pix_col;

void main(){

	color.rgb = pix_col;
	color.a=1;

}
