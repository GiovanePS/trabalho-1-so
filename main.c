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

// Constantes do Jogo
#define NUM_NAVES 10
#define NUM_FOGUETES 5
#define VELOCIDADE_NAVES 500000    // em microsegundos
#define VELOCIDADE_FOGUETES 200000 // em microsegundos

int tela_altura, tela_largura;

// Funções de Inicialização
void inicializa_jogo(EstadoJogo *jogo) {
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

// Função para Disparo de Foguetes
void *dispara_foguetes(void *arg) {
  EstadoJogo *jogo = (EstadoJogo *)arg;
  while (1) {
    pthread_mutex_lock(&jogo->mutex);
    if (jogo->foguetes_disponiveis > 0) {
      for (int i = 0; i < jogo->num_foguetes; i++) {
        if (!jogo->foguetes[i].ativa) {
          jogo->foguetes[i].ativa = 1;
          jogo->foguetes[i].x = rand() % tela_largura;
          jogo->foguetes[i].y = tela_altura - 1;
          jogo->foguetes_disponiveis--;
          break;
        }
      }
    }
    pthread_mutex_unlock(&jogo->mutex);
    usleep(VELOCIDADE_FOGUETES);
  }
  return NULL;
}

// Função para Atualizar a Interface do Jogo
void *atualiza_interface(void *arg) {
  EstadoJogo *jogo = (EstadoJogo *)arg;
  initscr();
  noecho();
  curs_set(FALSE);
  getmaxyx(stdscr, tela_altura, tela_largura);

  while (1) {
    clear();
    pthread_mutex_lock(&jogo->mutex);

    for (int i = 0; i < jogo->num_naves; i++) {
      if (jogo->naves[i].ativa) {
        mvprintw(jogo->naves[i].y, jogo->naves[i].x, "N");
      }
    }

    for (int i = 0; i < jogo->num_foguetes; i++) {
      if (jogo->foguetes[i].ativa) {
        mvprintw(jogo->foguetes[i].y, jogo->foguetes[i].x, "|");
      }
    }

    mvprintw(0, 0, "Foguetes Disponíveis: %d", jogo->foguetes_disponiveis);
    mvprintw(1, 0, "Naves Abatidas: %d", jogo->naves_abatidas);
    mvprintw(2, 0, "Naves Atingidas: %d", jogo->naves_atingidas);

    pthread_mutex_unlock(&jogo->mutex);
    refresh();
    usleep(100000);
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

  inicializa_jogo(&jogo);

  pthread_t thread_naves, thread_foguetes, thread_interface;

  pthread_create(&thread_naves, NULL, movimenta_naves, (void *)&jogo);
  pthread_create(&thread_foguetes, NULL, dispara_foguetes, (void *)&jogo);
  pthread_create(&thread_interface, NULL, atualiza_interface, (void *)&jogo);

  pthread_join(thread_naves, NULL);
  pthread_join(thread_foguetes, NULL);
  pthread_join(thread_interface, NULL);

  pthread_mutex_destroy(&jogo.mutex);
  free(jogo.naves);
  free(jogo.foguetes);

  return 0;
}

