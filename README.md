# Legend of Mana PSX Decompilation

A decompilation project for the PlayStation 1 game Legend of Mana, targeting MIPS assembly.

## Prerequisites

- **Git** - For cloning the repository
- **Docker Desktop** - Required for the build environment ([Download here](https://www.docker.com/products/docker-desktop))
- **Legend of Mana ROM** - Your legally obtained copy of `SLUS_010.13`

## Setup Instructions

### 1. Clone the Repository

Open a terminal (PowerShell or Command Prompt on Windows) and run:

```bash
git clone https://github.com/celophi/lom-decomp.git
cd lom-decomp
git submodule update --init --recursive
```

This downloads the project and all its dependencies.

### 2. Add Your Game ROM

Copy your `SLUS_010.13` file into the `disc` folder in the project directory.

### 3. Build the Compiler Container

The project uses an old PlayStation compiler (gcc-2.7.2) that runs in Docker. Build it with:

```bash
docker build -t old-gcc/gcc-2.7.2-psx -f tools/old-gcc/gcc-2.7.2-psx.Dockerfile tools/old-gcc
```

This step may take several minutes.

### 4. Build the Development Environment

Build the main development container:

```bash
docker build -t lom-dev .
```

### 5. Start the Development Environment

Launch the container with your project files mounted:

```bash
docker run --rm -ti -v "${PWD}:/lom" lom-dev
```

You should now be inside the container at a Linux terminal prompt.

### 6. Split the Game Binary

Inside the container, run the splat tool to extract the game binary into assembly files:

```bash
splat split config/SLUS_010.13.yaml
```

### 7. Build the Project

Compile the decompiled code:

```bash
make clean
make
make bin
```

The compiled output will be in the `build` directory as both an ELF executable and a BIN file.

## Workflow Summary

After initial setup, your typical workflow is:

1. Start the container: `docker run --rm -ti -v "${PWD}:/lom" lom-dev`
2. Make your code changes (outside the container, in your editor)
3. Build inside the container: `make clean && make && make bin`
4. Test the output from the `build` directory

## Troubleshooting

- If Docker commands fail, ensure Docker Desktop is running
- On Windows, use PowerShell or Command Prompt, not Git Bash, for the `docker run` command
- The `${PWD}` variable should automatically expand to your current directory path