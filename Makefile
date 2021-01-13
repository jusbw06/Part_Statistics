####### Input directories

# necessary pacakges libconfig gnuplot glfw-x11 glew eigen glm glad

SRCDIR   = ./src/
INCDIR	 = ./include/ -I./include/model/

TDMSSRC	 = ./libtdms/src/
GUISRCDIR   = ./gui/
MODELSRCDIR	= ./src/model_utils/

####### Output directory

OBJDIR   = tmp/
OUTPUTDIR= bin/

CXX		 = g++
CFLAGS	 = -g -Wall
LIBS 	 = -lm -lconfig 
LIBTDMS  = -lconfig
LIBGUI	 = `pkg-config --cflags --libs gtk+-3.0` -lglad
TDMS_INC = -I./libtdms/include/

part_stat_model_objects = 	$(OBJDIR)model_bitArray.o\
							$(OBJDIR)model_default.o\
							$(OBJDIR)model.o\
							#$(OBJDIR)settings_utils.o\

part_stat_objects	=	$(OBJDIR)part_stat.o\
						$(OBJDIR)plot_utils.o\
						$(OBJDIR)file_utils.o\
						$(OBJDIR)settings_utils.o\

gui_objects			=	$(OBJDIR)gui.o\
						$(OBJDIR)user_input.o\
						$(OBJDIR)app_control.o\
						$(OBJDIR)open_gl.o\
						$(OBJDIR)Program.o\
						$(OBJDIR)GLSL.o\
						$(OBJDIR)messages.o\
						$(OBJDIR)file_utils.o\
						$(OBJDIR)settings_utils.o\

part_stat_headers	=	$(INCDIR)globalVars.h\
						$(INCDIR)model.h\
						$(INCDIR)plotting.h\
						$(INCDIR)file.h\
						$(INCDIR)settings.h\
						$(INCDIR)GLSL.h\
						$(INCDIR)Program.h\
						$(INCDIR)gui.h\
						

part_stat_c_files	=	$(SRCDIR)manager.c\
						$(SRCDIR)model_utils.c\
						$(SRCDIR)plot_utils.c\
						$(SRCDIR)file_utils.c\
						$(SRCDIR)settings_utils.c\

part_stat_cpp_files	=	$(SRCDIR)GLSL.cpp\
						$(SRCDIR)gui.cpp\
						$(SRCDIR)Program.cpp\


tdms_objects		=	$(OBJDIR)tdms_readlayer.o\
						$(OBJDIR)settings_utils.o\
						$(OBJDIR)file_utils.o\
						

tdms_library_objects	=   $(OBJDIR)TdmsChannel.o\
							$(OBJDIR)TdmsGroup.o\
							$(OBJDIR)TdmsLeadIn.o\
							$(OBJDIR)TdmsMetaData.o\
							$(OBJDIR)TdmsObject.o\
							$(OBJDIR)TdmsParser.o\
							$(OBJDIR)TdmsSegment.o\

TARGET	 = part_stat tdms_readlayer gui_stat

####### Build rules

all: $(TARGET)

part_stat: $(part_stat_objects) $(part_stat_model_objects)
	$(CXX) -I$(INCDIR) $(CFLAGS) $(LIBS) -o $(OUTPUTDIR)$@ $(part_stat_objects) $(part_stat_model_objects)
	
gui_stat:  $(gui_objects) $(part_stat_model_objects)
	$(CXX) -I$(INCDIR) $(CFLAGS) $(LIBS) $(LIBGUI) -o $(OUTPUTDIR)$@ $(gui_objects) $(part_stat_model_objects)

tdms_readlayer: $(tdms_objects) $(tdms_library_objects) $(part_stat_model_objects)
	$(CXX) -I$(INCDIR) $(LIBTDMS) $(CFLAGS) -o $(OUTPUTDIR)$@ $(tdms_objects) $(tdms_library_objects) $(part_stat_model_objects)

clean:
	rm $(OBJDIR)*.o
	rm $(OUTPUTDIR)*

####### Compile
$(OBJDIR)%.o: $(SRCDIR)%.c
	$(CXX) -c $(CFLAGS) -I$(INCDIR) -o $@ $^
	
$(OBJDIR)%.o: $(SRCDIR)%.cpp
	$(CXX) -c $(CFLAGS) -I$(INCDIR) -o $@ $^
	
$(OBJDIR)%.o: $(GUISRCDIR)%.cpp
	$(CXX) -c $(CFLAGS) $(LIBGUI) -I$(INCDIR) -o $@ $^
	
$(OBJDIR)%.o: $(GUISRCDIR)%.c
	$(CXX) -c $(CFLAGS) $(LIBGUI) -I$(INCDIR) -o $@ $^
	
$(OBJDIR)%.o: $(MODELSRCDIR)%.c
	$(CXX) -c $(CFLAGS) $(LIBGUI) -I$(INCDIR) -o $@ $^

$(OBJDIR)tdms_readlayer.o: $(SRCDIR)tdms_readlayer.cpp $(tdms_library_objects)
	$(CXX) -c $(TDMS_INC) -I$(INCDIR) $(CFLAGS) -I$(INCDIR) -o $@ $<

$(OBJDIR)%.o: $(TDMSSRC)%.cpp
	$(CXX) -c -O2 $(TDMS_INC) -I$(INCDIR) $(CFLAGS) -I$(INCDIR) -o $@ $^
