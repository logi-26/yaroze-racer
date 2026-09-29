CFLAGS = -O3 -g -I/path/to/your/pad/
LINKER = -Xlinker -Ttext -Xlinker 80160000

# Windows (PSX3 in the VM) uses the tools on its PATH. Anywhere else (WSL)
# uses the toolchain from toolchain/, installed in /opt/yaroze.
ifneq ($(OS),Windows_NT)
CC = mipsel-unknown-ecoff-gcc
export PATH := /opt/yaroze/bin:$(PATH)
PCSX_REDUX ?= /mnt/c/pcsx-redux-23726-20260420-7/pcsx-redux.exe

# Files the auto script loads, so psx.exe is rebuilt when an asset changes.
ASSETS := $(shell tr -d '\r' < auto | tr '\\' '/' | sed -n 's/^local dload *\([^ \t]*\).*/\1/p')

# Link play: in an emulator the Net Yaroze monitor's tty driver isn't
# installed, so YarIO brings its own (engine/yario_emu.c) when built with
# LINK_EMU=1. On by default here, where the game runs in PCSX-Redux; build
# for a real Net Yaroze with: make LINK_EMU=0
LINK_EMU ?= 1
ifeq ($(LINK_EMU),1)
LINK_CFLAGS = -DYARIO_EMU
endif
# yario.o is rebuilt when LINK_EMU changes
LINK_STAMP := .link_emu_$(LINK_EMU)
endif

PROG = main.exe
OBJS = main.o state_manager.o graphics.o controller.o memcard.o audio.o font.o timer.o lang.o ui.o sincos.o asset_manager.o profiler.o menu_main.o menu_memcard.o menu_memcard_load.o menu_memcard_save.o menu_options.o menu_lobby.o menu_vehicle_select.o menu_pause.o gameplay.o gameover.o memcard_context.o message.o keyboard.o world.o light.o player.o model.o game.o suspension.o gear.o hud.o ground.o calculations.o car_controls.o brakelights.o vehicle_colour.o sky.o ai_racer.o link.o yario.o yario_emu.o

all: $(PROG) psx.exe

$(PROG): $(OBJS)
	$(CC) $(LINKER) -o $@ $^ -lps

main.o: main.c main.h engine/state_manager.h engine/graphics.h engine/controller.h engine/memcard.h engine/audio.h engine/font.h engine/timer.h
	$(CC) $(CFLAGS) -c main.c

graphics.o: engine/graphics.c engine/graphics.h
	$(CC) $(CFLAGS) -c engine/graphics.c

controller.o: engine/controller.c engine/controller.h
	$(CC) $(CFLAGS) -c engine/controller.c

memcard.o: engine/memcard.c engine/memcard.h
	$(CC) $(CFLAGS) -c engine/memcard.c

memcard_context.o: engine/memcard_context.c engine/memcard_context.h
	$(CC) $(CFLAGS) -c engine/memcard_context.c

audio.o: engine/audio.c engine/audio.h
	$(CC) $(CFLAGS) -c engine/audio.c

font.o: engine/font.c engine/font.h
	$(CC) $(CFLAGS) -c engine/font.c

keyboard.o: engine/keyboard.c engine/keyboard.h
	$(CC) $(CFLAGS) -c engine/keyboard.c

message.o: engine/message.c engine/message.h
	$(CC) $(CFLAGS) -c engine/message.c

timer.o: engine/timer.c engine/timer.h
	$(CC) $(CFLAGS) -c engine/timer.c

lang.o: engine/lang.c engine/lang.h
	$(CC) $(CFLAGS) -c engine/lang.c

ui.o: engine/ui.c engine/ui.h
	$(CC) $(CFLAGS) -c engine/ui.c


sincos.o: engine/sincos.c engine/sincos.h
	$(CC) $(CFLAGS) -c engine/sincos.c



asset_manager.o: engine/asset_manager.c engine/asset_manager.h
	$(CC) $(CFLAGS) -c engine/asset_manager.c


profiler.o: engine/profiler.c engine/profiler.h
	$(CC) $(CFLAGS) -c engine/profiler.c



link.o: engine/link.c engine/link.h engine/yario.h engine/state_manager.h engine/graphics.h
	$(CC) $(CFLAGS) -c engine/link.c

yario.o: engine/yario.c engine/yario.h engine/yario_emu.h $(LINK_STAMP)
	$(CC) $(CFLAGS) $(LINK_CFLAGS) -c engine/yario.c

# The BIOS calls this tty driver's functions: don't rely on $$gp there
yario_emu.o: engine/yario_emu.c engine/yario_emu.h
	$(CC) $(CFLAGS) -G 0 -c engine/yario_emu.c

state_manager.o: engine/state_manager.c engine/state_manager.h engine/ui.h
	$(CC) $(CFLAGS) -c engine/state_manager.c

menu_main.o: states/menu_main.c engine/state_manager.h engine/font.h engine/colours.h engine/controller.h engine/audio.h engine/graphics.h
	$(CC) $(CFLAGS) -c states/menu_main.c

menu_memcard.o: states/menu_memcard.c engine/state_manager.h engine/font.h engine/colours.h engine/controller.h engine/audio.h engine/graphics.h
	$(CC) $(CFLAGS) -c states/menu_memcard.c

menu_memcard_load.o: states/menu_memcard_load.c engine/state_manager.h engine/font.h engine/colours.h engine/controller.h engine/audio.h engine/graphics.h
	$(CC) $(CFLAGS) -c states/menu_memcard_load.c

menu_memcard_save.o: states/menu_memcard_save.c engine/state_manager.h engine/font.h engine/colours.h engine/controller.h engine/audio.h engine/graphics.h
	$(CC) $(CFLAGS) -c states/menu_memcard_save.c

menu_options.o: states/menu_options.c engine/state_manager.h engine/font.h engine/colours.h engine/controller.h engine/audio.h engine/graphics.h engine/ui.h
	$(CC) $(CFLAGS) -c states/menu_options.c


menu_lobby.o: states/menu_lobby.c engine/link.h engine/state_manager.h engine/font.h engine/colours.h engine/controller.h engine/audio.h engine/graphics.h engine/ui.h
	$(CC) $(CFLAGS) -c states/menu_lobby.c


menu_vehicle_select.o: states/menu_vehicle_select.c engine/state_manager.h engine/font.h engine/colours.h engine/controller.h engine/graphics.h engine/light.h engine/model.h game/player.h game/world.h
	$(CC) $(CFLAGS) -I. -c states/menu_vehicle_select.c

menu_pause.o: states/menu_pause.c engine/state_manager.h engine/font.h engine/colours.h engine/controller.h engine/audio.h engine/graphics.h
	$(CC) $(CFLAGS) -c states/menu_pause.c

gameplay.o: states/gameplay.c engine/state_manager.h engine/controller.h engine/graphics.h engine/model.h engine/light.h game/car_controls.h game/player.h game/game.h game/world.h game/hud.h game/brakelights.h
	$(CC) $(CFLAGS) -I. -c states/gameplay.c

gameover.o: states/gameover.c engine/state_manager.h engine/font.h engine/colours.h engine/controller.h engine/audio.h engine/graphics.h
	$(CC) $(CFLAGS) -c states/gameover.c


world.o: game/world.c game/world.h game/sky.h engine/graphics.h engine/model.h engine/calculations.h game/player.h game/ground.h
	$(CC) $(CFLAGS) -I. -c game/world.c

light.o: engine/light.c engine/light.h
	$(CC) $(CFLAGS) -c engine/light.c

player.o: game/player.c game/player.h engine/graphics.h game/game.h
	$(CC) $(CFLAGS) -I. -c game/player.c

model.o: engine/model.c engine/model.h engine/calculations.h
	$(CC) $(CFLAGS) -c engine/model.c

game.o: game/game.c game/game.h engine/graphics.h engine/calculations.h game/player.h game/world.h game/ground.h
	$(CC) $(CFLAGS) -I. -c game/game.c

suspension.o: game/suspension.c game/suspension.h
	$(CC) $(CFLAGS) -I. -c game/suspension.c

gear.o: game/gear.c game/gear.h
	$(CC) $(CFLAGS) -I. -c game/gear.c

hud.o: game/hud.c game/hud.h game/gear.h game/player.h game/ground.h engine/font.h engine/colours.h engine/ui.h engine/timer.h
	$(CC) $(CFLAGS) -I. -c game/hud.c

ground.o: game/ground.c game/ground.h
	$(CC) $(CFLAGS) -I. -c game/ground.c

calculations.o: engine/calculations.c engine/calculations.h
	$(CC) $(CFLAGS) -c engine/calculations.c

car_controls.o: game/car_controls.c game/car_controls.h game/player.h game/game.h engine/controller.h
	$(CC) $(CFLAGS) -I. -c game/car_controls.c

brakelights.o: game/brakelights.c game/brakelights.h
	$(CC) $(CFLAGS) -I. -c game/brakelights.c

vehicle_colour.o: game/vehicle_colour.c game/vehicle_colour.h
	$(CC) $(CFLAGS) -I. -c game/vehicle_colour.c

sky.o: game/sky.c game/sky.h game/world.h game/player.h engine/graphics.h
	$(CC) $(CFLAGS) -I. -c game/sky.c

ai_racer.o: game/ai_racer.c game/ai_racer.h game/player.h game/vehicle_attribs.h game/suspension.h game/game.h game/gear.h game/world.h
	$(CC) $(CFLAGS) -I. -c game/ai_racer.c

rebuild:
	$(MAKE) clean
	echo ---------- building ------
	$(MAKE) all

psx.exe: $(PROG) auto $(ASSETS)
	yarexe auto

ifneq ($(OS),Windows_NT)
# Rebuilds the TMD models from their RSD sources, like data/game/CONV_ALL.BAT:
# runs the rsdlink line of each CONV2.BAT it calls, in that folder
# (rsdlink from toolchain/).
.PHONY: models
models:
	@for bat in $$(tr -d '\r' < data/game/CONV_ALL.BAT | sed -n 's/^call  *//p' | tr '\\' '/'); do \
		echo "== data/game/$$(dirname $$bat)"; \
		(cd data/game/$$(dirname $$bat) && tr -d '\r' < $$(basename $$bat) | grep -i '^rsdlink' | sh) || exit 1; \
	done
endif

ifeq ($(OS),Windows_NT)
run: psx.exe
	nopsx psx.exe
else
run: psx.exe
	"$(PCSX_REDUX)" -run -loadexe psx.exe

# Link play in two PCSX-Redux linked through their serial ports (SIO1)
link: psx.exe
	powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$$(wslpath -w tools/run-linked.ps1)" \
		-Emulator "$$(wslpath -w '$(PCSX_REDUX)')" -Exe psx.exe

$(LINK_STAMP):
	rm -f .link_emu_*
	touch $@
endif

pcsxr: $(PROG)
	"C:\emulators\pcsxr\pcsxr.exe" -yaroze "C:\code\yaroze_engine\AUTO"

release:
	make clean
	echo ---------- RELEASE ------
	make CFLAGS=" -O3 -Werror -DNETYAROZE -D_RELEASE_"  all
	yarexe AUTO
	make psx.iso

psx.iso: psx.exe
	$(info yariso creates iso based on regions (SCEI, SCEA, SCEE) and folder, NYISO or EXEROM folders)
	$(info NY = NYISO)
	$(info - = EXEROM)
	rm -f psx.iso
	@echo putting psx.exe into psx.cue psx.bin
	yariso SCEI NY
	$(info yariso creates psx.iso in C:\psx3\sbin\yariso, you have to copy it back!)
	cp -u -f C:\psx3\sbin\yariso\psx.iso .
	cp -u -f C:\psx3\sbin\yariso\psx.iso  C:\code\yaroze_engine\

PLAYISO_NOPSX: psx.iso
	$(info nopsx has issues playing the .cue file???)
	call nopsx psx.iso

clean:
ifeq ($(OS),Windows_NT)
	$(RM) $(PROG)
	$(RM) $(OBJS)
	@if exist psx.exe del psx.exe
	@if exist main.exe del main.exe
	@if exist psx.iso del psx.iso
else
	rm -f $(PROG) main *.o psx.exe psx.iso combine.tmp combEco.exe .link_emu_*
endif