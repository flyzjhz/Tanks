#ARCH ?= arm-linux-
GCC ?= gcc
all:
	$(ARCH)$(GCC) -g -I./src ./src/client/tank.c ./src/core/level/level.c ./src/core/object/object.c -lSDL -o bin/cli
	$(ARCH)$(GCC) -g -I./src ./src/server/srv.c ./src/core/level/level.c ./src/core/object/object.c -lSDL -o bin/srv
