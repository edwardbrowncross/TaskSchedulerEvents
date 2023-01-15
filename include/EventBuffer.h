#ifndef EVENT_BUFFER_H
#define EVENT_BUFFER_H

#include <stdint.h>

typedef uint8_t byte;

typedef struct {
    uint16_t id;
    uint16_t size;
    void* data;
} Event;

class EventBuffer {
   public:
    // size should be divisible by 4
    EventBuffer(uint16_t size);
    ~EventBuffer();

    bool push(uint16_t id, void* data, uint16_t data_size);
    bool push(uint16_t id, char* data);
    bool push(uint16_t id);
    bool push(Event event);

    Event shift();
    Event next();
    uint16_t getSize();
    uint16_t getLength();
    uint16_t getFree();

   private:
    uint16_t maxSize;
    uint16_t head;
    uint16_t tail;
    uint16_t wrap;
    byte* buffer;
    uint16_t length;
    uint16_t size;
};

#endif