#ifndef GUI_H
#define GUI_H 1

#include <glad/glad.h>

#include <math.h>

#include <iostream>

#include <memory>

#include "GLSL.h"
#include "Program.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <gtk/gtk.h>

#include <ctime>

using namespace std;
using namespace glm;

#define MAX_PTS 5000000


class ssbo_data{

	public:
	vec4 pos[MAX_PTS];

};

struct gui_data{

	GtkWidget* file_window;
	GtkWidget* entry_bar;
	GtkTextBuffer* terminal_buffer;
	GtkTextView* terminal_textview;
	GtkScrolledWindow* scrolled_window;
	GtkScale* value_slider;

	//GtkButton* color_select_button;
	GtkColorChooserWidget* color_picker;
	GdkRGBA pixel_color;

	GIOChannel* term_channel;

	GIOChannel* bknd_in;
	GIOChannel* bknd_out;

	int fd_ack;
	int fd_bknd_in;
	int fd_bknd_out;

	int isInit;
	int fb_changed;

};


/***    User Input     ***/
// Mouse
gboolean on_button(GtkWidget *widget, GdkEventButton *event);
// On Mouse Hold
gboolean on_motion(GtkWidget *widget, GdkEventMotion *event);
// scroll
void on_scroll(GtkWidget *widget, GdkEventScroll *event);
//keyboard
gboolean on_key(GtkWidget *widget, GdkEventKey *event);

/*** Application Control ***/
void on_button_init (GtkButton *button, struct app_data* data);
void on_button_build (GtkButton *button, struct app_data* data);
void on_button_load (GtkButton *button, struct app_data* data);
void on_button_save (GtkButton *button, struct app_data* data);
void on_button_check (GtkButton *button, struct app_data* data);
void on_button_free (GtkButton *button, struct app_data* data);
void on_button_refresh (GtkButton *button, struct app_data* data);
void on_value_changed(GtkRange* range, struct app_data* data);

gboolean on_color_switch(GtkSwitch* widget, gboolean state, struct app_data* data);
void on_color_choose(GtkColorChooser* chooser, void* param, struct app_data* data);
//void on_button_select_color (GtkButton *button, struct app_data* data);



/***  Program Signal Handlers  ***/
// called when window is closed
void on_window_main_destroy(GtkWidget *object, struct app_data* data);



/*** OpenGL related Functions ***/
void on_realize (GtkGLArea *area, struct app_data* data);
gboolean on_render (GtkGLArea* area, GdkGLContext* context, struct app_data* data);

//Shader Initialization
void initGLSL();
void initBufferObjects(); // VBO VAO
void populateSSBO(struct app_data* data, ssbo_data* ssbo);

double getFrameTime();


/*** Backend Communication ***/

void send_InitMem(struct app_data* data);
void send_Close(struct app_data* data);
void send_Load(struct app_data* data);
void send_Save(struct app_data* data);
void send_Build(struct app_data* data);
void send_Zero(struct app_data* data);
void send_Free(struct app_data* data);
void send_Check(struct app_data* data);

/*** Radio ***/
void on_radio_default( GtkToggleButton *togglebutton, struct app_data* data);
void on_radio_bitmap( GtkToggleButton *togglebutton, struct app_data* data);


/*** Entries ***/
void on_entry_max_mem (GtkEntry *entry, struct app_data* data);
void on_entry_hdd (GtkEntry *entry, struct app_data* data);
void on_entry_resolution (GtkEntry *entry, struct app_data* data);
void on_entry_layers (GtkEntry *entry, struct app_data* data);
void on_entry_tdms_dir (GtkEntry *entry, struct app_data* data);
void on_entry_filename (GtkEntry *entry, struct app_data* data);


void sendCommand(const char* command, struct app_data* data);


class camera{

public:
	glm::vec3 pos, rot;
	int w, a, s, d, q, e, space, lshift, scroll_fwd, scroll_bwd, mb3;
	vec3 cam_trans, cam_rot;

	camera(){

		w = a = s = d = q = e = space = lshift = scroll_fwd = scroll_bwd = mb3 = 0;
		pos = rot = glm::vec3(0, 0, 0);
		cam_trans = cam_rot = vec3(0);

	}
	glm::mat4 process(double ftime);
};

#ifdef MAIN
clock_t last_frametime;
ssbo_data ssbo;
camera mycam;
int layer_num;
std::shared_ptr<Program> prog;
GLuint VAO;
GLuint VBO;
#else
extern clock_t last_frametime;
extern ssbo_data ssbo;
extern camera mycam;
extern int layer_num;
extern std::shared_ptr<Program> prog;
extern GLuint VAO;
extern GLuint VBO;
#endif

#endif
