CC = C:/raylib/w64devkit/bin/gcc

CFLAGS := -s -IEmiBase/include -IGame/include -IC:/raylib/raylib/src

CFLAGS_REL := -O2 -mwindows -DRELEASE
CFLAGS_DBG := -O0 \
			  -IEmiBase/external/raylib-nuklear/include \
			  -DGIT_HASH=\"$(shell git rev-parse --short HEAD | tr a-z A-Z)\" \
			  -DGIT_DIRTY=\"$(shell git diff --quiet -- EmiBase/src EmiBase/include/EmiBase EmiBase/include/EmiObject EmiBase/include/Libraries || echo -dirty)\" \
			  $(shell branch=$$(git rev-parse --abbrev-ref HEAD); \
				if [ "$$branch" != "main" ]; then \
					printf '%s' "-DGIT_BRANCH=\\\"-$$branch\\\""; \
				fi)

RAYLIB_SRC ?= C:\\raylib\\raylib\\src

LDFLAGS = -L. -L$(RAYLIB_SRC) -Wl,--gc-sections
LDLIBS  = -lraylib -lopengl32 -lgdi32 -ldwmapi -lwinmm

OBJ_DIR    := build_temp
RES_SOURCE := tools/resources.rc
RES_FILE   := tools/resources.res
OUT        := EmiBase.exe

SRCS := $(wildcard EmiBase/src/*.c EmiBase/src/**/*.c Game/src/*.c Game/src/**/*.c)
OBJS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

MAKEFLAGS += -j32
.PHONY: all debug release clean
.SECONDARY: $(PSTR_SRCS)
all: debug

debug: CFLAGS += $(CFLAGS_DBG)
debug: $(OBJS) $(RES_FILE)
	$(CC) $(CFLAGS) -o $(OUT) $^ $(LDFLAGS) $(LDLIBS)
	python tools/pak_builder.py
	@echo === Debug build success! ===
	./$(OUT)

release: CFLAGS += $(CFLAGS_REL)
release: $(OBJS) $(RES_FILE)
	$(CC) $(CFLAGS) -o $(OUT) $^ $(LDFLAGS) $(LDLIBS)
	python tools/pak_builder.py
	@echo === Release build success! ===

$(OBJ_DIR)/%.o: %.c
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -c $< -o $@

$(RES_FILE): $(RES_SOURCE)
	windres $(RES_SOURCE) -O coff -o $(RES_FILE)

clean:
	-rm -f $(OUT)
	-rm -f $(RES_FILE)
	-rm -rf $(OBJ_DIR)