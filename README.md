# EvaFile

EvaFile is a modern, high-performance C++20 / Qt6 file manager inspired by KDE Dolphin, designed with full Wayland / Hyprland tiling responsiveness, Eva ecosystem integration, and a Crimson Flame visual theme (`#cf2824` / `#e32a10`).

---

- **Hybrid QML + C++ Architecture**:
  - Modern, fluid presentation layer powered by QtQuick / QML with 60/120 FPS kinetic physics, obsidian glassmorphism, interactive 3D perspective tilt cards, and zero idle CPU power gating.
  - Robust C++20 backend controller and list models driving asynchronous file operations, Evalink aria2c RPC tasks, EvaSuite bridges, and live filesystem watching.
  - `--classic` CLI flag to launch the classic Qt Widgets desktop interface whenever desired.
- **Multi-Tab Sessions**: Open tabs with `Ctrl+T`, close with `Ctrl+W`, cycle with `Ctrl+Tab` or `Ctrl+Shift+Tab`, and restore previous sessions on launch.
- **Dual-Pane Split View (`F3`)**: Browse and transfer between directories side-by-side with synchronized active pane selection and breadcrumb tracking.
- **Interactive Breadcrumbs (`Ctrl+L`)**: Clickable segment pills for fast directory jumping, with instant inline editing and filesystem path auto-completion.
- **Crimson Flame Theme**: Custom dark theme with deep obsidian backgrounds (`#25090a`), vibrant crimson accents (`#cf2824`), and warm highlight glow (`#e32a10`).
- **Color-Coded Directories**: Normal directories are accented in crimson red, while hidden directories (starting with `.`) are styled in warm amber yellow for instant recognition.
- **Live Preview Drawer (`F11` / `Space`)**:
  - Syntax-highlighted code and plain text viewer.
  - Image preview with dimensions and file size.
  - PDF first-page rendering via Poppler-Qt6.
  - Media metadata viewer for audio and video files.
  - Real-time SHA256 checksum calculator.
- **Sidebar Places & Drives**: Access home, root, standard user folders (Documents, Downloads, Music, Pictures, Videos), mounted drives with live storage gauges, and user bookmarks.
- **Asynchronous File Operations**: Multithreaded Copy, Move, Trash, and Delete with transfer progress reporting and automatic duplicate conflict resolution (`file (1).ext`).
- **Interactive Drag and Drop**: Fluid, physics-aware drag-and-drop across file views, dual split panes (`F3`), sidebar places, breadcrumb pills, and external applications with Dolphin-style Move / Copy / Cancel action prompts and modifier keys (`Shift` to move, `Ctrl` to copy).
- **Tiling Window Manager Ready**: Dynamic responsive layout tailored for Hyprland, Sway, and i3 with auto-collapsing panels, eliding tabs, and column resizing.
- **Eva Suite Interoperability**:
  - `F4` or context menu: Launch EvaTerm directly in the current directory.
  - Context menu: Trigger EvaSort to organize messy folders automatically.
  - `Ctrl+D` / `Ctrl+Shift+D`: Built-in Evalink download accelerator with live progress drawer, speed indicators, and auto-refresh on download completion.

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl+T` | Open new tab |
| `Ctrl+W` | Close current tab |
| `Ctrl+Tab` / `Ctrl+Shift+Tab` | Switch to next / previous tab |
| `Ctrl+L` | Focus breadcrumb address bar |
| `Ctrl+H` | Toggle show hidden files |
| `Ctrl+B` | Toggle sidebar visibility |
| `Ctrl+F` | Open quick filter / search bar |
| `Ctrl+1` | Switch to Details view |
| `Ctrl+2` | Switch to Icons grid view |
| `Ctrl+D` | Add new download with Evalink |
| `Ctrl+Shift+D` | Toggle Evalink downloads drawer |
| `F3` | Toggle dual-pane split view |
| `F4` | Launch EvaTerm in current folder |
| `F5` | Reload current directory |
| `F11` / `Space` | Toggle live preview drawer |
| `Alt+Left` / `Alt+Right` | Navigate history back / forward |
| `Alt+Up` | Navigate to parent directory |
| `Ctrl+Shift+N` | Create new folder (with rename prompt) |
| `Ctrl+Alt+N` / `Ctrl+N` | Create new file (with rename prompt) |
| `Ctrl+C` / `Ctrl+X` / `Ctrl+V` | Copy / Cut / Paste selected files |
| `Ctrl+Shift+C` | Copy location / path to clipboard |
| `Delete` | Move selected items to Trash |
| `Shift+Delete` | Permanently delete selected items |
| `F2` | Rename selected item |

---

## Requirements

- **C++ Compiler**: GCC 11+ or Clang 13+ with C++20 support
- **Build System**: CMake 3.20+ and Make
- **Libraries**:
  - `qt6-base`
  - `qt6-declarative`
  - `qt6-svg`
  - `qt6-multimedia`
  - `poppler-qt6` (optional, for PDF preview rendering)
  - `pkg-config`

### Installing Dependencies on Arch Linux

```bash
sudo pacman -S base-devel cmake qt6-base qt6-declarative qt6-svg qt6-multimedia poppler-qt6 pkgconf
```

---

## Building and Installation

### Build from Source

```bash
git clone https://github.com/yourusername/evafile.git
cd evafile
make
```

### Install for Current User (Recommended)

Installs the binary to `~/.local/bin/evafile`, registers the desktop application entry, and installs application icons:

```bash
make install-user
```

Make sure `~/.local/bin` is in your `PATH`:

For Fish shell:
```fish
fish_add_path -m $HOME/.local/bin
```

For Bash / Zsh:
```bash
export PATH="$HOME/.local/bin:$PATH"
```

### Install System-Wide

```bash
make install
```

### Run Tests

```bash
make test
```

---

## Configuration

EvaFile persists user settings in `~/.config/evafile/evafile.conf`:

```ini
[%General]
theme=crimson_flame
default_view_mode=details
icon_size=64
restore_tabs=true
show_hidden=false
confirm_delete=true

[Layout]
show_sidebar=true
show_preview=false
show_split=false
sidebar_width=220
preview_width=300
window_size=@Size(1100 700)

[Session]
last_tabs=/home/camellia
```

---

## Eva Suite Ecosystem

- **EvaTerm**: GPU-accelerated terminal emulator with custom tabbed sessions and Crimson Flame theme.
- **EvaSort**: Intelligent rules-based directory organizer.
- **EvaFile**: Dolphin-inspired C++20 / Qt6 file manager.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
