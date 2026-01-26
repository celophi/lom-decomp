# Legend of Mana PSX Decompilation
Legend of Mana Decompilation Project

## Getting Started
1. Clone the repo and submodules:
    ```bash
    git clone https://github.com/celophi/lom-decomp.git
    cd lom-decomp
    git submodule update --init --recursive
    ```
   
2. Install Python dependencies (recommended: use a venv):
    ```bash
    python -m venv venv
    .\venv\Scripts\activate    # Windows
    pip install -r requirements-dev.txt
    ```
3. Copy your copy of SLUS_010.13 to the `disc` folder.

4. Split the binary using the following command. The output will go to the `asm` folder.
    ```bash
    splat split config/SLUS_010.13.yaml
    ```

5. Install WSL and dependencies
    ```bash
    wsl --install
    ```
6. On Ubuntu-22.04
    ```bash
    sudo apt update
    sudo apt install -y \
        build-essential \
        gcc-mipsel-linux-gnu \
        binutils-mipsel-linux-gnu \
        make python3
    ```
7. Build
    ```bash
    make clean
    make
    make bin
    ```
The output will be placed in the `build` directory.