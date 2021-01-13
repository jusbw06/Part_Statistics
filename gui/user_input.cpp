
#include "gui.hpp"


/***    User Input     ***/
// Mouse
dvec2 initial_mouse_loc;
gboolean on_button(GtkWidget *widget, GdkEventButton *event){

	GdkEventType action = event->type;
	guint button = event->button;
	dvec2 mouse_loc = dvec2(event->x,event->y);

	GtkAllocation allocation; // get width, height
	gtk_widget_get_allocation(widget, &allocation); // <-- GLArea widget

	if (event->button == 1 && action == GDK_BUTTON_PRESS){
		cout << "Received Button Press: " << event->x << " " << event->y << endl;
	}

	if (button == 2 && action == GDK_BUTTON_PRESS && mycam.mb3 == 0){
		//cout << "Received Middle Button Press: " << event->x << " " << event->y << endl;
		initial_mouse_loc = mouse_loc;
		mycam.mb3 = 1;
	}
	if (button == 2 && action == GDK_BUTTON_RELEASE){
		mycam.mb3 = 0;
	}

	return FALSE;
}
// Motion
gboolean on_motion(GtkWidget *widget, GdkEventMotion *event){

	dvec2 mouse_loc = dvec2(event->x,event->y);

	GtkAllocation allocation; // get width, height
	gtk_widget_get_allocation(widget, &allocation); // <-- GLArea widget

	//cout << "Received Pointer Motion Event" << endl;

	dvec2 delta_mouse_loc;
	delta_mouse_loc.x = initial_mouse_loc.x - mouse_loc.x;
	delta_mouse_loc.y = initial_mouse_loc.y - mouse_loc.y;

	mycam.cam_rot.x = delta_mouse_loc.x / allocation.width;
	mycam.cam_rot.y = delta_mouse_loc.y / allocation.height;

	return FALSE;
}
// Scroll
void on_scroll(GtkWidget *widget, GdkEventScroll *event){

	dvec2 mouse_loc = dvec2(event->x,event->y);
	GdkScrollDirection scroll_dir = event->direction;

	GtkAllocation allocation; // get width, height
	gtk_widget_get_allocation(widget, &allocation); // <-- GLArea widget

	//cout << "Scroll_dir: " << scroll_dir << endl;

	float yoffset = 1;
	if (scroll_dir == GDK_SCROLL_DOWN){
		yoffset = -1;
	}

	double posX, posY;
	posX = mouse_loc.x - allocation.width/2;
	posY = mouse_loc.y - allocation.height/2;

	mycam.cam_trans.x = posX / allocation.width;
	mycam.cam_trans.y = posY / allocation.height;
	mycam.cam_trans.z = yoffset;

	//std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;
	//std::cout << "Offset Y: " << yoffset << " Offset X: " << xoffset << endl;

}

//keyboard
gboolean on_key(GtkWidget *widget, GdkEventKey *event){

	/* ESC */
	if (event->keyval == GDK_KEY_Escape && event->type == GDK_KEY_PRESS){
		gtk_main_quit();
	}
	/* W */
	if (event->keyval == GDK_KEY_w && event->type == GDK_KEY_PRESS){
		mycam.w = 1;
	}
	if (event->keyval == GDK_KEY_w && event->type == GDK_KEY_RELEASE){
		mycam.w = 0;
	}
	/* S */
	if (event->keyval == GDK_KEY_s && event->type == GDK_KEY_PRESS){
		mycam.s = 1;
	}
	if (event->keyval == GDK_KEY_s && event->type == GDK_KEY_RELEASE){
		mycam.s = 0;
	}
	/* A */
	if (event->keyval == GDK_KEY_a && event->type == GDK_KEY_PRESS){
		mycam.a = 1;
	}
	if (event->keyval == GDK_KEY_a && event->type == GDK_KEY_RELEASE){
		mycam.a = 0;
	}
	/* D */
	if (event->keyval == GDK_KEY_d && event->type == GDK_KEY_PRESS){
		mycam.d = 1;
	}
	if (event->keyval == GDK_KEY_d && event->type == GDK_KEY_RELEASE){
		mycam.d = 0;
	}
	/* E */
	if (event->keyval == GDK_KEY_e && event->type == GDK_KEY_PRESS){
		mycam.e = 1;
	}
	if (event->keyval == GDK_KEY_e && event->type == GDK_KEY_RELEASE){
		mycam.e = 0;
	}
	/* Q */
	if (event->keyval == GDK_KEY_q && event->type == GDK_KEY_PRESS){
		mycam.q = 1;
	}
	if (event->keyval == GDK_KEY_q && event->type == GDK_KEY_RELEASE){
		mycam.q = 0;
	}
	/* SPACE */
	if (event->keyval == GDK_KEY_space && event->type == GDK_KEY_PRESS){
		mycam.space = 1;
	}
	if (event->keyval == GDK_KEY_space && event->type == GDK_KEY_RELEASE){
		mycam.space = 0;
	}
	/* LSHIFT */
	if (event->keyval == GDK_KEY_Shift_L && event->type == GDK_KEY_PRESS){
		mycam.lshift = 1;
	}
	if (event->keyval == GDK_KEY_Shift_L && event->type == GDK_KEY_RELEASE){
		mycam.lshift = 0;
	}
	/*if (event->type == GDK_KEY_RELEASE){
		cout << "KEY_RELEASE" << endl;
	}
	if (event->type == GDK_KEY_PRESS){
		cout << "KEY_PRESS" << endl;
	}*/


	return FALSE;
}


/* Camera */
glm::mat4 camera::process(double ftime){

	float xspeed, yspeed, zspeed;
	xspeed = yspeed = zspeed = 0;
	if (w == 1){
		xspeed = 10*ftime;
	}else if (a == 1){
		yspeed = 10*ftime;
	}else if (s == 1){
		xspeed = -10*ftime;
	}else if (d == 1){
		yspeed = -10*ftime;
	}else if (space == 1){
		zspeed = -10*ftime;
	}else if (lshift == 1){
		zspeed = 10*ftime;
	}

	if (abs(cam_trans.z) > 0.1){
		double z_rat, xy_rat;
		z_rat = xy_rat = 1;
		if (abs(pos.z) < 1)
			z_rat = pow(abs(pos.z),2);
		xy_rat = pow(abs(pos.z),1);

		xspeed = cam_trans.z*10*ftime * z_rat;
		yspeed = -cam_trans.z*cam_trans.x*10*ftime * xy_rat;
		zspeed = cam_trans.z*cam_trans.y*10*ftime * xy_rat;
	}

	float yangle=0;
	float xangle=0;

	if (mb3 == 1){

		yangle += cam_rot.x*ftime;
		xangle += cam_rot.y*ftime;

	}


	if (q == 1)
		yangle = -1*ftime;
	else if(e==1)
		yangle = 1*ftime;

	rot.y += yangle;
	rot.x += xangle;
	glm::mat4 YR = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
	//glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, axis);
	glm::mat4 XR = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));

	glm::vec4 dir = glm::vec4(yspeed, zspeed, xspeed,1);
	dir = dir*XR*YR;
	pos += glm::vec3(dir.x, dir.y, dir.z);
	glm::mat4 T = glm::translate(glm::mat4(1), pos);
	cam_trans = vec3(0);
	return YR*XR*T;
}

