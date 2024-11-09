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

//Create window
void ICGUI_Create() {
    ICG_MWTitle("Space Shooter");
    ICG_MWSize(700, 700);
}

void AnimationThread(ThreadParams* params) {
    while (params->isGameRunning()) {
        DisplayImage(params->FRM1, screenMatrix);
        Sleep(30);
    }

    // Game Over screen
    screenMatrix = 0x000055;
    ICG_SetFont(50, 0, "Arial");
    Impress12x20(screenMatrix, 275, 300, "GAME OVER", 0xFFFFFF);
    DisplayImage(params->FRM1, screenMatrix);
    delete params;
}

void ShipThread(ThreadParams* params) {
    while (params->isGameRunning()) {
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

//DrawExplosion
void DrawExplosion(GameObject* obj, ThreadParams* params) {
    FillRect(screenMatrix, params->box.x, params->box.y,
        params->box.width, params->box.height, 0);

    if (obj->explosionFrame > 10) {
        obj->isAlive = false;
        obj->explosionType = 0;
        obj->explosionFrame = 0;
        FillRect(screenMatrix, obj->x, obj->y, params->box.width, params->box.height, 0x000000);
        return;
    }

    FillRect(screenMatrix, obj->x, obj->y, params->box.width, params->box.height, 0x000000);
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
    FillRect(screenMatrix, obj->x, obj->y, params->box.width, params->box.height, 0xFFFF00);
    //FillRect(screenMatrix, obj->x+((params->box.width-(obj->explosionFrame*(params->box.width/10)))/2), obj->y, obj->explosionFrame*(params->box.width/10), params->box.height, 0x000000);
    //FillRect(screenMatrix, obj->x, obj->y+((params->box.height - (obj->explosionFrame*(params->box.width/10))) / 2), params->box.width, obj->explosionFrame*(params->box.height/10), 0x000000);

    switch (obj->explosionType) {
    case 1:
        //left shoot
        //FillRect(screenMatrix, obj->x , 
        //    (obj->y + params->box.height) - (obj->explosionFrame * (params->box.height / 10)), 
        //    obj->explosionFrame * (params->box.width / 10), 
        //    obj->explosionFrame * (params->box.height / 10), 0x000000);

        break;
    case 2:
        //middle shot
        FillRect(screenMatrix, obj->x + ((params->box.width - (obj->explosionFrame * (params->box.width / 10))) / 2), obj->y, obj->explosionFrame * (params->box.width / 10), params->box.height, 0x000000);
        FillRect(screenMatrix, obj->x, obj->y + ((params->box.height - (obj->explosionFrame * (params->box.width / 10))) / 2), params->box.width, obj->explosionFrame * (params->box.height / 10), 0x000000);

        /* FillRect(screenMatrix, obj->x+((params->box.width-(obj->explosionFrame*(params->box.width/10)))/2),
             obj->y, obj->explosionFrame*(params->box.width/10), params->box.height, 0x000000);*/
        break;
    case 3:
        //right shoot
        //FillRect(screenMatrix, (obj->x + params->box.width) - (obj->explosionFrame * (params->box.width / 10)), 
        //    (obj->y + params->box.height) - (obj->explosionFrame * (params->box.height / 10)), 
        //    obj->explosionFrame * (params->box.width / 10 ), 
        //    obj->explosionFrame * (params->box.height / 10), 0x000000);
        break;
    }


    obj->explosionFrame++;
}

void BoxThread(ThreadParams* params) {
    while (params->isGameRunning()) {
        if (!params->box.isAlive) {
            params->box.x = rand() % 580;
            params->box.y = 0;
            params->box.isAlive = true;
            params->box.explosionType = 0;
            params->box.explosionFrame = 0;
        }

        if (params->box.explosionType > 0) {
            Sleep(10);
            DrawExplosion(&params->box, params);
        }
        else {
            // Delete old position
            FillRect(screenMatrix, params->box.x, params->box.y,
                params->box.width, params->box.height, 0);

            // Move box
            params->box.y += 4;

            if (params->box.y > 600 ||
                (params->box.y + params->box.height >= params->ship.y &&
                    params->box.x + params->box.width >= params->ship.x &&
                    params->box.x <= params->ship.x + params->ship.width)) {
                params->isGameRunning() = false;
                params->box.isAlive = false;
                continue;
            }

            // Print new position of box
            FillRect(screenMatrix, params->box.x, params->box.y,
                params->box.width, params->box.height, 0x00FF00);
        }

        Sleep(10);
    }
}

void BulletThread(ThreadParams* params) {
    int score = 0;
    while (params->isGameRunning()) {
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
            if (params->box.isAlive &&
                params->bullet.y <= params->box.y + params->box.height &&
                params->bullet.y + params->bullet.height >= params->box.y &&
                params->bullet.x + params->bullet.width >= params->box.x &&
                params->bullet.x <= params->box.x + params->box.width) {

                int hitX = params->bullet.x - params->box.x;
                score++;
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

void StartGame(void* gameRunning) {
    bool* gameRunningPtr = (bool*)gameRunning;

    if (*gameRunningPtr) return;

    *gameRunningPtr = true;

    // Reset the screen
    screenMatrix = 0;

    // Define ThreadParams
    ThreadParams* params = new ThreadParams{
        {300, 580, 40, 10, true, 0, 0},        // ship

        //First requriment
        /*
        1. The falling boxes should each be 10*k pixels wide such as 10,20,50... The width is up to you.
        box size is 40x40  = 10*k x 10*k , k = 4   
        */
        {rand() % 580, 0, 40, 40, true, 0, 0}, // box

        {0, 0, 2, 10, false, 0, 0},            // bullet
        ICG_FrameMedium(5, 40, 700, 700),      // frm1
        gameRunningPtr                        // gameRunning durumu aktarıldı
    };

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnimationThread, params, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShipThread, params, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BoxThread, params, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BulletThread, params, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ScoreThread, params, 0, NULL);

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

    static bool gameRunning = false;
    ICG_Button(5, 5, 120, 25, "START GAME", StartGame, &gameRunning);

    ICG_SetOnKeyPressed(WhenKeyPressed);
}
