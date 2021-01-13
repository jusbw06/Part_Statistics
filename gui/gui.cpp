#define MAIN 1
#include "globalVars.h"
#include "gui.hpp"

#include <stdio.h>


// outputs float between 0-1
float randf(){
	return (float)(rand() / (float)RAND_MAX);
}

void on_file_chooser_select(GtkButton* button, struct app_data* data){

	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	GtkWidget* file_window = datag->file_window;

	gchar* filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(file_window));

	fprintf(stdout, "Filename: %s\n", filename);

	// if file not directory
	//		proceed

	g_free(filename);

	gtk_widget_hide(file_window);

}

void on_file_chooser_cancel(GtkButton* button, struct app_data* data){

	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	GtkWidget* file_window = datag->file_window;

	gtk_widget_hide(file_window);

}

void on_file_chooser_open (GtkMenuItem* menuitem, struct app_data* data){

	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	GtkWidget* file_window = datag->file_window;
	GtkWidget* entry_bar = datag->entry_bar;


	gtk_widget_show(file_window);

	gtk_widget_hide(entry_bar);

}

void on_file_chooser_save (GtkMenuItem* menuitem, struct app_data* data){

	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	GtkWidget* file_window = datag->file_window;
	GtkWidget* entry_bar = datag->entry_bar;


	gtk_widget_show(file_window);

	gtk_widget_hide(entry_bar);

}

void removeCarraigeReturns(char* str){

	int n = 0;
	int index = 0;
	while (str[n+1] != '\0'){
		if (str[n] == '%'){
			index = n;
		}
		n++;
	}

	int i = index;
	if (str[i] == '%'){
		i++;
	}
	int j = 0;
	while (str[i] != '\0'){

		if (str[i] != '\r'){
			str[j] = str[i];
			j++;
		}
		i++;
	}
	str[j] = '\0';
}



GtkTextIter iter;
GtkTextIter end_iter;
int line_num;
gboolean appendToTerminal(GIOChannel* source, GIOCondition condition, struct app_data* data){

	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	GtkTextBuffer* terminal_buffer = datag->terminal_buffer;
	GtkTextView* terminal_view = datag->terminal_textview;
	GtkScrolledWindow* scrolled_window = datag->scrolled_window;

	GError* err = NULL;
	char buffer[1024];
	gsize bytes_read;

	g_io_channel_read_chars(source, buffer, 1024, &bytes_read, &err);
	buffer[bytes_read] = '\0';

	char out_buffer[2048];
	if (buffer[bytes_read -1] == '%'){
		removeCarraigeReturns(buffer);
		line_num = gtk_text_buffer_get_line_count(terminal_buffer);
		gtk_text_buffer_get_iter_at_line_offset (terminal_buffer, &iter, line_num-1, 0);
		gtk_text_buffer_get_end_iter(terminal_buffer, &end_iter);
		gtk_text_buffer_delete(terminal_buffer, &iter, &end_iter);
		gtk_text_buffer_insert(terminal_buffer, &iter, buffer, -1);


	}else{
		sprintf(out_buffer, "\n[Message Backend]:\n%s\n", buffer);
		gtk_text_buffer_get_end_iter(terminal_buffer, &iter);
		gtk_text_buffer_insert(terminal_buffer, &iter, out_buffer, -1);
	}

	GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment(scrolled_window);
	gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));

	return TRUE;
}



#define FD_READ 0
#define FD_WRITE 1
void launch_backend(struct app_data* data){

	int fd_bknd_out[2];
	if ( pipe(fd_bknd_out) != 0 ){
		fprintf(stderr, "Backend Pipe Creation Failed, Exiting...\n");
	}

	int fd_bknd_in[2];
	if ( pipe(fd_bknd_in) != 0 ){
		fprintf(stderr, "Backend Pipe Creation Failed, Exiting...\n");
	}

	int fd_ack[2];
	if ( pipe(fd_ack) != 0 ){
		fprintf(stderr, "Backend Pipe Creation Failed, Exiting...\n");
	}

	/* Create Backend Process */
	pid_t pid = fork();
	if (pid == 0){

		// bknd stdout
		if ( dup2(fd_bknd_out[FD_WRITE],STDOUT_FILENO) == -1){
			fprintf(stderr, "Failed to duplicate file descriptor, Exiting...\n");

		}

		// bknd stdin
		if ( dup2(fd_bknd_in[FD_READ],STDIN_FILENO) == -1){
			fprintf(stderr, "Failed to duplicate file descriptor, Exiting...\n");

		}

		// bknd ack
		char fd_buff[5];
		sprintf(fd_buff, "%d", fd_ack[FD_WRITE]);

		//fprintf(stderr, "Ack FD: %s\n", fd_buff);


		//launch
		if (execl("part_stat","part_stat", fd_buff, NULL) == -1){
			fprintf(stderr, "Backend Exec Failed, Exiting...\n");
			exit(-1);
		}
	}

	/* Set Channels */
	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	datag->bknd_out = g_io_channel_unix_new (fd_bknd_out[FD_READ]);
	datag->bknd_in = g_io_channel_unix_new (fd_bknd_in[FD_WRITE]);
	datag->fd_ack =	fd_ack[FD_READ];
	datag->fd_bknd_out = fd_bknd_out[FD_READ];
	datag->fd_bknd_in =	fd_bknd_in[FD_READ];


}


int main(int argc, char** argv){

	struct app_data data;
	struct gui_data gui;
	data.gui_data = (void*) &gui;
	data.model_type = _default;
	data.resolution = 50;
	gui.isInit = 0;
	gui.fb_changed = 1;

	gui.pixel_color.red = 1.0;
	gui.pixel_color.green = 0.0;
	gui.pixel_color.blue = 0.0;
	gui.pixel_color.alpha = 1.0;


    GtkBuilder* builder;
	GtkWidget* window;
	GtkWidget* gl_area;

	gtk_init(&argc, &argv);
	builder = gtk_builder_new_from_file("../glade/window_main.glade");

	// Window
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
	gtk_window_set_title (GTK_WINDOW(window), "window_main");
	g_signal_connect(window, "destroy", G_CALLBACK(on_window_main_destroy), &data);

	g_signal_connect (window, "key-press-event", G_CALLBACK (on_key), NULL);
	g_signal_connect (window, "key-release-event", G_CALLBACK (on_key), NULL);

	// GTKGLArea Instance
	gl_area = GTK_WIDGET(gtk_builder_get_object(builder, "gl_area"));
	gtk_widget_set_events (gl_area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK | GDK_BUTTON2_MOTION_MASK);
	gtk_widget_set_events (window, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

	g_signal_connect (gl_area, "render", G_CALLBACK (on_render), &data);
	g_signal_connect (gl_area, "realize", G_CALLBACK (on_realize), &data);
	g_signal_connect (gl_area, "button-press-event", G_CALLBACK (on_button), NULL);
	g_signal_connect (gl_area, "button-release-event", G_CALLBACK (on_button), NULL);
	g_signal_connect (gl_area, "scroll-event", G_CALLBACK (on_scroll), NULL);
	g_signal_connect (gl_area, "motion-notify-event", G_CALLBACK(on_motion), NULL);

	// Build Settings Buttons
	GObject* build_button;
	GObject* load_button;
	GObject* save_button;
	GObject* check_button;
	GObject* free_button;
	GObject* init_button;
	GObject* refresh_button;
	build_button = gtk_builder_get_object(builder, "button_build");
	g_signal_connect (build_button, "clicked", G_CALLBACK (on_button_build), &data);
	load_button = gtk_builder_get_object(builder, "button_load");
	g_signal_connect (load_button, "clicked", G_CALLBACK (on_button_load), &data);
	save_button = gtk_builder_get_object(builder, "button_save");
	g_signal_connect (save_button, "clicked", G_CALLBACK (on_button_save), &data);
	check_button = gtk_builder_get_object(builder, "button_check");
	g_signal_connect (check_button, "clicked", G_CALLBACK (on_button_check), &data);
	free_button = gtk_builder_get_object(builder, "button_free");
	g_signal_connect (free_button, "clicked", G_CALLBACK (on_button_free), &data);
	init_button = gtk_builder_get_object(builder, "button_init");
	g_signal_connect (init_button, "clicked", G_CALLBACK (on_button_init), &data);
	refresh_button = gtk_builder_get_object(builder, "button_refresh");
	g_signal_connect (refresh_button, "clicked", G_CALLBACK (on_button_refresh), &data);

	// value slider
	GObject* value_slider;
	value_slider = gtk_builder_get_object(builder, "value_slider");
	g_signal_connect (value_slider, "value_changed", G_CALLBACK (on_value_changed), &data);
	gui.value_slider = GTK_SCALE(value_slider);

	//Radio Buttons
	GObject* default_radio;
	GObject* bitmap_radio;
//	GObject* largeFile_radio;
	default_radio = gtk_builder_get_object(builder, "model_type_radio_default");
	g_signal_connect (default_radio, "toggled", G_CALLBACK (on_radio_default), &data);
	bitmap_radio = gtk_builder_get_object(builder, "model_type_radio_bitmap");
	g_signal_connect (bitmap_radio, "toggled", G_CALLBACK (on_radio_bitmap), &data);


	// Build Settings Entries
	GtkEntry* mem_entry;
	GtkEntry* resolution_entry;
	GtkEntry* hdd_entry;
	GtkEntry* build_layers_entry;
	GtkEntry* tdms_dir_entry;
	GtkEntry* filename_entry;
	mem_entry = GTK_ENTRY( gtk_builder_get_object(builder, "max_mem_entry") );
	g_signal_connect (mem_entry, "activate", G_CALLBACK (on_entry_max_mem), &data);
	resolution_entry = GTK_ENTRY( gtk_builder_get_object(builder, "resolution_entry") );
	g_signal_connect (resolution_entry, "activate", G_CALLBACK (on_entry_resolution), &data);
	hdd_entry = GTK_ENTRY( gtk_builder_get_object(builder, "hdd_mem_entry") );
	g_signal_connect (hdd_entry, "activate", G_CALLBACK (on_entry_hdd), &data);
	build_layers_entry = GTK_ENTRY( gtk_builder_get_object(builder, "build_layers_entry") );
	g_signal_connect (build_layers_entry, "activate", G_CALLBACK (on_entry_layers), &data);
	tdms_dir_entry = GTK_ENTRY( gtk_builder_get_object(builder, "tdms_data_directory_entry") );
	g_signal_connect (tdms_dir_entry, "activate", G_CALLBACK (on_entry_tdms_dir), &data);
	filename_entry = GTK_ENTRY( gtk_builder_get_object(builder, "model_filename_entry") );
	g_signal_connect (filename_entry, "activate", G_CALLBACK (on_entry_filename), &data);





	// menu buttons
	GObject* open_file_button;
	open_file_button = gtk_builder_get_object(builder, "open_file_button");
	g_signal_connect (open_file_button, "activate", G_CALLBACK (on_file_chooser_open), &data);
	GObject* save_file_button;
	save_file_button = gtk_builder_get_object(builder, "save_file_button");
	g_signal_connect (save_file_button, "activate", G_CALLBACK (on_file_chooser_save), &data);
	GtkWidget* quit_button;
	quit_button = GTK_WIDGET(gtk_builder_get_object(builder, "quit_application_button"));
	g_signal_connect (quit_button, "activate", G_CALLBACK(on_window_main_destroy), &data);


	// Color Picker
	GObject* color_switch;
	color_switch = gtk_builder_get_object(builder, "show-color-picker");
	g_signal_connect (color_switch, "state-set", G_CALLBACK (on_color_switch), &data);

	GtkColorChooserWidget* color_picker;
	color_picker = GTK_COLOR_CHOOSER_WIDGET(gtk_builder_get_object(builder, "color-picker"));
	g_signal_connect ( (GObject*) color_picker, "notify::rgba", G_CALLBACK(on_color_choose), &data);
	gui.color_picker = color_picker;

	/*
	GObject* color_select_button;
	color_select_button = gtk_builder_get_object(builder, "color-select");
	g_signal_connect (color_select_button, "clicked", G_CALLBACK (on_button_select_color), &data);
	gui.color_select_button = GTK_BUTTON(color_select_button);
*/

	// File Chooser Dialog
	GtkWidget* file_window;
	file_window = GTK_WIDGET(gtk_builder_get_object(builder, "file_chooser_dialog"));
	gui.file_window = file_window;

	//g_signal_connect(window, "destroy", G_CALLBACK(on_window_main_destroy), &data);
	GObject* file_chooser_action_button;
	file_chooser_action_button = gtk_builder_get_object(builder, "file_chooser_open");
	g_signal_connect (file_chooser_action_button, "clicked", G_CALLBACK (on_file_chooser_select), &data);
	GObject* file_chooser_cancel_button;
	file_chooser_cancel_button = gtk_builder_get_object(builder, "file_chooser_exit");
	g_signal_connect (file_chooser_cancel_button, "clicked", G_CALLBACK (on_file_chooser_cancel), &data);
	GObject* file_chooser_entry_bar;
	file_chooser_entry_bar = gtk_builder_get_object(builder, "file_chooser_entry_bar");
	//g_signal_connect (file_chooser_cancel_button, "clicked", G_CALLBACK (on_file_chooser_cancel), &data);
	gui.entry_bar = GTK_WIDGET(file_chooser_entry_bar);


	// TextBuffer
	GtkTextBuffer* text_buffer;
	text_buffer = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "terminal_buffer"));
	gui.terminal_buffer = text_buffer;
	GtkTextView* text_view;
	text_view = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "terminal_textview"));
	gui.terminal_textview = text_view;
	GtkScrolledWindow* scrolled_window;
	scrolled_window = GTK_SCROLLED_WINDOW(gtk_builder_get_object(builder, "scrolled_window"));
	gui.scrolled_window = scrolled_window;



	/* Launch Terminal Backend */
	GError* err = NULL;
	launch_backend(&data);
	g_io_channel_set_flags(gui.bknd_out, G_IO_FLAG_NONBLOCK, &err);
	g_io_channel_set_flags(gui.bknd_in, G_IO_FLAG_IS_WRITABLE, &err);

	g_io_add_watch(gui.bknd_out, G_IO_IN, (GIOFunc) appendToTerminal, &data);


	/*  OpenGl Frametime   */
	last_frametime = clock();
	//cout << "Ticks per sec: " << CLOCKS_PER_SEC << endl;

	// destroy builder
    g_object_unref(builder);

	//maximize window
	gtk_window_maximize(GTK_WINDOW(window));

	//cout << "window show" << endl;
	// Show
	gtk_widget_show(window);
	gtk_main();

}
