# Mini GTA 3D Prototype

Protótipo **bem simples** em C++ + raylib de um jogo estilo GTA em 3D.

## O que tem

- Cidade pequena feita de prédios (cubos)
- Personagem controlável (andar + olhar)
- Carro que você pode entrar e dirigir
- Alguns NPCs andando pela cidade
- Câmera em terceira pessoa
- Colisão básica com prédios

**Não é um GTA San Andreas de verdade.** É só um protótipo divertido pra mostrar a ideia.

## Controles

| Tecla | Ação |
|-------|------|
| **W A S D** | Andar / Dirigir |
| **Mouse** | Olhar ao redor |
| **E** | Entrar / Sair do carro |
| **Espaço** | Pular (só a pé) |
| **ESC** | Sair |

## Como compilar

### Requisitos
- C++17 ou superior
- [raylib](https://www.raylib.com/) instalado

### Linux (Ubuntu/Debian)
```bash
sudo apt install libraylib-dev
mkdir build && cd build
cmake ..
make
./mini-gta-3d
```

### Windows
1. Instale raylib (via vcpkg ou baixe o binário)
2. Use CMake ou o Visual Studio

### macOS
```bash
brew install raylib
mkdir build && cd build
cmake ..
make
./mini-gta-3d
```

## Estrutura

- `main.cpp` → todo o jogo
- `CMakeLists.txt` → build system

---
Feito por diversão com a conta GitHub conectada 😄
