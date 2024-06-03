#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "pd_api.h"

static int update(void* userdata);
void ProcessInput(PlaydateAPI* pd);
void drawMaze(PlaydateAPI* pd);
LCDBitmap* getFrame(PlaydateAPI *pd, int x, int y);
void init(PlaydateAPI *pd);
int build_maze(PlaydateAPI *pd);
void drawPlayer(PlaydateAPI* pd);
double crank_angle(PlaydateAPI *pd);
int compute_scale(double angle);

const int MAZE_X = 12;
const int MAZE_Y = 7;
const int MAZE_LINEAR = MAZE_X * MAZE_Y;
const int X_OFFSET = 7;
const int Y_OFFSET = 8;
const int TILE_SIZE = 8;

const bool debug_output = false;

struct {
  int SCREEN_WIDTH; // Size of screen. Should be a const.
  int SCREEN_HEIGHT; // Size of screen. Should be a const.
  float frame; // ??
  LCDBitmapTable* bitmapTable; // Maze walls BitMap
	LCDBitmapTable* playerTable; //Player BitMap
  const char* err; // Any errors we want to capture.
	int maze[84]; // Linear array of maze, arranged as a full X row, then another.
	int player_x; // Player X relative to sqaure of maze.
	int player_y;	// Player Y relative to square of maze.
	double playerFrame; // Which player frame in the tileset to display.
	int scale; // Current scale we're using.
	int tile_scale; // How big each tile should be.
} GAMEDATA;


const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

#ifdef _WINDLL
__declspec(dllexport)
#endif

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	if ( event == kEventInit )
	{
		const char* err;
		font = pd->graphics->loadFont(fontpath, &err);
		
		if ( font == NULL )
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

		init(pd);

		// Note: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and doesn't run any Lua code in the game
		pd->system->setUpdateCallback(update, pd);
	}
	
	return 0;
}

void init(PlaydateAPI *pd)
{  
  const char* wall_table = "walls"; // "robot-table-8-8.png";
  GAMEDATA.bitmapTable = pd->graphics->loadBitmapTable( wall_table, &GAMEDATA.err );

  const char* player_table = "player"; // "robot-table-8-8.png";
  GAMEDATA.playerTable = pd->graphics->loadBitmapTable( player_table, &GAMEDATA.err );

	GAMEDATA.scale = 4;
	GAMEDATA.player_x = 0;
	GAMEDATA.player_y = 0;

	srand(pd->system->getCurrentTimeMilliseconds());

	build_maze(pd);
}

static int update(void* userdata)
{
	// Get API object, clear the screen.
	PlaydateAPI* pd = userdata;
	pd->graphics->clear(kColorWhite);

	// Process controls...
  ProcessInput(pd);
	GAMEDATA.scale = compute_scale(crank_angle(pd));
	GAMEDATA.tile_scale = 8 * GAMEDATA.scale;

	// Dow things with the data...
  drawMaze(pd);
	drawPlayer(pd);

	return 1;
}

void ProcessInput(PlaydateAPI* pd)
{
  PDButtons current;
  PDButtons pushed;
  PDButtons released;
  pd->system->getButtonState( &current, &pushed, &released );

  if ( pushed & kButtonLeft  ) { GAMEDATA.player_x--; }
  else if ( pushed & kButtonRight ) { GAMEDATA.player_x++; }
  else if ( pushed & kButtonUp    ) { GAMEDATA.player_y--; }
  else if ( pushed & kButtonDown  ) { GAMEDATA.player_y++; }
  else if ( pushed & kButtonA ) { build_maze(pd); }

	if (GAMEDATA.player_x <= 0) GAMEDATA.player_x = 0;
	if (GAMEDATA.player_x >= MAZE_X-1) GAMEDATA.player_x = MAZE_X-1;
	if (GAMEDATA.player_y <= 0) GAMEDATA.player_y = 0;
	if (GAMEDATA.player_y >= MAZE_Y-1) GAMEDATA.player_y = MAZE_Y-1;
}

void drawPlayer(PlaydateAPI* pd)
{
  GAMEDATA.playerFrame += 0.2;

	if (GAMEDATA.playerFrame > 4) GAMEDATA.playerFrame = 0;

	LCDBitmap *p = pd->graphics->getTableBitmap( GAMEDATA.playerTable, (int) GAMEDATA.playerFrame);
  pd->graphics->drawScaledBitmap( p, X_OFFSET+(GAMEDATA.player_x*GAMEDATA.tile_scale), Y_OFFSET+(GAMEDATA.player_y*GAMEDATA.tile_scale), GAMEDATA.scale, GAMEDATA.scale );
}

int compute_scale(double angle)
{
	if (angle > 0 && angle < 72) return 1;
	if (angle > 71 && angle < 144) return 2;
	if (angle > 143 && angle < 215) return 3;
	if (angle > 214 && angle < 286) return 4;
	if (angle > 287 && angle < 360) return 5;
}

void drawMaze(PlaydateAPI* pd)
{
	//Loop over the grid like it's 1992...
	int x = 0;
	int y = 0;

	// Until we can create a maze, just loop through different aspects.
  GAMEDATA.frame = 0;

	for (x=0;x<MAZE_X;x++)
	{
		for (y=0;y<MAZE_Y;y++)
		{
			LCDBitmap* frame = getFrame(pd, x, y);
      pd->graphics->drawScaledBitmap( frame, X_OFFSET+(x*GAMEDATA.tile_scale), Y_OFFSET+(y*GAMEDATA.tile_scale), GAMEDATA.scale, GAMEDATA.scale);
		}
	}
}

LCDBitmap* getFrame(PlaydateAPI *pd, int x, int y)
{
	// Y rows of X, plus the extra X.. this is counter intuitive, but essential.
	int location = (y * MAZE_X) + x;
	
	// GCC compiler goes bananas if our code looks like it can go out of bounds...
	if (location > MAZE_LINEAR) location = MAZE_LINEAR;

	//pd->system->logToConsole("Location: %d = (%d * %d) + %d", location, y, MAZE_X, x);

  return pd->graphics->getTableBitmap( GAMEDATA.bitmapTable, GAMEDATA.maze[location]);
}

int build_maze(PlaydateAPI *pd)
{
	for(int i=0;i<MAZE_LINEAR;i++)
	{	
	  GAMEDATA.maze[i] = rand() % 14;
	}
	return 0;
}

double crank_angle(PlaydateAPI *pd)
{
  // Check crank state
  double crankAngleDegrees = pd->system->getCrankAngle();
  //double crankAngleRadians = crankAngleDegrees * ( 3.14159 / 180 );
  //double crankChangeDegrees = pd->system->getCrankChange();

	if (debug_output) pd->system->logToConsole("Crank: %f", crankAngleDegrees);

	return crankAngleDegrees;
}