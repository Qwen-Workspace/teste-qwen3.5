/*
 * Fragmentos de Éter - MVP
 * Jogo 2D de ação e exploração com manipulação de fragmentos
 * 
 * Controles:
 * - A/D ou Setas: Mover esquerda/direita
 * - Espaço ou W: Pular (pulo duplo disponível)
 * - Shift: Dash
 * - J ou K: Atacar
 * - E: Restaurar fragmento (plataforma destruída)
 * - ESC: Sair
 */

#include "raylib.h"
#include <math.h>
#include <stdbool.h>

// ============================================================================
// CONSTANTES DO JOGO
// ============================================================================

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define GRAVITY 1500.0f
#define PLAYER_SPEED 300.0f
#define PLAYER_ACCEL 2000.0f
#define PLAYER_DEACCEL 1500.0f
#define JUMP_FORCE -550.0f
#define DOUBLE_JUMP_FORCE -450.0f
#define DASH_SPEED 800.0f
#define DASH_DURATION 0.15f
#define DASH_COOLDOWN 1.0f
#define ATTACK_DURATION 0.2f
#define ATTACK_COOLDOWN 0.4f
#define ATTACK_RANGE 60.0f
#define ATTACK_DAMAGE 100
#define PLAYER_MAX_HEALTH 100
#define ETHER_MAX 100.0f
#define ETHER_REGEN_RATE 5.0f
#define ETHER_RESTORE_AMOUNT 20.0f
#define FRAGMENT_RESTORE_COST 25.0f
#define FRAGMENT_DURATION 5.0f
#define ENEMY_SPEED 80.0f
#define ENEMY_PATROL_DISTANCE 150.0f
#define ENEMY_DAMAGE 20
#define ENEMY_HEALTH 100
#define INVINCIBILITY_TIME 1.0f
#define CAMERA_SMOOTHING 5.0f

// ============================================================================
// ESTRUTURAS DE DADOS
// ============================================================================

typedef enum {
    STATE_INTACT,
    STATE_DESTROYED,
    STATE_RESTORED
} FragmentState;

typedef struct {
    Rectangle bounds;
    FragmentState state;
    float restoreTimer;
    bool isFragment; // true se é um fragmento restaurável
} Platform;

typedef enum {
    ENEMY_PATROLLING,
    ENEMY_CHASING,
    ENEMY_ATTACKING,
    ENEMY_DEAD
} EnemyState;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Rectangle bounds;
    EnemyState state;
    int health;
    float patrolStartX;
    float patrolEndX;
    float attackCooldown;
    float detectionRange;
    bool facingRight;
} Enemy;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Rectangle bounds;
    int health;
    int maxHealth;
    bool onGround;
    int jumpsLeft;
    bool canDoubleJump;
    bool isDashing;
    float dashTimer;
    float dashCooldown;
    bool isAttacking;
    float attackTimer;
    float attackCooldown;
    float invincibilityTimer;
    float ether;
    bool facingRight;
} Player;

typedef struct {
    Vector2 position;
    float amount;
    bool collected;
} EtherPickup;

typedef struct {
    Rectangle bounds;
    bool reached;
} Goal;

typedef struct {
    Player player;
    Platform platforms[50];
    int platformCount;
    Enemy enemies[10];
    int enemyCount;
    EtherPickup pickups[20];
    int pickupCount;
    Goal goal;
    float cameraX;
    float cameraY;
    bool gameWon;
    bool gameOver;
} GameState;

// ============================================================================
// VARIÁVEIS GLOBAIS
// ============================================================================

static GameState game;

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

void InitGame(void) {
    game.gameWon = false;
    game.gameOver = false;
    game.cameraX = 0;
    game.cameraY = 0;
    
    // Inicializar jogador
    game.player.position = (Vector2){ 100, 500 };
    game.player.velocity = (Vector2){ 0, 0 };
    game.player.bounds = (Rectangle){ 0, 0, 40, 60 };
    game.player.health = PLAYER_MAX_HEALTH;
    game.player.maxHealth = PLAYER_MAX_HEALTH;
    game.player.onGround = false;
    game.player.jumpsLeft = 2;
    game.player.canDoubleJump = true;
    game.player.isDashing = false;
    game.player.dashTimer = 0;
    game.player.dashCooldown = 0;
    game.player.isAttacking = false;
    game.player.attackTimer = 0;
    game.player.attackCooldown = 0;
    game.player.invincibilityTimer = 0;
    game.player.ether = ETHER_MAX;
    game.player.facingRight = true;
    
    // Criar plataformas
    game.platformCount = 0;
    
    // Plataforma inicial
    game.platforms[game.platformCount++] = (Platform){
        .bounds = { 0, 600, 300, 40 },
        .state = STATE_INTACT,
        .restoreTimer = 0,
        .isFragment = false
    };
    
    // Plataforma flutuante (fragmento)
    game.platforms[game.platformCount++] = (Platform){
        .bounds = { 350, 500, 150, 30 },
        .state = STATE_DESTROYED,
        .restoreTimer = 0,
        .isFragment = true
    };
    
    // Plataforma após primeiro fragmento
    game.platforms[game.platformCount++] = (Platform){
        .bounds = { 550, 500, 200, 40 },
        .state = STATE_INTACT,
        .restoreTimer = 0,
        .isFragment = false
    };
    
    // Segundo fragmento (mais alto)
    game.platforms[game.platformCount++] = (Platform){
        .bounds = { 800, 400, 120, 30 },
        .state = STATE_DESTROYED,
        .restoreTimer = 0,
        .isFragment = true
    };
    
    // Plataforma superior
    game.platforms[game.platformCount++] = (Platform){
        .bounds = { 950, 400, 200, 40 },
        .state = STATE_INTACT,
        .restoreTimer = 0,
        .isFragment = false
    };
    
    // Terceiro fragmento (para chegar ao objetivo)
    game.platforms[game.platformCount++] = (Platform){
        .bounds = { 1200, 300, 150, 30 },
        .state = STATE_DESTROYED,
        .restoreTimer = 0,
        .isFragment = true
    };
    
    // Plataforma final
    game.platforms[game.platformCount++] = (Platform){
        .bounds = { 1400, 300, 300, 40 },
        .state = STATE_INTACT,
        .restoreTimer = 0,
        .isFragment = false
    };
    
    // Chão inicial para tutorial
    game.platforms[game.platformCount++] = (Platform){
        .bounds = { -200, 650, 400, 100 },
        .state = STATE_INTACT,
        .restoreTimer = 0,
        .isFragment = false
    };
    
    // Inimigos
    game.enemyCount = 0;
    
    // Inimigo patrulha na primeira área
    game.enemies[game.enemyCount++] = (Enemy){
        .position = { 600, 460 },
        .velocity = { 0, 0 },
        .bounds = { 0, 0, 40, 50 },
        .state = ENEMY_PATROLLING,
        .health = ENEMY_HEALTH,
        .patrolStartX = 550,
        .patrolEndX = 750,
        .attackCooldown = 0,
        .detectionRange = 200,
        .facingRight = false
    };
    
    // Inimigo na área superior
    game.enemies[game.enemyCount++] = (Enemy){
        .position = { 1000, 360 },
        .velocity = { 0, 0 },
        .bounds = { 0, 0, 40, 50 },
        .state = ENEMY_PATROLLING,
        .health = ENEMY_HEALTH,
        .patrolStartX = 950,
        .patrolEndX = 1150,
        .attackCooldown = 0,
        .detectionRange = 200,
        .facingRight = true
    };
    
    // Pickups de Éter
    game.pickupCount = 0;
    
    game.pickups[game.pickupCount++] = (EtherPickup){
        .position = { 200, 550 },
        .amount = ETHER_RESTORE_AMOUNT,
        .collected = false
    };
    
    game.pickups[game.pickupCount++] = (EtherPickup){
        .position = { 650, 450 },
        .amount = ETHER_RESTORE_AMOUNT,
        .collected = false
    };
    
    game.pickups[game.pickupCount++] = (EtherPickup){
        .position = { 1050, 350 },
        .amount = ETHER_RESTORE_AMOUNT,
        .collected = false
    };
    
    game.pickups[game.pickupCount++] = (EtherPickup){
        .position = { 1550, 250 },
        .amount = ETHER_RESTORE_AMOUNT,
        .collected = false
    };
    
    // Objetivo final
    game.goal = (Goal){
        .bounds = { 1600, 240, 60, 60 },
        .reached = false
    };
}

bool CheckRectCollision(Rectangle a, Rectangle b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

float GameClamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// ============================================================================
// LÓGICA DO JOGADOR
// ============================================================================

void UpdatePlayer(float dt) {
    Player* p = &game.player;
    
    // Atualizar timers
    if (p->dashTimer > 0) p->dashTimer -= dt;
    if (p->dashCooldown > 0) p->dashCooldown -= dt;
    if (p->attackTimer > 0) p->attackTimer -= dt;
    if (p->attackCooldown > 0) p->attackCooldown -= dt;
    if (p->invincibilityTimer > 0) p->invincibilityTimer -= dt;
    
    // Regenerar Éter lentamente
    if (p->ether < ETHER_MAX) {
        p->ether += ETHER_REGEN_RATE * dt;
        if (p->ether > ETHER_MAX) p->ether = ETHER_MAX;
    }
    
    // Finalizar ataque
    if (p->isAttacking && p->attackTimer <= 0) {
        p->isAttacking = false;
    }
    
    // Input horizontal
    float inputX = 0;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) inputX -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) inputX += 1;
    
    // Atualizar direção do personagem
    if (inputX != 0) {
        p->facingRight = (inputX > 0);
    }
    
    // Movimento
    if (p->isDashing) {
        // Durante dash, mantém velocidade constante na direção
        p->velocity.x = (p->facingRight ? DASH_SPEED : -DASH_SPEED);
    } else {
        if (inputX != 0) {
            p->velocity.x += inputX * PLAYER_ACCEL * dt;
            p->velocity.x = GameClamp(p->velocity.x, -PLAYER_SPEED, PLAYER_SPEED);
        } else {
            // Desaceleração
            if (p->velocity.x > 0) {
                p->velocity.x -= PLAYER_DEACCEL * dt;
                if (p->velocity.x < 0) p->velocity.x = 0;
            } else if (p->velocity.x < 0) {
                p->velocity.x += PLAYER_DEACCEL * dt;
                if (p->velocity.x > 0) p->velocity.x = 0;
            }
        }
        
        // Aplicar gravidade
        p->velocity.y += GRAVITY * dt;
    }
    
    // Pulo
    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W)) && !p->isDashing) {
        if (p->onGround) {
            p->velocity.y = JUMP_FORCE;
            p->onGround = false;
            p->jumpsLeft = 1;
            p->canDoubleJump = true;
        } else if (p->canDoubleJump && p->jumpsLeft > 0) {
            p->velocity.y = DOUBLE_JUMP_FORCE;
            p->jumpsLeft--;
            p->canDoubleJump = false;
        }
    }
    
    // Dash
    if (IsKeyPressed(KEY_LEFT_SHIFT) && p->dashCooldown <= 0 && !p->isDashing) {
        p->isDashing = true;
        p->dashTimer = DASH_DURATION;
        p->dashCooldown = DASH_COOLDOWN;
        p->velocity.y = 0; // Cancela queda durante dash
    }
    
    // Ataque
    if ((IsKeyPressed(KEY_J) || IsKeyPressed(KEY_K)) && 
        p->attackCooldown <= 0 && !p->isAttacking) {
        p->isAttacking = true;
        p->attackTimer = ATTACK_DURATION;
        p->attackCooldown = ATTACK_COOLDOWN;
        
        // Verificar hits em inimigos
        Rectangle attackBox;
        if (p->facingRight) {
            attackBox = (Rectangle){ 
                p->position.x + p->bounds.width, 
                p->position.y + 10, 
                ATTACK_RANGE, 
                p->bounds.height - 20 
            };
        } else {
            attackBox = (Rectangle){ 
                p->position.x - ATTACK_RANGE, 
                p->position.y + 10, 
                ATTACK_RANGE, 
                p->bounds.height - 20 
            };
        }
        
        for (int i = 0; i < game.enemyCount; i++) {
            Enemy* e = &game.enemies[i];
            if (e->state != ENEMY_DEAD) {
                Rectangle enemyRect = { 
                    e->position.x, e->position.y, 
                    e->bounds.width, e->bounds.height 
                };
                if (CheckRectCollision(attackBox, enemyRect)) {
                    e->health -= ATTACK_DAMAGE;
                    // Knockback
                    e->velocity.x = p->facingRight ? 200 : -200;
                    e->velocity.y = -100;
                    if (e->health <= 0) {
                        e->state = ENEMY_DEAD;
                    }
                }
            }
        }
    }
    
    // Restaurar fragmento
    if (IsKeyPressed(KEY_E) && p->ether >= FRAGMENT_RESTORE_COST) {
        // Encontrar fragmento destruído próximo
        Rectangle playerRect = { 
            p->position.x, p->position.y, 
            p->bounds.width, p->bounds.height 
        };
        
        for (int i = 0; i < game.platformCount; i++) {
            Platform* plat = &game.platforms[i];
            if (plat->isFragment && plat->state == STATE_DESTROYED) {
                // Verificar proximidade
                float dist = sqrtf(powf((plat->bounds.x + plat->bounds.width/2) - 
                               (p->position.x + p->bounds.width/2), 2) +
                          powf((plat->bounds.y + plat->bounds.height/2) - 
                               (p->position.y + p->bounds.height/2), 2));
                
                if (dist < 150) {
                    plat->state = STATE_RESTORED;
                    plat->restoreTimer = FRAGMENT_DURATION;
                    p->ether -= FRAGMENT_RESTORE_COST;
                    break;
                }
            }
        }
    }
    
    // Atualizar posição
    p->position.x += p->velocity.x * dt;
    p->position.y += p->velocity.y * dt;
    
    // Colisão com plataformas
    p->onGround = false;
    Rectangle playerRect = { 
        p->position.x, p->position.y, 
        p->bounds.width, p->bounds.height 
    };
    
    for (int i = 0; i < game.platformCount; i++) {
        Platform* plat = &game.platforms[i];
        
        // Ignorar fragmentos destruídos
        if (plat->isFragment && plat->state == STATE_DESTROYED) continue;
        
        if (CheckRectCollision(playerRect, plat->bounds)) {
            // Colisão vertical (aterrissando)
            if (p->velocity.y > 0 && 
                p->position.y + p->bounds.height - p->velocity.y * dt <= plat->bounds.y) {
                p->position.y = plat->bounds.y - p->bounds.height;
                p->velocity.y = 0;
                p->onGround = true;
                p->jumpsLeft = 2;
                p->canDoubleJump = true;
            }
            // Colisão vindo de baixo (batendo cabeça)
            else if (p->velocity.y < 0 && 
                     p->position.y - p->velocity.y * dt >= plat->bounds.y + plat->bounds.height) {
                p->position.y = plat->bounds.y + plat->bounds.height;
                p->velocity.y = 0;
            }
            // Colisão horizontal
            else if (p->velocity.x > 0) {
                p->position.x = plat->bounds.x - p->bounds.width;
                p->velocity.x = 0;
            } else if (p->velocity.x < 0) {
                p->position.x = plat->bounds.x + plat->bounds.width;
                p->velocity.x = 0;
            }
            
            playerRect = (Rectangle){ 
                p->position.x, p->position.y, 
                p->bounds.width, p->bounds.height 
            };
        }
    }
    
    // Limites do mundo
    if (p->position.y > 1000) {
        // Caiu no abismo
        p->health = 0;
    }
    
    // Coletar pickups de Éter
    for (int i = 0; i < game.pickupCount; i++) {
        if (!game.pickups[i].collected) {
            Rectangle pickupRect = { 
                game.pickups[i].position.x, 
                game.pickups[i].position.y, 
                30, 30 
            };
            if (CheckRectCollision(playerRect, pickupRect)) {
                game.pickups[i].collected = true;
                p->ether += game.pickups[i].amount;
                if (p->ether > ETHER_MAX) p->ether = ETHER_MAX;
            }
        }
    }
    
    // Checar objetivo
    if (CheckRectCollision(playerRect, game.goal.bounds)) {
        game.gameWon = true;
    }
    
    // Checar morte
    if (p->health <= 0) {
        game.gameOver = true;
    }
}

// ============================================================================
// LÓGICA DOS INIMIGOS
// ============================================================================

void UpdateEnemies(float dt) {
    Player* p = &game.player;
    
    for (int i = 0; i < game.enemyCount; i++) {
        Enemy* e = &game.enemies[i];
        
        if (e->state == ENEMY_DEAD) continue;
        
        // Atualizar cooldowns
        if (e->attackCooldown > 0) e->attackCooldown -= dt;
        
        // Física básica
        e->velocity.y += GRAVITY * dt;
        
        // Detectar jogador
        float distToPlayer = sqrtf(powf(e->position.x - p->position.x, 2) + 
                                    powf(e->position.y - p->position.y, 2));
        
        if (distToPlayer < e->detectionRange && p->health > 0) {
            e->state = ENEMY_CHASING;
        } else if (e->state == ENEMY_CHASING && distToPlayer > e->detectionRange * 1.5f) {
            e->state = ENEMY_PATROLLING;
        }
        
        // Comportamento
        if (e->state == ENEMY_PATROLLING) {
            // Patrulhar entre pontos
            if (e->position.x <= e->patrolStartX) {
                e->velocity.x = ENEMY_SPEED;
                e->facingRight = true;
            } else if (e->position.x >= e->patrolEndX) {
                e->velocity.x = -ENEMY_SPEED;
                e->facingRight = false;
            } else {
                // Continuar na direção atual
                if (e->facingRight) {
                    e->velocity.x = ENEMY_SPEED;
                } else {
                    e->velocity.x = -ENEMY_SPEED;
                }
            }
        } else if (e->state == ENEMY_CHASING) {
            // Perseguir jogador
            if (p->position.x > e->position.x) {
                e->velocity.x = ENEMY_SPEED * 1.2f;
                e->facingRight = true;
            } else {
                e->velocity.x = -ENEMY_SPEED * 1.2f;
                e->facingRight = false;
            }
            
            // Atacar se perto
            if (distToPlayer < 50 && e->attackCooldown <= 0) {
                e->state = ENEMY_ATTACKING;
                e->attackCooldown = 1.0f;
            }
        } else if (e->state == ENEMY_ATTACKING) {
            e->velocity.x = 0;
            if (e->attackCooldown <= 0.5f) {
                e->state = ENEMY_CHASING;
            }
        }
        
        // Aplicar movimento
        e->position.x += e->velocity.x * dt;
        e->position.y += e->velocity.y * dt;
        
        // Colisão com plataformas
        Rectangle enemyRect = { 
            e->position.x, e->position.y, 
            e->bounds.width, e->bounds.height 
        };
        
        for (int j = 0; j < game.platformCount; j++) {
            Platform* plat = &game.platforms[j];
            if (plat->isFragment && plat->state == STATE_DESTROYED) continue;
            
            if (CheckRectCollision(enemyRect, plat->bounds)) {
                if (e->velocity.y > 0 && 
                    e->position.y + e->bounds.height - e->velocity.y * dt <= plat->bounds.y) {
                    e->position.y = plat->bounds.y - e->bounds.height;
                    e->velocity.y = 0;
                } else if (e->velocity.x > 0) {
                    e->position.x = plat->bounds.x - e->bounds.width;
                    e->velocity.x = -e->velocity.x;
                    e->facingRight = !e->facingRight;
                } else if (e->velocity.x < 0) {
                    e->position.x = plat->bounds.x + plat->bounds.width;
                    e->velocity.x = -e->velocity.x;
                    e->facingRight = !e->facingRight;
                }
                
                enemyRect = (Rectangle){ 
                    e->position.x, e->position.y, 
                    e->bounds.width, e->bounds.height 
                };
            }
        }
        
        // Dano ao jogador por contato
        if (p->invincibilityTimer <= 0 && p->health > 0) {
            Rectangle playerRect = { 
                p->position.x, p->position.y, 
                p->bounds.width, p->bounds.height 
            };
            if (CheckRectCollision(playerRect, enemyRect)) {
                p->health -= ENEMY_DAMAGE;
                p->invincibilityTimer = INVINCIBILITY_TIME;
                // Knockback no jogador
                p->velocity.y = -300;
                p->velocity.x = e->facingRight ? -400 : 400;
            }
        }
    }
}

// ============================================================================
// LÓGICA DOS FRAGMENTOS
// ============================================================================

void UpdateFragments(float dt) {
    for (int i = 0; i < game.platformCount; i++) {
        Platform* plat = &game.platforms[i];
        if (plat->isFragment && plat->state == STATE_RESTORED) {
            plat->restoreTimer -= dt;
            if (plat->restoreTimer <= 0) {
                plat->state = STATE_DESTROYED;
            }
        }
    }
}

// ============================================================================
// CÂMERA
// ============================================================================

void UpdateCamera2D(float dt) {
    Player* p = &game.player;
    
    // Alvo da câmera segue o jogador com suavização
    float targetX = p->position.x + p->bounds.width / 2 - SCREEN_WIDTH / 2;
    float targetY = p->position.y + p->bounds.height / 2 - SCREEN_HEIGHT / 2;
    
    game.cameraX += (targetX - game.cameraX) * CAMERA_SMOOTHING * dt;
    game.cameraY += (targetY - game.cameraY) * CAMERA_SMOOTHING * dt;
    
    // Limites da câmera
    game.cameraX = GameClamp(game.cameraX, -100, 1700);
    game.cameraY = GameClamp(game.cameraY, -100, 200);
}

// ============================================================================
// RENDERIZAÇÃO
// ============================================================================

void DrawGame(void) {
    BeginDrawing();
    ClearBackground((Color){ 20, 20, 40 });
    
    // Aplicar câmera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ game.cameraX + SCREEN_WIDTH/2, game.cameraY + SCREEN_HEIGHT/2 };
    camera.offset = (Vector2){ SCREEN_WIDTH/2, SCREEN_HEIGHT/2 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    BeginMode2D(camera);
    
    // Desenhar fundo (parallax simples)
    for (int i = 0; i < 5; i++) {
        DrawRectangle(i * 400 - (int)game.cameraX * 0.3f, 
                      100 + i * 50, 300, 200, 
                      (Color){ 40 + i * 10, 40 + i * 10, 60 + i * 10, 100 });
    }
    
    // Desenhar plataformas
    for (int i = 0; i < game.platformCount; i++) {
        Platform* plat = &game.platforms[i];
        
        if (plat->isFragment && plat->state == STATE_DESTROYED) {
            // Fragmento destruído - mostrar apenas contornos/partículas
            DrawRectangleLinesEx(plat->bounds, 2, GRAY);
            
            // Partículas flutuantes
            for (int j = 0; j < 5; j++) {
                float px = plat->bounds.x + (j * 25) % (int)plat->bounds.width;
                float py = plat->bounds.y + plat->bounds.height/2 + 
                           sinf(GetTime() * 3 + j) * 5;
                DrawCircle(px, py, 3, DARKGRAY);
            }
        } else if (plat->isFragment && plat->state == STATE_RESTORED) {
            // Fragmento restaurado - cor especial com fade
            float alpha = plat->restoreTimer / FRAGMENT_DURATION;
            Color c = (Color){ 
                (unsigned char)(100 * alpha + 50), 
                (unsigned char)(200 * alpha), 
                (unsigned char)(255 * alpha), 
                200 
            };
            DrawRectangleRec(plat->bounds, c);
            DrawRectangleLinesEx(plat->bounds, 2, SKYBLUE);
            
            // Timer visual
            DrawText("⏳", plat->bounds.x + plat->bounds.width/2 - 10, 
                     plat->bounds.y - 25, 20, WHITE);
        } else {
            // Plataforma normal
            DrawRectangleRec(plat->bounds, DARKGREEN);
            DrawRectangleLinesEx(plat->bounds, 2, GREEN);
            
            // Detalhe de grama no topo
            DrawRectangle(plat->bounds.x, plat->bounds.y, 
                         plat->bounds.width, 5, LIME);
        }
    }
    
    // Desenhar pickups de Éter
    for (int i = 0; i < game.pickupCount; i++) {
        if (!game.pickups[i].collected) {
            Vector2 pos = game.pickups[i].position;
            float bob = sinf(GetTime() * 4 + i) * 5;
            
            // Cristal de Éter
            DrawTriangle(
                (Vector2){ pos.x + 15, pos.y + bob },
                (Vector2){ pos.x, pos.y + 30 + bob },
                (Vector2){ pos.x + 30, pos.y + 30 + bob },
                PURPLE
            );
            DrawCircle(pos.x + 15, pos.y + 15 + bob, 5, VIOLET);
        }
    }
    
    // Desenhar inimigos
    for (int i = 0; i < game.enemyCount; i++) {
        Enemy* e = &game.enemies[i];
        
        if (e->state == ENEMY_DEAD) {
            // Inimigo morto
            DrawRectangle(e->position.x, e->position.y + e->bounds.height - 10,
                         e->bounds.width, 10, DARKGRAY);
            continue;
        }
        
        Color enemyColor;
        if (e->state == ENEMY_ATTACKING) {
            enemyColor = RED;
        } else if (e->state == ENEMY_CHASING) {
            enemyColor = ORANGE;
        } else {
            enemyColor = BROWN;
        }
        
        // Corpo do inimigo
        DrawRectangleRec((Rectangle){ 
            e->position.x, e->position.y, 
            e->bounds.width, e->bounds.height 
        }, enemyColor);
        
        // Olhos
        int eyeOffset = e->facingRight ? 5 : -5;
        DrawCircle(e->position.x + e->bounds.width/2 + eyeOffset, 
                  e->position.y + 15, 5, YELLOW);
        
        // Barra de vida
        float healthPct = (float)e->health / ENEMY_HEALTH;
        DrawRectangle(e->position.x, e->position.y - 10, e->bounds.width, 5, RED);
        DrawRectangle(e->position.x, e->position.y - 10, e->bounds.width * healthPct, 5, GREEN);
    }
    
    // Desenhar jogador
    Player* p = &game.player;
    
    // Flash quando invencível
    if (p->invincibilityTimer > 0 && (int)(p->invincibilityTimer * 20) % 2 == 0) {
        // Piscando
    } else {
        // Corpo
        Color playerColor = p->isDashing ? SKYBLUE : BLUE;
        DrawRectangleRec((Rectangle){ 
            p->position.x, p->position.y, 
            p->bounds.width, p->bounds.height 
        }, playerColor);
        
        // Cabeça
        DrawCircle(p->position.x + p->bounds.width/2, 
                  p->position.y + 15, 12, BEIGE);
        
        // Olho (direção)
        int eyeX = p->facingRight ? 
                   p->position.x + p->bounds.width/2 + 4 : 
                   p->position.x + p->bounds.width/2 - 4;
        DrawCircle(eyeX, p->position.y + 15, 4, BLACK);
        
        // Efeito de dash
        if (p->isDashing) {
            for (int i = 1; i <= 3; i++) {
                DrawRectangleRec((Rectangle){ 
                    p->position.x - (p->facingRight ? i * 15 : -i * 15), 
                    p->position.y, 
                    p->bounds.width, p->bounds.height 
                }, (Color){ 100, 200, 255, 100 - i * 30 });
            }
        }
        
        // Efeito de ataque
        if (p->isAttacking) {
            Rectangle attackRect;
            if (p->facingRight) {
                attackRect = (Rectangle){ 
                    p->position.x + p->bounds.width, 
                    p->position.y + 10, 
                    ATTACK_RANGE, 
                    p->bounds.height - 20 
                };
            } else {
                attackRect = (Rectangle){ 
                    p->position.x - ATTACK_RANGE, 
                    p->position.y + 10, 
                    ATTACK_RANGE, 
                    p->bounds.height - 20 
                };
            }
            DrawRectangleRec(attackRect, (Color){ 255, 200, 100, 150 });
            DrawRectangleLinesEx(attackRect, 2, ORANGE);
        }
    }
    
    // Desenhar objetivo
    DrawRectangleRec(game.goal.bounds, GOLD);
    DrawRectangleLinesEx(game.goal.bounds, 3, YELLOW);
    DrawText("★", game.goal.bounds.x + 15, game.goal.bounds.y + 10, 40, WHITE);
    
    EndMode2D();
    
    // UI (não afetada pela câmera)
    
    // Barra de Vida
    DrawRectangle(20, 20, 200, 25, DARKGRAY);
    float healthPct = (float)p->health / PLAYER_MAX_HEALTH;
    DrawRectangle(20, 20, 200 * healthPct, 25, 
                  healthPct > 0.5 ? GREEN : (healthPct > 0.25 ? YELLOW : RED));
    DrawRectangleLines(20, 20, 200, 25, WHITE);
    DrawText("VIDA", 30, 25, 15, WHITE);
    
    // Barra de Éter
    DrawRectangle(20, 55, 200, 20, DARKGRAY);
    float etherPct = p->ether / ETHER_MAX;
    DrawRectangle(20, 55, 200 * etherPct, 20, PURPLE);
    DrawRectangleLines(20, 55, 200, 20, WHITE);
    DrawText("ÉTER", 30, 60, 15, WHITE);
    
    // Controles
    DrawText("CONTROLES:", 20, SCREEN_HEIGHT - 120, 16, LIGHTGRAY);
    DrawText("A/D ou Setas: Mover", 20, SCREEN_HEIGHT - 100, 14, GRAY);
    DrawText("Espaço/W: Pular (duplo no ar)", 20, SCREEN_HEIGHT - 85, 14, GRAY);
    DrawText("Shift: Dash", 20, SCREEN_HEIGHT - 70, 14, GRAY);
    DrawText("J/K: Atacar", 20, SCREEN_HEIGHT - 55, 14, GRAY);
    DrawText("E: Restaurar Fragmento (custa 25 Éter)", 20, SCREEN_HEIGHT - 40, 14, GRAY);
    
    // Dicas contextuais
    if (p->ether >= FRAGMENT_RESTORE_COST) {
        // Verificar se há fragmento próximo
        bool fragmentNearby = false;
        for (int i = 0; i < game.platformCount; i++) {
            Platform* plat = &game.platforms[i];
            if (plat->isFragment && plat->state == STATE_DESTROYED) {
                float dist = sqrtf(powf((plat->bounds.x + plat->bounds.width/2) - 
                               (p->position.x + p->bounds.width/2), 2) +
                          powf((plat->bounds.y + plat->bounds.height/2) - 
                               (p->position.y + p->bounds.height/2), 2));
                if (dist < 150) {
                    fragmentNearby = true;
                    break;
                }
            }
        }
        if (fragmentNearby) {
            DrawText(">>> PRESSIONE 'E' PARA RESTAURAR <<<", 
                    SCREEN_WIDTH/2 - 150, 100, 20, SKYBLUE);
        }
    }
    
    // Mensagens de fim de jogo
    if (game.gameWon) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 200 });
        DrawText("VOCÊ VENCEU!", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 50, 40, GOLD);
        DrawText("Pressione R para reiniciar", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 20, 25, WHITE);
    }
    
    if (game.gameOver) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 200 });
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 50, 40, RED);
        DrawText("Pressione R para reiniciar", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 20, 25, WHITE);
    }
    
    EndDrawing();
}

// ============================================================================
// MAIN
// ============================================================================

int main(void) {
    // Configurar janela
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fragmentos de Éter - MVP");
    SetTargetFPS(60);
    
    // Inicializar jogo
    InitGame();
    
    // Loop principal
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        // Reiniciar jogo
        if ((game.gameWon || game.gameOver) && IsKeyPressed(KEY_R)) {
            InitGame();
        }
        
        // Sair com ESC
        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }
        
        // Atualizar lógica (apenas se jogo não terminou)
        if (!game.gameWon && !game.gameOver) {
            UpdatePlayer(dt);
            UpdateEnemies(dt);
            UpdateFragments(dt);
            UpdateCamera2D(dt);
        }
        
        // Renderizar
        DrawGame();
    }
    
    CloseWindow();
    return 0;
}
