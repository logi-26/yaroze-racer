#include "skybox.h"

void Skybox_Init(Skybox* skybox, u_long tim_addr) {
    GsIMAGE tim_info;
    
    // Load texture
    LoadTexture((long)tim_addr);
    GsGetTimInfo((u_long *)(tim_addr + 4), &tim_info);
    
    // Configure sprite
    skybox->sprite.attribute = 0; // Critical for PS1 rendering
    skybox->sprite.w = tim_info.pw;
    skybox->sprite.h = tim_info.ph;
    skybox->sprite.u = skybox->sprite.v = 0;
    skybox->sprite.cx = skybox->sprite.cy = 128;
    skybox->sprite.r = skybox->sprite.g = skybox->sprite.b = 128;
    skybox->sprite.mx = skybox->sprite.my = 0;
    skybox->sprite.scalex = ONE * 4;
    skybox->sprite.scaley = ONE * 4;
    skybox->sprite.rotate = 0;
    
    // Set texture page (verify coordinates don't overlap other textures)
    skybox->tpage = GetTPage(
        (tim_info.pmode >> 3) & 1 ? 0 : 2,
        0,
        tim_info.px,
        tim_info.py
    );
}

void Skybox_Draw(Skybox* skybox, GsRVIEW2* view, GsOT* ot) {
    // HARDCODED TEST VALUES
    skybox->sprite.x = 160;  // Screen center X
    skybox->sprite.y = 120;  // Screen center Y
    skybox->sprite.w = 256;
    skybox->sprite.h = 256;
    skybox->sprite.tpage = GetTPage(2, 0, 0, 0); // Force VRAM position 0,0
    skybox->sprite.r = 255;  // Red tint for visibility
    skybox->sprite.g = 0;
    skybox->sprite.b = 0;
    
    GsSortSprite(&skybox->sprite, ot, 0);
    DrawSync(0);
}