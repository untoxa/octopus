#include <gb/gb.h>
#include <rand.h>
#include "octopus_gfx.h"

#define OCTOPUS_X 7
#define OCTOPUS_Y 4
#define OCTOPUS_TENTACLE_COUNT 4
#define AQUA_X OCTOPUS_X - 5
#define AQUA_Y OCTOPUS_Y
#define AQUA_STEP_COUNT 6  
#define AQUA_TEAM_X OCTOPUS_X - 4
#define AQUA_TEAM_Y OCTOPUS_Y - 3
#define AQUA_TEAM_SIZE 3

// octopus
const s_data * tentacle_1[] = {&tent_1_0, &tent_1_1, &tent_1_2, &tent_1_3, &tent_1_4};
const s_data * tentacle_2[] = {&tent_2_0, &tent_2_1, &tent_2_2, &tent_2_3, &tent_2_4, &tent_2_5}; 
const s_data * tentacle_3[] = {&tent_3_0, &tent_3_1, &tent_3_2, &tent_3_3, &tent_3_4}; 
const s_data * tentacle_4[] = {&tent_4_0, &tent_4_1, &tent_4_2, &tent_4_3}; 

const s_coord const tentacle_1_coord[] = {{OCTOPUS_X - 1, OCTOPUS_Y + 1}, {OCTOPUS_X - 2, OCTOPUS_Y + 1}, {OCTOPUS_X - 3, OCTOPUS_Y + 2}, {OCTOPUS_X - 3, OCTOPUS_Y + 3}, {OCTOPUS_X - 4, OCTOPUS_Y + 5}};
const s_coord const tentacle_2_coord[] = {{OCTOPUS_X,     OCTOPUS_Y + 3}, {OCTOPUS_X,     OCTOPUS_Y + 3}, {OCTOPUS_X,     OCTOPUS_Y + 4}, {OCTOPUS_X,     OCTOPUS_Y + 6}, {OCTOPUS_X,     OCTOPUS_Y + 7}, {OCTOPUS_X - 1, OCTOPUS_Y + 8}};
const s_coord const tentacle_3_coord[] = {{OCTOPUS_X + 3, OCTOPUS_Y + 5}, {OCTOPUS_X + 3, OCTOPUS_Y + 5}, {OCTOPUS_X + 3, OCTOPUS_Y + 6}, {OCTOPUS_X + 3, OCTOPUS_Y + 7}, {OCTOPUS_X + 3, OCTOPUS_Y + 8}};
const s_coord const tentacle_4_coord[] = {{OCTOPUS_X + 6, OCTOPUS_Y + 6}, {OCTOPUS_X + 6, OCTOPUS_Y + 6}, {OCTOPUS_X + 6, OCTOPUS_Y + 7}, {OCTOPUS_X + 6, OCTOPUS_Y + 8}};

const s_data * * tentacles[OCTOPUS_TENTACLE_COUNT] = {&tentacle_1, &tentacle_2, &tentacle_3, &tentacle_4};
const s_coord * tentacle_coords[OCTOPUS_TENTACLE_COUNT] = {&tentacle_1_coord, &tentacle_2_coord, &tentacle_3_coord, &tentacle_4_coord};

// aquanauts
const s_data * aqua_empty[AQUA_STEP_COUNT] = {&aqua_0_empty, &aqua_1_empty, &aqua_2_empty, &aqua_3_empty, &aqua_4_empty, &aqua_5_0_empty};
const s_data * aqua_full[AQUA_STEP_COUNT] = {&aqua_0_full, &aqua_1_full, &aqua_2_full, &aqua_3_full, &aqua_4_full, &aqua_5_0_full};
const s_data * aqua_erase[AQUA_STEP_COUNT] = {&aqua_0_out, &null_sprite, &null_sprite, &null_sprite, &null_sprite, &null_sprite};
const s_coord const aqua_coords[AQUA_STEP_COUNT] = {{AQUA_X - 2, AQUA_Y - 3}, {AQUA_X - 2, AQUA_Y + 1}, {AQUA_X - 1, AQUA_Y + 6}, {AQUA_X + 4, AQUA_Y + 9}, {AQUA_X + 7, AQUA_Y + 9}, {AQUA_X + 10, AQUA_Y + 9}};

const s_data * aqua_team[AQUA_TEAM_SIZE + 1] = {&zero_left, &zero_left, &one_left, &two_left};

// animations
const s_data * yield_ani[6] = {&aqua_0_empty, &aqua_0_full, &aqua_0_empty, &aqua_0_full, &aqua_0_empty, &aqua_0_full};
const s_data * grab_ani_empty[3] = {&aqua_5_0_full, &aqua_5_1_empty, &aqua_5_2_empty};
const s_data * grab_ani_full[3] = {&aqua_5_0_full, &aqua_5_1_full, &aqua_5_2_full};
const s_data * agony_ani[9] = {&agony_over, &agony_1, &agony_2, &agony_1, &agony_2, &agony_1, &agony_2, &agony_1, &agony_2};
const s_coord const agony_coord = {OCTOPUS_X, OCTOPUS_Y + 3};

typedef struct {
    UBYTE pos, old_pos, dir, len;
} t_params;

static t_params tentacles_params[OCTOPUS_TENTACLE_COUNT] = {{0, 0, 1, 5}, {0, 0, 1, 6}, {0, 0, 1, 5}, {0, 0, 1, 4}};


static UBYTE current;                  // current tentacle number
static UBYTE aqua_pos, aqua_pos_old;   // aquanaut position

static s_data * * tentacle;            // points to current tentacle sprites
static s_coord * tentacle_coord;       // points to current tentacle sprites coords
static t_params * tentacle_params;     // points to current tentacle params

static s_coord * aqua_coord;           // current coordinates of aquanaut or animation
static s_data * aqua_man;              // current aquanaut sprite or animation
static s_data * aqua_man_del;          // current aquanaut empty sprite 


static UWORD seed;                     // random seed value 
static UBYTE time, rndval;             // time in vsync's; current random value
static UBYTE i, j, joy;                // misc variables
static UBYTE bag, bag_old, team_size;  // bag size

enum animation_type { ANI_NONE, ANI_YIELD, ANI_GRAB, ANI_AGONY };
enum reinit_signal { NONE, REINIT, KILLED, STARTUP };
enum game_state {GAME, GAMEOVER};

static enum animation_type ani_type;
static enum reinit_signal reinit;
static enum game_state gamestate;

static UBYTE animation, animation_old; // animation clock

static UWORD score;

void main()
{
    disable_interrupts();
    
    LCDC_REG = 0x55;
    /*
     * LCD        = Off
     * WindowBank = 0x9C00
     * Window     = Off (0x20)
     * BG Chr     = 0x8000 (0x16)
     * BG Bank    = 0x9800
     * OBJ        = 8x16
     * OBJ        = Off
     * BG         = On
     */

    BGP_REG = OBP0_REG = OBP1_REG = 0xE4U;                  // set palettes

    set_bkg_data(0x0, tile_data.count, tile_data.data);     // initialize tiles data

    for (i = 0; i < 18; i += 4)                             // clear background
        for (j = 0; j < 20; j += 4)
            set_bkg_tiles(j, i, null_sprite.dim.x, null_sprite.dim.y, null_sprite.data);

    DISPLAY_ON;
    enable_interrupts();

    seed = DIV_REG;
    seed |= (UWORD)DIV_REG << 8;
    initrand(seed);        
    
    time = 0;
    current = 0; 
    bag = 0; bag_old = bag;
    aqua_pos = 0; aqua_pos_old = 1;
    animation = 0; animation_old = animation; ani_type = ANI_NONE;
    team_size = AQUA_TEAM_SIZE; score = 0; 
    reinit = REINIT; gamestate = GAME;
    while(1) {
        wait_vbl_done();
        time++;

        if (animation > 0) {
            if (animation != animation_old) {
                if (ani_type == ANI_YIELD) {
                    aqua_coord = &aqua_coords[0];
                    aqua_man = yield_ani[animation - 1];
                } else if (ani_type == ANI_GRAB) {
                    aqua_coord = &aqua_coords[AQUA_STEP_COUNT - 1];
                    if (!bag) aqua_man = grab_ani_empty[animation - 1]; else aqua_man = grab_ani_full[animation - 1];
                } else if (ani_type == ANI_AGONY) {
                    aqua_coord = &agony_coord;
                    aqua_man = agony_ani[animation - 1];
                    if (animation == 1) {
                        reinit = KILLED;
                    }
                }                    
                set_bkg_tiles(aqua_coord->x, aqua_coord->y, aqua_man->dim.x, aqua_man->dim.y, aqua_man->data);
            }
            animation_old = animation;
            if ((time & 3) == 0) animation--;
        } else {
            joy = joypad();
            if (gamestate == GAME) {
                if ((joy & J_B) || (joy & J_LEFT)) {
                    if (aqua_pos > 0) {
                        aqua_pos--;
                    };
                    waitpadup();
                } else if ((joy & J_A) || (joy & J_RIGHT)) {
                    if (aqua_pos < AQUA_STEP_COUNT - 1) {
                        aqua_pos++;
                    } else {
                        bag++;
                        animation = 3;
                        ani_type = ANI_GRAB;
                    }
                    waitpadup();
                }
            } else if (gamestate == GAMEOVER) {
                if (joy & J_START) {
                    waitpadup();
                    reinit = STARTUP;
                }
            }
        }

        // handle start game
        if (reinit == STARTUP) {
            // init some default values
            bag = 0; bag_old = bag;
            aqua_pos = 0; aqua_pos_old = 1;
            team_size = AQUA_TEAM_SIZE; score = 0;             
            // draw boat
            aqua_man = aqua_team[team_size];
            set_bkg_tiles(AQUA_TEAM_X, AQUA_TEAM_Y, aqua_man->dim.x, aqua_man->dim.y, aqua_man->data);
            // back to game
            reinit = NONE; gamestate = GAME;            
        }

        // handle kill event
        if (reinit == KILLED) {
            bag = 0;
            if (team_size != 0) team_size--;
            if (!team_size) {
                aqua_pos = AQUA_STEP_COUNT;
                gamestate = GAMEOVER;
            } else aqua_pos = 0;  
            reinit = REINIT;
        }
                
        // draw initial screen at startup/after death
        if (reinit == REINIT) {
            set_bkg_tiles(OCTOPUS_X, OCTOPUS_Y, octo_head.dim.x, octo_head.dim.y, octo_head.data); // draw octopus body
            set_bkg_tiles(OCTOPUS_X + 9, OCTOPUS_Y + 10, chest.dim.x, chest.dim.y, chest.data);    // draw treasures 
            // draw boat
            aqua_man = aqua_team[team_size];
            set_bkg_tiles(AQUA_TEAM_X, AQUA_TEAM_Y, aqua_man->dim.x, aqua_man->dim.y, aqua_man->data);
            
            ani_type = ANI_NONE;
            reinit = NONE;
        }
        
        // draw aquanaut
        if ((aqua_pos < AQUA_STEP_COUNT) && (aqua_pos != aqua_pos_old) || (!bag_old && bag)) {
            // delete previous sprite
            aqua_coord = &aqua_coords[aqua_pos_old];
            if (!bag) aqua_man = aqua_empty[aqua_pos_old]; else aqua_man = aqua_full[aqua_pos_old];
            aqua_man_del = aqua_erase[aqua_pos_old]; 
            set_bkg_tiles(aqua_coord->x, aqua_coord->y, aqua_man->dim.x, aqua_man->dim.y, aqua_man_del->data);
            // draw new sprite
            aqua_coord = &aqua_coords[aqua_pos];
            if (!bag) aqua_man = aqua_empty[aqua_pos]; else aqua_man = aqua_full[aqua_pos];
            set_bkg_tiles(aqua_coord->x, aqua_coord->y, aqua_man->dim.x, aqua_man->dim.y, aqua_man->data);
            // update old position and bag;
            aqua_pos_old = aqua_pos;
            bag_old = bag;
            
            if (!aqua_pos) { 
                score += bag;
                bag = 0;
                if (bag_old != bag) { 
                    if (bag_old > 10) animation = 5; else animation = 3;
                    ani_type = ANI_YIELD;
                }
            }    
        }
        
        // move tentacles and redraw them
        if (((time & 7) == 0) && (ani_type != ANI_AGONY)) {        
            rndval = rand();
            current = rndval % OCTOPUS_TENTACLE_COUNT;
            tentacle_params = &tentacles_params[current];
            
            i = tentacle_params->pos;
            if (tentacle_params->dir & 1) {
                if (i > 0) {
                    tentacle_params->pos--; 
                } else tentacle_params->dir++;
            } else {
                if (i < tentacle_params->len - 1) { 
                    tentacle_params->pos++;
                } else tentacle_params->dir++;
            }
                
            i = tentacle_params->pos;
            j = tentacle_params->old_pos;
            if (i != j) {
                tentacle = tentacles[current];
                tentacle_coord = tentacle_coords[current];
                if (i < j) set_bkg_tiles(tentacle_coord[j].x, tentacle_coord[j].y, tentacle[j]->dim.x, tentacle[j]->dim.y, null_sprite.data);
                set_bkg_tiles(tentacle_coord[i].x, tentacle_coord[i].y, tentacle[i]->dim.x, tentacle[i]->dim.y, tentacle[i]->data); 
                tentacle_params->old_pos = tentacle_params->pos;
            }
        }
        
        // check aquanaut is caught
        if ((aqua_pos > 1) && (aqua_pos < OCTOPUS_TENTACLE_COUNT + 2)) {
            tentacle_params = &tentacles_params[aqua_pos - 2];
            if (tentacle_params->pos == tentacle_params->len - 1) {
                // prepare octopus for animation (all tentacles in)
                for (i = 0; i < OCTOPUS_TENTACLE_COUNT; i++) {
                    tentacle_params = &tentacles_params[i];
                    tentacle_params->pos = 0; tentacle_params->old_pos = 0; tentacle_params->dir = 1;                    

                    tentacle = tentacles[i];
                    tentacle_coord = tentacle_coords[i];
                    for (j = 1; j < tentacle_params->len; j++) {
                        set_bkg_tiles(tentacle_coord[j].x, tentacle_coord[j].y, tentacle[j]->dim.x, tentacle[j]->dim.y, null_sprite.data);
                    }    
                    set_bkg_tiles(tentacle_coord[0].x, tentacle_coord[0].y, tentacle[0]->dim.x, tentacle[0]->dim.y, tentacle[0]->data);                     
                }
                // delete aquanaut
                aqua_coord = &aqua_coords[aqua_pos];
                if (!bag) aqua_man = aqua_empty[aqua_pos]; else aqua_man = aqua_full[aqua_pos];
                aqua_man_del = aqua_erase[aqua_pos]; 
                set_bkg_tiles(aqua_coord->x, aqua_coord->y, aqua_man->dim.x, aqua_man->dim.y, aqua_man_del->data);
                
                animation = 9;
                ani_type = ANI_AGONY;
            }
        }    
    }
}