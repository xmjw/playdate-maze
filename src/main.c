#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pd_api.h"

static int update(void* userdata);
void ProcessInput(PlaydateAPI* pd);
void drawMaze(PlaydateAPI* pd);
LCDBitmap* getFrame(PlaydateAPI *pd, int x, int y);
void init(PlaydateAPI *pd);
int build_maze(PlaydateAPI *pd);
void drawPlayer(PlaydateAPI* pd);

const int MAZE_X = 12;
const int MAZE_Y = 7;
const int MAZE_LINEAR = 84;
const int X_OFFSET = 7;
const int Y_OFFSET = 8;

struct {
  int SCREEN_WIDTH;
  int SCREEN_HEIGHT;
  float frame;
  LCDBitmapTable* bitmapTable;
	LCDBitmapTable* playerTable;
  const char* err;
	int maze[84];
	int player_x;
	int player_y;	
	double playerFrame;
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

	if (GAMEDATA.bitmapTable == NULL)
	{
    pd->system->error( "Could not load wall bitmap table: %s", GAMEDATA.err );
	}
	else pd->system->logToConsole( "Got wall bitmap, ready to roll.");

  const char* player_table = "player"; // "robot-table-8-8.png";
  GAMEDATA.playerTable = pd->graphics->loadBitmapTable( player_table, &GAMEDATA.err );

	if (GAMEDATA.playerTable == NULL)
	{
    pd->system->error( "Could not load player bitmap table: %s", GAMEDATA.err );
	}
	else pd->system->logToConsole( "Got player bitmap, ready to roll.");


	GAMEDATA.player_x = 0;
	GAMEDATA.player_y = 0;

	build_maze(pd);

	// srand(time(NULL));
}

static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;
	pd->graphics->clear(kColorWhite);
  ProcessInput(pd);
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
  else if ( pushed & kButtonUp    ) { GAMEDATA.player_y--; }
  else if ( pushed & kButtonDown  ) { GAMEDATA.player_y++; }
  else if ( pushed & kButtonRight ) { GAMEDATA.player_x++; }
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
  pd->graphics->drawScaledBitmap( p, X_OFFSET+(GAMEDATA.player_x*32), Y_OFFSET+(GAMEDATA.player_y*32), 4, 4 );
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
       pd->graphics->drawScaledBitmap( getFrame(pd, x, y), X_OFFSET+(x*32), Y_OFFSET+(y*32), 4, 4 );
		}
	}

	//need to draw a player...
}

LCDBitmap* getFrame(PlaydateAPI *pd, int x, int y)
{
  return pd->graphics->getTableBitmap( GAMEDATA.bitmapTable, GAMEDATA.maze[x*y]);
}

int build_maze(PlaydateAPI *pd)
{
	for(int i=0;i<MAZE_LINEAR;i++)
	{	
	  GAMEDATA.maze[i] = rand() % 14;
	}
	return 0;
}