#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Estruturas de Dados
typedef struct {
  int x, y;
  int ativa;
} Nave;

typedef struct {
  int x, y;
  int direcao;
  int ativa;
} Foguete;

typedef struct {
  Nave *naves;
  Foguete *foguetes;
  int num_naves;
  int num_foguetes;
  int foguetes_disponiveis;
  int naves_abatidas;
  int naves_atingidas;
  pthread_mutex_t mutex;
} EstadoJogo;

// Estrutura da Torre
typedef struct {
  int x, y;
  int direcao;
} Torre;

typedef struct {
  EstadoJogo *jogo;
  Torre *torre;
} entrada_args;

// Constantes do Jogo
#define NUM_NAVES 10
#define NUM_FOGUETES 5
#define VELOCIDADE_NAVES 500000   // em microsegundos
#define VELOCIDADE_FOGUETES 20000 // em microsegundos

int tela_altura, tela_largura;

// Funções de Inicialização
void inicializa_jogo(EstadoJogo *jogo, Torre *torre) {
  jogo->naves = (Nave *)malloc(NUM_NAVES * sizeof(Nave));
  jogo->foguetes = (Foguete *)malloc(NUM_FOGUETES * sizeof(Foguete));
  jogo->num_naves = NUM_NAVES;
  jogo->num_foguetes = NUM_FOGUETES;
  jogo->foguetes_disponiveis = NUM_FOGUETES;
  jogo->naves_abatidas = 0;
  jogo->naves_atingidas = 0;
  pthread_mutex_init(&jogo->mutex, NULL);

  for (int i = 0; i < NUM_NAVES; i++) {
    jogo->naves[i].x = rand() % tela_largura;
    jogo->naves[i].y = 0;
    jogo->naves[i].ativa = 1;
  }

  for (int i = 0; i < NUM_FOGUETES; i++) {
    jogo->foguetes[i].ativa = 0;
  }

  // Inicializa a torre no centro da tela
  torre->x = tela_largura / 2;
  torre->y = tela_altura - 5; // Ajusta para o tamanho da torre
}

// Função para Movimentação das Naves
void *movimenta_naves(void *arg) {
  EstadoJogo *jogo = (EstadoJogo *)arg;
  while (1) {
    pthread_mutex_lock(&jogo->mutex);
    for (int i = 0; i < jogo->num_naves; i++) {
      if (jogo->naves[i].ativa) {
        jogo->naves[i].y++;
        if (jogo->naves[i].y >= tela_altura) {
          jogo->naves[i].ativa = 0;
          jogo->naves_atingidas++;
        }
      }
    }
    pthread_mutex_unlock(&jogo->mutex);
    usleep(VELOCIDADE_NAVES);
  }
  return NULL;
}

typedef struct {
  Foguete *foguete;
  int direcao;
} disparo_args;

// Função para Movimentar um Foguete Disparado pela Torre
void *movimenta_foguete_torre(void *arg) {
  disparo_args *args = (disparo_args *)arg;
  Foguete *foguete = (Foguete *)args->foguete;
  int direcao = (int)args->direcao;
  foguete->direcao = direcao;
  while (foguete->y > 0 && foguete->x > 0 && foguete->x < tela_largura) {
    if (direcao == 0) {
      foguete->x--;
    } else if (direcao == 1) {
      foguete->y--;
      foguete->x -= 2;
    } else if (direcao == 2) {
      foguete->y--;
    } else if (direcao == 3) {
      foguete->y--;
      foguete->x += 2;
    } else {
      foguete->x++;
    }
    usleep(VELOCIDADE_FOGUETES);
  }
  foguete->ativa = 0;
  return NULL;
}

// Função para a Torre Disparar Foguetes
void torre_dispara(EstadoJogo *jogo, Torre *torre) {
  disparo_args *args = (disparo_args *)malloc(sizeof(disparo_args));
  pthread_mutex_lock(&jogo->mutex);
  if (jogo->foguetes_disponiveis > 0) {
    for (int i = 0; i < jogo->num_foguetes; i++) {
      if (!jogo->foguetes[i].ativa) {
        jogo->foguetes[i].ativa = 1;
        jogo->foguetes[i].x = torre->x;
        jogo->foguetes[i].y = torre->y - 1;

        // Cria uma thread para mover o foguete da torre
        pthread_t thread_foguete;
        args->foguete = &jogo->foguetes[i];
        args->direcao = torre->direcao;
        pthread_create(&thread_foguete, NULL, movimenta_foguete_torre,
                       (void *)args);
        pthread_detach(thread_foguete);

        jogo->foguetes_disponiveis--;
        break;
      }
    }
  }
  pthread_mutex_unlock(&jogo->mutex);
}

void recarrega_torre(EstadoJogo *jogo, Torre *torre) {
  pthread_mutex_lock(&jogo->mutex);
  jogo->foguetes_disponiveis++;
  pthread_mutex_unlock(&jogo->mutex);
}

// Função para Capturar Entrada do Usuário
void *captura_entrada(void *arg) {
  entrada_args *args = (entrada_args *)arg;
  EstadoJogo *jogo = (EstadoJogo *)args->jogo;
  Torre *torre = (Torre *)args->torre;
  while (1) {
    int ch = getch();
    if (ch == ' ') {
      torre_dispara(jogo, torre);
    }

    if (ch == 'r') {
      recarrega_torre(jogo, torre);
    }

    if (ch == 'a' && torre->direcao != 0) {
      torre->direcao -= 1;
    } else if (ch == 'd' && torre->direcao != 4) {
      torre->direcao += 1;
    }

    usleep(100000);
  }
  return NULL;
}

// Função para Atualizar a Interface do Jogo
void *atualiza_interface(void *arg) {
  entrada_args *args = (entrada_args *)arg;
  EstadoJogo *jogo = (EstadoJogo *)args->jogo;
  Torre *torre = (Torre *)args->torre;
  initscr();
  noecho();
  curs_set(FALSE);
  timeout(0);
  getmaxyx(stdscr, tela_altura, tela_largura);

  while (1) {
    clear();
    pthread_mutex_lock(&jogo->mutex);

    for (int i = 0; i < jogo->num_naves; i++) {
      if (jogo->naves[i].ativa) {
        mvprintw(jogo->naves[i].y, jogo->naves[i].x, "<<(o)>>");
      }
    }

    for (int i = 0; i < jogo->num_foguetes; i++) {
      Foguete foguete = jogo->foguetes[i];
      if (foguete.ativa) {
        if (foguete.direcao == 0 || foguete.direcao == 4) {
          mvprintw(foguete.y, foguete.x, "__");
        } else if (foguete.direcao == 1) {
          mvprintw(foguete.y, foguete.x, "\\");
        } else if (foguete.direcao == 2) {
          mvprintw(foguete.y, foguete.x, "|");
        } else if (foguete.direcao == 3) {
          mvprintw(foguete.y, foguete.x, "/");
        }
      }
    }

    // Desenha a torre
    int torre_x = tela_largura / 2;
    if (torre->direcao == 0) {
      mvprintw(tela_altura - 5, torre_x - 3, "____");
    } else if (torre->direcao == 1) {
      mvprintw(tela_altura - 6, torre_x - 1, "\\  ");
      mvprintw(tela_altura - 5, torre_x - 1, " \\ ");
    } else if (torre->direcao == 2) {
      mvprintw(tela_altura - 6, torre_x - 1, " | ");
      mvprintw(tela_altura - 5, torre_x - 1, " | ");
    } else if (torre->direcao == 3) {
      mvprintw(tela_altura - 6, torre_x - 1, "  /");
      mvprintw(tela_altura - 5, torre_x - 1, " / ");
    } else {
      mvprintw(tela_altura - 5, torre_x, "____");
    }

    mvprintw(tela_altura - 4, torre_x - 1, "/|\\");
    mvprintw(tela_altura - 3, torre_x - 2, "/_|_\\");
    mvprintw(tela_altura - 2, torre_x - 2, " | | ");
    mvprintw(tela_altura - 1, torre_x - 2, " | | ");

    mvprintw(0, 0, "Foguetes Disponíveis: %d", jogo->foguetes_disponiveis);
    mvprintw(1, 0, "Naves Abatidas: %d", jogo->naves_abatidas);
    mvprintw(2, 0, "Naves Atingidas: %d", jogo->naves_atingidas);

    pthread_mutex_unlock(&jogo->mutex);
    refresh();

    usleep(100000); // Sleep to avoid high CPU usage
  }

  endwin();
  return NULL;
}

// Função Principal
int main() {
  srand(time(NULL));
  EstadoJogo jogo;

  initscr();
  getmaxyx(stdscr, tela_altura, tela_largura);
  endwin(); // End ncurses mode so we can safely initialize game

  Torre torre = {tela_largura / 2, tela_altura - 4, 2};

  inicializa_jogo(&jogo, &torre);

  pthread_t thread_naves, thread_interface, thread_entrada;

  entrada_args args = {&jogo, &torre};
  pthread_create(&thread_naves, NULL, movimenta_naves, (void *)&jogo);
  pthread_create(&thread_interface, NULL, atualiza_interface, (void *)&args);
  pthread_create(&thread_entrada, NULL, captura_entrada, (void *)&args);

  pthread_join(thread_naves, NULL);
  pthread_join(thread_interface, NULL);
  pthread_join(thread_entrada, NULL);

  pthread_mutex_destroy(&jogo.mutex);
  free(jogo.naves);
  free(jogo.foguetes);

  return 0;
}
