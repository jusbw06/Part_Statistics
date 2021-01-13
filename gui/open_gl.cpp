#include "gui.hpp"
#include "model.h"
#include "file.h"
#include "settings.h"

int pts_added;


double time_incr;

/* GL related callbacks */
void on_realize (GtkGLArea *area, struct app_data* data){

	/* Launch Memory Segment */
	updateGlobalVariables("../SETTINGS.cfg", data, 0);


	// We need to make the context current if we want to
	// call GL API
	gtk_gl_area_make_current (area);

	if (gtk_gl_area_get_error (area) != NULL)
		return;

	//cout << "Called realize" << endl;

	//Note that any gl calls must always happen after a GL state is initialized
	if(!gladLoadGL()) {
		cout << "Failed Initialize GLAD" << endl;
        exit(-1);
    }
	//fprintf(stderr, "OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);



	// Initialize Shaders
	initGLSL();

	// Initialize Buffer Objects
	initBufferObjects();

	layer_num = 0;
	for (int i = 0; i < MAX_PTS; i++){
		ssbo.pos[i] = vec4(0);
	}

	// Get frame clock:
	GdkGLContext *glcontext = gtk_gl_area_get_context(area);
	GdkWindow *glwindow = gdk_gl_context_get_window(glcontext);
	GdkFrameClock *frame_clock = gdk_window_get_frame_clock(glwindow);

	// Connect update signal:
	g_signal_connect_swapped(frame_clock, "update", G_CALLBACK(gtk_gl_area_queue_render), area);

	// Start updating:
	gdk_frame_clock_begin_updating(frame_clock);

	time_incr = 0;

}

gboolean on_render (GtkGLArea* area, GdkGLContext* context, struct app_data* data){

	struct gui_data* datag = (struct gui_data*) (data->gui_data);

	if ( (datag->fb_changed) || time_incr > 3){ // update every 3 second
		populateSSBO(data, &ssbo);
		datag->fb_changed = 0;
		time_incr = 0;
	}

	double dt = getFrameTime();
	time_incr += dt;

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * pts_added, ssbo.pos, GL_DYNAMIC_COPY);

	// Get current frame buffer size.
	int width, height;
	GtkAllocation allocation;
	gtk_widget_get_allocation(GTK_WIDGET(area), &allocation);
	width = allocation.width;
	height = allocation.height;
	float aspect = width/(float)height;
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create the matrix stacks - please leave these alone for now

	glm::mat4 V, M, P; //View, Model and Perspective matrix
	V = glm::mat4(1);
	M = glm::mat4(1);
	// Apply orthographic projection....
/*	P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
	if (width < height){
		P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 100.0f);
	}*/
	// ...but we overwrite it (optional) with a perspective projection.
	P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

	// Draw the box using GLSL.
	prog->bind();

	V = mycam.process(dt*5);

	vec3 color;
	color.r = datag->pixel_color.red;
	color.g = datag->pixel_color.green;
	color.b = datag->pixel_color.blue;
	//send the matrices to the shaders
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
	glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
	glUniform3fv(prog->getUniform("pix_col"), 1, &(color.r));

	glBindVertexArray(VAO);

	glDrawArrays(GL_POINTS, 0, pts_added);

	prog->unbind();

	return TRUE;
}


/* GL related functions */

double getFrameTime(){ // make prev_frametime argument

	clock_t curr_frametime;
	curr_frametime = clock();
	clock_t dt = curr_frametime - last_frametime;

	last_frametime = curr_frametime;
	//cout << "dt: " << dt << " Realtime: " << ((float)dt)/CLOCKS_PER_SEC << endl;

	double realtime = ((double)dt)/CLOCKS_PER_SEC;
	return realtime; //<--- microseconds

}

/***      Program Initialization       ***/
//Shader Initialization
void initGLSL(){

	GLSL::checkVersion();

	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Initialize the GLSL program.
	prog = std::make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames("../resources/shader_vertex.glsl", "../resources/shader_fragment.glsl");
	if (!prog->init()){
		cout << "One or more shaders failed to compile... exiting!" << std::endl;
		exit(1);
	}
	prog->addUniform("P");
	prog->addUniform("V");
	prog->addUniform("M");
	prog->addUniform("pix_col");

	prog->addAttribute("vertPos");

}

//Note that any gl calls must always happen after a GL state is initialized
void initBufferObjects(){

	//generate the VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);


	//glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * MAX_PTS, ssbo.pos, GL_DYNAMIC_COPY);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void default_populateSSBO(struct app_data* data, ssbo_data* ssbo_ptr){

	// Place into new array
	pts_added = 0;
	struct node val;

	for (int i = 0; i < data->X_DIM; i++){
		for (int j = 0; j < data->Y_DIM; j++){

			model_getVal(layer_num, i, j, &val, data->model_type);

			if (val.area > 0.1){
				ssbo_ptr->pos[pts_added].z = layer_num;
				ssbo_ptr->pos[pts_added].x = i;
				ssbo_ptr->pos[pts_added].y = j;
				pts_added++;
			}

			if (pts_added >= MAX_PTS){
				return;
			}
		}
	}

}


void bitmap_populateSSBO(struct app_data* data, ssbo_data* ssbo_ptr){

	// Place into new array
	pts_added = 0;
	uint8_t val;

	for (int i = 0; i < data->X_DIM; i++){
		for (int j = 0; j < data->Y_DIM; j++){

			model_getVal(layer_num, i, j, &val, data->model_type);

			if (val == 1){
				ssbo_ptr->pos[pts_added].z = layer_num;
				ssbo_ptr->pos[pts_added].x = i;
				ssbo_ptr->pos[pts_added].y = j;
				pts_added++;
			}

			if (pts_added >= MAX_PTS){
				return;
			}
		}
	}


}


void populateSSBO(struct app_data* data, ssbo_data* ssbo_ptr){

	struct gui_data* datag = (struct gui_data*) (data->gui_data);

	if (datag->isInit == 0){
		pts_added = 0;
		return;
	}

	switch (data->model_type){
		case _default:
			default_populateSSBO(data, ssbo_ptr);
			break;
		case largeFile:
			break;
		case bitArray:
			bitmap_populateSSBO(data, ssbo_ptr);
			break;
		default:
			fprintf(stderr, "Populate SSBO: Invalid model_type\n");
	}


}

