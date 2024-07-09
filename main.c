#include <linux/limits.h>
#include <ncurses.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  int x, y;
  int ativa;
  pthread_mutex_t mutex;
} Nave;

typedef struct {
  int x, y;
  int direcao;
  int ativo;
  pthread_mutex_t mutex;
} Foguete;

typedef struct {
  int x, y;
  int direcao;
  pthread_mutex_t mutex;
} Torre;

typedef struct {
  Torre *torre;
} entrada_args;

Torre *torre;

int NUM_NAVES;
int NUM_FOGUETES;

int foguetes_disponiveis;
pthread_mutex_t mutex_foguetes_disponiveis;

int naves_abatidas = 0;
pthread_mutex_t mutex_naves_abatidas;

int naves_atingidas = 0;
pthread_mutex_t mutex_naves_atingidas;

int VELOCIDADE_NAVES = 500000;
int VELOCIDADE_FOGUETES = 20000;

int tela_altura, tela_largura;

pthread_t *naves_threads;
Nave *naves;
pthread_t *foguetes_threads;
Foguete *foguetes;

void inicializa_jogo() {
  naves_threads = (pthread_t *)malloc(NUM_NAVES * sizeof(pthread_t));
  naves = (Nave *)malloc(NUM_NAVES * sizeof(Nave));
  for (int i = 0; i < NUM_NAVES; i++) {
    naves[i] = *(Nave *)malloc(sizeof(Nave));
  }

  foguetes_threads = (pthread_t *)malloc(NUM_FOGUETES * sizeof(pthread_t));
  foguetes = (Foguete *)malloc(NUM_FOGUETES * sizeof(Foguete));
  for (int i = 0; i < NUM_FOGUETES; i++) {
    foguetes[i] = *(Foguete *)malloc(sizeof(Foguete));
  }
  foguetes_disponiveis = NUM_FOGUETES;
}

// int todas_naves_morreram(EstadoJogo *jogo) {
//   pthread_mutex_lock(&jogo->mutex);
//   int all_dead = 1;
//   for (int i = 0; i < jogo->num_naves; i++) {
//     if (jogo->naves[i].ativa) {
//       all_dead = 0;
//       break;
//     }
//   }
//   pthread_mutex_unlock(&jogo->mutex);
//   return all_dead;
// }

// void verifica_fim_jogo(EstadoJogo *jogo) {
//   pthread_mutex_lock(&jogo->mutex);
//   int naves_abatidas = jogo->naves_abatidas;
//   int naves_atingidas = jogo->naves_atingidas;
//   int total_naves = jogo->num_naves;
//   pthread_mutex_unlock(&jogo->mutex);
//
//   if (naves_abatidas >= total_naves / 2 && todas_naves_morreram(jogo)) {
//     // Vitória do jogador
//     clear();
//     mvprintw(tela_altura / 2, tela_largura / 2 - 5, "Vitória!");
//     refresh();
//     usleep(2000000); // Pausa para exibir a mensagem
//     endwin();
//     exit(0);
//   } else if (naves_atingidas >= total_naves / 2) {
//     // Derrota do jogador
//     clear();
//     mvprintw(tela_altura / 2, tela_largura / 2 - 5, "Derrota!");
//     refresh();
//     usleep(2000000); // Pausa para exibir a mensagem
//     endwin();
//     exit(0);
//   }
// }

void *movimenta_nave(void *arg) {
  Nave *nave = (Nave *)arg;
  bool fim_nave = false;
  while (!fim_nave) {
    pthread_mutex_lock(&nave->mutex);
    nave->y++;
    if (nave->y >= tela_altura) {
      nave->ativa = false;
      fim_nave = true;
      pthread_mutex_lock(&mutex_naves_atingidas);
      naves_atingidas++;
      pthread_mutex_unlock(&mutex_naves_atingidas);
    }
    pthread_mutex_unlock(&nave->mutex);
    usleep(VELOCIDADE_NAVES);
  }
  return NULL;
}

void *criador_de_naves() {
  for (int i = 0; i < NUM_NAVES; i++) {
    int x = rand() % (tela_largura - 6);
    int y = 0;

    pthread_t thread_nave = naves_threads[i];
    Nave *nave = &naves[i];
    pthread_mutex_init(&nave->mutex, NULL);
    nave->x = x;
    nave->y = y;
    nave->ativa = true;
    pthread_create(&thread_nave, NULL, movimenta_nave, (void *)nave);
    pthread_detach(thread_nave);
    usleep(10000); // Tempo de espera até a criação da próxima nave
  }

  return NULL;
}

typedef struct {
  Foguete *foguete;
  int direcao;
} disparo_args;

// void *verifica_colisao(void *arg) {
//   EstadoJogo *jogo = (EstadoJogo *)arg;
//   while (1) {
//     for (int i = 0; i < NUM_NAVES; i++) {
//       pthread_mutex_lock(&jogo->mutex);
//       Nave nave = jogo->naves[i];
//       pthread_mutex_unlock(&jogo->mutex);
//
//       if (!nave.ativa) {
//         continue;
//       }
//
//       for (int j = 0; j < NUM_FOGUETES; j++) {
//         pthread_mutex_lock(&jogo->mutex);
//         Foguete foguete = jogo->foguetes[j];
//         pthread_mutex_unlock(&jogo->mutex);
//
//         if (!foguete.ativa) {
//           continue;
//         }
//
//         if (foguete.x >= nave.x && foguete.x <= nave.x + 6 &&
//             foguete.y == nave.y) {
//           pthread_mutex_lock(&jogo->mutex);
//           jogo->naves[i].ativa = 0;
//           jogo->foguetes[j].ativa = 0;
//           jogo->naves_abatidas++;
//           pthread_mutex_unlock(&jogo->mutex);
//         }
//       }
//     }
//
//     usleep(10000);
//   }
// }
//

// void *movimenta_foguete_torre(void *arg) {
//   disparo_args *args = (disparo_args *)arg;
//   Foguete *foguete = (Foguete *)args->foguete;
//   int direcao = (int)args->direcao;
//   foguete->direcao = direcao;
//   while (foguete->y > 0 && foguete->x > 0 && foguete->x < tela_largura) {
//     if (direcao == 0) {
//       foguete->x--;
//     } else if (direcao == 1) {
//       foguete->y--;
//       foguete->x -= 2;
//     } else if (direcao == 2) {
//       foguete->y--;
//     } else if (direcao == 3) {
//       foguete->y--;
//       foguete->x += 2;
//     } else {
//       foguete->x++;
//     }
//     usleep(VELOCIDADE_FOGUETES);
//   }
//   foguete->ativa = 0;
//   return NULL;
// }

// Função para a Torre Disparar Foguetes
// void torre_dispara(EstadoJogo *jogo, Torre *torre) {
//   disparo_args *args = (disparo_args *)malloc(sizeof(disparo_args));
//   pthread_mutex_lock(&jogo->mutex);
//   if (jogo->foguetes_disponiveis > 0) {
//     for (int i = 0; i < jogo->num_foguetes; i++) {
//       if (!jogo->foguetes[i].ativa) {
//         jogo->foguetes[i].ativa = 1;
//         jogo->foguetes[i].x = torre->x;
//         jogo->foguetes[i].y = torre->y - 1;
//
//         // Cria uma thread para mover o foguete da torre
//         pthread_t thread_foguete;
//         args->foguete = &jogo->foguetes[i];
//         args->direcao = torre->direcao;
//         pthread_create(&thread_foguete, NULL, movimenta_foguete_torre,
//                        (void *)args);
//         pthread_detach(thread_foguete);
//
//         jogo->foguetes_disponiveis--;
//         break;
//       }
//     }
//   }
//   pthread_mutex_unlock(&jogo->mutex);
// }

// void recarrega_torre(EstadoJogo *jogo, Torre *torre) {
//   pthread_mutex_lock(&jogo->mutex);
//
//   if (jogo->foguetes_disponiveis < NUM_FOGUETES) {
//     jogo->foguetes_disponiveis++;
//   }
//
//   pthread_mutex_unlock(&jogo->mutex);
// }

// Função para Capturar Entrada do Usuário
void *captura_entrada(void *arg) {
  Torre *torre = (Torre *)arg;
  while (1) {
    int ch = getch();
    // if (ch == ' ') {
    //   torre_dispara(jogo, torre);
    // }

    // if (ch == 'r') {
    //   recarrega_torre(jogo, torre);
    // }

    pthread_mutex_lock(&torre->mutex);
    if (ch == 'a' && torre->direcao != 0) {
      torre->direcao -= 1;
    } else if (ch == 'd' && torre->direcao != 4) {
      torre->direcao += 1;
    }
    pthread_mutex_unlock(&torre->mutex);

    usleep(100000);
  }
  return NULL;
}

// Função para Atualizar a Interface do Jogo
void *atualiza_interface(void *arg) {
  Torre *torre = (Torre *)arg;
  initscr();
  noecho();
  curs_set(FALSE);
  timeout(0);
  getmaxyx(stdscr, tela_altura, tela_largura);

  while (1) {
    clear();
    for (int i = 0; i < NUM_NAVES; i++) {
      pthread_mutex_lock(&naves[i].mutex);
      if (naves[i].ativa) {
        mvprintw(naves[i].y, naves[i].x, "<<(o)>>");
      }
      pthread_mutex_unlock(&naves[i].mutex);
    }

    for (int i = 0; i < NUM_FOGUETES; i++) {
      pthread_mutex_lock(&foguetes[i].mutex);
      Foguete foguete = foguetes[i];
      if (foguetes[i].ativo) {
        pthread_mutex_lock(&torre->mutex);
        if (foguete.direcao == 0 || foguete.direcao == 4) {
          mvprintw(foguete.y, foguete.x, "__");
        } else if (foguete.direcao == 1) {
          mvprintw(foguete.y, foguete.x, "\\");
        } else if (foguete.direcao == 2) {
          mvprintw(foguete.y, foguete.x, "|");
        } else if (foguete.direcao == 3) {
          mvprintw(foguete.y, foguete.x, "/");
        }
        pthread_mutex_unlock(&torre->mutex);
      }
      pthread_mutex_unlock(&foguetes[i].mutex);
    }

    // Desenha a torre
    int torre_x = tela_largura / 2;
    pthread_mutex_lock(&torre->mutex);
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
    pthread_mutex_unlock(&torre->mutex);

    mvprintw(tela_altura - 4, torre_x - 1, "/|\\");
    mvprintw(tela_altura - 3, torre_x - 2, "/_|_\\");
    mvprintw(tela_altura - 2, torre_x - 2, " | | ");
    mvprintw(tela_altura - 1, torre_x - 2, " | | ");

    pthread_mutex_lock(&mutex_foguetes_disponiveis);
    mvprintw(0, 0, "Foguetes Disponíveis: %d", foguetes_disponiveis);
    pthread_mutex_unlock(&mutex_foguetes_disponiveis);

    pthread_mutex_lock(&mutex_naves_abatidas);
    mvprintw(1, 0, "Naves Abatidas: %d", naves_abatidas);
    pthread_mutex_unlock(&mutex_naves_abatidas);

    pthread_mutex_lock(&mutex_naves_atingidas);
    mvprintw(2, 0, "Naves Atingidas: %d", naves_atingidas);
    pthread_mutex_unlock(&mutex_naves_atingidas);

    refresh();

    // verifica_fim_jogo(jogo);

    usleep(100000);
  }

  endwin();
  return NULL;
}

int main() {
  srand(time(NULL));

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  printw("Escolha o nível de dificuldade:\n");
  printw("[1] Fácil\n");
  printw("[2] Médio\n");
  printw("[3] Difícil\n");
  int input = getch();
  switch (input) {
  case '1':
    NUM_NAVES = 5;
    NUM_FOGUETES = 10;
    VELOCIDADE_NAVES = 1000000;
    break;
  case '2':
    NUM_NAVES = 10;
    NUM_FOGUETES = 5;
    VELOCIDADE_NAVES = 500000;
    break;
  case '3':
    NUM_NAVES = 15;
    NUM_FOGUETES = 3;
    VELOCIDADE_NAVES = 200000;
    break;
  default:
    printw("Opção inválida. Saindo...\n");
    endwin();
    return 1;
  };

  getmaxyx(stdscr, tela_altura, tela_largura);
  endwin(); // End ncurses mode so we can safely initialize game

  torre = (Torre *)malloc(sizeof(Torre));
  torre->x = tela_largura / 2;
  torre->y = tela_altura - 5;
  torre->direcao = 2;
  pthread_mutex_init(&torre->mutex, NULL);

  inicializa_jogo();

  pthread_t thread_interface, thread_criador_de_naves, thread_inputs;

  pthread_create(&thread_criador_de_naves, NULL, criador_de_naves, NULL);
  pthread_create(&thread_interface, NULL, atualiza_interface, (void *)torre);
  pthread_create(&thread_inputs, NULL, captura_entrada, (void *)torre);
  // pthread_create(&thread_colisao, NULL, verifica_colisao, (void *)&jogo);

  pthread_join(thread_criador_de_naves, NULL);
  pthread_join(thread_interface, NULL);
  pthread_join(thread_inputs, NULL);
  // pthread_join(thread_colisao, NULL);

  return 0;
}
