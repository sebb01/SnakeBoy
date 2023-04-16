#include <gb/gb.h>
#include <gb/hardware.h>
#include <rand.h>
#include <stdio.h>
#include <stdlib.h>
#include "Snake.c"
#include "time.h"
#include "BackgroundMap.c"
#include "BackgroundTiles.c"

#define SCREEN_WIDTH 20
#define SCREEN_HEIGHT 19
#define INIT_X 12 // Snake initial starting position
#define INIT_Y 12 // Snake initial starting position
#define SCALE 8   // Coordinates get multiplied by this amount
#define DELAY 150
#define FOOD_SPRITE 0
#define HEAD_SPRITE 1
#define BACKGROUND_TILE 7
#define SEGMENT_TILE 8
#define MUSIC_SPEED 5

typedef struct Segment Segment;

struct Segment
{
    Segment *child;
    UINT8 x;
    UINT8 y;
};

Segment *head = NULL;
int lastLatchedInput = J_RIGHT;
int latchedInput = J_RIGHT;
int nextLatchedInput = J_RIGHT;
UINT8 food_x;
UINT8 food_y;
UINT8 consumedFood = 0;
UINT8 loop_counter = SCALE;
UINT8 is_logic_frame = 1;

int opposite(int in)
{
    switch (in)
    {
    case J_LEFT:
        return J_RIGHT;
    case J_RIGHT:
        return J_LEFT;
    case J_UP:
        return J_DOWN;
    case J_DOWN:
        return J_UP;
    default:
        return NULL;
    }
}

Segment *findPenultimateSegment()
{
    Segment *segment = head;
    if (head->child != NULL)
    {
        while (segment->child->child != NULL)
        {
            segment = segment->child;
        }
    }

    return segment;
}

int numberOfSegments()
{
    Segment *segment = head;
    int count = 0;
    while (segment->child != NULL)
    {
        segment = segment->child;
        count++;
    }

    return count;
}

void showTitleScreen()
{
    printf("\n\n\n\n\n\n\n      SnakeBoy\n      by  Sebb\n\n   START to play");
    while (joypad() != J_START)
    {
        // Wait
    }
}

void gameOver()
{
    // TODO: Stop music
    delay(600);
    HIDE_SPRITES;
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("     GAME OVER\n     Score:  %d\n\n   START to reset\n\n\n\n\n\n\n", numberOfSegments());
    while (joypad() != J_START)
    {
        // Wait
    }
    reset();
}

void collisionCheck()
{
    Segment *segment = head->child;
    while (segment != NULL)
    {
        if (segment->x == head->x && segment->y == head->y)
        {
            gameOver();
        }
        segment = segment->child;
    }
    if (head->x <= 1 || head->x >= SCREEN_WIDTH || head->y <= 2 || head->y >= SCREEN_HEIGHT)
    {
        gameOver();
    }
}

void fetchInput()
{
    UINT8 input = joypad();
    if ((input == J_UP || input == J_DOWN || input == J_LEFT || input == J_RIGHT) && input != opposite(lastLatchedInput))
    {
        nextLatchedInput = input;
    }
}

void move(int x_dist, int y_dist)
{
    // Scroll head sprite
    scroll_sprite(HEAD_SPRITE, x_dist, y_dist);

    if (is_logic_frame)
    {
        Segment *beforeTail = findPenultimateSegment();
        Segment *segment; // Either the tail or a new segment

        if (consumedFood)
        {
            segment = (Segment *)calloc(1, sizeof(Segment));
        }
        else
        {
            segment = beforeTail->child;
            if (segment != NULL)
            {
                set_bkg_tile_xy(segment->x - 1, segment->y - 2, BACKGROUND_TILE); // Remove tail segment tile
            }
            // I have no idea why i have to subtract these values from the coordinates but it works
        }

        if (segment != NULL)
        {
            // Put segment to where head is
            segment->x = head->x;
            segment->y = head->y;
            set_bkg_tile_xy(head->x - 1, head->y - 2, SEGMENT_TILE);
        }
    
        head->x += x_dist;
        head->y += y_dist;

        // Update children
        if (consumedFood)
        {
            segment->child = head->child;
            head->child = segment;
            consumedFood = 0;
        }
        else if (beforeTail != head)
        {
            Segment *temp = head->child;
            head->child = segment;
            segment->child = temp;
            beforeTail->child = NULL;
        }

        collisionCheck();
    }
}

void randomFood()
{
    // Randomize food position, repeat if it lands on the player
    do
    {
        food_x = rand() % (SCREEN_WIDTH - 4) + 3;
        food_y = rand() % (SCREEN_HEIGHT - 3) + 3;
    } while (food_x == head->x && food_y == head->y);
    // TODO: check for all snake segments

    move_sprite(FOOD_SPRITE, food_x * SCALE, food_y * SCALE);
}

void moveAccordingToInput(int input)
{
    switch (input)
    {
    case J_LEFT:
        move(-1, 0);
        break;
    case J_RIGHT:
        move(1, 0);
        break;
    case J_UP:
        move(0, -1);
        break;
    case J_DOWN:
        move(0, 1);
        break;
    }
}

void initialize()
{
    // Initialize head of snake to initial coordinates
    head = (Segment *)calloc(1, sizeof(Segment));
    head->x = INIT_X;
    head->y = INIT_Y;

    // Load tiles
    set_sprite_data(0, 3, Snake);
    set_sprite_tile(0, FOOD_SPRITE);
    set_sprite_tile(1, HEAD_SPRITE);

    set_bkg_data(0, 9, BackgroundTiles);
    set_bkg_tiles(0, 0, 20, 18, BackgroundMap);

    move_sprite(HEAD_SPRITE, head->x * SCALE, head->y * SCALE);

    // intirand() has to be called once for rand() to work
    initrand(DIV_REG);
    randomFood();

    SHOW_SPRITES;
    SHOW_BKG;
}

void main()
{
    showTitleScreen();
    initialize();

    while (1)
    {
        is_logic_frame = (loop_counter >= SCALE - 1);

        // If there is valid input, load into nextLatchedInput
        fetchInput();
        if (is_logic_frame)
            latchedInput = nextLatchedInput;

        moveAccordingToInput(latchedInput);

        if (is_logic_frame)
        {
            if (head->x == food_x && head->y == food_y)
            {
                randomFood();
                consumedFood = 1;
            }

            // Save this to know which direction snake is currently travelling
            lastLatchedInput = latchedInput;
            loop_counter = 0;
        }
        else
        {
            loop_counter += 1;
        }

        // Wait until next frame
        wait_vbl_done();
    }
}