#include "EventHandlerTask.h"
#include "EventHandler.h"
#include "EventBus.h"

using namespace TSEvents;

EventHandlerTask::EventHandlerTask(Scheduler* s, EventBus* _e, EventHandler* _h) : Task(0, TASK_ONCE, s, false) {
    h = _h;
    e = _e;
}

bool EventHandlerTask::OnEnable() {
    seq = e->getSeq();
    return seq != -1;
}

bool EventHandlerTask::Callback() {
    Event event = e->getCurrentEvent();
    h->HandleEvent(event);
    e->done(seq);
    return true;
}