
#include "gui.hpp"
#include "file.h"
#include "model.h"
#include<sys/shm.h>
#include<sys/ipc.h>

/*** Application Control ***/
void on_button_init (GtkButton *button, struct app_data* data){
	send_InitMem(data);
	model_init(data, 0);

	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	datag->isInit = 1;

	send_Zero(data);
}

void on_button_build (GtkButton *button, struct app_data* data){
	send_Build(data);
}

void on_button_load (GtkButton *button, struct app_data* data){
	send_Load(data);
}

void on_button_save (GtkButton *button, struct app_data* data){
	send_Save(data);
}


void on_button_check (GtkButton *button, struct app_data* data){
	send_Check(data);
}

void on_button_free (GtkButton *button, struct app_data* data){
	send_Free(data);
	model_free(data, 0);
	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	datag->isInit = 0;
}

void on_button_refresh (GtkButton *button, struct app_data* data){
	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	datag->fb_changed = 1;
}


void on_value_changed(GtkRange* range, struct app_data* data){
	layer_num = (int) ( gtk_range_get_value(range) - data->start_slice) * ((double)data->Z_DIM/data->NUM_SLICES); // in dim [1-1141], not [0,1141) need output [0-1140]
	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	datag->fb_changed = 1;
	//fprintf(stderr, "%d\n", layer_num );

}

/*GdkRGBA currently_selected_color;
void on_button_select_color (GtkButton *button, struct app_data* data){
	struct gui_data* datag = (struct gui_data*) (data->gui_data);

	datag->pixel_color = currently_selected_color;

	fprintf(stderr, "Color: %f\n", currently_selected_color.red);
}*/
void on_color_choose(GtkColorChooser* chooser, void* param, struct app_data* data){
	struct gui_data* datag = (struct gui_data*) (data->gui_data);

	gtk_color_chooser_get_rgba (chooser, &(datag->pixel_color));

	//fprintf(stderr, "Color: %s\n", gdk_rgba_to_string(&(datag->pixel_color)));
}

gboolean on_color_switch(GtkSwitch* sw, gboolean state, struct app_data* data){

	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	GtkColorChooserWidget* chooser = datag->color_picker;
	//GtkButton* sel_button = datag->color_select_button;


	if (state == TRUE){
		gtk_widget_show( GTK_WIDGET(chooser) );
		//gtk_widget_show( GTK_WIDGET(sel_button) );

	}else{
		gtk_widget_hide( GTK_WIDGET(chooser) );
		//gtk_widget_hide( GTK_WIDGET(sel_button) );

	}

	return FALSE;
}


void on_radio_default(GtkToggleButton *togglebutton, struct app_data* data){

	char buffer[1024];
	gboolean toggled = gtk_toggle_button_get_active (togglebutton);
	if (!toggled){
		data->model_type = bitArray;
		sprintf(buffer,"update_variable model_type %d\n", bitArray);
		sendCommand(buffer, data);
	}

}

void on_radio_bitmap(GtkToggleButton *togglebutton, struct app_data* data){

	char buffer[1024];
	gboolean toggled = gtk_toggle_button_get_active (togglebutton);
	if (!toggled){
		data->model_type = _default;
		sprintf(buffer,"update_variable model_type %d\n", _default);
		sendCommand(buffer, data);
	}

}

void on_entry_max_mem (GtkEntry *entry, struct app_data* data){
	const char* max_mem_str = gtk_entry_get_text(entry);
	data->MODEL_MEM_SIZE_GB = atoi(max_mem_str);
	char buffer[1024];
	sprintf(buffer,"update_variable mem_max %d\n", data->MODEL_MEM_SIZE_GB);
	sendCommand(buffer, data);
}

void on_entry_hdd (GtkEntry *entry, struct app_data* data){
	const char* max_hdd_str = gtk_entry_get_text(entry);
	data->MODEL_HDD_SIZE_GB = atoi(max_hdd_str);
	char buffer[1024];
	sprintf(buffer,"update_variable hdd_max %d\n", data->MODEL_HDD_SIZE_GB);
	sendCommand(buffer, data);
}

void on_entry_resolution (GtkEntry *entry, struct app_data* data){
	const char* res_str = gtk_entry_get_text(entry);
	data->resolution = atoi(res_str);
	char buffer[1024];
	sprintf(buffer,"update_variable res %d\n", data->resolution);
	sendCommand(buffer, data);
}

void on_entry_layers (GtkEntry *entry, struct app_data* data){
	const char* layer_num_str = gtk_entry_get_text(entry);
	char buffer[1024];
	strcpy(buffer,layer_num_str);
	char* slice = strtok(buffer, "-");
	if (slice == NULL){
		fprintf(stdout,"Build Layer Set: Invalid Syntax\nPlease use format \"#-#\"; check for erroneous spaces\n");
	}
	data->start_slice = atoi(slice);
	slice = strtok(NULL, " \n");
	if (slice == NULL){
		fprintf(stdout,"Build Layer Set: Invalid Syntax\nPlease use format \"#-#\"; check for erroneous spaces\n");
	}
	data->end_slice = atoi(slice);
	sprintf(buffer,"update_variable start_slice %d\n", data->start_slice);
	sendCommand(buffer, data);
	sprintf(buffer,"update_variable end_slice %d\n", data->end_slice);
	sendCommand(buffer, data);


	//update slider
	struct gui_data* datag = (struct gui_data*) (data->gui_data);

	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(datag->value_slider));
	gtk_adjustment_set_lower(adj, data->start_slice);
	gtk_adjustment_set_upper(adj, data->end_slice);
	gtk_range_set_value(GTK_RANGE(datag->value_slider),data->start_slice);


}

void on_entry_tdms_dir (GtkEntry *entry, struct app_data* data){
	const char* temp = gtk_entry_get_text(entry);
	strcpy(	data->tdms_dir, temp);
	char buffer[1024];
	sprintf(buffer,"update_variable tdms_dir %s\n", data->tdms_dir);
	sendCommand(buffer, data);
}

void on_entry_filename (GtkEntry *entry, struct app_data* data){
	const char* temp = gtk_entry_get_text(entry);
	strcpy(	data->filename, temp);
	char buffer[1024];
	sprintf(buffer,"update_variable filename %s\n", data->filename);
	sendCommand(buffer, data);
}

/***  Program Signal Handlers  ***/
// called when window is closed
void on_window_main_destroy(GtkWidget *object, struct app_data* data){

	struct gui_data* datag = (struct gui_data*) (data->gui_data);

	send_Close(data);

	GError* err = NULL;
	//g_io_channel_shutdown(datag->term_channel)
	g_io_channel_shutdown(datag->bknd_in, FALSE, &err);
	g_io_channel_shutdown(datag->bknd_out, FALSE, &err);

	close(datag->fd_ack);
	close(datag->fd_bknd_in);
	close(datag->fd_bknd_out);

    gtk_main_quit();
}
