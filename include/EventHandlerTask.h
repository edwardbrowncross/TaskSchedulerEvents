#define _TASK_STATUS_REQUEST
#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>

namespace TSEvents {

class EventBus;
class EventHandler;

class EventHandlerTask : public Task {
 public:
  EventHandlerTask(Scheduler* s, EventBus* _e, EventHandler* _h);

  bool OnEnable();
  bool Callback();

 private:
  EventHandler* h;
  EventBus* e;
  int seq;
};

}  // namespace TSEvents