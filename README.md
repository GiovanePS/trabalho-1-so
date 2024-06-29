# Projeto de criação de um jogo para a matéria de Sistemas Operacionais (INE 5611-2024.01) do curso de Sistemas de Informação.

### O Objetivo aqui é a criação de um jogo multithreads em C, em que as regras já estão definidas na postagem do trabalho, no qual colocarei abaixo:

Uma bateria antiaérea fixa, com seu lançador guiado pelo jogador, deve impedir que uma invasão alienígena à Terra se concretize. Para tal, o jogador deve tentar abater as naves alienígenas que se deslocam do topo da tela em direção ao solo. O lançador tem cinco posições para disparo:  vertical (90 graus |), diagonal para a esquerda (45 graus \), diagonal para a direita (45 graus /), horizontal para a esquerda (180 graus __) e horizontal para a direita (180 graus __). O jogador controla a posição do lançador com teclas a serem definidas no programa. O disparo é feito pressionado a tecla de espaço.

A bateria antiaérea tem capacidade para armazenar k foguetes, onde k é um dos parâmetros do grau de dificuldade do jogo. Quando os foguetes se esgotam, ela não dispara mais e deve ser carregada. A interface do jogo deve mostrar em algum canto da tela a quantidade disponível de foguetes. O jogador pode acionar alguma tecla para iniciar a recarga a qualquer momento e pode continuar disparando enquanto a bateria é recarregada.

As naves começam a descida das posições (x, 0), onde x é um valor definido aleatoriamente em tempo real. Elas descem apenas verticalmente,  em velocidade constante, não mudando de posição x nem subindo (diminuindo y). A velocidade de descida é outro parâmetro do grau de dificuldade do jogo.

A quantidade de naves é m, um terceiro parâmetro do grau de dificuldade do jogo.

O jogo termina numa das duas situações abaixo:

a bateria abateu ao menos 50% das naves (vitória do jogador);
mais de 50% das naves chegaram no solo  (derrota do jogador).
 O jogo tem 3 graus de dificuldade: fácil, médio e difícil. Os projetistas sintonizam os valores dos parâmetros arbitrariamente para definir esses graus.

 ### Integrantes do trabalho:
- Giovane Pimentel de Sousa (22202685)
- Isabela Vill de Aquino (22201632)
- Guilherme Henriques do Carmo (22201630)

## Como excecutar:

### 1. Clone o repositorio:
`git clone https://github.com/GiovanePS/space-defenders.git`

### 2. Compile com CMake:
```
gcc -o main main.c -lpthread -lncurses
```

### 3. Execute:
`./main`

## Dependências:

### Foi utilizada a biblioteca ncurses para apoio na criação da interface do jogo.

### Para instalar, execute:
`sudo apt-get install libncurses5-dev libncursesw5-dev`
