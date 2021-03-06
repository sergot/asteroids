#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <math.h>
#include <stdio.h>

#include "game.h"
#include "config.h"
#include "asteroid.h"
#include "shooter.h"

int config_pos = 0;
ALLEGRO_FONT *ttf_font; // font to use

void inc_config_pos() {
    if(++config_pos > 1) config_pos = 0;
}

void add_to_config(int a) {
    if(config_pos == 0) {
        change_config("display_width", a ? 10 : -10);
    }
    else if(config_pos == 1) {
        change_config("display_height", a ? 10 : -10);
    }
}

game* new_game(point *s) {
    game *g = calloc(1, sizeof(game));
    g->Ship = new_ship(p_times(s, 0.5));
    g->Size = s;
    g->status = Pause;
    g->score = 0;

    int i;
    for(i = 0; i <= ASTEROID_COUNT; i++)
        new_asteroid(ASTEROID_MAX_LVL);
    
    return g;
}

void del_game(game *g) {
    del_asteroids(first_a);  
    del_shoots(first_s);
    free(g->Ship);
    free(g);
}

bool font() {
    ttf_font = NULL;
    al_init_font_addon();
    if(!al_init_ttf_addon())
        return false;
    ttf_font = al_load_ttf_font("font.ttf", 20, 0);
    return ttf_font != NULL;
}

void update_game(game *g) {
    if(g->status == Quit) {
        del_game(g);
    } else {
        if(first_a == NULL)
            g->status = Win;
        else {
            update_shoots();
            update_asteroids();
            update_collisions(g);

            update_ship(g, first_a);
        }
    }
}

void draw_game(game *g) {
    int i;
    
    al_clear_to_color(BACKGROUND_COLOR);
    if(g->status == Pause || g->status == Win || g->status == Lose)
        draw_menu(g);
    else if(g->status == Config)
        draw_config(g);
    else if(g->status != Quit) {
        draw_ui(g);
        draw_ship(g);
        draw_shoots(g);
        draw_asteroids(g);
    }
}

void draw_config(game *g) {
    ALLEGRO_TRANSFORM trans;
    al_identity_transform(&trans);
    al_use_transform(&trans);
    
    int x, y;
    x = (g->Size->x - 150) / 2;
    y = 0;
    al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y, ALLEGRO_ALIGN_LEFT, "CONFIGURATION");
    
    if(get_config("display_width") == NULL) return;
    
    x = 100;
    y = 100;
    char msg[22];
    sprintf(msg, "DISPLAY WIDTH: %d", atoi(get_config("display_width")));
    al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y, ALLEGRO_ALIGN_LEFT, msg);
    
    x = 100;
    y = 125;
    sprintf(msg, "DISPLAY HEIGHT: %d", atoi(get_config("display_height")));
    al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y, ALLEGRO_ALIGN_LEFT, msg);
    
    al_draw_circle(80, 110 + config_pos * 25, 3.0, SHOOT_COLOR, 1.5);    
}

void draw_ship(game *game) {
    ALLEGRO_TRANSFORM trans;
    al_identity_transform(&trans);
    al_rotate_transform(&trans, game->Ship->angle);
    al_translate_transform(&trans,
                           (int)game->Ship->position->x,
                           (int)game->Ship->position->y);
    al_use_transform(&trans);
    
    float w = game->Ship->time >= 0 ? 1 : 4;
    
    al_draw_line(-8, 20, 0, 0, SHIP_COLOR, w);
    al_draw_line(8, 20, 0, 0, SHIP_COLOR, w);
    al_draw_line(-6, 15, -1, 15, SHIP_COLOR, w);
    al_draw_line(6, 15, 1, 15, SHIP_COLOR, w);
}

void draw_shoots(game *g) {
    ALLEGRO_TRANSFORM trans;
    al_identity_transform(&trans);
    al_use_transform(&trans);
    
    shoot *pt;
    pt = first_s;
    while(pt != NULL) {
        al_draw_pixel(pt->position->x, pt->position->y, SHOOT_COLOR);
        pt = pt->next;
    }
}

void draw_asteroids(game *g) {
    ALLEGRO_TRANSFORM trans;
    al_identity_transform(&trans);
    al_use_transform(&trans);
    
    asteroid *pt = first_a;
    while(pt != NULL) {
        al_draw_line(
                pt->position->x, pt->position->y + pt->lvl*10,
                pt->position->x + pt->lvl*10, pt->position->y,
                ASTEROID_COLOR, 2.0
        );
        al_draw_line(
                pt->position->x, pt->position->y - pt->lvl*10,
                pt->position->x + pt->lvl*10, pt->position->y,
                ASTEROID_COLOR, 2.0
        );
        al_draw_line(
                pt->position->x, pt->position->y + pt->lvl*10,
                pt->position->x - pt->lvl*10, pt->position->y,
                ASTEROID_COLOR, 2.0
        );
        al_draw_line(
                pt->position->x, pt->position->y - pt->lvl*10,
                pt->position->x - pt->lvl*10, pt->position->y,
                ASTEROID_COLOR, 2.0
        );
        
        pt = pt->next;
    }
}

void draw_menu(game *g) {
    ALLEGRO_TRANSFORM trans;
    al_identity_transform(&trans);
    al_use_transform(&trans);
    
    int x, y;
    x = (g->Size->x - 60) / 2;
    y = 0;
    al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y, ALLEGRO_ALIGN_LEFT, "MENU");
    
    
    char *msg;
    
    if(g->status == Win || g->status == Lose) {
        x = (g->Size->x - 250) / 2;
        y = 50;
        msg = g->status == Win ? "YOU WON!! GAME OVER" : "YOU LOST. GAME OVER";
        al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y, ALLEGRO_ALIGN_LEFT, msg);
    }
    
    msg = "'c' to configure";
    x = (g->Size->x - 640+100) / 2;
    y = (g->Size->y + 5) / 2;
    al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y,
                 ALLEGRO_ALIGN_LEFT, msg);
    
    msg = "Enter to play";
    x = (g->Size->x - 640+100) / 2;
    y = (g->Size->y + 50) / 2;
    al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y,
                 ALLEGRO_ALIGN_LEFT, msg);
    
    msg = "Escape to exit";
    x = (g->Size->x - 640+100) / 2;
    y = (g->Size->y + 100) / 2;
    al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y,
                 ALLEGRO_ALIGN_LEFT, msg);
    
    msg = "Use the arrow keys to move and space to shoot";
    y = g->Size->y - 130;
    x = (g->Size->x - 640+100)/2;
    al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y,
                 ALLEGRO_ALIGN_LEFT, msg);
    
    msg = "If your ship is bold, you cannot be destroyed";
    y = g->Size->y - 80;
    x = (g->Size->x - 640+100)/2;
    al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y,
                 ALLEGRO_ALIGN_LEFT, msg);
}

void draw_ui(game *g) {
    ALLEGRO_TRANSFORM trans;
    al_identity_transform(&trans);
    al_use_transform(&trans);
    
    int x, y;
    char *msg;
    msg = calloc(8, sizeof(char));
    sprintf(msg, "Life: %d", g->Ship->life);
    
    x = 10;
    y = 10;
    al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y,
                 ALLEGRO_ALIGN_LEFT, msg);
    
    free(msg);
    
    msg = calloc(8 + count_digits(g->score), sizeof(char));
    sprintf(msg, "Score: %d", g->score);
    x += 150;
    al_draw_text(ttf_font, al_map_rgb(200, 200, 200), x, y,
                 ALLEGRO_ALIGN_LEFT, msg);
    free(msg);
}

void bound_position(point *p) {
    float x = atoi(get_config("display_width")), y = atoi(get_config("display_height"));
    
    while(p->x > x) {
        p->x -= x;
    }
    while(p->x < 0.) {
        p->x += x;
    }
    while(p->y > y) {
        p->y -= y;
    }
    while(p->y < 0.) {
        p->y += y;
    }
}

void move_object(point *pos, float angle, int speed) {
    pos->x -= speed * sin(-angle);
    pos->y -= speed * cos(-angle);
    bound_position(pos);
}

void update_collisions(game *g) {
    asteroid *shot = hit_shoot(first_s, first_a);
    if(shot != NULL) {
        g->score += 4-shot->lvl;
        split_asteroid(shot);
    }
}

void update_ship(game *g, asteroid *a) {
    if(g->Ship->time > 0)
        g->Ship->time -= 0.1;
    
    point *n1 = new_point(), *n2 = new_point();
    n1->x = g->Ship->position->x - 8 * sin(g->Ship->angle);
    n1->y = g->Ship->position->y + 20 * cos(g->Ship->angle);
    
    n2->x = g->Ship->position->x + 8 * sin(g->Ship->angle);
    n2->y = g->Ship->position->y + 20 * cos(g->Ship->angle);
    
    if((collision(a, g->Ship->position) || collision(a, n1) || collision(a,n2) ) && g->Ship->time <= 0) {
        if(--g->Ship->life <= 0)
            g->status = Lose;
        g->Ship->position = p_times(g->Size, 0.5);
        g->Ship->time = 5;
    }
    
    free(n1); free(n2);
}

int count_digits(int n) {
    int c = 0;
    while(n != 0) {
        c++;
        n /= 10;
    }
    if(n == 0) c++;
    
    return c;
}
