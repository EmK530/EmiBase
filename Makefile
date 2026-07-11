CC = C:/raylib/w64devkit/bin/gcc

CFLAGS := -O0 -s \
         -IEmiBase/include \
         -IGame/include \
         -IEmiBase/external/raylib-nuklear/include \
         -DGIT_HASH=\"$(shell git rev-parse --short HEAD | tr a-z A-Z)\" \
         -DGIT_DIRTY=\"$(shell git diff --quiet -- EmiBase/src EmiBase/include/EmiBase EmiBase/include/EmiObject EmiBase/include/Libraries || echo -dirty)\"

RAYLIB_SRC ?= C:\\raylib\\raylib\\src

LDFLAGS = -L. -L$(RAYLIB_SRC) -Wl,--gc-sections
LDLIBS  = -lraylib -lopengl32 -lgdi32 -ldwmapi -lwinmm

OBJ_DIR    := build_temp
RES_SOURCE := tools/resources.rc
RES_FILE   := tools/resources.res
UPX        := tools\upx.exe
OUT        := EmiBase.exe

SRCS := $(wildcard EmiBase/src/*.c EmiBase/src/**/*.c Game/src/*.c Game/src/**/*.c)
OBJS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

PSTR_SRC_DIR := $(OBJ_DIR)/pstr_src
PSTR_OBJ_DIR := $(OBJ_DIR)/pstr_obj
PSTR_SRCS    := $(patsubst %.c,$(PSTR_SRC_DIR)/%.c,$(SRCS))
PSTR_OBJS    := $(patsubst %.c,$(PSTR_OBJ_DIR)/%.o,$(SRCS))

MAKEFLAGS += -j32
.PHONY: all debug release clean upx
.SECONDARY: $(PSTR_SRCS)
all: debug

debug: $(OBJS) $(RES_FILE)
	$(CC) $(CFLAGS) -o $(OUT) $^ $(LDFLAGS) $(LDLIBS)
	python tools/pak_builder.py
	@echo === Debug build success! ===
	./$(OUT)

release: CFLAGS += -mwindows -DRELEASE
release: $(PSTR_OBJS) $(RES_FILE)
	$(CC) $(CFLAGS) -o $(OUT) $^ $(LDFLAGS) $(LDLIBS)
	python tools/pak_builder.py
	@echo === Release build success! ===

$(OBJ_DIR)/%.o: %.c
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -c $< -o $@

$(PSTR_SRC_DIR)/%.c: %.c
	@mkdir -p "$(dir $@)"
	python tools/pstr_replacer.py $< $@

$(PSTR_OBJ_DIR)/%.o: $(PSTR_SRC_DIR)/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -c $< -o $@

$(RES_FILE): $(RES_SOURCE)
	windres $(RES_SOURCE) -O coff -o $(RES_FILE)

upx: release
	cmd /C "if exist $(UPX) $(UPX) --ultra-brute $(OUT) -o EmiBase_UPX.exe"

clean:
	-rm -f $(OUT) EmiBase_UPX.exe
	-rm -f $(RES_FILE)
	-rm -rf $(OBJ_DIR)