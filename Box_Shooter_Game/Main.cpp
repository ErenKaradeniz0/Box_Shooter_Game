#include "icb_gui.h"

// Globals
/*
5. Note that the use of global variables is restricted, except for the designated ICBYTES
object and the keypressed variable.
*/
int keypressed;
ICBYTES screenMatrix;

// Create window
void ICGUI_Create() {
    ICG_MWTitle("Space Shooter");
    ICG_MWSize(540, 600);
}

void DrawStartupAndTransition(int* data) {
    
    ICBYTES startScreen;
    CreateImage(startScreen, 500, 500, ICB_UINT);
    int* gameRunning = &data[23];
    int* f1Frame = &data[21];

    // Startup Animation
    for (int frame = 0; frame < 120 && *gameRunning; frame++) {
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

        if (!*gameRunning) return;
        DisplayImage(*f1Frame, startScreen); // Assuming gameData[0] stores FRM1 reference
        Sleep(33);
    }

    // Transition Animation
    for (int frame = 0; frame < 30 && *gameRunning; frame++) {
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

        if (!*gameRunning) return;
        DisplayImage(*f1Frame, startScreen); // Assuming gameData[0] stores FRM1 reference
        Sleep(16);
    }

    if (!*gameRunning) return;

    // Final flash
    startScreen = 0xFFFFFF;
    DisplayImage(*f1Frame, startScreen); // Assuming gameData[0] stores FRM1 reference
    Sleep(50);
    startScreen = 0x000000;
    DisplayImage(*f1Frame, startScreen); // Assuming gameData[0] stores FRM1 reference
}

void DrawExplosion(int* data) {
    int* boxX = &data[5];
    int* boxY = &data[6];
    int* boxWidth = &data[7];
    int* boxHeight = &data[8];
    int* boxIsAlive = &data[9];

    int* boxExplosionType = &data[10];
    int* boxExplosionFrame = &data[11];


    if ((*boxExplosionFrame > 10 && *boxExplosionType == 2) ||
        *boxX <= 0 - *boxHeight || *boxX >= 600 ||
        *boxY < 40 - *boxHeight) {

        *boxIsAlive = false;
        *boxExplosionType = 0;
        *boxExplosionFrame = 0;
        FillRect(screenMatrix, *boxX, *boxY, *boxWidth, *boxHeight, 0x000000); // Patlama alanını temizle
        FillRect(screenMatrix, 0, 0, 500, 40, 0x333333); // Ekranın üst kısmını temizle
        return;
    }

    int color = 0xFFFF00; // Patlama rengi sarı
    FillRect(screenMatrix, *boxX, *boxY, *boxWidth, *boxHeight, color);

    // Patlama Merkezi (Explosion Middle) - Siyah bir boşluk oluşturuyor
    if (*boxExplosionType == 2) {
        // Patlama genişliği, mevcut kareye göre merkezden itibaren küçülüyor
        FillRect(screenMatrix, *boxX + ((*boxWidth - (*boxExplosionFrame * (*boxWidth / 10))) / 2),
            *boxY, *boxExplosionFrame * (*boxWidth / 10), *boxHeight, 0x000000);
        FillRect(screenMatrix, *boxX, *boxY + ((*boxHeight - (*boxExplosionFrame * (*boxWidth / 10))) / 2),
            *boxWidth, *boxExplosionFrame * (*boxHeight / 10), 0x000000);
    }

    // Patlama hareketi - Explosion type'a göre farklı hareket sağlıyor
    switch (*boxExplosionType) {
    case 1:
        *boxX += 8; // Sağa ve yukarıya hareket
        *boxY -= 8;
        break;
    case 2:
        *boxY -= 8; // Yukarıya hareket
        break;
    case 3:
        *boxX -= 8; // Sola ve yukarıya hareket
        *boxY -= 8;
        break;
    }
    (*boxExplosionFrame)++; // Patlama karesi arttırılır

    FillRect(screenMatrix, 0, 0, 500, 40, 0x333333); // Üst bölümü temizle
}

void ShipThread(int* data) {
    int* shipX = &data[0];
    int* shipY = &data[1];
    int* gameRunning = &data[23];



    while (*gameRunning) {

        // Move Ship
        if (keypressed == 37)
        {
            *shipX = max(0, *shipX - 10);
            keypressed = 0;
        }
        if (keypressed == 39)
        {
            *shipX = min(460, *shipX + 10);
            keypressed = 0;
        }
        Sleep(1);
    }
}

void BoxThread(int* data) {
    int* shipX = &data[0];
    int* shipY = &data[1];
    int* shipWidth = &data[2];

    int* boxX = &data[5];
    int* boxY = &data[6];
    int* boxWidth = &data[7];
    int* boxHeight = &data[8];
    int* boxIsAlive = &data[9];

    int* boxExplosionType = &data[10];
    int* boxExplosionFrame = &data[11];

    int* lifeCount = &data[17];

    int* gameRunning = &data[23];



    while (*gameRunning) {
        if (!*boxIsAlive && *boxExplosionType == 0) {
            // Respawn box
            *boxX = rand() % 430;
            *boxY = 40;
            *boxIsAlive = true;
            *boxExplosionType = 0;
            *boxExplosionFrame = 0;
        }
        // Move box
        else if (*boxIsAlive) {
            *boxY += 4;
            // Collision to ship or cross to border
            if (*boxY > *shipY ||
                (*boxY + *boxHeight >= *shipY &&
                    *boxX + *boxWidth >= *shipX &&
                    *boxX <= *shipX + *shipWidth)) {
                 (*lifeCount)--;
                PlaySound("sound/crash.wav", NULL, SND_ASYNC);
                *boxIsAlive = false;
                if (*lifeCount == 0)
                    *gameRunning = 0;
            }
        }
        Sleep(30);
    }
}

void BulletThread(int* data) {
    int* shipX = &data[0];
    int* shipY = &data[1];
    int* shipWidth = &data[2];

    int* boxX = &data[5];
    int* boxY = &data[6];
    int* boxWidth = &data[7];
    int* boxHeight = &data[8];
    int* boxIsAlive = &data[9];

    int* boxExplosionType = &data[10];

    int* bulletX = &data[12];
    int* bulletY = &data[13];
    int* bulletWidth = &data[14];
    int* bulletHeight = &data[15];
    int* bulletIsAlive = &data[16];

    int* score = &data[22];
    int* gameRunning = &data[23];

    while (*gameRunning) {
        // Create Bullet
        if (keypressed == 32 && !*bulletIsAlive) {
            PlaySound("sound/shoot.wav", NULL, SND_ASYNC);
            *bulletX = *shipX + (*shipWidth / 2) - (*bulletWidth / 2);
            *bulletY = *shipY - *bulletHeight;
            *bulletIsAlive = true;
            keypressed = 0;
        }
        // Move Bullet
        if (*bulletIsAlive) {
            *bulletY-= 18;
            if (*boxIsAlive &&
                *bulletY <= *boxY + *boxHeight &&
                *bulletY + *bulletHeight >= *boxY &&
                *bulletX + *bulletWidth >= *boxX &&
                *bulletX <= *boxX + *boxWidth) {


                int hitX = *bulletX - *boxX;
                /*
                2. When a bullet strikes the left side of a box, specifically within the leftmost 3-unit
                region, the box will move to the top-right corner.
                */

                int leftPart = *boxWidth / 3 - 1;

                /*
                4. A bullet that hits the central 4-unit region of the box will result in the box being
                destroyed.
                */
                int middlePart = *boxWidth - leftPart;
                if (hitX <= leftPart) { //3*x*k
                    PlaySound("sound/miss.wav", NULL, SND_ASYNC);
                    *boxExplosionType = 1;
                }
                else if (hitX <= middlePart) { //4*x*k
                    PlaySound("sound/explosion.wav", NULL, SND_ASYNC);
                    *boxExplosionType = 2;
                    (*score)++;
                    if (*score > 9) {
                        *gameRunning = 0;
                    }
                }
                /*
                3. If a bullet hits the right side of the box, within the rightmost 3-unit region, the box
                will instead move to the top-left corner.
                */
                else {  //3*x*k
                    PlaySound("sound/miss.wav", NULL, SND_ASYNC);
                    *boxExplosionType = 3;
                }
                *boxIsAlive = false;
                *bulletIsAlive = false;
            }
            // Screen overflow check
            if (*bulletY < 40) {
                *bulletIsAlive = false;
            }
        }
        Sleep(30);
        FillRect(screenMatrix, 0, 0, 500, 40, 0x333333);
    }
}

void DrawThread(LPVOID lpParam) {
    int* data = (int*)lpParam;

    int* shipX = &data[0];
    int* shipY = &data[1];
    int* shipWidth = &data[2];
    int* shipHeight = &data[3];
    int* shipIsAlive = &data[4];

    int* boxX = &data[5];
    int* boxY = &data[6];
    int* boxWidth = &data[7];
    int* boxHeight = &data[8];
    int* boxIsAlive = &data[9];

    int* boxExplosionType = &data[10];

    int* bulletX = &data[12];
    int* bulletY = &data[13];
    int* bulletWidth = &data[14];
    int* bulletHeight = &data[15];
    int* bulletIsAlive = &data[16];

    int* lifeCount = &data[17];
    int* lifeX = &data[18];
    int* lifeY = &data[19];
    int* lifeSize = &data[20];

    int* frame = &data[21];
    int* score = &data[22];
    int* gameRunning = &data[23];


    ICG_SetFont(50, 0, "Arial");
    char scoreText[9] = "Score:  ";

    // Intro animation
    PlaySound("sound/intro.wav", NULL, SND_ASYNC);
    DrawStartupAndTransition(data);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShipThread, data, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BoxThread, data, 0, NULL);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BulletThread, data, 0, NULL);

    while (*gameRunning) {
        // Clear screen
        screenMatrix = 0;

        // GreyLine
        FillRect(screenMatrix, 0, 0, 500, 40, 0x333333);

        // Draw ship
        if (*shipIsAlive) {
            FillRect(screenMatrix, *shipX, *shipY, *shipWidth, *shipHeight, 0xFF0000);
        }

        // Draw box or explosion
        if (*boxIsAlive) {
            FillRect(screenMatrix, *boxX, *boxY, *boxWidth, *boxHeight, 0x00FF00);
        }
        else if (*boxExplosionType > 0) {
            DrawExplosion(data); // box verilerini patlama çizim fonksiyonuna gönder
        }

        // Draw bullet
        if (*bulletIsAlive) {
            FillRect(screenMatrix, *bulletX, *bulletY, *bulletWidth, *bulletHeight, 0x0000FF);
        }

        // Draw hearts (life count)
        int h_x = *lifeX;
        int h_y = *lifeY;
        int h_size = *lifeSize;

        for (int i = 1; i <= *lifeCount; i++) {
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
        }

        // Update score
        scoreText[6] = '0' + *score;
        Impress12x20(screenMatrix, 400, 13, scoreText, 0xFFFFFF);

        DisplayImage(*frame, screenMatrix);
        Sleep(30);
    }

    // Display game over or win screen
    Sleep(50);
    if (*score > 9) {
        screenMatrix = 0x005500;
        Impress12x20(screenMatrix, 200, 250, "You win!", 0xFFFFFF);
        DisplayImage(*frame, screenMatrix);
    }
    else {
        PlaySound("sound/death.wav", NULL, SND_ASYNC);

        // delete objects
        *boxIsAlive = false;
        *shipIsAlive = false;
        *bulletIsAlive = false;

        // GAME OVER Screen
        screenMatrix = 0x000055;
        Impress12x20(screenMatrix, 200, 250, "GAME OVER", 0xFFFFFF);
        Impress12x20(screenMatrix, 220, 275, scoreText, 0xFFFFFF);
        DisplayImage(*frame, screenMatrix);
    }
}

void StartGame(void* gameRunning) {
    
    SetFocus(ICG_GetMainWindow());
    // Reset the screen
    screenMatrix = 0;
    
    int* gameRunningPtr = (int*)gameRunning;
    if (*gameRunningPtr) return;
    *gameRunningPtr = 1;

    int gameData[24] = {
        250, 485, 40, 10, 1,                    // Ship verileri (x, y, width, height, isAlive)
        rand() % 460, 40, 40, 40, 1,            // Box verileri (x, y, width, height, isAlive)
        0,0,                                    // Box explosion type and frame        
        0, 0, 2, 10, 0,                         // Bullet verileri (x, y, width, height, isAlive)
        3, 10, 10, 3,                           // Heart verileri (count, x, y, size)
        ICG_FrameMedium(5, 40, 1, 1),                                      // Frame değeri
        0,                                      // Score
        *gameRunningPtr                         // Game Running
    };

    int* gameDataPtr = (int*)malloc(sizeof(int) * 24);
    memcpy(gameDataPtr, gameData, sizeof(int) * 24);

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DrawThread, (LPVOID)gameDataPtr, 0, NULL);
}

void WhenKeyPressed(int k) {
    keypressed = k;
}

void ICGUI_main() {
    CreateImage(screenMatrix, 500, 500, ICB_UINT);
    screenMatrix = 0;

    static int gameRunning = 0;
    ICG_Button(5, 5, 120, 25, "START GAME", StartGame, &gameRunning);
    ICG_SetOnKeyPressed(WhenKeyPressed);
}