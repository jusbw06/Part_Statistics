
#include "globalVars.h"
#include "gui.hpp"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>

void waitAcknoledgement(struct app_data* data){

	/* Set Channels */
	struct gui_data* datag = (struct gui_data*) (data->gui_data);

	char buffer[100];
	read(datag->fd_ack, buffer, 3);

	//fprintf(stderr, "Ack Received\n");

	if (strcmp(buffer, "OK\n") != 0){
		fprintf(stderr, "Jarbled Ack Received\n");
	}

}

void sendCommand(const char* command, struct app_data* data){

	struct gui_data* datag = (struct gui_data*) (data->gui_data);
	int len = strlen(command);

	gsize bytes_written;
	GError* err = NULL;
	g_io_channel_write_chars (datag->bknd_in, command, len, &bytes_written, &err);
	g_io_channel_flush (datag->bknd_in, &err);

}

void send_InitMem(struct app_data* data){
	sendCommand("create_model\n", data);
	waitAcknoledgement(data);
}

void send_Close(struct app_data* data){
	sendCommand("exit\n", data);
	waitAcknoledgement(data);
}

void send_Check(struct app_data* data){
	sendCommand("print_model_stats\n", data);
}

void send_Free(struct app_data* data){
	sendCommand("free_model\n", data);
}

void send_Load(struct app_data* data){
	sendCommand("load_model\n",data);
}

void send_Save(struct app_data* data){
	sendCommand("save_model\n", data);
}

void send_Build(struct app_data* data){
	sendCommand("build_model\n", data);
}

void send_Zero(struct app_data* data){
	sendCommand("clear_model\n", data);
}
