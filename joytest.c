#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

void ProcessAxisMotion(SDL_Event *ev)
{
    static int lastjoy = -1, lastaxis, lastdirection;
    int direction;

    direction = ev->jaxis.value > 0;

    if (ev->jaxis.which == lastjoy && ev->jaxis.axis == lastaxis
     && direction == lastdirection)
    {
        return;
    }

    // Only print a message when pushed to the extreme
    if (abs(ev->jaxis.value) < 30000)
    {
        return;
    }

    printf("%i: Axis %i pushed to extreme: %i\n",
           ev->jaxis.which, ev->jaxis.axis, ev->jaxis.value);

    lastjoy = ev->jaxis.which;
    lastaxis = ev->jaxis.axis;
    lastdirection = direction;
}

void ProcessBallMotion(SDL_Event *ev)
{
    printf("%i: Ball %i motion: %i, %i\n",
           ev->jball.which, ev->jball.ball, ev->jball.xrel, ev->jball.yrel);
}

char *HatDirection(int value)
{
    switch (value)
    {
        case SDL_HAT_UP:
            return "UP";
        case SDL_HAT_DOWN:
            return "DOWN";
        case SDL_HAT_LEFT:
            return "LEFT";
        case SDL_HAT_RIGHT:
            return "RIGHT";
        default:
            return NULL;
    }
}

void ProcessHatMotion(SDL_Event *ev)
{
    char *dirstr;

    dirstr = HatDirection(ev->jhat.value);

    if (dirstr != NULL)
    {
        printf("%i: Hat %i moved to %s", ev->jhat.which, ev->jhat.hat,
               dirstr);
    }
}

void ProcessButtonDown(SDL_Event *ev)
{
    printf("%i: Button %i pressed\n", ev->jbutton.which, ev->jbutton.button);
}

void StartJoysticks(void)
{
    SDL_Joystick *js;
    int i;

    // Open all joysticks.

    printf("%i joysticks:\n", SDL_NumJoysticks());

    for (i = 0 ; i < SDL_NumJoysticks(); ++i)
    {
        printf("\t#%i: '%s'\n", i, SDL_JoystickName(i));

        js = SDL_JoystickOpen(i);
        printf("\t\taxes: %i\n", SDL_JoystickNumAxes(js));
        printf("\t\tbuttons: %i\n", SDL_JoystickNumButtons(js));
        printf("\t\tballs: %i\n", SDL_JoystickNumBalls(js));
        printf("\t\thats: %i\n", SDL_JoystickNumHats(js));
    }
}

int main(int argc, char *argv[])
{
    SDL_Event ev;

    SDL_Init(SDL_INIT_JOYSTICK|SDL_INIT_VIDEO);

    if (SDL_NumJoysticks() == 0)
    {
        fprintf(stderr, "Found no joysticks to configure!\n");
        return 1;
    }

    SDL_SetVideoMode(400, 200, 0, 0);
    SDL_WM_SetCaption("Close this window to quit.", NULL);

    StartJoysticks();

    for (;;)
    {
        SDL_PumpEvents();
        SDL_WaitEvent(&ev);
        switch (ev.type)
        {
            case SDL_QUIT:
                goto exitloop;

            case SDL_KEYDOWN:
                if (ev.key.keysym.sym == SDLK_ESCAPE)
                {
                    goto exitloop;
                }
                break;

            case SDL_JOYAXISMOTION:
                ProcessAxisMotion(&ev);
                break;

            case SDL_JOYBALLMOTION:
                ProcessBallMotion(&ev);
                break;

            case SDL_JOYHATMOTION:
                ProcessHatMotion(&ev);
                break;

            case SDL_JOYBUTTONDOWN:
                ProcessButtonDown(&ev);
                break;
        }
    }
exitloop:

    SDL_Quit();

    return 0;
}

