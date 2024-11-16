#include "icb_gui.h"

// Globals
/*
5. Note that the use of global variables is restricted, except for the designated ICBYTES
object and the keypressed variable.
*/
int keypressed;
ICBYTES screenMatrix;

HANDLE HMutex;


struct GameObject {
    int x, y;
    int width, height;
    bool isAlive;
    int explosionType;
    int explosionFrame;
};

struct Heart {
    int count;
    int x, y;
    int size;
    //int* arr = new int[count];
};

struct ThreadParams {
    GameObject ship;
    GameObject box;
    GameObject bullet;
    Heart life;
    int FRM1;
    int score;
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

void DrawStartupAndTransition(ThreadParams* params) {
    ICBYTES startScreen;
    CreateImage(startScreen, 500, 500, ICB_UINT);

    // Startup Animation
    for (int frame = 0; frame < 120 && params->isGameRunning(); frame++) {
        startScreen = 0x000000; // Clear screen with black

        // ESD Studios Logo Animation - Matrix effect
        if (frame < 40) {
            ICG_SetFont(35 + frame / 4, 0, "Arial");
            // Create matrix-like random characters behind logo
            for (int i = 0; i < 10; i++) {
                int x = rand() % 450;
                int y = rand() % 450;
                int green = rand() % 155 + 100;
                Impress12x20(startScreen, x, y, ".", green << 8);
            }

            int alpha = (frame * 6) > 255 ? 255 : (frame * 6);
            int color = alpha | (alpha << 8) | (alpha << 16);

            // Draw ESD with glowing effect
            if (frame > 20) {
                ICG_SetFont(45, 0, "Arial Bold");
                Impress12x20(startScreen, 178, 178, "ESD", 0x00FF00);
                Impress12x20(startScreen, 180, 180, "ESD", color);
            }
            // Draw Studios with delay
            if (frame > 30) {
                ICG_SetFont(45, 0, "Arial");
                Impress12x20(startScreen, 278, 180, "Studios", color);
            }
        }
        else {
            ICG_SetFont(45, 0, "Arial Bold");
            Impress12x20(startScreen, 178, 178, "ESD", 0x00FF00);
            Impress12x20(startScreen, 180, 180, "ESD", 0xFFFFFF);
            ICG_SetFont(45, 0, "Arial");
            Impress12x20(startScreen, 278, 180, "Studios", 0xFFFFFF);
        }

        // Game Title Animation
        if (frame >= 40) {
            int titleFrame = frame - 40;

            // Draw shooting lines
            if (titleFrame < 20) {
                for (int i = 0; i < titleFrame; i++) {
                    int y = 250 - i * 10;
                    FillRect(startScreen, 160 + i * 15, y, 2, 20, 0xFF0000);
                }
            }

            // Draw Box Shooter text with glowing effect
            if (titleFrame >= 20) {
                ICG_SetFont(40, 0, "Arial Bold");
                int yOffset = (titleFrame % 16 > 8) ? 2 : -2; // Subtle floating effect

                // Shadow/Glow effect
                Impress12x20(startScreen, 158, 248, "BOX", 0x800000);
                Impress12x20(startScreen, 158, 248 + yOffset, "BOX", 0xFF0000);

                Impress12x20(startScreen, 278, 248, "SHOOTER", 0x800000);
                Impress12x20(startScreen, 278, 248 + yOffset, "SHOOTER", 0xFF0000);
            }
        }

        if (!params->isGameRunning()) return;
        DisplayImage(params->FRM1, startScreen);
        Sleep(33);
    }

    // Transition Animation
    for (int frame = 0; frame < 30 && params->isGameRunning(); frame++) {
        startScreen = 0x000000;

        // Create multiple rectangular wipes
        for (int i = 0; i < 5; i++) {
            int offset = i * 100;
            int width = frame * 20;

            // Top to bottom wipe
            FillRect(startScreen, offset, 0, 100, width, 0xFF0000);
            // Bottom to top wipe
            FillRect(startScreen, offset, 500 - width, 100, width, 0x0000FF);
        }

        // Add some particle effects
        for (int i = 0; i < 20; i++) {
            int x = rand() % 500;
            int y = rand() % 500;
            int size = rand() % 4 + 1;
            int color = rand() % 2 == 0 ? 0xFF0000 : 0x0000FF;
            FillRect(startScreen, x, y, size, size, color);
        }

        if (!params->isGameRunning()) return;
        DisplayImage(params->FRM1, startScreen);
        Sleep(16);
    }

    if (!params->isGameRunning()) return;

    // Final flash
    startScreen = 0xFFFFFF;
    DisplayImage(params->FRM1, startScreen);
    Sleep(50);
    startScreen = 0x000000;
    DisplayImage(params->FRM1, startScreen);
}

void DrawExplosion(GameObject* obj, ThreadParams* params) {
    //Draw Explosion

    if ((obj->explosionFrame > 10 && obj->explosionType == 2) || obj->x <= 0 - obj->height || obj->x >= 600 || obj->y < 40 - obj->height) {
        obj->isAlive = false;
        obj->explosionType = 0;
        obj->explosionFrame = 0;
        FillRect(screenMatrix, obj->x, obj->y, params->box.width, params->box.height, 0x000000); //Clear Explosion
        FillRect(screenMatrix, 0, 0, 500, 40, 0x333333);
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

    FillRect(screenMatrix, 0, 0, 500, 40, 0x333333);


}
void ShipThread(ThreadParams* params) {
    while (params->isGameRunning()) {
        // Move Ship
        if (keypressed == 37)
        {
            params->ship.x = max(0, params->ship.x - 8);
            keypressed = 0;
        }
        if (keypressed == 39)
        {
            params->ship.x = min(460, params->ship.x + 8);
            keypressed = 0;
        }
        Sleep(1);
    }
}

void BoxThread(ThreadParams* params) {
    DWORD dwWaitResult;
    while (params->isGameRunning()) {
        if (!params->box.isAlive && params->box.explosionType == 0) {
            // Respawn box
            params->box.x = rand() % 430;
            params->box.y = 40;
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
                params->life.count--;
                PlaySound("sound/crash.wav", NULL, SND_ASYNC);
                params->box.isAlive = false;
                if (params->life.count == 0)
                    *(params->gameRunning) = false;

            }
        }
        Sleep(30);
    }
}


void* FireKeyPressed(ThreadParams* params) {
    DWORD dwWaitResult;
    while (1)
    {
        if (keypressed == 32) {
            dwWaitResult = WaitForSingleObject(HMutex, 100);
            if (dwWaitResult == WAIT_OBJECT_0)
            {
                PlaySound("sound/shoot.wav", NULL, SND_ASYNC);
                params->bullet.x = params->ship.x + (params->ship.width / 2) - (params->bullet.width / 2);
                params->bullet.y = params->ship.y - params->bullet.height;
                params->bullet.isAlive = true;
                keypressed = 0;
                ReleaseMutex(HMutex);
            }
        };

    }
    return 0;
}

void* BulletThread(ThreadParams* params) {
    while (params->isGameRunning()) {
        // Create Bullet
       // if (keypressed == 32 && !params->bullet.isAlive) {

        bool mutexTaken = false;
        // Move Bullet
        while (1)
        {
            if (!params->bullet.isAlive)
            {
                if(mutexTaken)
                    mutexTaken = ReleaseMutex(HMutex) == FALSE;
                continue;
            }

            if (!mutexTaken)
            {
                WaitForSingleObject(HMutex, INFINITE);
                mutexTaken = true;
            }

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
                if (hitX <= leftPart) { //3*x*k
                    PlaySound("sound/miss.wav", NULL, SND_ASYNC);
                    params->box.explosionType = 1;
                }
                else if (hitX <= middlePart) { //4*x*k
                    PlaySound("sound/explosion.wav", NULL, SND_ASYNC);
                    params->box.explosionType = 2;
                    params->score++;
                    if (params->score > 9) {
                        *(params->gameRunning) = false;
                    }
                }
                /*
                3. If a bullet hits the right side of the box, within the rightmost 3-unit region, the box
                will instead move to the top-left corner.
                */
                else {  //3*x*k
                    PlaySound("sound/miss.wav", NULL, SND_ASYNC);
                    params->box.explosionType = 3;
                }

                params->box.isAlive = false;
                params->bullet.isAlive = false;




            }
            // Screen overflow check
            if (params->bullet.y < 40) {

                params->bullet.isAlive = false;

            }

            Sleep(30);
            FillRect(screenMatrix, 0, 0, 500, 40, 0x333333);
           
            // Sleep(10);
        }

    }
    return 0;
}


// Drawing Thread
void DrawThread(ThreadParams* params) {

    ICG_SetFont(50, 0, "Arial");
    char score[9] = "Score:  ";

    //Intro animation
   PlaySound("sound/intro.wav", NULL, SND_ASYNC);
   DrawStartupAndTransition(params);

    DWORD dw;
    HMutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);    //it doesn't have a name LPCTSTR lpName



    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShipThread, params, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BoxThread, params, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FireKeyPressed, params, 0, &dw);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BulletThread, params, 0, &dw);



    while (params->isGameRunning()) {
        // Clear screen
        screenMatrix = 0;

        //GreyLine
        FillRect(screenMatrix, 0, 0, 500, 40, 0x333333);
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
        int h_x = params->life.x;
        int h_y = params->life.y;
        int h_size = params->life.size;

        //params->life.arr[0] = h_x;

        for (int i = 1; i <= params->life.count; i++) {
            FillRect(screenMatrix, h_x, h_y + h_size, h_size, h_size * 3, 0XFF0000);
            FillRect(screenMatrix, h_x + h_size, h_y, h_size, h_size * 5, 0XFF0000);
            FillRect(screenMatrix, h_x + h_size * 2, h_y, h_size, h_size * 6, 0XFF0000);
            FillRect(screenMatrix, h_x + h_size * 3, h_y + h_size, h_size, h_size * 6, 0XFF0000);
            FillRect(screenMatrix, h_x + h_size * 4, h_y + h_size * 2, h_size, h_size * 6, 0XFF0000);
            FillRect(screenMatrix, h_x + h_size * 5, h_y + h_size, h_size, h_size * 6, 0XFF0000);
            FillRect(screenMatrix, h_x + h_size * 6, h_y, h_size, h_size * 6, 0XFF0000);
            FillRect(screenMatrix, h_x + h_size * 7, h_y, h_size, h_size * 5, 0XFF0000);
            FillRect(screenMatrix, h_x + h_size * 8, h_y + h_size, h_size, h_size * 3, 0XFF0000);
            h_x += h_size * 9 + 5;
            //params->life.arr[i] = h_x;
        }




        score[6] = '0' + params->score;
        Impress12x20(screenMatrix, 400, 13, score, 0xFFFFFF);

        DisplayImage(params->FRM1, screenMatrix);
        Sleep(30);
    }

    Sleep(50);
    if (params->score > 9) {
        screenMatrix = 0x005500;
        Impress12x20(screenMatrix, 200, 250, "You win!", 0xFFFFFF);
        DisplayImage(params->FRM1, screenMatrix);

    }
    else {

        PlaySound("sound/death.wav", NULL, SND_ASYNC);

        //delete objects
        params->box.isAlive = false;
        params->ship.isAlive = false;
        params->bullet.isAlive = false;

        // GAME OVER Screen
        screenMatrix = 0x000055;
        Impress12x20(screenMatrix, 200, 250, "GAME OVER", 0xFFFFFF);
        Impress12x20(screenMatrix, 220, 275, score, 0xFFFFFF);
        DisplayImage(params->FRM1, screenMatrix);
    }
}

void StartGame(void* gameRunning) {
    SetFocus(ICG_GetMainWindow());
    bool* gameRunningPtr = (bool*)gameRunning;
    if (*gameRunningPtr) return;

    *gameRunningPtr = true;

    // Reset the screen
    screenMatrix = 0;

    // Define ThreadParams
    int gameScreenX = 500, GameScreenY = 500;
    int shipX = 250, shipY = 485;
    int boxSize = 40, boxY = 40;
    int bulletWidth = 2, bulletHeight = 10;
    int life = 3;
    int score = 0;


    ThreadParams* params = new ThreadParams{
        {shipX, shipY, 40, 10, true, 0, 0},                                         //ship
        {rand() % (gameScreenX - boxSize) , boxY, boxSize, boxSize, true, 0, 0},    //box
        {0, 0, bulletWidth, bulletHeight, false, 0, 0},                             //bullet
        {life,10,10,3},                                                                //heart
        ICG_FrameMedium(5, 40, 1, 1),                                               //frame
        score,                                                                      //score
        gameRunningPtr                                                              //gameRunning
    };

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DrawThread, params, 0, NULL);
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