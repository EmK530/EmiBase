CC_WIN = C:\\raylib\\w64devkit\\bin\\gcc

BASE_CFLAGS = -O2 -s -Iinclude

RAYLIB_SRC_PATH       ?= C:\\raylib\\raylib\\src
RAYLIB_INCLUDE_PATH   ?= $(RAYLIB_SRC_PATH)
RAYLIB_LIB_PATH       ?= $(RAYLIB_SRC_PATH)

LDFLAGS_WIN = -L. -L$(RAYLIB_LIB_PATH) -Wl,--gc-sections
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm -lcomdlg32 -lole32 -ldwmapi
LDLIBS += -static -lpthread

OBJ_DIR := build_temp
SRC_DIR := src
RES_FILE := tools/resources.res
UPX := tools\upx.exe

OUT_NAME := EmiBase.exe
UPX_NAME := EmiBase_UPX.exe

SRCS := $(wildcard src/*.c) \
		$(wildcard src/Libraries/*.c) \
		$(wildcard src/Scenes/*.c) \
		$(wildcard src/EmiObject/*.c) \
		$(wildcard src/EmiBase/*.c)

CFLAGS_WIN := $(BASE_CFLAGS)
CFLAGS_WIN_REL := $(CFLAGS_WIN) -mwindows -DSUB_WINDOWS
# mwindows disables console and DSUB_WINDOWS disables internal logging!

OBJS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean upx

all: compiledbg

release: compile #upx

compiledbg: $(OBJS) $(RES_FILE)
	$(CC_WIN) $(CFLAGS_WIN) -o $(OUT_NAME) $^ $(LDFLAGS_WIN) $(LDLIBS)
	python tools/pak_builder.py
	./$(OUT_NAME)

compile: CFLAGS_WIN := $(CFLAGS_WIN_REL)
compile: $(OBJS) $(RES_FILE)
	$(CC_WIN) $(CFLAGS_WIN) -o $(OUT_NAME) $^ $(LDFLAGS_WIN) $(LDLIBS)
	python tools/pak_builder.py

$(OBJ_DIR)/%.o: %.c
	@mkdir -p "$(dir $@)"
	$(CC_WIN) $(CFLAGS_WIN) -c $< -o $@

upx: compile
	cmd /C "if exist $(UPX) $(UPX) --ultra-brute $(OUT_NAME) -o $(UPX_NAME)"

clean:
	-rm -f $(OUT_NAME) $(UPX_NAME)
	-rm -rf $(OBJ_DIR)