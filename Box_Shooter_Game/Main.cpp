#include "icb_gui.h"

//globals
int keypressed;
ICBYTES screenMatrix, spriteSheet;
int FRM1;
bool gameRunning = false;

HANDLE TAnimation = NULL;   // Animation Thread handle
HANDLE TShip = NULL;        // Ship Thread handle
HANDLE TAlien = NULL;       // Alien Thread handle
HANDLE TBullet = NULL;      // Bullet Thread handle

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
};

ThreadParams* globalParams = nullptr;

//Thread functions headers
void _WaitThread(HANDLE thread);
void _CreateThread(HANDLE thread, void* threadMain);

void ICGUI_Create() {
    ICG_MWTitle("Space Shooter");
    ICG_MWSize(700, 700);
}

void DrawExplosion(GameObject* obj) {
    FillRect(screenMatrix, globalParams->alien.x, globalParams->alien.y,
        globalParams->alien.width, globalParams->alien.height,
        0);
    if (obj->explosionFrame > 10) {
        obj->isAlive = false;
        obj->explosionType = 0;
        obj->explosionFrame = 0;
        FillRect(screenMatrix, obj->x, obj->y, 30, 30, 0x000000);

        return;
    }

    FillRect(screenMatrix, obj->x, obj->y, 30, 30, 0x000000);
    switch (obj->explosionType) {
    case 1:
        obj->x = obj->x + 2;
        obj->y = obj->y - 2;
        break;
    case 2:
        obj->y = obj->y - 2;
        break;
    case 3:
        obj->x = obj->x - 2;
        obj->y = obj->y - 2;
        break;
    }
    FillRect(screenMatrix, obj->x, obj->y, 30, 30, 0xFFFFFF);
    obj->explosionFrame++;

}

void AnimationThread(LPVOID param) {
    while (gameRunning) {
        DisplayImage(FRM1, screenMatrix);
        Sleep(30);
    }
}

void ShipThread(LPVOID param) {
    while (gameRunning) {
        // Delete old position
        FillRect(screenMatrix, globalParams->ship.x, globalParams->ship.y,
            globalParams->ship.width, globalParams->ship.height,
            0);

        // Move Ship
        if (keypressed == 37) globalParams->ship.x = max(0, globalParams->ship.x - 10);
        if (keypressed == 39) globalParams->ship.x = min(610, globalParams->ship.x + 10);

        // Print new position
        FillRect(screenMatrix, globalParams->ship.x, globalParams->ship.y,
            globalParams->ship.width, globalParams->ship.height,
            0xFF0000);

        Sleep(10);
    }
}

void AlienThread(LPVOID param) {
    while (gameRunning) {
        if (!globalParams->alien.isAlive) {
            globalParams->alien.x = rand() % 580;
            globalParams->alien.y = 0;
            globalParams->alien.isAlive = true;
            globalParams->alien.explosionType = 0;
            globalParams->alien.explosionFrame = 0;
        }

        if (globalParams->alien.explosionType > 0) {
            Sleep(10);
            DrawExplosion(&globalParams->alien);

        }
        else {
            // Delete old position
            FillRect(screenMatrix, globalParams->alien.x, globalParams->alien.y,
                globalParams->alien.width, globalParams->alien.height, 0);

            // Move alien
            globalParams->alien.y += 2;

            // Screen overflow check
            if (globalParams->alien.y > 600) {
                globalParams->alien.isAlive = false;
                continue;
            }

            // Print new position of alien
            FillRect(screenMatrix, globalParams->alien.x, globalParams->alien.y,
                globalParams->alien.width, globalParams->alien.height, 0x00FF00);
        }

        Sleep(10);
    }
}

void BulletThread(LPVOID param) {
    while (gameRunning) {
        if (keypressed == 32 && !globalParams->bullet.isAlive) {
            globalParams->bullet.x = globalParams->ship.x +
                (globalParams->ship.width / 2) -
                (globalParams->bullet.width / 2);
            globalParams->bullet.y = globalParams->ship.y - globalParams->bullet.height;
            globalParams->bullet.isAlive = true;
            keypressed = 0;
        }

        if (globalParams->bullet.isAlive) {
            // Delete old position of bullet
            FillRect(screenMatrix, globalParams->bullet.x, globalParams->bullet.y,
                globalParams->bullet.width, globalParams->bullet.height,
                0);

            // Move Bullet
            globalParams->bullet.y -= 10;

            // Collision Check
            if (globalParams->alien.isAlive &&
                globalParams->bullet.y <= globalParams->alien.y + globalParams->alien.height &&
                globalParams->bullet.y + globalParams->bullet.height >= globalParams->alien.y &&
                globalParams->bullet.x + globalParams->bullet.width >= globalParams->alien.x &&
                globalParams->bullet.x <= globalParams->alien.x + globalParams->alien.width) {

                int hitX = globalParams->bullet.x - globalParams->alien.x;
                if (hitX < globalParams->alien.width / 3)
                    globalParams->alien.explosionType = 1;
                else if (hitX < (globalParams->alien.width * 2) / 3)
                    globalParams->alien.explosionType = 2;
                else
                    globalParams->alien.explosionType = 3;

                globalParams->bullet.isAlive = false;
                continue;
            }

            // Screen overflow check
            if (globalParams->bullet.y < 0) {
                globalParams->bullet.isAlive = false;
                continue;
            }

            // Print new position of bullet
            FillRect(screenMatrix, globalParams->bullet.x, globalParams->bullet.y,
                globalParams->bullet.width, globalParams->bullet.height,
                0x0000FF);
        }

        Sleep(10);
    }
}


void StartGame() {
    if (!gameRunning) {
        if (globalParams != nullptr) {
            delete globalParams;
        }

        globalParams = new ThreadParams{
            {300, 580, 60, 20, true, 0, 0},    // ship
            {rand() % 580, 0, 40, 40, true, 0, 0},  // alien
            {0, 0, 10, 20, false, 0, 0}        // bullet
        };

        screenMatrix = 0;
        gameRunning = true;

        _WaitThread(TAnimation);
        _CreateThread(TAnimation, AnimationThread);

        _WaitThread(TShip);
        _CreateThread(TShip, ShipThread);

        _WaitThread(TAlien);
        _CreateThread(TAlien, AlienThread);

        _WaitThread(TBullet);
        _CreateThread(TBullet, BulletThread);


        SetFocus(ICG_GetMainWindow());
    }
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
    FRM1 = ICG_FrameMedium(5, 40, 700, 700);

    ICG_SetOnKeyPressed(WhenKeyPressed);
}

//Wait execution and close thread
void _WaitThread(HANDLE thread)
{
    if (thread == NULL) {
        return;
    }
    // Wait for thread complete execution
    WaitForSingleObject(TAnimation, INFINITE);
    // Close the handle
    CloseHandle(TAnimation);
    thread = NULL;
}

//Creates new Thread
void _CreateThread(HANDLE thread, void* threadMain)
{
    thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadMain, NULL, 0, NULL);
}

