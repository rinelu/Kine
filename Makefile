SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.PHONY: build format

BUILD_DIR ?= build
EXTERNAL_DIR ?= external
BIN_DIR ?= bin
TARGET ?= Kine
CMAKE_BUILD_TYPE ?= Debug
JOBS ?= $(shell (nproc 2>/dev/null || echo 4))
FORMAT_SOURCES := $(shell find . -regex '.*\.\(h\|hpp\|cpp\|cxx\|cc\)')

# GLFW
GLFW_REPO := https://github.com/glfw/glfw.git
GLFW_TAG := 3.4 # or master

FREETYPE_VERSION := 2.13.3
FREETYPE_URL := https://download.savannah.gnu.org/releases/freetype/freetype-$(FREETYPE_VERSION).tar.gz

IMGUI_URL := https://github.com/ocornut/imgui/archive/refs/heads/master.zip
LUA_URL := https://www.lua.org/ftp/lua-5.4.6.tar.gz

# Header-only libs
GLM_VERSION := 1.0.2
GLM_URL := https://github.com/g-truc/glm/releases/download/$(GLM_VERSION)/glm-$(GLM_VERSION).zip
ENTT_URL := https://github.com/skypjack/entt/archive/refs/heads/master.zip
JSON_URL := https://github.com/nlohmann/json/releases/latest/download/json.hpp
STB_URL := https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
MINIAUDIO_URL := https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h

ifeq (, $(shell which curl 2>/dev/null || true))
    DOWNLOAD = wget -q -O -
else
    DOWNLOAD = curl -sL
endif

all: build run
default: help

$(EXTERNAL_DIR):
	@mkdir -p $(EXTERNAL_DIR)

help:
	@echo ""
	@echo "Available commands:"
	@echo ""
	@echo "  make deps           Install all dependencies into ./external/"
	@echo "  make build          Configure + build using CMake"
	@echo "  make format		 Applies clang-fomrat to all .h, .hpp, .cpp, .cxx, .cc files recursively"
	@echo "  make run            Run the built executable ($(TARGET))"
	@echo "  make clean          Remove build/ and bin/ directories"
	@echo "  make distclean      Remove build/, bin/, and ALL dependencies"
	@echo ""
	@echo "Options:"
	@echo ""
	@echo "  CMAKE_BUILD_TYPE=<type>"
	@echo "      Specify build configuration."
	@echo "      Available: Debug, Release"
	@echo "      Default: Debug"
	@echo ""
	@echo "  JOBS=<n>"
	@echo "      Number of parallel build threads."
	@echo "      Default: auto-detected CPU cores ($(JOBS))"
	@echo ""
	@echo "  TARGET=<name>"
	@echo "      Name of the output executable."
	@echo "      Default: Kine"
	@echo ""
	@echo "  BUILD_DIR=<dir>"
	@echo "      CMake build directory."
	@echo "      Default: build/"
	@echo ""
	@echo "  BIN_DIR=<dir>"
	@echo "      Directory where the executable is placed."
	@echo "      Default: bin/"
	@echo ""
	@echo "  EXTERNAL_DIR=<dir>"
	@echo "      Folder where dependencies are downloaded."
	@echo "      Default: external/"
	@echo ""
	@echo "Examples:"
	@echo "    make deps"
	@echo "    make build CMAKE_BUILD_TYPE=Release"
	@echo "    make run"
	@echo "    make distclean"

deps: $(EXTERNAL_DIR)
	@echo "==> Installing dependencies into $(EXTERNAL_DIR)"

	@if [ ! -d $(EXTERNAL_DIR)/glfw ]; then \
		echo "Installing GLFW (Wayland + X11)"; \
		tmp=$$(mktemp -d); \
		git clone --depth 1 --branch $(GLFW_TAG) $(GLFW_REPO) "$$tmp/glfw"; \
		mkdir -p "$$tmp/glfw/build"; \
		cmake -S "$$tmp/glfw" -B "$$tmp/glfw/build" \
			-DBUILD_SHARED_LIBS=OFF \
			-DGLFW_BUILD_EXAMPLES=OFF \
			-DGLFW_BUILD_TESTS=OFF \
			-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
			-DGLFW_BUILD_DOCS=OFF \
			-DGLFW_BUILD_WAYLAND=ON \
			-DGLFW_BUILD_X11=ON; \
		cmake --build "$$tmp/glfw/build" -- -j$(JOBS); \
		mkdir -p $(EXTERNAL_DIR)/glfw/include $(EXTERNAL_DIR)/glfw/lib; \
		cp -r "$$tmp/glfw/include/GLFW" $(EXTERNAL_DIR)/glfw/include/; \
		cp "$$tmp/glfw/build/src/libglfw3.a" $(EXTERNAL_DIR)/glfw/lib/; \
		rm -rf "$$tmp"; \
	else echo "GLFW already present"; fi


	@if [ ! -f $(EXTERNAL_DIR)/glm/glm.hpp ]; then \
		echo "Installing GLM (header-only)"; \
		tmp=$$(mktemp -d); \
		$(DOWNLOAD) "$(GLM_URL)" > "$$tmp/glm.zip"; \
		unzip -q "$$tmp/glm.zip" -d "$$tmp"; \
		mkdir -p $(EXTERNAL_DIR)/glm/glm; \
		mv "$$tmp"/glm $(EXTERNAL_DIR)/glm || mv "$$tmp"/glm-* $(EXTERNAL_DIR)/glm; \
		rm -rf "$$tmp"; \
	else echo "GLM already present"; fi

	@if [ ! -f $(EXTERNAL_DIR)/entt/entt/entt.hpp ]; then \
		echo "Installing EnTT"; \
		tmp=$$(mktemp -d); \
		$(DOWNLOAD) "$(ENTT_URL)" > "$$tmp/entt.zip"; \
		unzip -q "$$tmp/entt.zip" -d "$$tmp"; \
		mkdir -p $(EXTERNAL_DIR)/entt/entt; \
		cp -r "$$tmp"/entt-*/single_include/* $(EXTERNAL_DIR)/entt; \
		rm -rf "$$tmp"; \
	else echo "EnTT already present"; fi

	@if [ ! -f $(EXTERNAL_DIR)/json/json.hpp ]; then \
		echo "Installing nlohmann/json"; \
		mkdir -p $(EXTERNAL_DIR)/json; \
		$(DOWNLOAD) "$(JSON_URL)" > $(EXTERNAL_DIR)/json/json.hpp; \
	else echo "JSON already present"; fi

	@if [ ! -f $(EXTERNAL_DIR)/stb/stb_image.h ]; then \
		echo "Installing stb_image"; \
		mkdir -p $(EXTERNAL_DIR)/stb; \
		$(DOWNLOAD) "$(STB_URL)" > $(EXTERNAL_DIR)/stb/stb_image.h; \
	else echo "stb_image already present"; fi

	@if [ ! -f $(EXTERNAL_DIR)/freetype/lib/libfreetype.a ]; then \
		echo "Installing FreeType $(FREETYPE_VERSION)"; \
		tmp=$$(mktemp -d); \
		$(DOWNLOAD) "$(FREETYPE_URL)" > "$$tmp/freetype.tar.gz"; \
		tar -xzf "$$tmp/freetype.tar.gz" -C "$$tmp"; \
		src=$$(find "$$tmp" -maxdepth 1 -type d -name "freetype-*"); \
		mkdir -p "$$tmp/freetype-build"; \
		cmake -S "$$src" -B "$$tmp/freetype-build" \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
			-DBUILD_SHARED_LIBS=OFF \
			-DFT_DISABLE_ZLIB=ON \
			-DFT_DISABLE_PNG=ON \
			-DFT_DISABLE_BZIP2=ON \
			-DFT_DISABLE_HARFBUZZ=ON; \
		cmake --build "$$tmp/freetype-build" -- -j$(JOBS); \
		mkdir -p $(EXTERNAL_DIR)/freetype/include $(EXTERNAL_DIR)/freetype/lib; \
		cp -r "$$tmp"/freetype-$(FREETYPE_VERSION)/include/* $(EXTERNAL_DIR)/freetype/include/; \
		cp "$$tmp"/freetype-build/libfreetype.a $(EXTERNAL_DIR)/freetype/lib/; \
		rm -rf "$$tmp"; \
	else echo "FreeType already present"; fi

	@if [ ! -f $(EXTERNAL_DIR)/miniaudio/miniaudio.h ]; then \
		echo "Installing MiniAudio"; \
		mkdir -p $(EXTERNAL_DIR)/miniaudio; \
		$(DOWNLOAD) "$(MINIAUDIO_URL)" > $(EXTERNAL_DIR)/miniaudio/miniaudio.h; \
	else echo "MiniAudio already present"; fi

	@if [ ! -f $(EXTERNAL_DIR)/imgui/imgui.h ]; then \
		echo "Installing ImGui (official)"; \
		tmp=$$(mktemp -d); \
		$(DOWNLOAD) "$(IMGUI_URL)" > "$$tmp/imgui.zip"; \
		unzip -q "$$tmp/imgui.zip" -d "$$tmp"; \
		mv "$$tmp"/imgui-* $(EXTERNAL_DIR)/imgui || mv "$$tmp"/imgui-master $(EXTERNAL_DIR)/imgui; \
		rm -rf "$$tmp"; \
	else echo "ImGui already present"; fi

	@if [ ! -f $(EXTERNAL_DIR)/sol2/sol.hpp ]; then \
		echo "Installing sol2 (header-only)"; \
		mkdir -p $(EXTERNAL_DIR)/sol2/sol; \
		$(DOWNLOAD) "https://github.com/ThePhD/sol2/releases/latest/download/sol.hpp" > $(EXTERNAL_DIR)/sol2/sol.hpp; \
		$(DOWNLOAD) "https://github.com/ThePhD/sol2/releases/latest/download/config.hpp" > $(EXTERNAL_DIR)/sol2/sol/config.hpp; \
		$(DOWNLOAD) "https://github.com/ThePhD/sol2/releases/latest/download/forward.hpp" > $(EXTERNAL_DIR)/sol2/sol/forward.hpp; \
	else echo "sol2 already present"; fi

	@if [ ! -d $(EXTERNAL_DIR)/lua ]; then \
		echo "Installing Lua 5.4"; \
		tmp=$$(mktemp); \
		$(DOWNLOAD) "$(LUA_URL)" > "$$tmp"; \
		mkdir -p $(EXTERNAL_DIR)/lua-src; \
		tar -xzf "$$tmp" -C $(EXTERNAL_DIR)/lua-src --strip-components=1; \
		rm -f "$$tmp"; \
		if [ "$$(uname -s)" = "Linux" ]; then \
			$(MAKE) -C $(EXTERNAL_DIR)/lua-src linux MYCFLAGS="-fPIC"; \
		elif [ "$$(uname -s)" = "Darwin" ]; then \
			$(MAKE) -C $(EXTERNAL_DIR)/lua-src macosx MYCFLAGS="-fPIC"; \
		else \
			$(MAKE) -C $(EXTERNAL_DIR)/lua-src mingw MYCFLAGS="-fPIC"; \
		fi; \
		mkdir -p $(EXTERNAL_DIR)/lua/include $(EXTERNAL_DIR)/lua/lib; \
		cp $(EXTERNAL_DIR)/lua-src/src/*.h $(EXTERNAL_DIR)/lua/include/; \
		cp $(EXTERNAL_DIR)/lua-src/src/liblua.a $(EXTERNAL_DIR)/lua/lib/; \
		rm -rf $(EXTERNAL_DIR)/lua-src; \
	else echo "Lua already present"; fi

	@echo "==> All dependencies installed."

format:
	@echo "==> Formatting Script (using clang-format)"
	@if ! command -v clang-format >/dev/null 2>&1; then \
		echo "> clang-format is not found"; \
		exit 1; \
	fi
	@find engine game -type f \( -name "*.hpp" -o -name "*.cpp" \) | while read -r FILE; do \
		echo "> Formatting $$FILE"; \
		clang-format -i "$$FILE"; \
	done
	@echo "==> Formatting complete"


build:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)
	@cd $(BUILD_DIR) && \
		( [ -f CMakeCache.txt ] || cmake -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) .. ) && \
		cmake --build . -- -j$(JOBS)
	@echo "==> Build complete."

run:
	@if [ -f $(BIN_DIR)/$(TARGET) ]; then \
		echo "Running $(TARGET)..."; \
		./$(BIN_DIR)/$(TARGET); \
	else echo "Executable $(TARGET) not found. Run 'make build'."; fi


clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

distclean: clean
	rm -rf $(EXTERNAL_DIR)

