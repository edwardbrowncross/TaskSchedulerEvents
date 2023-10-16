#pragma once

#define _TASK_STATUS_REQUEST
#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>

#include "EventBuffer.h"

namespace TSEvents {

class EventHandlerTask;
class EventBus;

class EventEmitter {
 public:
  EventEmitter(EventBus* e);

 protected:
  bool dispatch(uint16_t id, const void* data, uint16_t data_size);
  bool dispatch(uint16_t id, const char* data);
  bool dispatch(uint16_t id);
  bool dispatch(Event event);

 private:
  EventBus* e;
};

class EventHandler : public EventEmitter {
 public:
  EventHandler(Scheduler* s, EventBus* e);
  ~EventHandler();

 protected:
  virtual void HandleEvent(Event event) = 0;

 private:
  EventHandlerTask* t;
  friend class EventHandlerTask;
};

}  // namespace TSEvents