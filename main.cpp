#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <iostream>
#include <algorithm>  // Pour std::none_of
#include <string>     // Pour std::to_string

// Constantes du jeu
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 20;
const int BALL_SIZE = 15;
const int BRICK_WIDTH = 80;
const int BRICK_HEIGHT = 30;
const int MAX_LEVELS = 3;  // Nombre total de niveaux

// Structure pour les objets du jeu
struct GameObject {
    SDL_FRect rect;  // Utilisation de SDL_FRect pour plus de précision
    bool active = true;
    SDL_Color color = {255, 255, 255, 255};
};

class Game {
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    GameObject paddle;
    GameObject ball;
    std::vector<GameObject> bricks;
    bool running = true;
    float ballSpeedX = 5.0f;
    float ballSpeedY = -5.0f;
    int score = 0;
    bool gameWon = false;
    TTF_Font* font = nullptr;
    int currentLevel = 1;

    bool allBricksDestroyed() const {
        return std::none_of(bricks.begin(), bricks.end(), 
            [](const GameObject& brick) { return brick.active; });
    }

    void renderText(const std::string& text, int x, int y, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {x, y, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, nullptr, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }

public:
    bool initialize() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cout << "Erreur SDL: " << SDL_GetError() << std::endl;
            return false;
        }

        if (TTF_Init() < 0) {
            std::cout << "Erreur TTF: " << TTF_GetError() << std::endl;
            return false;
        }

        // Chargement de la police
        font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 24);
        if (!font) {
            std::cout << "Erreur de chargement de la police: " << TTF_GetError() << std::endl;
            return false;
        }

        window = SDL_CreateWindow("Casse-Briques", 
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) return false;

        renderer = SDL_CreateRenderer(window, -1, 
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) return false;

        // Initialisation de la raquette
        paddle.rect = {
            WINDOW_WIDTH/2.0f - PADDLE_WIDTH/2.0f,
            WINDOW_HEIGHT - PADDLE_HEIGHT - 10.0f,
            static_cast<float>(PADDLE_WIDTH),
            static_cast<float>(PADDLE_HEIGHT)
        };

        // Initialisation de la balle
        resetBall();

        // Création des briques
        createBricks();

        return true;
    }

    void resetBall() {
        ball.rect = {
            WINDOW_WIDTH/2.0f - BALL_SIZE/2.0f,
            WINDOW_HEIGHT/2.0f - BALL_SIZE/2.0f,
            static_cast<float>(BALL_SIZE),
            static_cast<float>(BALL_SIZE)
        };
        ballSpeedX = 5.0f;
        ballSpeedY = -5.0f;
    }

    void createBricks() {
        bricks.clear();
        
        switch(currentLevel) {
            case 1:
                createLevel1();
                break;
            case 2:
                createLevel2();
                break;
            case 3:
                createLevel3();
                break;
        }
    }

    void createLevel1() {
        SDL_Color colors[] = {
            {255, 0, 0, 255},    // Rouge
            {0, 255, 0, 255},    // Vert
            {0, 0, 255, 255},    // Bleu
            {255, 255, 0, 255},  // Jaune
            {255, 0, 255, 255}   // Magenta
        };

        // Configuration standard (5 rangées de 8 briques)
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 8; j++) {
                GameObject brick;
                brick.rect = {
                    j * (BRICK_WIDTH + 2.0f) + 60.0f,
                    i * (BRICK_HEIGHT + 2.0f) + 50.0f,
                    static_cast<float>(BRICK_WIDTH),
                    static_cast<float>(BRICK_HEIGHT)
                };
                brick.color = colors[i];
                bricks.push_back(brick);
            }
        }
    }

    void createLevel2() {
        SDL_Color colors[] = {
            {255, 165, 0, 255},   // Orange
            {128, 0, 128, 255},   // Violet
            {0, 255, 255, 255},   // Cyan
        };

        // Configuration en pyramide ajustée
        int startX = (WINDOW_WIDTH - (12 * (BRICK_WIDTH + 2))) / 2;  // Centrer horizontalement
        
        for (int i = 0; i < 6; i++) {
            for (int j = i; j < 12 - i; j++) {
                GameObject brick;
                brick.rect = {
                    startX + j * (BRICK_WIDTH + 2.0f),
                    i * (BRICK_HEIGHT + 2.0f) + 50.0f,  // Démarrer un peu plus bas
                    static_cast<float>(BRICK_WIDTH),
                    static_cast<float>(BRICK_HEIGHT)
                };
                brick.color = colors[i % 3];
                bricks.push_back(brick);
            }
        }
        // Augmenter la vitesse de la balle
        ballSpeedX = 6.0f;
        ballSpeedY = -6.0f;
    }

    void createLevel3() {
        SDL_Color colors[] = {
            {255, 0, 0, 255},    // Rouge
            {255, 255, 255, 255}, // Blanc
            {0, 0, 255, 255},    // Bleu
        };

        // Configuration en motif alterné
        for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 10; j++) {
                if ((i + j) % 2 == 0) {  // Crée un motif en damier
                    GameObject brick;
                    brick.rect = {
                        j * (BRICK_WIDTH + 2.0f) + 40.0f,
                        i * (BRICK_HEIGHT + 2.0f) + 30.0f,
                        static_cast<float>(BRICK_WIDTH),
                        static_cast<float>(BRICK_HEIGHT)
                    };
                    brick.color = colors[i % 3];
                    bricks.push_back(brick);
                }
            }
        }
        // Augmenter encore la vitesse
        ballSpeedX = 7.0f;
        ballSpeedY = -7.0f;
    }

    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        const Uint8* state = SDL_GetKeyboardState(nullptr);
        float paddleSpeed = 7.0f;
        
        if (state[SDL_SCANCODE_LEFT] && paddle.rect.x > 0) {
            paddle.rect.x -= paddleSpeed;
        }
        if (state[SDL_SCANCODE_RIGHT] && paddle.rect.x < WINDOW_WIDTH - paddle.rect.w) {
            paddle.rect.x += paddleSpeed;
        }
    }

    void update() {
        ball.rect.x += ballSpeedX;
        ball.rect.y += ballSpeedY;

        // Collisions avec les murs
        if (ball.rect.x <= 0 || ball.rect.x >= WINDOW_WIDTH - ball.rect.w) {
            ballSpeedX = -ballSpeedX;
        }
        if (ball.rect.y <= 0) {
            ballSpeedY = -ballSpeedY;
        }
        if (ball.rect.y >= WINDOW_HEIGHT) {
            resetBall();
            score = std::max(0, score - 100); // Pénalité quand la balle est perdue
        }

        // Collision avec la raquette
        SDL_FRect paddleRect = paddle.rect;
        if (checkCollision(ball.rect, paddleRect)) {
            ballSpeedY = -ballSpeedY;
            // Ajout d'un effet selon la position de collision sur la raquette
            float relativeIntersectX = (paddle.rect.x + (paddle.rect.w/2)) - (ball.rect.x + (ball.rect.w/2));
            float normalizedIntersect = relativeIntersectX / (paddle.rect.w/2);
            ballSpeedX = -normalizedIntersect * 5.0f;
        }

        // Collisions avec les briques
        for (auto& brick : bricks) {
            if (brick.active && checkCollision(ball.rect, brick.rect)) {
                brick.active = false;
                ballSpeedY = -ballSpeedY;
                score += 10;
            }
        }

        // Vérifier si toutes les briques sont détruites
        if (allBricksDestroyed() && !gameWon) {
            gameWon = true;
            SDL_Delay(100);  // Petit délai pour éviter les problèmes de timing
            askForReplay();
        }
    }

    bool checkCollision(const SDL_FRect& a, const SDL_FRect& b) {
        return (a.x < b.x + b.w &&
                a.x + a.w > b.x &&
                a.y < b.y + b.h &&
                a.y + a.h > b.y);
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Raquette
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_FRect paddleRect = paddle.rect;
        SDL_RenderFillRectF(renderer, &paddleRect);

        // Balle
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_FRect ballRect = ball.rect;
        SDL_RenderFillRectF(renderer, &ballRect);

        // Briques
        for (const auto& brick : bricks) {
            if (brick.active) {
                SDL_SetRenderDrawColor(renderer, 
                    brick.color.r, brick.color.g, brick.color.b, brick.color.a);
                SDL_FRect brickRect = brick.rect;
                SDL_RenderFillRectF(renderer, &brickRect);
            }
        }

        // Affichage du score et du niveau
        std::string scoreText = "Score: " + std::to_string(score);
        std::string levelText = "Niveau: " + std::to_string(currentLevel);
        SDL_Color textColor = {255, 255, 255, 255};
        renderText(scoreText, 10, 10, textColor);
        renderText(levelText, WINDOW_WIDTH - 150, 10, textColor);

        // Afficher les messages de niveau
        if (gameWon) {
            SDL_Color textColor = {255, 255, 255, 255};
            if (currentLevel < MAX_LEVELS) {
                renderText("Niveau " + std::to_string(currentLevel) + " terminé !", 
                    WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT/2 - 60, textColor);
                renderText("ESPACE: Niveau suivant  R: Recommencer  Q: Quitter", 
                    WINDOW_WIDTH/2 - 250, WINDOW_HEIGHT/2, textColor);
            }
        }

        SDL_RenderPresent(renderer);
    }

    void gameLoop() {
        while (running) {
            handleEvents();
            update();
            render();
        }
    }

    void cleanup() {
        if (font) {
            TTF_CloseFont(font);
        }
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void askForReplay() {
        SDL_Event event;
        bool waiting = true;
        gameWon = true;  // S'assurer que gameWon est true
        
        while (waiting) {
            // Rendre l'écran pendant l'attente
            render();
            
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        case SDLK_SPACE:
                            if (currentLevel < MAX_LEVELS) {
                                nextLevel();
                                waiting = false;
                            }
                            break;
                        case SDLK_r:
                            resetGame();
                            waiting = false;
                            break;
                        case SDLK_q:
                            running = false;
                            waiting = false;
                            break;
                    }
                }
                else if (event.type == SDL_QUIT) {
                    running = false;
                    waiting = false;
                }
            }
        }
    }

    void nextLevel() {
        currentLevel++;
        resetBall();
        createBricks();
        gameWon = false;
    }

    void resetGame() {
        score = 0;
        currentLevel = 1;
        gameWon = false;
        createBricks();
        resetBall();
        paddle.rect.x = WINDOW_WIDTH/2.0f - PADDLE_WIDTH/2.0f;
    }
};

int main(int argc, char* argv[]) {
    Game game;
    
    if (!game.initialize()) {
        std::cout << "Erreur d'initialisation du jeu!" << std::endl;
        return 1;
    }

    game.gameLoop();
    game.cleanup();

    return 0;
}
