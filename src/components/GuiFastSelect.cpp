#include "GuiFastSelect.h"
#include "../Renderer.h"
#include <iostream>
#include "../Input.h"
using namespace Input;

const std::string GuiFastSelect::LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int GuiFastSelect::SCROLLSPEED = 100;
const int GuiFastSelect::SCROLLDELAY = 507;

GuiFastSelect::GuiFastSelect(GuiComponent* parent, GuiList<FileData*>* list, char startLetter, GuiBoxData data, int textcolor) {
  mLetterID = LETTERS.find(toupper(startLetter));
  if(mLetterID == std::string::npos)
    mLetterID = 0;

  Renderer::registerComponent(this);
  registerComponent(this);

  mParent = parent;
  mList = list;

  mScrolling = false;
  mScrollTimer = 0;
  mScrollOffset = 0;

  unsigned int sw = Renderer::getScreenWidth(), sh = Renderer::getScreenHeight();
  mBox = new GuiBox(sw * 0.2, sh * 0.2, sw * 0.6, sh * 0.6);
  mBox->setData(data);

  mTextColor = textcolor;

  mParent->pause();
}

GuiFastSelect::~GuiFastSelect() {
  Renderer::unregisterComponent(this);
  unregisterComponent(this);

  delete mBox;

  mParent->resume();
}

void GuiFastSelect::onRender() {
  unsigned int sw = Renderer::getScreenWidth(), sh = Renderer::getScreenHeight();

  if(!mBox->hasBackground())
    Renderer::drawRect(sw * 0.2, sh * 0.2, sw * 0.6, sh * 0.6, 0x000FF0FF);

  mBox->render();

  Renderer::drawCenteredText(LETTERS.substr(mLetterID, 1), 0, sh * 0.5 - (Renderer::getDefaultFont(Renderer::MEDIUM)->getHeight() * 0.5), mTextColor, Renderer::getDefaultFont(Renderer::MEDIUM));
}

void GuiFastSelect::onInput(InputButton button, bool keyDown) {
  if(button == AXIS_UP && keyDown) {
    mScrollOffset = -1;
    scroll();
  }

  if(button == AXIS_DOWN && keyDown) {
    mScrollOffset = 1;
    scroll();
  }

  if((button == AXIS_UP || button == AXIS_DOWN) && !keyDown) {
    mScrolling = false;
    mScrollTimer = 0;
    mScrollOffset = 0;
  }

  if(button == BTN_SELECT && !keyDown) {
    setListPos();
    delete this;
    return;
  }
}

void GuiFastSelect::onTick(int deltaTime) {
  if(mScrollOffset != 0) {
    mScrollTimer += deltaTime;

    if(!mScrolling && mScrollTimer >= SCROLLDELAY) {
      mScrolling = true;
      mScrollTimer = SCROLLSPEED;
    }

    if(mScrolling && mScrollTimer >= SCROLLSPEED) {
      mScrollTimer = 0;
      scroll();
    }
  }
}

void GuiFastSelect::scroll() {
  setLetterID(mLetterID + mScrollOffset);
}

void GuiFastSelect::setLetterID(int id) {
  while(id < 0)
    id += LETTERS.length();
  while(id >= (int)LETTERS.length())
    id -= LETTERS.length();

  mLetterID = (size_t)id;
}

void GuiFastSelect::setListPos() {
  char letter = LETTERS[mLetterID];

  int min = 0;
  int max = mList->getObjectCount() - 1;

  int mid = 0;

  while(max >= min) {
    mid = ((max - min) / 2) + min;

    char checkLetter = toupper(mList->getObject(mid)->getName()[0]);

    if(checkLetter < letter) {
      min = mid + 1;
    }else if(checkLetter > letter) {
      max = mid - 1;
    }else{
      //exact match found
      break;
    }
  }

  mList->setSelection(mid);
}
