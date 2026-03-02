#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_primitives.h>

#define LARGURA_TELA 960
#define ALTURA_TELA 540
#define FPS 60

#define LARGURA_CANHAO 80
#define ALTURA_CANHAO 48
#define VELOCIDADE_CANHAO 5

#define MAX_LINHAS 8
#define MAX_COLUNAS 10
#define LARGURA_NAVE 40
#define ALTURA_NAVE 20
#define ESPACO_NAVES 30
#define VELOCIDADE_NAVES_INICIAL 1.5
#define DESCIDA_NAVES 12

#define LARGURA_TIRO 4
#define ALTURA_TIRO 12
#define VELOCIDADE_TIRO 8

#define LINHA_LIMITE_Y 450

float canhao_y = LINHA_LIMITE_Y - (ALTURA_CANHAO * 0.6);

typedef struct {
    float x, y;
    int ativa;
    int tipo; 
} Nave;

typedef struct {
    float x, y;
    int ativa;
} Tiro;

int colisao(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    return !(x1 > x2 + w2 || x1 + w1 < x2 || y1 > y2 + h2 || y1 + h1 < y2);
}

int ler_recorde() {
    FILE *f = fopen("recorde.txt", "r");
    if (!f) return 0;
    int r;
    if (fscanf(f, "%d", &r) != 1) r = 0; 
    fclose(f);
    return r;
}

void salvar_recorde(int r) {
    FILE *f = fopen("recorde.txt", "w");
    if (f) {
        fprintf(f, "%d", r); 
        fclose(f);
    }
}

int naves_vivas;

void iniciar_naves(Nave naves[MAX_LINHAS][MAX_COLUNAS], int *naves_vivas, int linhas, int colunas) {
    *naves_vivas = 0;
for (int i=0; i<linhas; i++) {
        for (int j=0; j<colunas; j++) {
            naves[i][j].x = 100 + j*(LARGURA_NAVE + ESPACO_NAVES);
            naves[i][j].y = 50 + i*(ALTURA_NAVE + ESPACO_NAVES);
            naves[i][j].ativa = 1;
            if (i == 0)
            naves[i][j].tipo = 2;
            else if (i <= 2)
            naves[i][j].tipo = 1;
            else
            naves[i][j].tipo = 0; 
		 (*naves_vivas)++;
        }
    }
}

int main() {
	
	

    al_init();
    al_install_keyboard();
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(5);
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();
	al_init_primitives_addon();


    ALLEGRO_DISPLAY *display = al_create_display(LARGURA_TELA, ALTURA_TELA);
    ALLEGRO_EVENT_QUEUE *fila = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
	
	ALLEGRO_BITMAP *img_canhao;
    ALLEGRO_BITMAP *img_nave10;
    ALLEGRO_BITMAP *img_nave20;
    ALLEGRO_BITMAP *img_nave30;


    ALLEGRO_FONT *fonte = al_load_ttf_font("arial.ttf", 20, 0);
    ALLEGRO_FONT *fonte_media = al_load_ttf_font("arial.ttf", 26, 0);
    ALLEGRO_FONT *fonte_grande = al_load_ttf_font("arial.ttf", 42, 0);

    ALLEGRO_BITMAP *img_fundo = al_load_bitmap("fundo_jogo.png");
    ALLEGRO_BITMAP *img_menu = al_load_bitmap("menu.png");
	
    img_canhao = al_load_bitmap("canhao.png");
    al_convert_mask_to_alpha(img_canhao, al_map_rgb(255, 255, 255));

    img_nave10 = al_load_bitmap("nave10.png");
    al_convert_mask_to_alpha(img_nave10, al_map_rgb(255, 255, 255));

    img_nave20 = al_load_bitmap("nave20.png");
    al_convert_mask_to_alpha(img_nave20, al_map_rgb(255, 255, 255));

    img_nave30 = al_load_bitmap("nave30.png");
    al_convert_mask_to_alpha(img_nave30, al_map_rgb(255, 255, 255));


    ALLEGRO_SAMPLE *som_tiro = al_load_sample("tiro.wav");
    ALLEGRO_SAMPLE *som_vitoria = al_load_sample("vitoria.wav");
    ALLEGRO_SAMPLE *som_derrota = al_load_sample("derrota.wav");
	ALLEGRO_SAMPLE *som_explosao = al_load_sample("explosao.wav");

    al_register_event_source(fila, al_get_display_event_source(display));
    al_register_event_source(fila, al_get_timer_event_source(timer));
    al_register_event_source(fila, al_get_keyboard_event_source());
	
    al_flush_event_queue(fila);
	
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_bitmap(img_menu, 0, 0, 0);
    al_draw_text(fonte_media, al_map_rgb(255,255,255), LARGURA_TELA/2, ALTURA_TELA - 50, ALLEGRO_ALIGN_CENTER, "ENTER para começar, ESC para sair");
    al_flip_display();

    int iniciar = 0;

    while (!iniciar) {
    ALLEGRO_EVENT ev;
    al_wait_for_event(fila, &ev);

    if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) return 0;

    if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
            iniciar = 1;
        }
        if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            return 0;
        }
    }
    al_flip_display();
}

    Nave naves[MAX_LINHAS][MAX_COLUNAS];
    float canhao_x = LARGURA_TELA/2 - LARGURA_CANHAO/2;
    int teclas[ALLEGRO_KEY_MAX] = {0};
    Tiro tiro = {0, 0, 0};
    float direcao = VELOCIDADE_NAVES_INICIAL;
    int indo_direita = 1;
    int pontuacao = 0;
    int recorde = ler_recorde();
    int fim = 0;
    int venceu = 0; 

    int contador = 20;
    //20 naves
    int linhas = 4;
    int colunas = 5;
    iniciar_naves(naves, &naves_vivas, linhas, colunas);
 

    al_start_timer(timer);
    int rodando = 1;
    while (rodando) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(fila, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER && fim == 0) {
            if (teclas[ALLEGRO_KEY_LEFT] && canhao_x > 0) canhao_x -= VELOCIDADE_CANHAO;
            if (teclas[ALLEGRO_KEY_RIGHT] && canhao_x < LARGURA_TELA - LARGURA_CANHAO) canhao_x += VELOCIDADE_CANHAO;

            int precisa_descer = 0;
            for (int i=0; i<linhas; i++) {
                for (int j=0; j<colunas; j++) {
                    if (!naves[i][j].ativa) continue;
                    naves[i][j].x += (indo_direita ? direcao : -direcao);
                    if ((indo_direita && naves[i][j].x + LARGURA_NAVE > LARGURA_TELA) ||
                        (!indo_direita && naves[i][j].x < 0)) precisa_descer = 1;
                }
            }
            if (precisa_descer) {
                indo_direita = !indo_direita;
                for (int i=0; i<linhas; i++)
                    for (int j=0; j<colunas; j++)
                        naves[i][j].y += DESCIDA_NAVES;
                direcao += 0.75;
            }

            if (tiro.ativa) {
                tiro.y -= VELOCIDADE_TIRO;
                if (tiro.y < 0) tiro.ativa = 0;
            }

            if (tiro.ativa) {
                for (int i=0; i<linhas; i++) {
                    for (int j=0; j<colunas; j++) {
                        if (naves[i][j].ativa && colisao(tiro.x, tiro.y, LARGURA_TIRO, ALTURA_TIRO, naves[i][j].x, naves[i][j].y, LARGURA_NAVE, ALTURA_NAVE)) {
                            naves[i][j].ativa = 0;
                            tiro.ativa = 0;
                            int pontos[] = {10, 20, 30};
							contador --;
							if(contador>=10){
                            pontuacao += pontos[naves[i][j].tipo];
							}
							if(contador<10){
								pontuacao += 2 * (pontos[naves[i][j].tipo]);
							}
							 if (som_explosao)
                            al_play_sample(som_explosao, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                        }
                    }
                }
            }

            for (int i=0; i<linhas; i++)
                for (int j=0; j<colunas; j++)
                    if (naves[i][j].ativa && naves[i][j].y + ALTURA_NAVE >= LINHA_LIMITE_Y) 
					fim = 1;
				  for (int i=0; i<linhas; i++) {
                     for (int j=0; j<colunas; j++) {
                        if (naves[i][j].ativa && colisao(
                            naves[i][j].x, naves[i][j].y, LARGURA_NAVE, ALTURA_NAVE,
                              canhao_x, canhao_y, LARGURA_CANHAO, ALTURA_CANHAO)) {
                                fim = 1;
        }
    }
}


            al_draw_bitmap(img_fundo, 0, 0, 0);
            al_draw_line(0, LINHA_LIMITE_Y, LARGURA_TELA, LINHA_LIMITE_Y, al_map_rgb(255,255,255), 2);
            al_draw_bitmap(img_canhao, canhao_x, canhao_y, 0);

            for (int i=0; i<linhas; i++) {
                for (int j=0; j<colunas; j++) {
                    if (!naves[i][j].ativa) continue;
                    ALLEGRO_BITMAP *img;
                    if (naves[i][j].tipo == 0) img = img_nave10;
                    else if (naves[i][j].tipo == 1) img = img_nave20;
                    else img = img_nave30;
                    al_draw_bitmap(img, naves[i][j].x, naves[i][j].y, 0);
                }
            }

            if (tiro.ativa) al_draw_filled_rectangle(tiro.x, tiro.y, tiro.x + LARGURA_TIRO, tiro.y + ALTURA_TIRO, al_map_rgb(255,255,255));

            char texto[64];
            sprintf(texto, "Pontuação: %d | Recorde: %d", pontuacao, recorde);
            al_draw_text(fonte_media, al_map_rgb(255,255,255), 10, 10, 0, texto);
            al_flip_display();
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            teclas[ev.keyboard.keycode] = 1;
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) rodando = 0;
            if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE && !tiro.ativa && !fim) {
                tiro.x = canhao_x + LARGURA_CANHAO/2 - LARGURA_TIRO/2;
                tiro.y = ALTURA_TELA - ALTURA_CANHAO - 10;
                tiro.ativa = 1;
                if (som_tiro) al_play_sample(som_tiro, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP) teclas[ev.keyboard.keycode] = 0;
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) rodando = 0;

        if (!fim) {
            int sobrou = 0;
            for (int i=0; i<linhas; i++)
                for (int j=0; j<colunas; j++)
                    if (naves[i][j].ativa) sobrou = 1;
				
            if (!sobrou) { 
			fim = 1; 
			venceu = 1; 
			}
			
        }

        if (fim) {
            if (pontuacao > recorde) { 
			salvar_recorde(pontuacao); 
			recorde = pontuacao; 
		}
            al_draw_bitmap(img_fundo, 0, 0, 0);
			
            if (venceu) {
                al_draw_text(fonte_grande, al_map_rgb(0,255,0), LARGURA_TELA/2, 200, ALLEGRO_ALIGN_CENTER, "Você venceu!");
                if (som_vitoria) al_play_sample(som_vitoria, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            } else {
                al_draw_text(fonte_grande, al_map_rgb(255,0,0), LARGURA_TELA/2, 200, ALLEGRO_ALIGN_CENTER, "Game Over!");
                if (som_derrota) al_play_sample(som_derrota, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            char msg[100];
            sprintf(msg, "Pontuação: %d | Recorde: %d", pontuacao, recorde);
            al_draw_text(fonte_media, al_map_rgb(255,255,255), LARGURA_TELA/2, 260, ALLEGRO_ALIGN_CENTER, msg);
            al_flip_display();
            al_rest(4);
            rodando = 0;
        }
    }
    
	

    al_destroy_bitmap(img_fundo);
    al_destroy_bitmap(img_menu);
    al_destroy_bitmap(img_canhao);
    al_destroy_bitmap(img_nave10);
    al_destroy_bitmap(img_nave20);
    al_destroy_bitmap(img_nave30);
    al_destroy_sample(som_tiro);
    al_destroy_sample(som_vitoria);
    al_destroy_sample(som_derrota);
    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(display);
    al_destroy_font(fonte);
    al_destroy_font(fonte_media);
    al_destroy_font(fonte_grande);
	al_destroy_sample(som_explosao);

    return 0;
} 