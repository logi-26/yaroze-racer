LIBPS_BIN = C:\libps.bin
TRANSFER_TOOL = C:\ps1transfer.exe
AUTO_FILE = auto
DATA_DIR = C:\code\car-basic-working

CFLAGS = -O3 -g -I/path/to/your/pad/
LINKER = -Xlinker -Ttext -Xlinker 80140000

PROG = main.exe
OBJS = main.o world.o light.o graphics.o player.o model.o game.o ground.o calculations.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LINKER) -o $@ $?

main.o: main.c main.h game/world.h game/light.h game/graphics.h game/player.h game/model.h game/game.h
	$(CC) $(CFLAGS) -c main.c

world.o: game/world.c game/world.h
	$(CC) $(CFLAGS) -I. -c game/world.c

light.o: game/light.c game/light.h
	$(CC) $(CFLAGS) -I. -c game/light.c

graphics.o: game/graphics.c game/graphics.h
	$(CC) $(CFLAGS) -I. -c game/graphics.c

player.o: game/player.c game/player.h
	$(CC) $(CFLAGS) -I. -c game/player.c

model.o: game/model.c game/model.h
	$(CC) $(CFLAGS) -I. -c game/model.c

game.o: game/game.c game/game.h
	$(CC) $(CFLAGS) -I. -c game/game.c
	
ground.o: game/ground.c game/ground.h
	$(CC) $(CFLAGS) -I. -c game/ground.c
	
calculations.o: game/calculations.c game/calculations.h
	$(CC) $(CFLAGS) -I. -c game/calculations.c

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
