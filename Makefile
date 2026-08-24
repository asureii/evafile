BUILD_DIR = build
PREFIX ?= /usr/local
USER_BIN = $(HOME)/.local/bin

.PHONY: all build clean run test install install-user

all: build

build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . -j$$(nproc)

clean:
	@rm -rf $(BUILD_DIR)

run: build
	@./$(BUILD_DIR)/evafile

test:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build . -j$$(nproc) && ctest --output-on-failure

install-user: build
	@mkdir -p $(USER_BIN)
	@cp --remove-destination $(BUILD_DIR)/evafile $(USER_BIN)/evafile
	@mkdir -p $(HOME)/.local/share/applications
	@cp resources/evafile.desktop $(HOME)/.local/share/applications/evafile.desktop
	@mkdir -p $(HOME)/.local/share/icons/hicolor/scalable/apps
	@cp resources/icons/evafile.svg $(HOME)/.local/share/icons/hicolor/scalable/apps/evafile.svg
	@update-desktop-database $(HOME)/.local/share/applications 2>/dev/null || true
	@gtk-update-icon-cache $(HOME)/.local/share/icons/hicolor 2>/dev/null || true
	@echo "Installed evafile to $(USER_BIN)/evafile and registered desktop entry"

install: build
	@sudo cmake --install $(BUILD_DIR)
