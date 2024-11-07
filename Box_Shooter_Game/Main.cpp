#include "icb_gui.h"

// Globals
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
    GameObject alien;
    GameObject bullet;
    int FRM1;
    bool gameRunning;
};

ThreadParams* globalParams = nullptr;

// Thread function headers
void _WaitThread(HANDLE thread);
void _CreateThread(HANDLE thread, void* threadMain);

void ICGUI_Create() {
    ICG_MWTitle("Space Shooter");
    ICG_MWSize(700, 700);
}

void DrawExplosion(GameObject* obj, ThreadParams* params) {
    FillRect(screenMatrix, params->alien.x, params->alien.y,
        params->alien.width, params->alien.height, 0);

    if (obj->explosionFrame > 10) {
        obj->isAlive = false;
        obj->explosionType = 0;
        obj->explosionFrame = 0;
        FillRect(screenMatrix, obj->x, obj->y, params->alien.width, params->alien.height, 0x000000);
        return;
    }

    FillRect(screenMatrix, obj->x, obj->y, params->alien.width, params->alien.height, 0x000000);
    switch (obj->explosionType) {
    case 1:
        obj->x = obj->x + 8;
        obj->y = obj->y - 8;
        break;
    case 2:
        obj->y = obj->y - 8;
        break;
    case 3:
        obj->x = obj->x - 8;
        obj->y = obj->y - 8;
        break;
    }
    FillRect(screenMatrix, obj->x, obj->y, params->alien.width, params->alien.height, 0xFFFF00);
    obj->explosionFrame++;
}

void AnimationThread(ThreadParams* params) {
    while (params->gameRunning) {
        DisplayImage(params->FRM1, screenMatrix);
        Sleep(30);
    }

    // Game Over screen
    screenMatrix = 0x000055;
    ICG_SetFont(50, 0, "Arial");
    Impress12x20(screenMatrix, 275, 300, "GAME OVER", 0xFFFFFF);
    DisplayImage(params->FRM1, screenMatrix);
}

void ShipThread(ThreadParams* params) {
    while (params->gameRunning) {
        // Delete old position
        FillRect(screenMatrix, params->ship.x, params->ship.y,
            params->ship.width, params->ship.height, 0);

        // Move Ship
        if (keypressed == 37) params->ship.x = max(0, params->ship.x - 10);
        if (keypressed == 39) params->ship.x = min(610, params->ship.x + 10);

        // Print new position
        FillRect(screenMatrix, params->ship.x, params->ship.y,
            params->ship.width, params->ship.height, 0xFF0000);

        Sleep(10);
    }
}

void AlienThread(ThreadParams* params) {
    while (params->gameRunning) {
        if (!params->alien.isAlive) {
            params->alien.x = rand() % 580;
            params->alien.y = 0;
            params->alien.isAlive = true;
            params->alien.explosionType = 0;
            params->alien.explosionFrame = 0;
        }

        if (params->alien.explosionType > 0) {
            Sleep(10);
            DrawExplosion(&params->alien, params);
        }
        else {
            // Delete old position
            FillRect(screenMatrix, params->alien.x, params->alien.y,
                params->alien.width, params->alien.height, 0);

            // Move alien
            params->alien.y += 4;

            if (params->alien.y > 600 ||
                (params->alien.y + params->alien.height >= params->ship.y &&
                    params->alien.x + params->alien.width >= params->ship.x &&
                    params->alien.x <= params->ship.x + params->ship.width)) {
                params->gameRunning = false;
                params->alien.isAlive = false;
                continue;
            }

            // Print new position of alien
            FillRect(screenMatrix, params->alien.x, params->alien.y,
                params->alien.width, params->alien.height, 0x00FF00);
        }

        Sleep(10);
    }
}

void BulletThread(ThreadParams* params) {
    while (params->gameRunning) {
        if (keypressed == 32 && !params->bullet.isAlive) {
            params->bullet.x = params->ship.x + (params->ship.width / 2) - (params->bullet.width / 2);
            params->bullet.y = params->ship.y - params->bullet.height;
            params->bullet.isAlive = true;
            keypressed = 0;
        }

        if (params->bullet.isAlive) {
            // Delete old position of bullet
            FillRect(screenMatrix, params->bullet.x, params->bullet.y,
                params->bullet.width, params->bullet.height, 0);

            // Move Bullet
            params->bullet.y -= 10;

            // Collision Check
            if (params->alien.isAlive &&
                params->bullet.y <= params->alien.y + params->alien.height &&
                params->bullet.y + params->bullet.height >= params->alien.y &&
                params->bullet.x + params->bullet.width >= params->alien.x &&
                params->bullet.x <= params->alien.x + params->alien.width) {

                int hitX = params->bullet.x - params->alien.x;
                if (hitX < params->alien.width / 3)
                    params->alien.explosionType = 1;
                else if (hitX < (params->alien.width * 2) / 3)
                    params->alien.explosionType = 2;
                else
                    params->alien.explosionType = 3;

                params->bullet.isAlive = false;
                continue;
            }

            // Screen overflow check
            if (params->bullet.y < 0) {
                params->bullet.isAlive = false;
                continue;
            }

            // Print new position of bullet
            FillRect(screenMatrix, params->bullet.x, params->bullet.y,
                params->bullet.width, params->bullet.height, 0x0000FF);
        }

        Sleep(10);
    }
}

void ScoreThread(ThreadParams* params) {
    // Score processing can be added here.
}

void StartGame() {
    if (globalParams) {
        // Reset the previous game
        delete globalParams;
        screenMatrix = 0; // Clear the entire screen
    }

    // Starting a new game
    globalParams = new ThreadParams{
        {300, 580, 60, 20, true, 0, 0},      // ship
        {rand() % 580, 0, 40, 40, true, 0, 0},  // alien
        {0, 0, 10, 20, false, 0, 0},         // bullet
        ICG_FrameMedium(5, 40, 700, 700),    // frm1
        true                                 // gameRunning
    };

    // Start threads passing `globalParams`
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnimationThread, globalParams, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShipThread, globalParams, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AlienThread, globalParams, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BulletThread, globalParams, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ScoreThread, globalParams, 0, NULL);

    SetFocus(ICG_GetMainWindow());
}


void WhenKeyPressed(int k) {
    keypressed = k;
    Sleep(30);
    keypressed = 0;
}

void ICGUI_main() {
    CreateImage(screenMatrix, 700, 700, ICB_UINT);
    screenMatrix = 0;

    ICG_Button(5, 5, 120, 25, "START GAME", StartGame);
    ICG_SetOnKeyPressed(WhenKeyPressed);
}
