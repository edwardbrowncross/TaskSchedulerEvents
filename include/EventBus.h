#define _TASK_STATUS_REQUEST
#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>
#include <EventBuffer.h>

#ifndef EVENT_MAX_LISTENERS
#define EVENT_MAX_LISTENERS 64
#endif

#ifndef EVENT_BUFFER_SIZE
#define EVENT_BUFFER_SIZE 256
#endif

class EventBus {
   public:
    EventBus();
    ~EventBus();
    void addListener(Task* t);
    Event getCurrentEvent();
    bool dispatch(Event evt);
  bool dispatch(uint16_t id, const void* data, uint16_t data_size);
  bool dispatch(uint16_t id, const char* data);
    bool dispatch(uint16_t id);
    void done(int seq);
    int getSeq();

   private:
    void triggerHandlers();
    Task* handlers[EVENT_MAX_LISTENERS];
    int handlersLen;
    int toHandle;
    int handled;
    int seq;
    EventBuffer* events;
};