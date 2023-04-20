#include <gb/gb.h>
#include <gb/hardware.h>
#include <rand.h>
#include <stdio.h>
#include <stdlib.h>
#include "Snake.c"
#include "time.h"
#include "BackgroundMap.c"
#include "BackgroundTiles.c"
//#include "hUGEDriver/hUGEDriver.h"

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
#define GAMEOVER_WAIT_TIME 500

//extern const hUGESong_t Song;

typedef unsigned char UINT8;
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

void soundInitialization()
{
    // Enable sound playback
    NR52_REG = 0x80;
    NR50_REG = 0x77;
    NR51_REG = 0xFF;

    // see https://github.com/bwhitman/pushpin/blob/master/src/gbsound.txt
    // comments adapted from https://gist.github.com/gingemonster/600c33f7dd97ecbf785eca8c84772c9a
    // chanel 1 register 0, Frequency sweep settings
    // 7	Unused
    // 6-4	Sweep time(update rate) (if 0, sweeping is off)
    // 3	Sweep Direction (1: decrease, 0: increase)
    // 2-0	Sweep RtShift amount (if 0, sweeping is off)
    NR10_REG = 0; 

    // chanel 1 register 1: Wave pattern duty and sound length
    // Channels 1 2 and 4
    // 7-6	Wave pattern duty cycle 0-3 (12.5%, 25%, 50%, 75%), duty cycle is how long a quadrangular  wave is "on" vs "of" so 50% (2) is both equal.
    // 5-0 sound length (higher the number shorter the sound)
    // 01000000 is 0x40, duty cycle 1 (25%), wave length 0 (long)
    NR11_REG = 0b10001100;

    // chanel 1 register 2: Volume Envelope (Makes the volume get louder or quieter each "tick")
    // On Channels 1 2 and 4
    // 7-4	(Initial) Channel Volume
    // 3	Volume sweep direction (0: down; 1: up)
    // 2-0	Length of each step in sweep (if 0, sweeping is off)
    // NOTE: each step is n/64 seconds long, where n is 1-7	
    // 0111 0011 is 0x73, volume 7, sweep down, step length 3
    NR12_REG = 0b11110001;
}

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
    printf("\n\n\n\n\n\n      SnakeBoy\n      by  Sebb\n\n\n    Press  Start");
    while (joypad() != J_START)
    {
        // Wait
    }
}

void playGameOverSound()
{
    // see https://github.com/bwhitman/pushpin/blob/master/src/gbsound.txt
    // chanel 1 register 0, Frequency sweep settings
    // 7	Unused
    // 6-4	Sweep time(update rate) (if 0, sweeping is off)
    // 3	Sweep Direction (1: decrease, 0: increase)
    // 2-0	Sweep RtShift amount (if 0, sweeping is off)
    NR10_REG = 0b00111111; 

    // chanel 1 register 1: Wave pattern duty and sound length
    // Channels 1 2 and 4
    // 7-6	Wave pattern duty cycle 0-3 (12.5%, 25%, 50%, 75%), duty cycle is how long a quadrangular  wave is "on" vs "of" so 50% (2) is both equal.
    // 5-0 sound length (higher the number shorter the sound)
    NR11_REG = 0b00000000;
    
    // chanel 1 register 2: Volume Envelope (Makes the volume get louder or quieter each "tick")
    // On Channels 1 2 and 4
    // 7-4	(Initial) Channel Volume
    // 3	Volume sweep direction (0: down; 1: up)
    // 2-0	Length of each step in sweep (if 0, sweeping is off)
    // NOTE: each step is n/64 seconds long, where n is 1-7	
    // 0111 0011 is 0x73, volume 7, sweep down, step length 3
    NR12_REG = 0b11111000;

    // chanel 1 register 3: Frequency LSbs (Least Significant bits) and noise options
    // 7-0	8 Least Significant bits of frequency (3 Most Significant Bits are set in register 4)
    NR13_REG = 0;   

    // chanel 1 register 4: Playback and frequency MSbs
    // 7	Initialize (trigger channel start, AKA channel INIT) (Write only)
    // 6	Consecutive select/length counter enable (Read/Write). When "0", regardless of the length of data on the NR11 register, sound can be produced consecutively.  When "1", sound is generated during the time period set by the length data contained in register NR11.  After the sound is ouput, the Sound 1 ON flag, at bit 0 of register NR52 is reset.
    // 5-3	Unused
    // 2-0	3 Most Significant bits of frequency
    NR14_REG = 0b10000110;	  
}

void stopSound()
{
    NR52_REG = 0;
}

void gameOver()
{
    //remove_VBL(hUGE_dosound);
    playGameOverSound();
    delay(GAMEOVER_WAIT_TIME);
    HIDE_SPRITES;
    int numSegments = numberOfSegments();
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    if (numSegments >= 287) // Maximum amount of segments that can be on screen
    {
        printf("      YOU  WIN\n      Nice One\n     Score:  %d\n\n\n   START to reset\n\n\n\n\n\n\n", numSegments);
    }
    else{
        printf("     GAME OVER\n     Score: %d\n\n\n   START to reset\n\n\n\n\n\n\n", numSegments);
    }
    stopSound();
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

void pauseGame()
{
    UINT8 input = J_A;
    HIDE_SPRITES;

    //remove_VBL(hUGE_dosound);
    soundInitialization();
    NR13_REG = 0;   
    // chanel 1 register 4: Playback and frequency MSbs
    // 7	Initialize (trigger channel start, AKA channel INIT) (Write only)
    // 6	Consecutive select/length counter enable (Read/Write). When "0", regardless of the length of data on the NR11 register, sound can be produced consecutively.  When "1", sound is generated during the time period set by the length data contained in register NR11.  After the sound is ouput, the Sound 1 ON flag, at bit 0 of register NR52 is reset.
    // 5-3	Unused
    // 2-0	3 Most Significant bits of frequency
    NR14_REG = 0b11000111;	    

    delay(500);
    while (input != J_START)
    {
        input = joypad();
    }

    // chanel 1 register 3: Frequency LSbs (Least Significant bits) and noise options
    // 7-0	8 Least Significant bits of frequency (3 Most Significant Bits are set in register 4)
    NR13_REG = 64;
    NR14_REG = 0b11000111;
    delay(300);
    SHOW_SPRITES;    
    //add_VBL(hUGE_dosound);
}

void fetchInput()
{
    UINT8 input = joypad();
    if ((input == J_UP || input == J_DOWN || input == J_LEFT || input == J_RIGHT) && input != opposite(lastLatchedInput))
    {
        nextLatchedInput = input;
    }
    if (input == J_START)
    {
        pauseGame();
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

UINT8 foodCollisionWithBody(UINT8 food_x, UINT8 food_y)
{
    if (food_x == head->x && food_y == head->y)
    {
        return 1;
    }
    Segment* segment = head;
    while (segment != NULL)
    {
        if (food_x == segment->x && food_y == segment->y)
        {
            return 1;
        }
        segment = segment->child;
    }
    return 0;
}

void randomFood()
{
    // Randomize food position, repeat if it lands on the snake
    do
    {
        food_x = rand() % (SCREEN_WIDTH - 4) + 3;
        food_y = rand() % (SCREEN_HEIGHT - 3) + 3;
    } while (foodCollisionWithBody(food_x, food_y));

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
    soundInitialization();
    
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

    //hUGE_init(&Song);
    //add_VBL(hUGE_dosound);
}

void playFoodSound()
{
    // see https://github.com/bwhitman/pushpin/blob/master/src/gbsound.txt
    // chanel 1 register 0, Frequency sweep settings
    // 7	Unused
    // 6-4	Sweep time(update rate) (if 0, sweeping is off)
    // 3	Sweep Direction (1: decrease, 0: increase)
    // 2-0	Sweep RtShift amount (if 0, sweeping is off)
    NR10_REG = 0b00010110; 

    // chanel 1 register 1: Wave pattern duty and sound length
    // Channels 1 2 and 4
    // 7-6	Wave pattern duty cycle 0-3 (12.5%, 25%, 50%, 75%), duty cycle is how long a quadrangular  wave is "on" vs "of" so 50% (2) is both equal.
    // 5-0 sound length (higher the number shorter the sound)
    NR11_REG = 0b11001100;

    // chanel 1 register 2: Volume Envelope (Makes the volume get louder or quieter each "tick")
    // On Channels 1 2 and 4
    // 7-4	(Initial) Channel Volume
    // 3	Volume sweep direction (0: down; 1: up)
    // 2-0	Length of each step in sweep (if 0, sweeping is off)
    // NOTE: each step is n/64 seconds long, where n is 1-7	
    // 0111 0011 is 0x73, volume 7, sweep down, step length 3
    NR12_REG = 0b11111000;

    // chanel 1 register 3: Frequency LSbs (Least Significant bits) and noise options
    // 7-0	8 Least Significant bits of frequency (3 Most Significant Bits are set in register 4)
    NR13_REG = 0;   

    // chanel 1 register 4: Playback and frequency MSbs
    // 7	Initialize (trigger channel start, AKA channel INIT) (Write only)
    // 6	Consecutive select/length counter enable (Read/Write). When "0", regardless of the length of data on the NR11 register, sound can be produced consecutively.  When "1", sound is generated during the time period set by the length data contained in register NR11.  After the sound is ouput, the Sound 1 ON flag, at bit 0 of register NR52 is reset.
    // 5-3	Unused
    // 2-0	3 Most Significant bits of frequency
    NR14_REG = 0b11000111;	  
}

void main()
{
    showTitleScreen();
    initialize();
    delay(300);

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
                playFoodSound();
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