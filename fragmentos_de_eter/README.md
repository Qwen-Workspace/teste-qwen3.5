# Fragmentos de Éter - MVP

Jogo 2D de ação e exploração com manipulação de fragmentos, desenvolvido em C usando raylib.

## Visão Geral

Fragmentos de Éter é um jogo onde o jogador controla um personagem capaz de manipular "fragmentos" do ambiente — reconstruindo, revertendo ou estabilizando partes do cenário — para navegar, combater inimigos e desbloquear novas áreas.

## Controles

| Tecla | Ação |
|-------|------|
| **A / D** ou **Setas** | Mover esquerda/direita |
| **Espaço** ou **W** | Pular (pulo duplo disponível no ar) |
| **Shift** | Dash (avanço rápido) |
| **J** ou **K** | Atacar |
| **E** | Restaurar fragmento (plataforma destruída) |
| **ESC** | Sair do jogo |
| **R** | Reiniciar (após vitória/derrota) |

## Mecânicas Principais

### Movimento
- Aceleração e desaceleração suaves
- Pulo duplo para ajuste de trajetória no ar
- Dash tático com tempo de recarga

### Sistema de Éter
- Barra de energia limitada (100 pontos)
- Regeneração lenta automática (5 pontos/segundo)
- Consumida ao usar habilidades especiais
- Pode ser restaurada coletando cristais roxos (+20 pontos)

### Manipulação de Fragmentos (Mecânica Central)
- Pressione **E** perto de plataformas destruídas (cinzas com partículas)
- Custo: 25 pontos de Éter
- Duração: 5 segundos
- Cria caminhos temporários essenciais para progressão

### Combate
- Ataque corpo a corpo de curto alcance
- Derrote inimigos marrons/laranjas/vermelhos
- Inimigos causam 20 de dano por contato
- Jogador tem 100 de vida

## Estrutura do Level

O jogo possui uma área interconectada com:
1. **Área inicial** - Tutorial de movimento
2. **Primeiro fragmento** - Plataforma quebrada que precisa ser restaurada
3. **Área com inimigo** - Combate introdutório
4. **Segundo fragmento** - Acesso a plataforma superior
5. **Terceiro fragmento** - Caminho final para o objetivo
6. **Objetivo** - Estrela dourada no final

## Como Compilar

### Pré-requisitos
- GCC
- Raylib (incluído no repositório)
- Bibliotecas X11, GL, pthread, dl, rt

### Comandos
```bash
cd fragmentos_de_eter
gcc -o fragmentos_de_eter main.c -I./raylib_src/src -L./raylib_src/src -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

### Executar
```bash
./fragmentos_de_eter
```

## Elementos do Jogo

### Jogador
- Vida: 100 HP
- Éter: 100 pontos
- Invencibilidade temporária após dano (1 segundo)

### Inimigos
- Vida: 100 HP
- Dano: 20 HP por contato
- Estados: Patrulha (marrom), Perseguindo (laranja), Atacando (vermelho)
- Morrem com 2 hits do jogador

### Plataformas
- **Verdes**: Normais, sempre ativas
- **Cinzas com partículas**: Destruídas, precisam ser restauradas
- **Azuis brilhantes**: Restauradas temporariamente (5 segundos)

### Pickups de Éter
- Cristais roxos flutuantes
- Restauram 20 pontos de Éter
- 4 pickups espalhados pelo level

## Condições de Vitória/Derrota

### Vitória
- Alcançar a estrela dourada no final do level

### Derrota
- Vida chegar a zero
- Cair no abismo (y > 1000)

## Dicas

1. Gerencie seu Éter sabiamente - você precisa dele para progredir
2. Colete os cristais de Éter pelo caminho
3. Use o dash para evitar ataques e atravessar gaps
4. O pulo duplo ajuda a alcançar plataformas altas
5. Derrote inimigos quando possível, mas foque na progressão
6. Preste atenção no timer das plataformas restauradas (⏳)

## Desenvolvimento

Este é um MVP (Minimum Viable Product) que valida o conceito central do jogo:
- Movimento preciso e responsivo
- Mecânica de fragmentos como elemento central
- Loop de gameplay completo (explorar → obstáculo → usar habilidade → progredir)

---

**Engine:** Raylib  
**Linguagem:** C  
**Resolução:** 1024x768
