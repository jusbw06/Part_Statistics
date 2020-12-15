####### Input directories

SRCDIR   = ./src/
INCDIR	 = ./include/

TDMSSRC	 = ./libtdms/src/

####### Output directory

OBJDIR   = tmp/
OUTPUTDIR= bin/

CFLAGS	 = -g -Wall #-pedantic
LIBS 	 = -lm -lconfig 
LIBTDMS  = -lconfig
TDMS_INC = -I./libtdms/include/


part_stat_objects	=	$(OBJDIR)manager.o\
						$(OBJDIR)model_utils.o\
						$(OBJDIR)plot_utils.o\
						$(OBJDIR)file_utils.o\
						$(OBJDIR)settings_utils.o\
						
part_stat_headers	=	$(INCDIR)globalVars.h\
						$(INCDIR)model.h\
						$(INCDIR)plotting.h\
						$(INCDIR)file.h\
						$(INCDIR)settings.h\

part_stat_c_files	=	$(SRCDIR)manager.c\
						$(SRCDIR)model_utils.c\
						$(SRCDIR)plot_utils.c\
						$(SRCDIR)file_utils.c\
						$(SRCDIR)settings_utils.c\

tdms_objects		=	$(OBJDIR)tdms_readlayer.o\
						$(OBJDIR)model_utils.o\
						$(OBJDIR)settings_utils.o\
						

tdms_library_objects	=   $(OBJDIR)TdmsChannel.o\
							$(OBJDIR)TdmsGroup.o\
							$(OBJDIR)TdmsLeadIn.o\
							$(OBJDIR)TdmsMetaData.o\
							$(OBJDIR)TdmsObject.o\
							$(OBJDIR)TdmsParser.o\
							$(OBJDIR)TdmsSegment.o\

TARGET	 = part_stat tdms_readlayer

####### Build rules

all: $(TARGET)

part_stat: $(part_stat_objects)
	g++ -I$(INCDIR) $(CFLAGS) $(LIBS) -o $(OUTPUTDIR)$@ $(part_stat_objects)

tdms_readlayer: $(tdms_objects)
	g++ -I$(INCDIR) $(LIBTDMS) $(CFLAGS) -o $(OUTPUTDIR)$@ $(tdms_objects) $(tdms_library_objects)

clean:
	rm $(OBJDIR)*.o
	rm $(OUTPUTDIR)*

####### Compile
$(OBJDIR)%.o: $(SRCDIR)%.c
	g++ -c $(CFLAGS) -I$(INCDIR) -o $@ $^

$(OBJDIR)tdms_readlayer.o: $(SRCDIR)tdms_readlayer.cpp $(tdms_library_objects)
	g++ -c $(TDMS_INC) -I$(INCDIR) $(CFLAGS) -I$(INCDIR) -o $@ $<

$(OBJDIR)%.o: $(TDMSSRC)%.cpp
	g++ -c -O2 $(TDMS_INC) -I$(INCDIR) $(CFLAGS) -I$(INCDIR) -o $@ $^

#gcc manager.c -g -Wall -pedantic -o manager
#g++ tdms_readlayer.cpp -g -O2 -Wall -Llibtdms/lib -ltdms -Ilibtdms/include -o tdmsread
#gcc settings.c -g -Wall -pedantic -lconfig -lm -o settings
