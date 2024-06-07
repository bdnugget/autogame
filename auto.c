#include "raylib.h"
#include <stdlib.h> // For NULL
#include <time.h>   // For time function

// Comment/uncomment for WASM
// #define PLATFORM_WEB

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define SQUARE_SIZE 200
#define FPS_TARGET 60
#define GARAGE_COUNT 5

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Car {
  Vector2 position;
  Vector2 size;
  Color color;
  int direction;
} Car;

typedef struct Garage {
  Vector2 position;
  Vector2 size;
  Color color;
} Garage;

typedef struct {
  int score;
  Car car;
  int framesCounter;
  bool gameOver;
  bool pause;
  bool allowMove;
  Garage *garages;
} GameState;

//----------------------------------------------------------------------------------
// Global Variables Declaration
//----------------------------------------------------------------------------------
static const int screenWidth = 1600;
static const int screenHeight = 1000;

static Texture autoTexture;
static Texture garageTexture;

static Color garageColors[GARAGE_COUNT] = {RED, GREEN, BLUE, YELLOW, PURPLE};

static Sound autoHappy;
static Sound autoSad;

static GameState gameState;

static Vector2 offset = {screenWidth % SQUARE_SIZE, screenHeight % SQUARE_SIZE};

//----------------------------------------------------------------------------------
// Module Functions Declaration (local)
//----------------------------------------------------------------------------------
static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);
static void InitGarages(void);

//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
int main(void) {
  InitWindow(screenWidth, screenHeight, "Dikke Vette Cargame voor Milo");
  InitAudioDevice();
  autoHappy = LoadSound("resources/auto_happy_vob.ogg");
  autoSad = LoadSound("resources/auto_sad_vob.ogg");
  autoTexture = LoadTexture("resources/car_200px.png");

  InitGame();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, FPS_TARGET, 1);
#else
  SetTargetFPS(FPS_TARGET);
  while (!WindowShouldClose()) {
    UpdateDrawFrame();
  }
#endif

  UnloadGame();

  CloseWindow();

  return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definitions (local)
//----------------------------------------------------------------------------------

// Initialize game variables
void InitGame(void) {
  SetRandomSeed(time(NULL));
  int randomIndex = GetRandomValue(0, 4);

  gameState.score = 0;
  gameState.framesCounter = 0;
  gameState.gameOver = false;
  gameState.pause = false;
  gameState.allowMove = false;
  gameState.car =
      (Car){(Vector2){offset.x / 2, offset.y / 2 + 2 * SQUARE_SIZE},
            (Vector2){SQUARE_SIZE, SQUARE_SIZE}, garageColors[randomIndex], 0};
  gameState.garages = (Garage *)malloc(GARAGE_COUNT * sizeof(Garage));

  offset.x = screenWidth % SQUARE_SIZE;
  offset.y = screenHeight % SQUARE_SIZE;

  InitGarages();
}

// Update game (one frame)
void UpdateGame(void) {
  if (!gameState.gameOver) {
    if (IsKeyPressed('P'))
      gameState.pause = !gameState.pause;

    if (!gameState.pause) {
      if (IsKeyPressed(KEY_UP) && gameState.allowMove &&
          gameState.car.position.y > 0) {
        gameState.car.direction = -1;
        gameState.allowMove = false;
      }
      if (IsKeyPressed(KEY_DOWN) && gameState.allowMove &&
          gameState.car.position.y < (screenHeight - SQUARE_SIZE)) {
        gameState.car.direction = 1;
        gameState.allowMove = false;
      }

      if ((gameState.framesCounter % (FPS_TARGET / 60)) == 0) {
        gameState.car.position.x += (float)SQUARE_SIZE / FPS_TARGET;
        gameState.car.position.y += gameState.car.direction * SQUARE_SIZE;
        gameState.allowMove = true;
        gameState.car.direction = 0;
      }

      for (int i = 0; i < GARAGE_COUNT; i++) {
        if (CheckCollisionRecs(
                (Rectangle){gameState.car.position.x, gameState.car.position.y,
                            gameState.car.size.x, gameState.car.size.y},
                (Rectangle){gameState.garages[i].position.x,
                            gameState.garages[i].position.y,
                            gameState.garages[i].size.x,
                            gameState.garages[i].size.y})) {
          if (ColorToInt(gameState.car.color) ==
              ColorToInt(gameState.garages[i].color)) {
            PlaySound(autoHappy);
            gameState.score++;
            InitGame();
          } else {
            PlaySound(autoSad);
            gameState.gameOver = true;
          }
        }
      }

      gameState.framesCounter++;
    }
  } else {
    if (IsKeyPressed(KEY_ENTER)) {
      gameState.score = 0;
      InitGame();
    }
  }
}

// Draw game (one frame)
void DrawGame(void) {
  BeginDrawing();

  ClearBackground(DARKGRAY);

  if (!gameState.gameOver) {
    for (int i = 1; i < screenHeight / SQUARE_SIZE + 1; i++) {
      for (int j = 0; j < screenWidth / (SQUARE_SIZE / 2); j += 2) {
        DrawRectangle(j * ((float)SQUARE_SIZE / 2) + offset.x / 2,
                      SQUARE_SIZE * i + offset.y / 2, SQUARE_SIZE / 2,
                      SQUARE_SIZE / 8, RAYWHITE);
      }
    }

    // Draw garages
    for (int i = 0; i < GARAGE_COUNT; i++) {
      DrawRectangleRec((Rectangle){gameState.garages[i].position.x,
                                   gameState.garages[i].position.y,
                                   gameState.garages[i].size.x,
                                   gameState.garages[i].size.y},
                       gameState.garages[i].color);
    }

    DrawTextureV(autoTexture, gameState.car.position, gameState.car.color);

    // Draw score
    DrawText(TextFormat("Score: %d", gameState.score), 20, 20, 20, RAYWHITE);

    if (gameState.pause)
      DrawText("GAME PAUSED",
               screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2,
               screenHeight / 2 - 40, 40, GRAY);
  } else
    DrawText(
        "A WINRAR IS NOT YOU :(\nPRESS [ENTER] TO PLAY AGAIN",
        GetScreenWidth() / 2 -
            MeasureText("A WINRAR IS NOT YOU :(\nPRESS [ENTER] TO PLAY AGAIN",
                        20) /
                2,
        GetScreenHeight() / 2 - 50, 20, GRAY);

  EndDrawing();
}

void InitGarages(void) {
  for (int i = 0; i < GARAGE_COUNT; i++) {
    gameState.garages[i].size = (Vector2){SQUARE_SIZE, SQUARE_SIZE};
    gameState.garages[i].position =
        (Vector2){screenWidth - gameState.garages[i].size.x,
                  i * gameState.garages[i].size.y};
    gameState.garages[i].color = garageColors[i];
  }
}

// Unload game variables
void UnloadGame(void) {
  UnloadTexture(autoTexture);
  UnloadTexture(garageTexture);
  UnloadSound(autoSad);
  UnloadSound(autoHappy);
  free(gameState.garages);
  CloseAudioDevice();
}

// Update and Draw (one frame)
void UpdateDrawFrame(void) {
  UpdateGame();
  DrawGame();
}
