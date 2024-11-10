#include "icb_gui.h"

// Globals
/*
5. Note that the use of global variables is restricted, except for the designated ICBYTES
object and the keypressed variable.
*/
int keypressed;
ICBYTES screenMatrix;

struct GameObject {
    int x, y;
    int width, height;
    bool isAlive;
    int explosionType;
    int explosionFrame;
};

struct ThreadParams {
    GameObject ship;
    GameObject box;
    GameObject bullet;
    int FRM1;
    bool* gameRunning;
    bool& isGameRunning() {
        return *gameRunning;
    }
};

// Create window
void ICGUI_Create() {
    ICG_MWTitle("Space Shooter");
    ICG_MWSize(540, 600);
}


void DrawExplosion(GameObject* obj, ThreadParams* params) {
    //Draw Explosion
    if (obj->explosionFrame > 10) {
        obj->isAlive = false;
        obj->explosionType = 0;
        obj->explosionFrame = 0;
        FillRect(screenMatrix, obj->x, obj->y, params->box.width, params->box.height, 0x000000); // Patlama alanını temizle
        return;
    }

    
    int color = 0xFFFF00; // Explosion Color Yellow
    FillRect(screenMatrix, obj->x, obj->y, params->box.width, params->box.height, color);

    // Explosion Middle
    if (obj->explosionType == 2) {
        FillRect(screenMatrix, obj->x + ((params->box.width - (obj->explosionFrame * (params->box.width / 10))) / 2), obj->y, obj->explosionFrame * (params->box.width / 10), params->box.height, 0x000000);
        FillRect(screenMatrix, obj->x, obj->y + ((params->box.height - (obj->explosionFrame * (params->box.width / 10))) / 2), params->box.width, obj->explosionFrame * (params->box.height / 10), 0x000000);
    }
    // Case 1 Case 3  diagonally move Case 2 explosion 
    switch (obj->explosionType) {
    case 1:
        obj->x += 8;
        obj->y -= 8;
        break;
    case 2:
        obj->y -= 8;
        break;
    case 3:
        obj->x -= 8;
        obj->y -= 8;
        break;
    }



    obj->explosionFrame++;
}

 
// Drawing Thread
void DrawThread(ThreadParams* params) {
    while (params->isGameRunning()) {
        // Clear screen
        screenMatrix = 0;

        // Draw ship
        FillRect(screenMatrix, params->ship.x, params->ship.y,
            params->ship.width, params->ship.height, 0xFF0000);

        // Draw box or explosion
        if (params->box.isAlive) {
            FillRect(screenMatrix, params->box.x, params->box.y,
                params->box.width, params->box.height, 0x00FF00);
        }
        else if (params->box.explosionType > 0) {
            DrawExplosion(&params->box, params);
        }

        // Draw bullet
        if (params->bullet.isAlive) {
            FillRect(screenMatrix, params->bullet.x, params->bullet.y,
                params->bullet.width, params->bullet.height, 0x0000FF);
        }

        DisplayImage(params->FRM1, screenMatrix);
        Sleep(30);
    }
}


void ShipThread(ThreadParams* params) {
    while (params->isGameRunning()) {
        // Move Ship
        if (keypressed == 37) params->ship.x = max(0, params->ship.x - 10);
        if (keypressed == 39) params->ship.x = min(460, params->ship.x + 10);
        keypressed = 0;
        Sleep(30);
    }
}

void BoxThread(ThreadParams* params) {
    while (params->isGameRunning()) {
        if (!params->box.isAlive && params->box.explosionType == 0) {
            // Respawn box
            params->box.x = rand() % 430;
            params->box.y = 0;
            params->box.isAlive = true;
            params->box.explosionType = 0;
            params->box.explosionFrame = 0;
        }
        // Move box
        else if (params->box.isAlive) {
            params->box.y += 4;
            // Collision to ship or cross to border
            if (params->box.y > params->ship.y ||
                (params->box.y + params->box.height >= params->ship.y &&
                    params->box.x + params->box.width >= params->ship.x &&
                    params->box.x <= params->ship.x + params->ship.width)) {

                //delete objects
                params->box.isAlive = false;
                params->ship.isAlive = false;
                params->bullet.isAlive = false;

                // GameOver flag
                *(params->gameRunning) = false;
                Sleep(50);
                // GAME OVER Screen
                screenMatrix = 0x000055;
                ICG_SetFont(50, 0, "Arial");
                Impress12x20(screenMatrix, 200, 250, "GAME OVER", 0xFFFFFF);
                DisplayImage(params->FRM1, screenMatrix);



            }
        }
        Sleep(30);
    }
}

void BulletThread(ThreadParams* params) {
    while (params->isGameRunning()) {
        // Create Bullet
        if (keypressed == 32 && !params->bullet.isAlive) {
            params->bullet.x = params->ship.x + (params->ship.width / 2) - (params->bullet.width / 2);
            params->bullet.y = params->ship.y - params->bullet.height;
            params->bullet.isAlive = true;
            keypressed = 0;
        }
        // Move Bullet
        if (params->bullet.isAlive) {
            params->bullet.y -= 10;
            if (params->box.isAlive &&
                params->bullet.y <= params->box.y + params->box.height &&
                params->bullet.y + params->bullet.height >= params->box.y &&
                params->bullet.x + params->bullet.width >= params->box.x &&
                params->bullet.x <= params->box.x + params->box.width) {


                int hitX = params->bullet.x - params->box.x;
                /*
                2. When a bullet strikes the left side of a box, specifically within the leftmost 3-unit
                region, the box will move to the top-right corner.
                */
                int leftPart = params->box.width / 3 - 1;

                /*
                4. A bullet that hits the central 4-unit region of the box will result in the box being
                destroyed.
                */
                int middlePart = params->box.width - leftPart;
                if (hitX <= leftPart) //3*x*k

                    params->box.explosionType = 1;
                else if (hitX <= middlePart) //4*x*k
                    params->box.explosionType = 2;

                /*
                3. If a bullet hits the right side of the box, within the rightmost 3-unit region, the box
                will instead move to the top-left corner.
                */
                else  //3*x*k
                    params->box.explosionType = 3;

                params->box.isAlive = false;
                params->bullet.isAlive = false;
            }
            // Screen overflow check
            if (params->bullet.y < 0) {
                params->bullet.isAlive = false;
            }
        }
        Sleep(30);
    }
}

void StartGame(void* gameRunning) {
    bool* gameRunningPtr = (bool*)gameRunning;
    if (*gameRunningPtr) return;

    *gameRunningPtr = true;
    // Reset the screen
    screenMatrix = 0;

    // Define ThreadParams
    int gameScreenX = 500, GameScreenY = 500;
    int shipX = 250, shipY = 485;
    int boxSize = 40;
    int bulletWidth = 2, bulletHeight = 10;

    ThreadParams* params = new ThreadParams{
        {shipX, shipY, 40, 10, true, 0, 0},
        {rand() % (gameScreenX - boxSize) , 0, boxSize, boxSize, true, 0, 0},
        {0, 0, bulletWidth, bulletHeight, false, 0, 0},
        ICG_FrameMedium(5, 40, 1, 1),
        gameRunningPtr
    };

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DrawThread, params, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShipThread, params, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BoxThread, params, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BulletThread, params, 0, NULL);

    SetFocus(ICG_GetMainWindow());
}

void WhenKeyPressed(int k) {
    keypressed = k;
}

void ICGUI_main() {
    CreateImage(screenMatrix, 500, 500, ICB_UINT);
    screenMatrix = 0;

    static bool gameRunning = false;
    ICG_Button(5, 5, 120, 25, "START GAME", StartGame, &gameRunning);
    ICG_SetOnKeyPressed(WhenKeyPressed);
}