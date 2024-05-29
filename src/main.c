#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

static int update(void* userdata);
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


#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

int x = (400-TEXT_WIDTH)/2;
int y = (240-TEXT_HEIGHT)/2;
int dx = 1;
int dy = 2;
char buttonState[100];


struct {
  int SCREEN_WIDTH;
  int SCREEN_HEIGHT;
  float frame;
  float frameMax;
  LCDBitmapTable* bitmapTable;
  const char* err;
} GAMEDATA;

void ProcessInput(PlaydateAPI* pd);
void drawMaze(PlaydateAPI* pd);
LCDBitmap* getFrame(PlaydateAPI *pd);

void init(PlaydateAPI *pd)
{  
	// LCDBitmapTable* playdate->graphics->loadBitmapTable(const char* path, const char** outerr);
  GAMEDATA.bitmapTable = pd->graphics->loadBitmapTable( wall_table, &GAMEDATA.err );

	if (GAMEDATA.bitmapTable == NULL)
	{
    pd->system->error( "Could not load bitmap table: %s", GAMEDATA.err );
	}
	else pd->system->logToConsole( "Got bitmap");
}

static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;
	
	pd->graphics->clear(kColorWhite);
	pd->graphics->setFont(font);

  ProcessInput(pd);

	pd->graphics->drawText(buttonState, strlen(buttonState), kASCIIEncoding, x, y);

	// x += dx;
	// y += dy;
	
	// if ( x < 0 || x > LCD_COLUMNS - TEXT_WIDTH )
	// 	dx = -dx;
	
	// if ( y < 0 || y > LCD_ROWS - TEXT_HEIGHT )
	// 	dy = -dy;

  drawMaze(pd);

	// pd->system->drawFPS(0,0);

	return 1;
}

void ProcessInput(PlaydateAPI* pd)
{
  PDButtons current;
  PDButtons pushed;
  PDButtons released;
  pd->system->getButtonState( &current, &pushed, &released );

  // if ( pushed & kButtonLeft  ) { GAMEDATA.frame--;   }
  // else if ( pushed & kButtonUp    ) { GAMEDATA.frame++;     }
  // else if ( pushed & kButtonDown  ) { GAMEDATA.frame--;   }
  // else if ( pushed & kButtonRight ) { GAMEDATA.frame++;  }

  // if ( current & kButtonLeft  ) { strcpy( buttonState, "left held" );   }
  // else if ( current & kButtonUp    ) { strcpy( buttonState, "up held" );     }
  // else if ( current & kButtonDown  ) { strcpy( buttonState, "down held" );   }
  // else if ( current & kButtonRight ) { strcpy( buttonState, "right held" );  }
  // else if ( current & kButtonA     ) { strcpy( buttonState, "a held" );      }
  // else if ( current & kButtonB     ) { strcpy( buttonState, "b held" );      }
  // else if ( released ) { strcpy( buttonState, "" );   }
}


void drawMaze(PlaydateAPI* pd)
{
//  enum LCDBitmapColor white = kColorWhite;
	//pd->graphics->clear(kColorWhite);


  // Load a bitmap table
  const char* wall_table = "walls"; // "robot-table-8-8.png";


 

	// char *frame;
	// sprintf(frame, "%1.0f", GAMEDATA.frame );
  // strcpy( buttonState, frame); 


	int x = 0;
	int y = 0;

  GAMEDATA.frame = 0;

	for (x=0;x<20;x++)
	{
		for (y=0;y<20;y++)
		{
			 
       pd->graphics->drawBitmap( getFrame(pd), 10+(x*8), 10+(y*8), 0 );
		}
	}
}


LCDBitmap* getFrame(PlaydateAPI *pd)
{
  LCDBitmap* currentFrame = pd->graphics->getTableBitmap( GAMEDATA.bitmapTable, GAMEDATA.frame );
	GAMEDATA.frame++;
	if ( GAMEDATA.frame >= 14 || GAMEDATA.frame <= 0)
  {
      GAMEDATA.frame = 0;
  }
  return currentFrame;
}