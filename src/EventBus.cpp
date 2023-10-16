#include "EventBus.h"
#include <Arduino.h>

using namespace TSEvents;

EventBus::EventBus() {
    events = new EventBuffer(EVENT_BUFFER_SIZE);
}

EventBus::~EventBus() {
    delete events;
}

void EventBus::addListener(Task* t) {
    handlers[handlersLen] = t;
    handlersLen++;
}

Event EventBus::getCurrentEvent() {
    return events->next();
}

bool EventBus::dispatch(Event evt) {
    bool ok = events->push(evt);
    if (ok && events->getLength() == 1) {
        triggerHandlers();
    }
    return ok;
}

bool EventBus::dispatch(uint16_t id, const void* data, uint16_t data_size){
    bool ok = events->push(id, data, data_size);
    if (ok && events->getLength() == 1) {
        triggerHandlers();
    }
    return ok;
}
bool EventBus::dispatch(uint16_t id, const char* data){
    bool ok = events->push(id, data);
    if (ok && events->getLength() == 1) {
        triggerHandlers();
    }
    return ok;
}
bool EventBus::dispatch(uint16_t id){
    bool ok = events->push(id);
    if (ok && events->getLength() == 1) {
        triggerHandlers();
    }
    return ok;
}

void EventBus::done(int _seq) {
    if (_seq != seq) {
        return;
    }
    handled++;
    if (handled == toHandle) {
        seq++;
        events->shift();
        if (events->getLength() > 0) {
            triggerHandlers();
        }
    }
}

int EventBus::getSeq() {
    if (events->getLength() == 0) {
        return -1;
    }
    return seq;
}

void EventBus::triggerHandlers() {
    toHandle = handlersLen;
    handled = 0;
    for (int i = 0; i < handlersLen; i++) {
        Task* t = handlers[i];
        t->setIterations(TASK_ONCE);
        t->enable();
    }
}