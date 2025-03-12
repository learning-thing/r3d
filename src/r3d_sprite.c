#include "r3d.h"

#include <raymath.h>

R3D_Sprite R3D_LoadSprite(Texture2D texture, int xFrameCount, int yFrameCount)
{
    R3D_Sprite sprite = { 0 };

    sprite.material = LoadMaterialDefault();
    sprite.material.maps[MATERIAL_MAP_ALBEDO].texture = texture;
    sprite.material.maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;

    sprite.frameSize.x = texture.width / xFrameCount;
    sprite.frameSize.y = texture.height / yFrameCount;

    sprite.xFrameCount = xFrameCount;
    sprite.yFrameCount = yFrameCount;

    return sprite;
}

void R3D_UnloadSprite(R3D_Sprite sprite)
{
    if (IsMaterialValid(sprite.material)) {
        UnloadMaterial(sprite.material);
    }
}

void R3D_UpdateSprite(R3D_Sprite* sprite, float speed)
{
    R3D_UpdateSpriteEx(sprite, 0, sprite->xFrameCount * sprite->yFrameCount, speed);
}

void R3D_UpdateSpriteEx(R3D_Sprite* sprite, int firstFrame, int lastFrame, float speed)
{
    sprite->currentFrame = Wrap(sprite->currentFrame + speed, firstFrame, lastFrame);
}

Vector2 R3D_GetCurrentSpriteFrameCoord(const R3D_Sprite* sprite)
{
    int xFrame = (int)(sprite->currentFrame) % sprite->xFrameCount;
    int yFrame = (int)(sprite->currentFrame) / sprite->yFrameCount;
    return Vector2Multiply((Vector2) { (float)xFrame, (float)yFrame }, sprite->frameSize);
}

Rectangle R3D_GetCurrentSpriteFrameRect(const R3D_Sprite* sprite)
{
    Vector2 coord = R3D_GetCurrentSpriteFrameCoord(sprite);

    return (Rectangle) {
        coord.x, coord.y,
        sprite->frameSize.x,
        sprite->frameSize.y
    };
}
