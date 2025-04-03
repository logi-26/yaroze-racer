LIBPS_BIN = C:\libps.bin
TRANSFER_TOOL = C:\ps1transfer.exe
AUTO_FILE = auto
DATA_DIR = C:\code\car-basic-working

CFLAGS = -O3 -g -I/path/to/your/pad/
LINKER = -Xlinker -Ttext -Xlinker 80140000

PROG = main.exe
OBJS = main.o world.o light.o graphics.o controller.o player.o model.o game.o pad.o ground.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LINKER) -o $@ $?

main.o: main.c main.h world.h light.h graphics.h player.h model.h game.h controller.h
	$(CC) $(CFLAGS) -c main.c

world.o: world.c world.h
	$(CC) $(CFLAGS) -c world.c

light.o: light.c light.h
	$(CC) $(CFLAGS) -c light.c

graphics.o: graphics.c graphics.h
	$(CC) $(CFLAGS) -c graphics.c

player.o: player.c player.h
	$(CC) $(CFLAGS) -c player.c

model.o: model.c model.h
	$(CC) $(CFLAGS) -c model.c

game.o: game.c game.h
	$(CC) $(CFLAGS) -c game.c

controller.o: controller.c controller.h
	$(CC) $(CFLAGS) -c controller.c

pad.o: pad.c pad.h
	$(CC) $(CFLAGS) -c pad.c
	
ground.o: ground.c ground.h
	$(CC) $(CFLAGS) -c ground.c

run: $(PROG)	
	del psx.exe 
	yarexe auto	
	nopsx psx.exe	

usb: $(PROG)
	@if exist ny.exe del ny.exe
	@if exist main.exe.exe del main.exe.exe
	C:\eco2exe main.exe
	mv main.exe.exe ny.exe
	
	$(TRANSFER_TOOL) -u -f $(LIBPS_BIN) -a 0x80010000
	
	@FOR /F "usebackq tokens=*" %%A IN (`FINDSTR /B /C:"local dload " $(AUTO_FILE)`) DO ( \
		FOR /F "tokens=3,4" %%B IN ("%%A") DO ( \
			echo %%~B :: 0x%%~C & \
			$(TRANSFER_TOOL) -u -f $(DATA_DIR)\%%~B -a 0x%%~C \
		) \
	)
	
	$(TRANSFER_TOOL) -e -f ny.exe

sio: $(PROG)	
		call cmd /C wincmd minicons auto

clean:
	del $(PROG)
	del *.o
	del psx.exe 
