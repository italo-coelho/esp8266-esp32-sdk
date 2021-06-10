#pragma once

#include "../SinricProRequest.h"
#include "../EventLimiter.h"
#include "../SinricProStrings.h"

#include "../SinricProNamespace.h"
namespace SINRICPRO_NAMESPACE {

FSTR(MUTE, mute);
FSTR(MUTE, setMute);

using MuteCallback = std::function<bool(const String &, bool &)>;

template <typename T>
class MuteController {
  public:
    MuteController();

    void onMute(MuteCallback cb);
    bool sendMuteEvent(bool mute, String cause = FSTR_SINRICPRO_PHYSICAL_INTERACTION);
  
  protected:
    virtual bool onMute(bool &muteState);
    bool handleMuteController(SinricProRequest &request);

  private:
    EventLimiter event_limiter;
    MuteCallback muteCallback;
};

template <typename T>
MuteController<T>::MuteController() 
: event_limiter(EVENT_LIMIT_STATE) { 
  T* device = static_cast<T*>(this);
  device->registerRequestHandler(std::bind(&MuteController<T>::handleMuteController, this, std::placeholders::_1));
}

template <typename T>
void MuteController<T>::onMute(MuteCallback cb) {
  muteCallback = cb;
}

template <typename T>
bool MuteController<T>::onMute(bool &muteState) {
  T* device = static_cast<T*>(this);
  if (muteCallback) return muteCallback(device->deviceId, muteState);
  return false;
}

template <typename T>
bool MuteController<T>::sendMuteEvent(bool mute, String cause) {
  if (event_limiter) return false;
  T* device = static_cast<T*>(this);

  DynamicJsonDocument eventMessage = device->prepareEvent(FSTR_MUTE_setMute, cause.c_str());
  JsonObject event_value = eventMessage[FSTR_SINRICPRO_payload][FSTR_SINRICPRO_value];
  event_value[FSTR_MUTE_mute] = mute;
  return device->sendEvent(eventMessage);
}

template <typename T>
bool MuteController<T>::handleMuteController(SinricProRequest &request) {
  bool success = false;

  if (request.action == FSTR_MUTE_setMute) {
    bool muteState = request.request_value[FSTR_MUTE_mute];
    success = onMute(muteState);
    request.response_value[FSTR_MUTE_mute] = muteState;
    return success;
  }
  return success;
}

} // SINRICPRO_NAMESPACE