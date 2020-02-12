#include <gb/gb.h>
#include <rand.h>
#include "octopus_gfx.h"

#define OCTOPUS_X 7
#define OCTOPUS_Y 4
#define OCTOPUS_LEG_COUNT 4
#define AQUA_X OCTOPUS_X - 5
#define AQUA_Y OCTOPUS_Y
#define AQUA_STEP_COUNT 6  
#define AQUA_TEAM_X OCTOPUS_X - 4
#define AQUA_TEAM_Y OCTOPUS_Y - 3

const s_data * tentacle_1[] = {&tent_1_0, &tent_1_1, &tent_1_2, &tent_1_3, &tent_1_4};
const s_data * tentacle_2[] = {&tent_2_0, &tent_2_1, &tent_2_2, &tent_2_3, &tent_2_4, &tent_2_5}; 
const s_data * tentacle_3[] = {&tent_3_0, &tent_3_1, &tent_3_2, &tent_3_3, &tent_3_4}; 
const s_data * tentacle_4[] = {&tent_4_0, &tent_4_1, &tent_4_2, &tent_4_3}; 

const s_coord const tentacle_1_coord[] = {{OCTOPUS_X - 1, OCTOPUS_Y + 1}, {OCTOPUS_X - 2, OCTOPUS_Y + 1}, {OCTOPUS_X - 3, OCTOPUS_Y + 2}, {OCTOPUS_X - 3, OCTOPUS_Y + 3}, {OCTOPUS_X - 4, OCTOPUS_Y + 5}};
const s_coord const tentacle_2_coord[] = {{OCTOPUS_X,     OCTOPUS_Y + 3}, {OCTOPUS_X,     OCTOPUS_Y + 3}, {OCTOPUS_X,     OCTOPUS_Y + 4}, {OCTOPUS_X,     OCTOPUS_Y + 6}, {OCTOPUS_X,     OCTOPUS_Y + 7}, {OCTOPUS_X - 1, OCTOPUS_Y + 8}};
const s_coord const tentacle_3_coord[] = {{OCTOPUS_X + 3, OCTOPUS_Y + 5}, {OCTOPUS_X + 3, OCTOPUS_Y + 5}, {OCTOPUS_X + 3, OCTOPUS_Y + 6}, {OCTOPUS_X + 3, OCTOPUS_Y + 7}, {OCTOPUS_X + 3, OCTOPUS_Y + 8}};
const s_coord const tentacle_4_coord[] = {{OCTOPUS_X + 6, OCTOPUS_Y + 6}, {OCTOPUS_X + 6, OCTOPUS_Y + 6}, {OCTOPUS_X + 6, OCTOPUS_Y + 7}, {OCTOPUS_X + 6, OCTOPUS_Y + 8}};

const s_data * * tentacles[OCTOPUS_LEG_COUNT] = {&tentacle_1, &tentacle_2, &tentacle_3, &tentacle_4};
const s_coord * tentacle_coords[OCTOPUS_LEG_COUNT] = {&tentacle_1_coord, &tentacle_2_coord, &tentacle_3_coord, &tentacle_4_coord};

const s_data * aqua_empty[] = {&aqua_0_empty, &aqua_1_empty, &aqua_2_empty, &aqua_3_empty, &aqua_4_empty, &aqua_5_0_empty};
const s_data * aqua_full[] = {&aqua_0_full, &aqua_1_full, &aqua_2_full, &aqua_3_full, &aqua_4_full, &aqua_5_0_full};
const s_data * aqua_erase[] = {&aqua_0_out, &null_sprite, &null_sprite, &null_sprite, &null_sprite, &null_sprite};
const s_coord const aqua_coords[] = {{AQUA_X - 2, AQUA_Y - 3}, {AQUA_X - 2, AQUA_Y + 1}, {AQUA_X - 1, AQUA_Y + 6}, {AQUA_X + 4, AQUA_Y + 9}, {AQUA_X + 7, AQUA_Y + 9}, {AQUA_X + 10, AQUA_Y + 9}};

const s_data * aqua_team[] = {&zero_left, &one_left, &two_left};

typedef struct {
    UBYTE pos, old_pos, dir, len;
} t_params;

static t_params tentacles_params[OCTOPUS_LEG_COUNT] = {{0, 0, 1, 5}, {0, 0, 1, 6}, {0, 0, 1, 5}, {0, 0, 1, 4}};


static UBYTE current;                  // current tentacle number
static UBYTE aqua_pos, aqua_pos_old;   // aquanaut position

static s_data * * tentacle;            // points to current tentacle sprites
static s_coord * tentacle_coord;       // points to current tentacle sprites coords
static t_params * tentacle_params;     // points to current tentacle params

static s_coord * aqua_coord;           // current coordinates of aquanaut
static s_data * aqua_man;              // current aquanaut sprite
static s_data * aqua_man_del;          // current aquanaut empty sprite 

static UWORD seed;                     // random seed value 
static UBYTE time, rndval;             // time in vsync's; current random value
static UBYTE x, y, i, j, joy;          // misc variables
static UBYTE bag, bag_old, team_size;  // bag size

void main()
{
    disable_interrupts();
    
    LCDC_REG = 0x57;
    /*
     * LCD        = Off
     * WindowBank = 0x9C00
     * Window     = Off (0x20)
     * BG Chr     = 0x8000 (0x16)
     * BG Bank    = 0x9800
     * OBJ        = 8x16
     * OBJ        = On
     * BG         = On
     */

    BGP_REG = OBP0_REG = OBP1_REG = 0xE4U;                  // Set palettes

    set_bkg_data(0x0, tile_data.count, tile_data.data);     // Initialize the background

    for (y = 0; y < 18; y += 4)                             // clear background
        for (x = 0; x < 20; x += 4)
            set_bkg_tiles(x, y, null_sprite.dim.x, null_sprite.dim.y, null_sprite.data);
 
    set_bkg_tiles(OCTOPUS_X, OCTOPUS_Y, octo_head.dim.x, octo_head.dim.y, octo_head.data); // draw octopus body
    set_bkg_tiles(OCTOPUS_X + 9, OCTOPUS_Y + 10, chest.dim.x, chest.dim.y, chest.data);    // draw treasures 

    DISPLAY_ON;
    enable_interrupts();


    seed = DIV_REG;
    seed |= (UWORD)DIV_REG << 8;
    initrand(seed);
        
    team_size = 2;    
    set_bkg_tiles(AQUA_TEAM_X, AQUA_TEAM_Y, aqua_team[team_size]->dim.x, aqua_team[team_size]->dim.y, aqua_team[team_size]->data);
    
    current = 0; 
    bag = 0; bag_old = bag;
    aqua_pos = 0; aqua_pos_old = 1;
    while(1) {
        wait_vbl_done();
        time++;

        joy = joypad();
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
            }
            waitpadup();
        }
        
        if ((aqua_pos != aqua_pos_old) || (!bag_old && bag) || (bag_old && !bag)) {
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
            
            if (!aqua_pos) bag = 0;
        }
        
        if ((time & 7) == 0) {        
            rndval = rand();
            current = rndval % OCTOPUS_LEG_COUNT;
            tentacle = tentacles[current];
            tentacle_coord = tentacle_coords[current];
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
                if (i < j) set_bkg_tiles(tentacle_coord[j].x, tentacle_coord[j].y, tentacle[j]->dim.x, tentacle[j]->dim.y, null_sprite.data);
                set_bkg_tiles(tentacle_coord[i].x, tentacle_coord[i].y, tentacle[i]->dim.x, tentacle[i]->dim.y, tentacle[i]->data); 
                tentacle_params->old_pos = tentacle_params->pos;
            }
        }
    }
}