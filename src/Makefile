ifeq ($(OS),Windows_NT)
	GUI_CFLAGS = -Wl,--export-all-symbols -g -pedantic -ansi -Wall \
		-finput-charset=UTF-8
else
	GUI_CFLAGS = -Wl,--export-dynamic -g -pedantic -ansi -Wall \
		-finput-charset=UTF-8
endif

CC = gcc
CLI_CFLAGS = -DCLI -g -pedantic -ansi -Wall -finput-charset=UTF-8 `pkg-config \
	--cflags libcurl libxml-2.0`
CLI_LDFLAGS = `pkg-config --libs libcurl libxml-2.0` -lm
CLI_OBJECTS = mediadata_wdtv.o parser.o writer.o
CLI_SOURCES = mediadata_wdtv.c parser.c writer.c
GUI_LDFLAGS = `pkg-config --cflags --libs gtk+-3.0`
GUI_SOURCE = mediadata_wdtv_gui.c

all: gui cli

gui:
	$(CC) $(GUI_CFLAGS) $(GUI_SOURCE) -o mediadata_wdtv_gui $(GUI_LDFLAGS)

cli: $(CLI_OBJECTS)
	$(CC) $(CLI_CFLAGS) $(CLI_OBJECTS) $(CLI_LDFLAGS) -o mediadata_wdtv

%.o: %.c
	$(CC) $(CLI_CFLAGS) -c $(CLI_SOURCES)

clean:
	rm -f *.o mediadata_wdtv mediadata_wdtv_gui mediadata_wdtv.exe \
		mediadata_wdtv_gui.exe
	