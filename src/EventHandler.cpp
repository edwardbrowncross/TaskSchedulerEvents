#include "EventHandler.h"

#include "EventBus.h"
#include "EventHandlerTask.h"

EventEmitter::EventEmitter(EventBus* e) : e(e) {}

bool EventEmitter::dispatch(Event event) {
    return e->dispatch(event);
}
bool EventEmitter::dispatch(uint16_t id, void* data, uint16_t data_size){
    return e->dispatch(id, data, data_size);
}
bool EventEmitter::dispatch(uint16_t id, char* data){
    return e->dispatch(id, data);
}
bool EventEmitter::dispatch(uint16_t id){
    return e->dispatch(id);
}

EventHandler::EventHandler(Scheduler* s, EventBus* e) : EventEmitter(e) {
    t = new EventHandlerTask(s, e, this);
    e->addListener(t);
}

EventHandler::~EventHandler() {
    delete t;
}