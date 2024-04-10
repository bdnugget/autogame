#include "raylib.h"

#include <time.h>   // For time function
#include <stdlib.h> // For NULL


#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// ----------------------------------------------------------------------------------
//Some Defines
// ----------------------------------------------------------------------------------
#define SQUARE_SIZE    200
#define FPS_TARGET 60
#define GARAGE_COUNT 5

//----------------------------------------------------------------------------------
//Types and Structures Definition
// ----------------------------------------------------------------------------------
typedef struct Car {
	Vector2		position;
	Vector2		size;
	Color		color;
	int		direction;
}		Car;

typedef struct Garage {
	Vector2		position;
	Vector2		size;
	Color		color;
}		Garage;

//------------------------------------------------------------------------------------
//Global Variables Declaration
// ------------------------------------------------------------------------------------
static const int screenWidth = 1600;
static const int screenHeight = 1000;

static int	framesCounter = 0;
static bool	gameOver = false;
static bool	pause = false;

static Car car = {
	0
};
static bool	allowMove = false;
static Vector2 offset = {
	0
};

static Texture autoTexture;
static Texture garageTexture;

static Garage garages[GARAGE_COUNT];
static Color garageColors[GARAGE_COUNT] = {
	RED, GREEN, BLUE, YELLOW, PURPLE
};

static Sound autoHappy;
static Sound autoSad;

//------------------------------------------------------------------------------------
//Module Functions Declaration(local)
// ------------------------------------------------------------------------------------
static void	InitGame(void);
static void	UpdateGame(void);
static void	DrawGame(void);
static void	UnloadGame(void);
static void	UpdateDrawFrame(void);
static void	InitGarages(void);

// ------------------------------------------------------------------------------------
//Program main entry point
// ------------------------------------------------------------------------------------
int main(void){
	InitWindow(screenWidth, screenHeight, "Dikke Vette Cargame voor Milo");
  InitAudioDevice();
  autoHappy = LoadSound("resources/auto_happy_vob.ogg");
  //autoHappy = LoadSound("resources/fart.mp3");
  autoSad = LoadSound("resources/auto_sad_vob.ogg");
	autoTexture = LoadTexture("resources/auto_200px.png");
	garageTexture = LoadTexture("resources/autogarage_200px.png");
 
	InitGame();

#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(UpdateDrawFrame, FPS_TARGET, 1);
#else
	SetTargetFPS(FPS_TARGET);
	//--------------------------------------------------------------------------------------

		//Main game loop
		while (!WindowShouldClose())
		//Detect window close button or ESC key
	{
		//Update and Draw
			// ----------------------------------------------------------------------------------
			UpdateDrawFrame();
		//----------------------------------------------------------------------------------
	}
#endif
	//De - Initialization
		// --------------------------------------------------------------------------------------
		UnloadGame();
    UnloadSound(autoSad);
    UnloadSound(autoHappy);
	//Unload loaded data(textures, sounds, models...)

    CloseAudioDevice();

		CloseWindow();
	//Close window and OpenGL context
		// --------------------------------------------------------------------------------------

		return 0;
}

//------------------------------------------------------------------------------------
//Module Functions Definitions(local)
// ------------------------------------------------------------------------------------

//Initialize game variables
void
InitGame(void)
{
	SetRandomSeed(time(NULL));

	int randomIndex = GetRandomValue(0, 4);
	car.color = garageColors[randomIndex];

	framesCounter = 0;
	gameOver = false;
	pause = false;

	allowMove = false;

	offset.x = screenWidth % SQUARE_SIZE;
	offset.y = screenHeight % SQUARE_SIZE;

	car.position = (Vector2) {
		offset.x / 2, offset.y / 2 + 2 * SQUARE_SIZE
	};
	car.size = (Vector2) {
		SQUARE_SIZE, SQUARE_SIZE
	};

	InitGarages();
}

//Update game(one frame)
void
UpdateGame(void)
{
	if (!gameOver) {
		if (IsKeyPressed('P'))
			pause = !pause;

		if (!pause) {
			//Player control
				if (IsKeyPressed(KEY_UP) && allowMove && car.position.y > 0) {
				car.direction = -1;
				allowMove = false;
			}
			if (IsKeyPressed(KEY_DOWN) && allowMove && car.position.y < (screenHeight - SQUARE_SIZE)) {
				car.direction = 1;
				allowMove = false;
			}

			//Car movement(once per second)
				if ((framesCounter % FPS_TARGET) == 0) {
				car.position.x += SQUARE_SIZE;
				car.position.y += car.direction * SQUARE_SIZE;
				allowMove = true;
				car.direction = 0;
			}

			//Check for collision with garages
					for (int i = 0; i < GARAGE_COUNT; i++) {
						if (CheckCollisionRecs((Rectangle) {
						car.position.x, car.position.y, car.size.x, car.size.y
						},
						(Rectangle) {
						garages[i].position.x, garages[i].position.y, garages[i].size.x, garages[i].size.y
					})) {
						if (car.color.r == garages[i].color.r && car.color.g == garages[i].color.g && car.color.b == garages[i].color.b) {
              PlaySound(autoHappy);
							InitGame();
						} else {
              PlaySound(autoSad);
							gameOver = true;
						}
					}
				}

			framesCounter++;
		}
	} else {
		if (IsKeyPressed(KEY_ENTER)) {
			InitGame();
			gameOver = false;
		}
	}
}

//Draw game(one frame)
void
DrawGame(void)
{
	BeginDrawing();

	ClearBackground(DARKGRAY);

	if (!gameOver) {
		//Draw grid lines
    /*
			for (int i = 0; i < screenWidth / SQUARE_SIZE + 1; i++) {
			DrawLineV((Vector2) {
				SQUARE_SIZE    *i + offset.x / 2, offset.y / 2
			}	       , (Vector2) {
				SQUARE_SIZE    *i + offset.x / 2, screenHeight - offset.y / 2
			}	       , RAYWHITE);
		}
    */
    
		for (int i = 0; i < screenHeight / SQUARE_SIZE + 1; i++) {
			DrawLineV((Vector2) {
				offset.x / 2, SQUARE_SIZE * i + offset.y / 2
			}, (Vector2) {
				screenWidth - offset.x / 20, SQUARE_SIZE * i + offset.y / 2
			}, RAYWHITE);
		}
    
		//Draw garages
			for (int i = 0; i < GARAGE_COUNT; i++) {
			DrawRectangleRec((Rectangle) {
				garages[i].position.x, garages[i].position.y, garages[i].size.x, garages[i].size.y
			}, garages[i].color);
		}

		//Draw car
		DrawTextureV(autoTexture, car.position, car.color);

		if (pause)
			DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2, screenHeight / 2 - 40, 40, GRAY);
	} else
		DrawText("A WINRAR IS NOT YOU :(\nPRESS [ENTER] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("A WINRAR IS NOT YOU :(\nPRESS [ENTER] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 - 50, 20, GRAY);

	EndDrawing();
}

void
InitGarages(void)
{
	for (int i = 0; i < GARAGE_COUNT; i++) {
		garages[i].size = (Vector2) {
			SQUARE_SIZE, SQUARE_SIZE
		};
		garages[i].position = (Vector2) {
			screenWidth - garages[i].size.x, i * garages[i].size.y
		};
		garages[i].color = garageColors[i];
	}
}

//Unload game variables
void
UnloadGame(void)
{
//TODO:	Unload all dynamic loaded data(textures, sounds, models...)
}

//Update and Draw(one frame)
void
UpdateDrawFrame(void)
{
	UpdateGame();
	DrawGame();
}
