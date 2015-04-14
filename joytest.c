#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

SDL_Joystick **joysticks;
FILE *logfile;

void LogPrintf(char *s, ...)
{
    va_list args;

    va_start(args, s);
    vprintf(s, args);
    va_end(args);

    va_start(args, s);
    vfprintf(logfile, s, args);
    va_end(args);
}

void DumpAxisStates(void)
{
    int js, axis;

    for (js = 0; js < SDL_NumJoysticks(); ++js)
    {
        printf("%i: axis states: ", js);
        for (axis = 0; axis < SDL_JoystickNumAxes(joysticks[js]); ++axis)
        {
            printf("% 7i", SDL_JoystickGetAxis(joysticks[js], axis));
        }
        printf("\n");
    }
}

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

    // Only print a message when pushed out of the dead zone.
    if (abs(ev->jaxis.value) > (32768 / 3))
    {
        return;
    }

    LogPrintf("%i: Axis %i pushed to extreme: %i\n",
              ev->jaxis.which, ev->jaxis.axis, ev->jaxis.value);

    lastjoy = ev->jaxis.which;
    lastaxis = ev->jaxis.axis;
    lastdirection = direction;
}

void ProcessBallMotion(SDL_Event *ev)
{
    LogPrintf("%i: Ball %i motion: %i, %i\n",
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
        LogPrintf("%i: Hat %i moved to %s\n", ev->jhat.which, ev->jhat.hat,
                  dirstr);
    }
}

void ProcessButtonDown(SDL_Event *ev)
{
    LogPrintf("%i: Button %i pressed\n",
              ev->jbutton.which, ev->jbutton.button);
}

void StartJoysticks(void)
{
    SDL_Joystick *js;
    int i;

    // Open all joysticks.

    LogPrintf("%i joysticks:\n", SDL_NumJoysticks());

    joysticks = calloc(SDL_NumJoysticks(), sizeof(SDL_Joystick *));

    for (i = 0 ; i < SDL_NumJoysticks(); ++i)
    {
        LogPrintf("\t#%i: '%s'\n", i, SDL_JoystickName(i));

        js = SDL_JoystickOpen(i);
        LogPrintf("\t\taxes: %i\n", SDL_JoystickNumAxes(js));
        LogPrintf("\t\tbuttons: %i\n", SDL_JoystickNumButtons(js));
        LogPrintf("\t\tballs: %i\n", SDL_JoystickNumBalls(js));
        LogPrintf("\t\thats: %i\n", SDL_JoystickNumHats(js));
        joysticks[i] = js;
    }
}

int main(int argc, char *argv[])
{
    SDL_Event ev;
    int poll_axes;
    int now, last_poll_time = 0;

    // Normally we detect when a stick has been pushed in a particular
    // direction by checking for extreme values, but sometimes this
    // doesn't work. So allow the -pollaxes flag to specify that we
    // should poll the state of axes once a second instead.
    poll_axes = argc >= 2 && !strcmp(argv[1], "-pollaxes");

    SDL_Init(SDL_INIT_JOYSTICK|SDL_INIT_VIDEO);

    if (SDL_NumJoysticks() == 0)
    {
        fprintf(stderr, "Found no joysticks to configure!\n");
        return 1;
    }

    SDL_SetVideoMode(400, 200, 0, 0);
    SDL_WM_SetCaption("Close this window to quit.", NULL);

    logfile = fopen("joytest.log", "w");

    StartJoysticks();

    for (;;)
    {
        now = SDL_GetTicks();
        if (poll_axes && now - last_poll_time > 1000)
        {
            last_poll_time = now;
            DumpAxisStates();
        }

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
                if (!poll_axes)
                {
                    ProcessAxisMotion(&ev);
                }
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
    fclose(logfile);

    return 0;
}

