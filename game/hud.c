#include "hud.h"
#include "gear.h"
#include "player.h"
#include "../engine/font.h"
#include "../engine/colours.h"


void DrawGameplayHUD(GsOT *ot) {
    char hudStr[32];
    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetColour(COL_DARKRED);

    sprintf(hudStr, "SPEED:%d MPH", (int)player1.speed / 2);
    FontFX_Print(20, 20, hudStr, ot, OT_UI);

    if (currentGear == 0)
        sprintf(hudStr, "GEAR:R %s", (gearboxMode == GEAR_MANUAL) ? "[M]" : "[A]");
    else
        sprintf(hudStr, "GEAR:%d %s", currentGear, (gearboxMode == GEAR_MANUAL) ? "[M]" : "[A]");
    
	FontFX_Print(20, 30, hudStr, ot, OT_UI);

    sprintf(hudStr, "X:%d Y:%d Z:%d", (int)player1.gsObjectCoord.coord.t[0], (int)player1.gsObjectCoord.coord.t[1], (int)player1.gsObjectCoord.coord.t[2]);
    FontFX_Print(20, 200, hudStr, ot, OT_UI);

    FontFX_FontEnd();
}