#include "game.h"

Car::Car(int x, int y, SDL_Renderer *renderer, CarColor color)
{
    this->rect.x = x;
    this->rect.y = y;
    this->rect.w = 64;
    this->rect.h = 64;
    this->defaultLanePos = x;
    
    const char *path = nullptr;
    
    if (color == CarColorBlack)
        path = "..\\src/Car-Black.png";
    if (color == CarColorBlue)
        path = "..\\src/Car-Blue.png";
    if (color == CarColorRed)
        path = "..\\src/Car-Red.png";
    if (color == CarColorGreen)
        path = "..\\src/Car-Green.png";
    if (color == CarColorBrown)
        path = "..\\src/Car-Brown.png";
    
    SDL_Surface *surface = IMG_Load(path);
    this->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void Car::Draw(SDL_Renderer *renderer)
{
    SDL_RenderCopyEx(renderer, this->texture, NULL, &this->rect, rotation, NULL, SDL_FLIP_NONE);
}

void Car::SetDirection(int dir)
{
    if (dir > 0)
    {
        direction = 1;
        rotation = 0;
    }

    if (dir < 0)
    {
        direction = -1;
        rotation = 180;
    }
}

void Car::SetSpeed(float speed)
{
    this->speed = speed;
}

void Car::SwitchLane()
{
    if (lane == 0)
    {
        lane = 1;

    } else
    {
        lane = 0;
    }
    switchingLane = true;
}

void Car::Update()
{
    this->rect.y -= speed * direction;
    
    if (switchingLane)
    {
        if (lane == 1 && this->rect.x < defaultLanePos)
        {
            this->rect.x += 2;
            rotation = 10;
        }
        else if (lane == 0 && this->rect.x > defaultLanePos - 64)
        {
            this->rect.x -= 2;
            rotation = -10;
        }
        else
        {
            rotation = 0;
            switchingLane = false;
        }
    }
}

int Car::GetLane()
{
    return lane;
}

Car::~Car()
{
    SDL_DestroyTexture(this->texture);
}

































