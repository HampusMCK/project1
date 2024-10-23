#include "raylib.h"
#include "raymath.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum
{
    gs_start,
    gs_shop,
    gs_aiming,
    gs_shooting,
    gs_game_over
} game_state;

typedef struct
{
    Vector2 position;
    Vector2 velocity;
    int radius;
    bool shooting;
    int id;
} Ball;

typedef struct
{
    int ID;
    int hits;
    Vector2 size;
    int position;
    bool alive;
} Block;

typedef struct
{
    int price;
    bool bought;
    bool selected;
    Color color;
} Ball_Custom;

long oldseed;
FILE *file;

int get_rand(int min, int max) // Returns random integer between min and max
{
    time_t seed;
    seed = time(NULL) + oldseed;
    oldseed = seed;

    srand(seed);

    int i = rand() % max;

    if (i < min)
        i = min;

    return i;
}

Vector2 shoot(int startx, int starty, Vector2 mousepos, float speed) // Returns a vector2 pointing towards the mouse position
{
    Vector2 start = {startx, starty};
    Vector2 dir = Vector2Subtract(mousepos, start);
    dir = Vector2Scale(Vector2Normalize(dir), speed);
    return dir;
}

void spawn_blocks(int block_pos[], Block blocks[], Rectangle recs[], int level, Vector2 screensize) // Places blocks on game screen
{
    if (level > 1)
    {
        for (int i = 0; i < 100; i++)
        {
            if (blocks[i].alive)
            {
                recs[i].y += screensize.x / 15;
            }
        }
        int count = 0;
        for (int i = 0; i < 100; i++)
        {
            if (!blocks[i].alive)
            {
                int type = get_rand(0, 3);
                switch (type)
                {
                case 0:

                    break;
                case 1:
                    blocks[i].hits = get_rand(level, level * 2);
                    blocks[i].position = i;
                    blocks[i].size = (Vector2){screensize.x / 15, screensize.x / 15};
                    blocks[i].alive = true;
                    blocks[i].ID = type;

                    recs[i] = (Rectangle){block_pos[i], 10, blocks[i].size.x, blocks[i].size.y};
                    break;
                case 2:
                    break;

                default:
                    break;
                }
                count++;
            }
            if (count == 15)
                break;
        }
    }
    else
    {
        for (int i = 0; i < 15; i++)
        {
            int type = get_rand(0, 2);
            switch (type)
            {
            case 0:

                break;
            case 1:
                blocks[i].hits = get_rand(level, level * 2);
                blocks[i].position = i;
                blocks[i].size = (Vector2){screensize.x / 15, screensize.x / 15};
                blocks[i].alive = true;

                recs[i] = (Rectangle){block_pos[i], 10, blocks[i].size.x, blocks[i].size.y};
                break;
            case 2:

                break;

            default:
                break;
            }
        }
    }
}

int main()
{
    //-----------Screen Creation------------
    InitWindow(800, 800, "Ballz");
    Vector2 screenSize = {GetMonitorWidth(0) - (GetMonitorWidth(0) / 10), GetMonitorHeight(0) - (GetMonitorHeight(0) / 10)};
    SetWindowSize(screenSize.x, screenSize.y);
    SetWindowPosition(GetMonitorWidth(0) / 20, GetMonitorHeight(0) / 20);
    SetTargetFPS(60);
    //--------------------------------------

    game_state gs = gs_start;

    //------Block Data-------
    int block_pos[1024];
    Block blocks[1024];
    Rectangle recs[1024];
    //-----------------------

    //--------Ball Data------
    Ball balls[1024];

    int startx = screenSize.x / 2;
    int ball_amount = 3;

    //----Shooting Data----
    float last_shot = 5;
    float ball_speed = 1000;
    int balls_shot = 0;
    int balls_landed = 0;
    int shotx = 0;
    Vector2 mouse_clicked;
    //---------------------
    //-----------------------

    //--------Save Data-------
    int highscore;
    int start_highscore_fontSize = 45;
    bool increase_fontSize = true;
    int coins = 0;
    file = fopen("highscore.txt", "r");
    fscanf(file, "%d", &highscore);
    fclose(file);
    file = fopen("coins.txt", "r");
    fscanf(file, "%d", &coins);
    fclose(file);
    int level = 1;
    //------------------------

    //------------Buttons--------
    Rectangle start_button = {screenSize.x / 2 - 100, screenSize.y / 2 - 50, 200, 100};
    Rectangle shop_button = {screenSize.x / 2 - 100, screenSize.y / 2 + 100, 200, 100};

    Rectangle restart_button = {screenSize.x / 2 - 100, screenSize.y / 2 - 50, 200, 100};

    //---------Shop Buttons--------
    Rectangle s_butt[4] = {
        {300, screenSize.y / 2 - 50, 200, 100},
        {550, screenSize.y / 2 - 50, 200, 100},
        {800, screenSize.y / 2 - 50, 200, 100},
        {1050, screenSize.y / 2 - 50, 200, 100}};

    Ball_Custom design[4] = {
        {.bought = true, .price = 0, .selected = true, .color = RED},
        {.bought = false, .price = 20, .selected = false, .color = GREEN},
        {.bought = false, .price = 50, .selected = false, .color = PURPLE},
        {.bought = false, .price = 100, .selected = false, .color = YELLOW}};

    Rectangle exit_s = {screenSize.x / 2 - 100, screenSize.y - (screenSize.y / 4), 200, 100};
    //-----------------------------
    //---------------------------

    for (int i = 0; i < 1024; i++) // Create all balls
    {
        balls[i].position = (Vector2){startx, screenSize.y - 100};
        balls[i].radius = 20;
        balls[i].id = i;
    }

    for (int i = 0; i < 15; i++) // Create all 15 possible position with an ID 1-15
    {
        block_pos[i] = (screenSize.x / 15) * i;
    }

    spawn_blocks(block_pos, blocks, recs, level, screenSize); // Spawn first blocks before game begins

    while (!WindowShouldClose())
    {
        //-----------------Logic---------------------
        Vector2 mousepos = GetMousePosition();

        last_shot += GetFrameTime(); // Shot timer

        for (int i = 0; i < 100; i++) // Check in on blocks
        {
            if (blocks[i].hits < 1) // If hits < 0, then delete block
            {
                blocks[i].alive = false;
                recs[i].height = 0;
                recs[i].width = 0;
                recs[i].x = 0;
                recs[i].y = 0;
            }
            if (blocks[i].alive && recs[i].y >= (screenSize.y / 15) * 11) // If any block reaches the bottom of the screen, game over
                gs = gs_game_over;
        }

        if (gs == gs_start)
        {
            if (increase_fontSize)
            {
                start_highscore_fontSize++;
                if (start_highscore_fontSize > 70)
                    increase_fontSize = false;
            }
            else
            {
                start_highscore_fontSize--;
                if (start_highscore_fontSize < 45)
                    increase_fontSize = true;
            }
        }

        if (gs == gs_shop)
        {
        }

        if (gs == gs_aiming)
        {
            if (IsMouseButtonPressed(0))
            {
                gs = gs_shooting;
                mouse_clicked = mousepos;
                shotx = startx;
            }
        }

        if (gs == gs_shooting)
        {
            if (last_shot > 0.1f && balls_shot < ball_amount) // Shoots balls every 0.1 seconds and stops when all balls are shot
            {
                balls[balls_shot].velocity = shoot(shotx, screenSize.y - 100, mouse_clicked, ball_speed);
                balls[balls_shot].shooting = true;
                balls_shot++;
                last_shot = 0;
            }

            for (int i = 0; i < ball_amount; i++) // Check in on balls while moving
            {
                if (balls[i].shooting)
                    balls[i].position = Vector2Add(balls[i].position, Vector2Scale(balls[i].velocity, GetFrameTime())); // Move balls

                for (int x = 0; x < 100; x++)
                {
                    if (CheckCollisionCircleRec(balls[i].position, balls[i].radius, recs[x])) // Check collision on the balls with the blocks
                    {
                        if (balls[i].position.y > recs[x].y - balls[i].radius && balls[i].position.y < recs[x].y + recs[x].height + balls[i].radius && (balls[i].position.x > recs[x].x + recs[x].width || balls[i].position.x < recs[x].x))
                            balls[i].velocity.x *= -1;
                        if (balls[i].position.x > recs[x].x - balls[i].radius && balls[i].position.x < recs[x].x + recs[x].width + balls[i].radius && (balls[i].position.y > recs[x].y + recs[x].height || balls[i].position.y < recs[x].y))
                            balls[i].velocity.y *= -1;

                        blocks[x].hits--; // If collision is registered, minus one on the block that was hit
                    }
                }

                //----------Collision with walls---------------
                if (balls[i].position.x >= screenSize.x - 20)
                    balls[i].velocity.x *= -1;
                if (balls[i].position.x <= 20)
                    balls[i].velocity.x *= -1;
                if (balls[i].position.y <= 20)
                    balls[i].velocity.y *= -1;
                //---------------------------------------------
                if (balls[i].position.y > screenSize.y - 100 && balls[i].shooting) // If a ball hits ground again, stop moving
                {
                    if (balls_landed == 0) // If it's the first ball to land, save x-position for next round
                    {
                        balls[i].position.y = screenSize.y - 100;
                        startx = balls[i].position.x;
                        balls[i].shooting = false;
                        balls_landed++;
                    }
                    else // If it's not first ball, set x-position to startx
                    {
                        balls[i].position.x = startx;
                        balls[i].position.y = screenSize.y - 100;
                        balls[i].shooting = false;
                        balls_landed++;
                    }
                }
            }

            if (balls_landed == ball_amount) // If all balls landed, reset data, next level, change game state to 'gs_aiming', increase ball amount, new ball x-position = startx, spawn new blocks
            {
                balls_landed = 0;
                balls_shot = 0;
                gs = gs_aiming;
                level++;
                coins++;
                ball_amount++;
                balls[ball_amount - 1].position.x = startx;
                spawn_blocks(block_pos, blocks, recs, level, screenSize);
            }
        }
        //-------------------------------------------

        //------------------Graphic------------------
        BeginDrawing();

        if (gs == gs_start)
        {
            ClearBackground(BLUE);

            DrawText(TextFormat("HIGHSCORE: %d", highscore), screenSize.x / 2 - (MeasureText(TextFormat("HIGHSCORE: %d", highscore), start_highscore_fontSize) / 2), 60, start_highscore_fontSize, RED);
            DrawText(TextFormat("Coins: %d", coins), screenSize.x / 2 - (MeasureText(TextFormat("Coins: %d", coins), 50) / 2), 120, 50, RED);

            DrawRectangleRec(start_button, RED);
            DrawRectangleLinesEx(start_button, 2, BLACK);
            DrawText("START", (start_button.x + (start_button.width / 2)) - (MeasureText("START", 20) / 2), (start_button.y + (start_button.height / 2)) - 10, 20, WHITE);
            if (CheckCollisionPointRec(mousepos, start_button))
            {
                DrawRectangleRec(start_button, PINK);
                DrawRectangleLinesEx(start_button, 3, BLACK);
                DrawText("START", (start_button.x + (start_button.width / 2)) - (MeasureText("START", 20) / 2), (start_button.y + (start_button.height / 2)) - 10, 20, BLACK);
                if (IsMouseButtonPressed(0))
                {
                    gs = gs_aiming;
                }
            }

            DrawRectangleRec(shop_button, RED);
            DrawRectangleLinesEx(shop_button, 2, BLACK);
            DrawText("SHOP", (shop_button.x + (shop_button.width / 2)) - (MeasureText("SHOP", 20) / 2), (shop_button.y + (shop_button.height / 2)) - 10, 20, WHITE);
            if (CheckCollisionPointRec(mousepos, shop_button))
            {
                DrawRectangleRec(shop_button, PINK);
                DrawRectangleLinesEx(shop_button, 3, BLACK);
                DrawText("SHOP", (shop_button.x + (shop_button.width / 2)) - (MeasureText("SHOP", 20) / 2), (shop_button.y + (shop_button.height / 2)) - 10, 20, BLACK);
                if (IsMouseButtonPressed(0))
                    gs = gs_shop;
            }
        }

        if (gs == gs_shop)
        {
            ClearBackground(BLUE);
            DrawText(TextFormat("Coins: %d", coins), (screenSize.x / 2) - (MeasureText(TextFormat("Coins: %d", coins), 50) / 2), screenSize.y / 10, 50, BLACK);

            DrawRectangleRec(exit_s, LIGHTGRAY);
            DrawRectangleLinesEx(exit_s, 2, BLACK);
            DrawText("Exit!", (exit_s.x + (exit_s.width / 2)) - (MeasureText("Exit!", 24) / 2), exit_s.y + (exit_s.height / 2), 24, BLACK);
            if (CheckCollisionPointRec(mousepos, exit_s))
            {
                DrawRectangleRec(exit_s, GRAY);
                DrawRectangleLinesEx(exit_s, 3, BLACK);
                DrawText("Exit!", (exit_s.x + (exit_s.width / 2)) - (MeasureText("Exit!", 24) / 2), exit_s.y + (exit_s.height / 2), 24, RED);
                if (IsMouseButtonPressed(0))
                    gs = gs_start;
            }

            for (int i = 0; i < 4; i++)
            {
                DrawRectangleRec(s_butt[i], LIGHTGRAY);
                DrawRectangleLinesEx(s_butt[i], 2, BLACK);
                DrawCircle(s_butt[i].x + (s_butt->width / 2), s_butt[i].y + (s_butt[i].height / 2), 30, design[i].color);
                if (CheckCollisionPointRec(mousepos, s_butt[i]))
                {
                    DrawRectangleRec(s_butt[i], GRAY);
                    DrawRectangleLinesEx(s_butt[i], 3, BLACK);
                    DrawCircle(s_butt[i].x + (s_butt->width / 2), s_butt[i].y + (s_butt[i].height / 2), 30, design[i].color);
                    if (IsMouseButtonPressed(0))
                    {
                        if (design[i].bought)
                        {
                            for (int x = 0; x < 4; x++)
                                if (design[x].selected)
                                    design[x].selected = false;

                            design[i].selected = true;
                        }
                        if (!design[i].bought)
                        {
                            if (coins - design[i].price >= 0)
                            {
                                coins -= design[i].price;
                                design[i].bought = true;

                                for (int x = 0; x < 4; x++)
                                    if (design[x].selected)
                                        design[x].selected = false;

                                design[i].selected = true;
                            }
                        }
                    }
                }
                if (!design[i].bought)
                {
                    DrawText(TextFormat("Cost: %d coins", design[i].price), (s_butt[i].x + (s_butt[i].width / 2)) - (MeasureText(TextFormat("Cost: %d coins", design[i].price), 24) / 2), s_butt[i].y + s_butt[i].height, 24, BLACK);
                    DrawRectangleRec(s_butt[i], (Color){0, 0, 0, 127});
                }
                if (design[i].selected)
                {
                    DrawRectangleLinesEx(s_butt[i], 2, GREEN);
                }
            }
        }

        if (gs == gs_aiming || gs == gs_shooting)
        {
            ClearBackground(WHITE);
            Color ball_color = RED;
            for (int i = 0; i < 4; i++)
            {
                if (design[i].selected)
                    ball_color = design[i].color;
            }

            DrawRectangle(0, screenSize.y - 60, screenSize.x, 60, RED);
            DrawRectangleLines(0, screenSize.y - 60, screenSize.x, 60, BLACK);
            DrawText(TextFormat("Highscore: %d", highscore), 10, screenSize.y - 50, 40, BLUE);
            DrawText(TextFormat("Level: %d", level), screenSize.x - MeasureText(TextFormat("Level: %d", level), 40) - 5, screenSize.y - 50, 40, BLUE);

            DrawLine(startx, screenSize.y - 100, mousepos.x, mousepos.y, BLACK); // Aim line
            for (int i = 0; i < ball_amount; i++)
                DrawCircle(balls[i].position.x, balls[i].position.y, balls[i].radius, ball_color);

            for (int i = 0; i < 100; i++)
            {
                if (blocks[i].alive)
                {
                    DrawRectangleRec(recs[i], RED);
                    DrawRectangleLinesEx(recs[i], 1, BLACK);
                    DrawText(TextFormat("%d", blocks[i].hits), (recs[i].x + (recs[i].width / 2)) - (MeasureText(TextFormat("%d", blocks[i].hits), 35) / 2), recs[i].y + (recs[i].height / 2), 35, BLACK);
                }
            }
        }

        if (gs == gs_game_over)
        {
            ClearBackground(BLUE);

            DrawRectangleRec(restart_button, LIGHTGRAY);
            DrawRectangleLinesEx(restart_button, 2, BLACK);
            DrawText("Restart", (restart_button.x + (restart_button.width / 2)) - (MeasureText("Restart", 24) / 2), restart_button.y + 12, 24, BLACK);
            if (CheckCollisionPointRec(mousepos, restart_button))
            {
                DrawRectangleRec(restart_button, GRAY);
                DrawRectangleLinesEx(restart_button, 3, BLACK);
                DrawText("Restart", (restart_button.x + (restart_button.width / 2)) - (MeasureText("Restart", 24) / 2), restart_button.y + 12, 24, BLACK);
                if (IsMouseButtonPressed(0))
                {
                    level = 1;
                    ball_amount = 3;
                    for (int i = 0; i < 100; i++)
                    {
                        if (blocks[i].alive)
                        {
                            blocks[i].hits = 0;
                            blocks[i].alive = false;
                        }
                    }
                    gs = gs_start;
                }
            }
        }
        EndDrawing();
        //-------------------------------------------

        if (level > highscore) // Update highscore
            highscore = level;
    }

    file = fopen("highscore.txt", "w");
    fprintf(file, "%d", highscore);
    fclose(file);

    file = fopen("coins.txt", "w");
    fprintf(file, "%d", coins);
    fclose(file);

    return 0;
}