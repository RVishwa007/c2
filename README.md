# Intelligent Traffic Management System (Nexus)

An advanced AI-based real-time traffic intersection simulation system running entirely inside the terminal, built in pure C.

## Windows Setup & Installation

Since you are running Windows and VS Code, you need a C compiler (`gcc`) and the `ncurses` library. The easiest way to get both on Windows is using **MSYS2**.

### 1. Install MSYS2
1. Download and install MSYS2 from [msys2.org](https://www.msys2.org/).
2. Once installed, open the **MSYS2 UCRT64** terminal (you can find it in your Start menu).

### 2. Install GCC and Ncurses
Run the following command inside the MSYS2 terminal to install the GCC compiler and the ncurses library:
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-ncurses make
```
Accept the prompts (press `Y` when asked).

### 3. Add to VS Code / Windows Path
To use this inside VS Code's integrated terminal, add the MSYS2 UCRT64 `bin` folder to your Windows system PATH:
- Typically, this folder is at: `C:\msys64\ucrt64\bin`

### 4. Compilation
Open the project directory in VS Code, open a new terminal, and simply run:
```bash
make
```

### 5. Running the Simulation
```bash
./nexus.exe
```
## Features
- **Real-Time Dashboard:** A colorful ncurses-based UI showing a live ASCII intersection.
- **Dynamic AI Decision Making:** Advanced multi-factor weighted priority algorithm considering vehicle count, waiting cycles, growth rate, and predicted congestion.
- **Starvation Prevention:** Forces a green light if a lane is ignored for too long.
- **Emergency System:** Ambulances and Police cars force instant overrides and lane clearing.
- **Congestion Prediction:** Tracks historical trends to predict impending gridlocks.
- **Historical Analytics:** View efficiency scores, average wait times, and a full file log (`traffic_log.txt`).

## Interactive Controls
- `S` - Start / Resume simulation
- `P` - Pause simulation
- `E` - Trigger emergency in a random lane
- `R` - Toggle Rush Hour mode (heavy incoming traffic)
- `N` - Toggle Night mode (sparse traffic)
- `A` - Toggle Accident mode (restricts flow)
- `M` - Manual Input mode (inject 5 cars to the most starved lane)
- `T` - View real-time analytics panel
- `H` - Generate & View Historical Report
- `F` - Change simulation speed (Fast/Slow toggle)
- `Q` - Quit the program
