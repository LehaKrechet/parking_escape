#include <SDL2/SDL.h>          // Основная библиотека SDL для работы с графикой, звуком и вводом
#include <SDL2/SDL_image.h>    // Дополнение SDL для работы с изображениями
#include <SDL2/SDL_ttf.h>      // Дополнение SDL для работы с шрифтами и текстом
#include <stdlib.h>           // Стандартная библиотека C (для функций rand(), srand())
#include <time.h>             // Библиотека для работы со временем (для srand(time(0)))
#include <string>             // Библиотека для работы со строками C++
#include <iostream>

// Константы игры
const int SCREEN_WIDTH = 800;  // Ширина игрового окна в пикселях
const int SCREEN_HEIGHT = 600; // Высота игрового окна в пикселях
const int GRID_SIZE = 50;      // Размер одной клетки парковки в пикселях
const int GRID_WIDTH = 10;     // Ширина парковки в клетках
const int GRID_HEIGHT = 10;    // Высота парковки в клетках
const int MAX_CARS = 30;       // Максимальное количество машин на парковке
const int EXIT_WIDTH = 2;      // Ширина выезда с парковки в клетках
const int LEFT_X = 150;        // Начало области парковки по х
const int LEFT_Y = 50;         // Начало области парковки по y

// Направления движения машин
enum Direction { UP, RIGHT, DOWN, LEFT };

// Состояния игры (меню, игра, победа)
enum GameState { MENU, PLAYING, WIN };

// Структура, описывающая машину
struct Car {
    int x, y;               // Координаты головы машины (первой клетки)
    int length;
    Direction dir;          // Направление движения машины
    SDL_Texture* texture;   // Текстура для отрисовки машины
    bool isSelected;        // Флаг, выбрана ли машина игроком
    bool exited;            // Флаг, выехала ли машина с парковки
    SDL_Rect drawRect;      // Прямоугольник для отрисовки всей машины
};

// Глобальные переменные
SDL_Window* window = NULL;      // Указатель на окно приложения
SDL_Renderer* renderer = NULL;  // Указатель на рендерер для отрисовки
TTF_Font* font = NULL;          // Указатель на шрифт для текста
TTF_Font* font_other = NULL;  
GameState gameState = MENU;     // Текущее состояние игры (по умолчанию меню)
int difficulty = 1;             // Уровень сложности (1-3)
Car cars[MAX_CARS];             // Массив машин на парковке
int carCount = 0;               // Количество машин на парковке
Car* selectedCar = NULL;        // Указатель на выбранную машину
int moves = 0;                  // Количество сделанных ходов

// Текстуры
SDL_Texture* backgroundTexture = NULL; // Текстура фона
SDL_Texture* carTexture = NULL;        // Текстура машины
SDL_Texture* exitTexture = NULL;       // Текстура выезда
SDL_Texture* winTexture = NULL;        // Текстура надписи "ПОБЕДА!"

// Позиции выездов с парковки (центры сторон)
SDL_Point exits[4] = {
    {0, GRID_HEIGHT/2},             // Левый край
    {GRID_WIDTH, GRID_HEIGHT/2},  // Правый край
    {GRID_WIDTH/2, 0},              // Верхний край
    {GRID_WIDTH/2, GRID_HEIGHT}   // Нижний край
};

// Функция загрузки текстуры из файла
SDL_Texture* loadTexture(const char* path, int* w = nullptr, int* h = nullptr) {
    // Загрузка изображения в поверхность (SDL_Surface)
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Не удалось загрузить изображение %s! Ошибка: %s\n", path, IMG_GetError());
        return NULL;
    }

    if (w) *w = surface->w;
    if (h) *h = surface->h;

    // Создание текстуры из поверхности
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Освобождение поверхности, так как она больше не нужна
    
    if (!texture) {
        printf("Не удалось создать текстуру из %s! Ошибка: %s\n", path, SDL_GetError());
    }

    return texture;
}

// Функция создания текстуры из текста
SDL_Texture* createTextTexture(const char* text, SDL_Color color, TTF_Font* font_in = font) {
    // Создание поверхности с текстом
    SDL_Surface* surface = TTF_RenderText_Solid(font_in, text, color);
    if (!surface) {
        printf("Не удалось создать поверхность из текста! Ошибка: %s\n", TTF_GetError());
        return NULL;
    }

    // Создание текстуры из поверхности
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Освобождение поверхности
    
    if (!texture) {
        printf("Не удалось создать текстуру из текста! Ошибка: %s\n", SDL_GetError());
    }

    return texture;
}

// Функция инициализации SDL и всех подсистем
bool initSDL() {
    // Инициализация основной библиотеки SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Ошибка инициализации SDL: %s\n", SDL_GetError());
        return false;
    }

    // Инициализация SDL_image для работы с изображениями
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("Ошибка инициализации SDL_image: %s\n", IMG_GetError());
        return false;
    }

    // Инициализация SDL_ttf для работы с шрифтами
    if (TTF_Init() == -1) {
        printf("Ошибка инициализации SDL_ttf: %s\n", TTF_GetError());
        return false;
    }

    // Создание окна приложения
    window = SDL_CreateWindow("Выезд с парковки", SDL_WINDOWPOS_UNDEFINED, 
                            SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Ошибка создания окна: %s\n", SDL_GetError());
        return false;
    }

    // Создание рендерера для отрисовки в окне
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Ошибка создания рендерера: %s\n", SDL_GetError());
        return false;
    }

    // Загрузка шрифта из файла
    font = TTF_OpenFont("font/arial.ttf", 24);
    font_other = TTF_OpenFont("font/arial.ttf", 48);
    if (!font) {
        printf("Не удалось загрузить шрифт! Ошибка: %s\n", TTF_GetError());
        return false;
    }

    // Загрузка всех необходимых текстур
    int carTexW, carTexH;
    backgroundTexture = loadTexture("assets/background.png"); // Фон
    carTexture = loadTexture("assets/car.png", &carTexW, &carTexH);              // Машина
    exitTexture = loadTexture("assets/exit.png");            // Выезд
    winTexture = createTextTexture("WIN!", {255, 255, 51, 255}, font_other); // Текст победы

    // Проверка, что все текстуры загружены успешно
    if (!backgroundTexture || !carTexture || !exitTexture || !winTexture) {
        return false;
    }

    return true;
}

// Функция освобождения ресурсов и завершения работы SDL
void closeSDL() {
    // Удаление всех текстур
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(carTexture);
    SDL_DestroyTexture(exitTexture);
    SDL_DestroyTexture(winTexture);
    
    // Закрытие шрифта
    TTF_CloseFont(font);
    // Удаление рендерера и окна
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    // Завершение работы всех подсистем SDL
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// Функция проверки, свободна ли указанная клетка
bool isCellFree(int x, int y) {
    // Проверка, не находится ли клетка на выезде
    for (int i = 0; i < 4; i++) {
        if ((x == exits[i].x && abs(y - exits[i].y) <= EXIT_WIDTH/2) ||
            (y == exits[i].y && abs(x - exits[i].x) <= EXIT_WIDTH/2)) {
            return true; // Клетка на выезде считается свободной
        }
    }

    // Проверка выхода за границы парковки
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT)
        return false;

    // Проверка всех машин на парковке
    for (int i = 0; i < carCount; i++) {
        if (cars[i].exited) continue; // Пропустить машины, которые уже выехали

        // Для вертикальных машин (движутся вверх/вниз)
        if (cars[i].dir == UP || cars[i].dir == DOWN) {
            for (int j = 0; j < cars[i].length; j++) {
                int cy = cars[i].y + (cars[i].dir == DOWN ? j : -j);
                if (cars[i].x == x && cy == y)
                    return false; // Клетка занята машиной
            }
        } 
        // Для горизонтальных машин (движутся влево/вправо)
        else {
            for (int j = 0; j < cars[i].length; j++) {
                int cx = cars[i].x + (cars[i].dir == RIGHT ? j : -j);
                if (cx == x && cars[i].y == y)
                    return false; // Клетка занята машиной
            }
        }
    }
    return true; // Клетка свободна
}
SDL_Rect calculateCarRect(const Car& car) {
    SDL_Rect rect;
    
    if (car.dir == UP || car.dir == DOWN) {
        // Вертикальные машины (2 клетки в высоту)
        rect.x = LEFT_X + car.x * GRID_SIZE;
        rect.y = LEFT_Y + (car.dir == DOWN ? car.y : car.y - 1) * GRID_SIZE;
        rect.w = GRID_SIZE;
        rect.h = 2 * GRID_SIZE; // Фиксированная длина
    } else {
        // Горизонтальные машины (2 клетки в ширину)
        rect.x = LEFT_X + (car.dir == RIGHT ? car.x : car.x - 1) * GRID_SIZE;
        rect.y = LEFT_Y + car.y * GRID_SIZE;
        rect.w = 2 * GRID_SIZE; // Фиксированная длина
        rect.h = GRID_SIZE;
    }
    
    return rect;
}
// Функция генерации случайной парковки
void generateParking() {
    carCount = 0;       // Сброс количества машин
    moves = 0;          // Сброс счетчика ходов
    selectedCar = NULL; // Сброс выбранной машины
    srand(time(0));     // Инициализация генератора случайных чисел

    // Количество машин зависит от сложности
    int numCars = 10 + (difficulty - 1) * 5;
    
    

    // Генерация машин
    for (int i = 0; i < numCars; i++) {
        Car car;
        car.length = 2; // Длина 2 
        car.dir = static_cast<Direction>(rand() % 4); // Случайное направление
        car.texture = carTexture;      // Текстура машины
        car.isSelected = false;        // Изначально не выбрана
        car.exited = false;            // Изначально не выехала

        bool placed = false; // Флаг, размещена ли машина
        int attempts = 0;    // Счетчик попыток размещения

        // Попытки разместить машину на парковке
        while (!placed && attempts < 100) {
            attempts++;
            
            // Генерация случайных координат в зависимости от направления
            if (car.dir == UP || car.dir == DOWN) {
                car.x = rand() % GRID_WIDTH;
                car.y = rand() % (GRID_HEIGHT - car.length + 1);
            } else {
                car.x = rand() % (GRID_WIDTH - car.length + 1);
                car.y = rand() % GRID_HEIGHT;
            }

            car.drawRect = calculateCarRect(car);

            placed = true; // Предполагаем, что размещение удалось
            // Проверка всех клеток, которые займет машина
            for (int j = 0; j < car.length; j++) {
                int cx, cy;
                
                if (car.dir == UP || car.dir == DOWN) {
                    cx = car.x;
                    cy = car.y + (car.dir == DOWN ? j : -j);
                } else {
                    cx = car.x + (car.dir == RIGHT ? j : -j);
                    cy = car.y;
                }

                // Проверка, не находится ли клетка на выезде
                for (int e = 0; e < 4; e++) {
                    if ((cx == exits[e].x && abs(cy - exits[e].y) <= EXIT_WIDTH/2) ||
                        (cy == exits[e].y && abs(cx - exits[e].x) <= EXIT_WIDTH/2)) {
                        placed = false; // Нельзя ставить машину на выезд
                        break;
                    }
                }

                if (!placed) break;

                // Проверка, свободна ли клетка
                if (!isCellFree(cx, cy)) {
                    placed = false;
                    break;
                }
            }
        }

        // Если машину удалось разместить, добавляем ее в массив
        if (placed) {
            cars[carCount++] = car;
        }
    }
}

// Функция проверки, может ли машина двигаться в указанном направлении
bool canMove(const Car* car, int dx, int dy) {
    if (car->exited) return false; // Уже выехавшие машины не могут двигаться

    // Проверка всех клеток, которые займет машина после движения
    for (int i = 0; i < car->length; i++) {
        int cx, cy;
        
        if (car->dir == UP || car->dir == DOWN) {
            cx = car->x + dx;
            cy = car->y + (car->dir == DOWN ? i : -i) + dy;
        } else {
            cx = car->x + (car->dir == RIGHT ? i : -i) + dx;
            cy = car->y + dy;
        }

        // Проверка, не находится ли клетка на выезде
        bool isExit = false;
        for (int e = 0; e < 4; e++) {
            if ((cx == exits[e].x && abs(cy - exits[e].y) <= EXIT_WIDTH/2) ||
                (cy == exits[e].y && abs(cx - exits[e].x) <= EXIT_WIDTH/2)) {
                isExit = true;
                break;
            }
        }
        
        if (isExit) continue; // Выезд считается допустимым

        // Проверка, свободна ли клетка
        if (!isCellFree(cx, cy)) {
            // Проверка, не является ли занятая клетка частью этой же машины
            bool isOurCar = false;
            for (int j = 0; j < car->length; j++) {
                int ox, oy;
                if (car->dir == UP || car->dir == DOWN) {
                    ox = car->x;
                    oy = car->y + (car->dir == DOWN ? j : -j);
                } else {
                    ox = car->x + (car->dir == RIGHT ? j : -j);
                    oy = car->y;
                }
                
                if (ox == cx && oy == cy) {
                    isOurCar = true;
                    break;
                }
            }
            
            // Если клетка занята другой машиной, движение невозможно
            if (!isOurCar)
                return false;
        }
    }
    return true; // Все клетки свободны или на выезде
}

// Функция перемещения машины
void moveCar(Car* car, int dx, int dy) {
    if (!canMove(car, dx, dy)) return; // Проверка возможности движения

    // Изменение координат машины
    car->x += dx;
    car->y += dy;
    moves++; // Увеличение счетчика ходов
    car->drawRect = calculateCarRect(*car); // Обновляем прямоугольник отрисовки

    // Проверка, выехала ли машина полностью
    bool exited = true;
    for (int i = 0; i < car->length; i++) {
        int cx, cy;
        
        if (car->dir == UP || car->dir == DOWN) {
            cx = car->x;
            cy = car->y + (car->dir == DOWN ? i : -i);
        } else {
            cx = car->x + (car->dir == RIGHT ? i : -i);
            cy = car->y;
        }

        // Проверка, находится ли клетка на выезде
        bool onExit = false;
        for (int e = 0; e < 4; e++) {
            if ((cx == exits[e].x && abs(cy - exits[e].y) <= EXIT_WIDTH/2) ||
                (cy == exits[e].y && abs(cx - exits[e].x) <= EXIT_WIDTH/2)) {
                onExit = true;
                break;
            }
        }

        // Если хотя бы одна клетка не на выезде, машина не выехала
        if (!onExit) {
            exited = false;
            break;
        }
    }

    car->exited = exited; // Установка флага выезда
}

// Функция проверки условия победы (все машины выехали)
bool checkWin() {
    for (int i = 0; i < carCount; i++) {
        if (!cars[i].exited)
            return false; // Найдена не выехавшая машина
    }
    return true; // Все машины выехали
}

// Функция отрисовки меню
void renderMenu() {
    // Отрисовка фона
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    //Отрисовка черного полупрозрачного прямоугольника
    SDL_SetRenderDrawColor(renderer, 0,0,0,128);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect back = {200, 150, 400, 300};
    SDL_RenderFillRect(renderer, &back);

    // Создание и отрисовка текста заголовка
    SDL_Color white = {255, 255, 255, 255};
    SDL_Texture* title = createTextTexture("Parking escape", white);
    SDL_Rect titleRect = {SCREEN_WIDTH/2 - 150, 150, 300, 60};
    SDL_RenderCopy(renderer, title, NULL, &titleRect);
    SDL_DestroyTexture(title); // Удаление временной текстуры

    // Массивы для кнопок сложности
    const char* difficulties[] = {"Low", "Medium", "High"};
    SDL_Color colors[] = {{0, 200, 0, 255}, {200, 200, 0, 255}, {200, 0, 0, 255}};
    
    // Отрисовка кнопок выбора сложности
    for (int i = 0; i < 3; i++) {
        SDL_Rect buttonRect = {SCREEN_WIDTH/2 - 90, 220 + i*70, 180, 50};
        
        // Отрисовка прямоугольника кнопки
        SDL_SetRenderDrawColor(renderer, colors[i].r, colors[i].g, colors[i].b, colors[i].a);
        SDL_RenderFillRect(renderer, &buttonRect);
        
        // Создание и отрисовка текста на кнопке
        SDL_Texture* text = createTextTexture(difficulties[i], white);
        int textW, textH;
        SDL_QueryTexture(text, NULL, NULL, &textW, &textH);
        SDL_Rect textRect = {
            buttonRect.x + (buttonRect.w - textW)/2,
            buttonRect.y + (buttonRect.h - textH)/2,
            textW,
            textH
        };
        SDL_RenderCopy(renderer, text, NULL, &textRect);
        SDL_DestroyTexture(text); // Удаление временной текстуры
    }

    SDL_RenderPresent(renderer); // Обновление экрана
}

// Функция отрисовки выездов с парковки
// Функция отрисовки выездов с парковки
void renderExits() {
    for (int i = 0; i < 4; i++) {
        SDL_Rect exitRect;
        double angle = 0.0; // Угол поворота по умолчанию (0 градусов)
        
        if (i == 0 || i == 1) { // Горизонтальные выезды (левый и правый)
            exitRect = {
                LEFT_X + exits[i].x * GRID_SIZE - (i == 0 ? 0 : GRID_SIZE),
                LEFT_Y + (exits[i].y - EXIT_WIDTH/2) * GRID_SIZE,
                GRID_SIZE,
                EXIT_WIDTH * GRID_SIZE
            };
        } else { // Вертикальные выезды (верхний и нижний)
            exitRect = {
                LEFT_X + (exits[i].x - EXIT_WIDTH/2) * GRID_SIZE,
                LEFT_Y + exits[i].y * GRID_SIZE - (i == 2 ? 0 : GRID_SIZE),
                EXIT_WIDTH * GRID_SIZE,
                GRID_SIZE
            };
            angle = 90.0; // Поворачиваем на 90 градусов для вертикальных выходов
        }

        // Центр поворота (середина текстуры)
        SDL_Point center = {exitRect.w/2, exitRect.h/2};
        
        // Отрисовка с поворотом
        SDL_RenderCopyEx(renderer, exitTexture, NULL, &exitRect, 
                        angle, &center, SDL_FLIP_NONE);
    }
}

// Функция отрисовки игрового поля
void renderGame() {
    // Отрисовка фона
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    //Отрисовка черного полупрозрачного прямоугольника
    SDL_SetRenderDrawColor(renderer, 0,0,0,128);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect back = {15, 15, 160, 75};
    SDL_RenderFillRect(renderer, &back);

    // Отрисовка парковки (серый прямоугольник)
    SDL_Rect parking = {LEFT_X, LEFT_Y, GRID_WIDTH*GRID_SIZE, GRID_HEIGHT*GRID_SIZE};
    SDL_SetRenderDrawColor(renderer, 126, 126, 126, 200); // Полупрозрачный серый
    SDL_RenderFillRect(renderer, &parking);

    // Отрисовка разметки парковки (белые линии)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i <= GRID_WIDTH; i++)
        SDL_RenderDrawLine(renderer, LEFT_X+i*GRID_SIZE, LEFT_Y, LEFT_X+i*GRID_SIZE, LEFT_Y+GRID_HEIGHT*GRID_SIZE);
    for (int i = 0; i <= GRID_HEIGHT; i++)
        SDL_RenderDrawLine(renderer, LEFT_X, LEFT_Y+i*GRID_SIZE, LEFT_X+GRID_WIDTH*GRID_SIZE, LEFT_Y+i*GRID_SIZE);

    // Отрисовка выездов
    renderExits();

    // Отрисовка всех машин
    for (int i = 0; i < carCount; i++) {
        if (cars[i].exited) continue;

        // Угол поворота в зависимости от направления
        double angle = 0;
        switch (cars[i].dir) {
            case UP:    angle = 0; break;
            case RIGHT: angle = 270; break;
            case DOWN: angle = 180; break;
            case LEFT: angle = 90; break;
        }

        // Центр поворота (середина текстуры)
        SDL_Point center = {cars[i].drawRect.w/2, cars[i].drawRect.h/2};
        
        // Отрисовка с поворотом
        SDL_RenderCopyEx(renderer, cars[i].texture, NULL, &cars[i].drawRect, 
                        angle, &center, SDL_FLIP_NONE);

        // Выделение выбранной машины
        if (cars[i].isSelected) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &cars[i].drawRect);
        }
    }
    

    // Отображение информации о сложности и количестве ходов
    SDL_Color white = {255, 255, 255, 255};
    std::string diffText = "Difficulty: " + std::to_string(difficulty);
    std::string movesText = "Steps: " + std::to_string(moves);
    
    SDL_Texture* diffTexture = createTextTexture(diffText.c_str(), white);
    SDL_Texture* movesTexture = createTextTexture(movesText.c_str(), white);
    
    SDL_Rect diffRect = {20, 20, 150, 30};
    SDL_Rect movesRect = {20, 60, 100, 30};
    
    SDL_RenderCopy(renderer, diffTexture, NULL, &diffRect);
    SDL_RenderCopy(renderer, movesTexture, NULL, &movesRect);
    
    SDL_DestroyTexture(diffTexture);
    SDL_DestroyTexture(movesTexture);

    SDL_RenderPresent(renderer); // Обновление экрана
}

// Функция отрисовки экрана победы
void renderWin() {
    // Отрисовка фона
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    //Отрисовка черного полупрозрачного прямоугольника
    SDL_SetRenderDrawColor(renderer, 0,0,0,128);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect back = {200, 150, 400, 300};
    SDL_RenderFillRect(renderer, &back);

    // Отрисовка текста "ПОБЕДА!"
    int winW, winH;
    SDL_QueryTexture(winTexture, NULL, NULL, &winW, &winH);
    SDL_Rect winRect = {SCREEN_WIDTH/2 - winW/2, 190, winW, winH};
    SDL_RenderCopy(renderer, winTexture, NULL, &winRect);

    // Отрисовка информации о количестве ходов
    SDL_Color white = {255, 255, 255, 255};
    std::string movesText = "Steps: " + std::to_string(moves);
    SDL_Texture* movesTexture = createTextTexture(movesText.c_str(), white);
    
    SDL_Rect movesRect = {SCREEN_WIDTH/2 - 100, 280, 200, 30};
    SDL_RenderCopy(renderer, movesTexture, NULL, &movesRect);
    SDL_DestroyTexture(movesTexture);

    // Отрисовка кнопки возврата в меню
    SDL_Rect menuButton = {SCREEN_WIDTH/2 - 100, 350, 200, 60};
    SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
    SDL_RenderFillRect(renderer, &menuButton);
    
    SDL_Texture* menuText = createTextTexture("Menu", white);
    int textW, textH;
    SDL_QueryTexture(menuText, NULL, NULL, &textW, &textH);
    SDL_Rect textRect = {
        menuButton.x + (menuButton.w - textW)/2,
        menuButton.y + (menuButton.h - textH)/2,
        textW,
        textH
    };
    SDL_RenderCopy(renderer, menuText, NULL, &textRect);
    SDL_DestroyTexture(menuText);

    SDL_RenderPresent(renderer); // Обновление экрана
}

// Функция обработки кликов мыши
void handleClick(int x, int y) {
    switch (gameState) {
        case MENU:
            // Обработка кликов по кнопкам сложности в меню
            for (int i = 0; i < 3; i++) {
                SDL_Rect rect = {SCREEN_WIDTH/2 - 90, 220 + i*70, 180, 50};
                if (x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h) {
                    difficulty = i + 1; // Установка сложности
                    generateParking();   // Генерация парковки
                    gameState = PLAYING; // Переход в игровой режим
                    return;
                }
            }
            break;
            
        case PLAYING: {
            // Проверка, что клик был внутри игрового поля
            if (x < LEFT || x >= LEFT_X + GRID_WIDTH*GRID_SIZE || y < 50 || y >= 50 + GRID_HEIGHT*GRID_SIZE)
                return;
                
            // Перевод координат клика в координаты сетки
            int gx = (x - LEFT_X) / GRID_SIZE;
            int gy = (y - LEFT_Y) / GRID_SIZE;
            
            // Сброс выделения всех машин
            for (int i = 0; i < carCount; i++)
                cars[i].isSelected = false;
            selectedCar = NULL;
            
            // Поиск машины по координатам клика
            for (int i = 0; i < carCount; i++) {
                if (cars[i].exited) continue; // Пропуск выехавших машин
                
                if (cars[i].dir == UP || cars[i].dir == DOWN) {
                    // Проверка вертикальных машин
                    for (int j = 0; j < cars[i].length; j++) {
                        int cy = cars[i].y + (cars[i].dir == DOWN ? j : -j);
                        if (cars[i].x == gx && cy == gy) {
                            cars[i].isSelected = true; // Выделение машины
                            selectedCar = &cars[i];    // Установка выбранной машины
                            return;
                        }
                    }
                } else {
                    // Проверка горизонтальных машин
                    for (int j = 0; j < cars[i].length; j++) {
                        int cx = cars[i].x + (cars[i].dir == RIGHT ? j : -j);
                        if (cx == gx && cars[i].y == gy) {
                            cars[i].isSelected = true; // Выделение машины
                            selectedCar = &cars[i];    // Установка выбранной машины
                            return;
                        }
                    }
                }
            }
            break;
        }
            
        case WIN: {
            // Обработка клика по кнопке "В меню" на экране победы
            SDL_Rect button = {SCREEN_WIDTH/2 - 100, 350, 200, 60};
            if (x >= button.x && x <= button.x + button.w && y >= button.y && y <= button.y + button.h) {
                gameState = MENU; // Возврат в меню
            }
            break;
        }
    }
}

// Главная функция программы
int main(int argc, char* argv[]) {
    if (!initSDL()) return 1; // Инициализация SDL, выход при ошибке

    bool running = true;  // Флаг работы главного цикла
    SDL_Event e;          // Структура для хранения событий
    
    // Главный игровой цикл
    while (running) {
        bool chit = false;
        // Обработка событий
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false; // Выход из игры при закрытии окна
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                // Обработка клика мыши
                int x, y;
                std::cout << x << " " << y << std::endl;
                SDL_GetMouseState(&x, &y);
                handleClick(x, y);
            } else if (e.type == SDL_KEYDOWN && gameState == PLAYING && selectedCar) {
                // Обработка нажатий клавиш для управления выбранной машиной
                switch (e.key.keysym.sym) {
                    case SDLK_UP: moveCar(selectedCar, 0, -1); break;    // Вверх
                    case SDLK_DOWN: moveCar(selectedCar, 0, 1); break;   // Вниз
                    case SDLK_LEFT: moveCar(selectedCar, -1, 0); break;  // Влево
                    case SDLK_RIGHT: moveCar(selectedCar, 1, 0); break;  // Вправо
                    case SDLK_q: chit = true; break;
                }
                
                // Проверка условия победы после каждого хода
                if (checkWin() || chit) gameState = WIN;
            }
        }
        
        // Отрисовка текущего состояния игры
        switch (gameState) {
            case MENU: renderMenu(); break;
            case PLAYING: renderGame(); break;
            case WIN: renderWin(); break;
        }
        
        SDL_Delay(16); // Небольшая задержка для снижения нагрузки на CPU
    }
    
    closeSDL(); // Освобождение ресурсов перед выходом
    return 0;
}