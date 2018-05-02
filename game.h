#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

enum CarColor
{
    CarColorRed,
    CarColorBlue,
    CarColorGreen,
    CarColorBrown,
    CarColorBlack,
};

class Car
{
public:
    SDL_Texture *texture;
    SDL_Rect rect;
    Car(int x, int y, SDL_Renderer *renderer, CarColor color);
    ~Car();
    void Draw(SDL_Renderer *renderer);
    void SetSpeed(float speed);
    void SetDirection(int dir);
    void SwitchLane();
    void Update();
	int GetLane();
private:
    int rotation = 0;
    int direction = 1;
    int speed = 0;
    int lane = 1;
    int defaultLanePos = 0;
    bool switchingLane = false;
};
