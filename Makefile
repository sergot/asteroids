CC=gcc
LDFLAGS=-lallegro -lallegro_primitives -lallegro_font -lallegro_ttf -lallegro_color -lm
SOURCES=src/point.c \
        src/asteroid.c \
        src/ship.c \
        src/shooter.c \
		src/config.c \
        src/game.c \
        src/main.c
OBJ_DIR=obj
SRC_DIR=src
HEADERS=$(SOURCES:.c=.h)
OBJECTS=$(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
EXECUTABLE=asteroids

all: $(EXECUTABLE) $(OBJECTS)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJECTS): obj/%.o : src/%.c
	$(CC) -c $< -o $@

clean:
	rm -f obj/*.o $(EXECUTABLE)
