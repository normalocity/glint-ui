#include "Input.h"
#include "../GuiComponent.h"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

namespace Input {
  map<int, InputButton> joystickButtonMap, joystickAxisPosMap, joystickAxisNegMap;
  map<int, int> axisState;
  InputButton hatState = UNKNOWN;

  vector<GuiComponent*> inputVector;
  SDL_Event* lastEvent = NULL;

  void registerComponent(GuiComponent* comp) {
    inputVector.push_back(comp);
  }

  void unregisterComponent(GuiComponent* comp) {
    for(unsigned int i = 0; i < inputVector.size(); i++) {
      if(inputVector.at(i) == comp) {
        inputVector.erase(inputVector.begin() + i);
        break;
      }
    }
  }

  void processEvent(SDL_Event* event) {
    bool keyDown = false;
    InputButton button = UNKNOWN;

    lastEvent = event;

    //keyboard events
    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
      if(event->key.state == SDL_PRESSED)
        keyDown = true;

      switch (event->key.keysym.sym) {
        case SDLK_LEFT:
          button = AXIS_LEFT;
          break;
        case SDLK_RIGHT:
          button = AXIS_RIGHT;
          break;
        case SDLK_UP:
          button = AXIS_UP;
          break;
        case SDLK_DOWN:
          button = AXIS_DOWN;
          break;
        case SDLK_PAGEUP:
          button = BTN_PAGEUP;
          break;
        case SDLK_PAGEDOWN:
          button = BTN_PAGEDOWN;
          break;
        case SDLK_RETURN:
          button = BTN_1;
          break;
        case SDLK_ESCAPE:
          button = BTN_2;
          break;
        case SDLK_F1:
          button = BTN_MENU;
          break;
        case SDLK_F2:
          button = BTN_SELECT;
          break;

        default:
          button = UNKNOWN;
          break;
      }

      //catch emergency quit event
      if (event->key.keysym.sym == SDLK_F4 && keyDown) {
        //I have no idea if SDL will delete this event, but we're quitting, so I don't think it really matters
        SDL_Event* quit = new SDL_Event();
        quit->type = SDL_QUIT;
        SDL_PushEvent(quit);
        cout << "Pushing quit event\n";
      }
    } else {
      if (event->type == SDL_JOYBUTTONDOWN || event->type == SDL_JOYBUTTONUP) {
        if(event->type == SDL_JOYBUTTONDOWN)
          keyDown = true;

        button = joystickButtonMap[event->jbutton.button];
      } else {
        if (event->type == SDL_JOYHATMOTION) {
          int hat = event->jhat.value;

          // if centered
          if (hat == 0) {
            //we need to send a keyUp event for the last hat
            //keyDown is already false
            button = hatState;
          } else {
            keyDown = true;
          }

          if (hat & SDL_HAT_LEFT)
            button = AXIS_LEFT;
          if (hat & SDL_HAT_RIGHT)
            button = AXIS_RIGHT;

          if (hat & SDL_HAT_UP)
            button = AXIS_UP;
          if (hat & SDL_HAT_DOWN)
            button = AXIS_DOWN;

          if (button == hatState && keyDown) {
            //ignore this hat event since the user most likely just made it a diagonal (but it still is using the old direction)
            button = UNKNOWN;
          } else {
            if (hatState != UNKNOWN && keyDown) {
              //this will occur if the user went down -> downLeft -> Left or similar
              button = hatState;
              keyDown = false;
              hatState = UNKNOWN;
              processEvent(event);
            } else {
              if(!keyDown)
                hatState = UNKNOWN;
              else
                hatState = button;
            }
          }
        } else {
          if (event->type == SDL_JOYAXISMOTION) {
            int axis = event->jaxis.axis;
            int value = event->jaxis.value;

            // if this axis was previously not centered, it can only keyUp
            if (axisState[axis] != 0) {
              if(abs(value) < DEADZONE) {
                if(axisState[axis] > 0)
                  button = joystickAxisPosMap[axis];
                else
                  button = joystickAxisNegMap[axis];
                axisState[axis] = 0;
              }
            } else {
              if (value > DEADZONE) {
                //axisPos keyDown
                axisState[axis] = 1;
                keyDown = true;
                button = joystickAxisPosMap[axis];
              } else if(value < -DEADZONE) {
                axisState[axis] = -1;
                keyDown = true;
                button = joystickAxisNegMap[axis];
              }
            }
          }
        }
      }
    }

    for (unsigned int i = 0; i < inputVector.size(); i++) {
      inputVector.at(i)->onInput(button, keyDown);
    }
  }

} // namespace