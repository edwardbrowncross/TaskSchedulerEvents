#include <string.h>
#include <math.h>
#include "EventBuffer.h"

EventBuffer::EventBuffer(uint16_t size) : maxSize(size), wrap(size), size(0) {
    buffer = (byte*)new uint32_t[size/4];
    length = head = tail = 0;
}

EventBuffer::~EventBuffer() {
    delete buffer;
}

bool EventBuffer::push(uint16_t id, void* data, uint16_t dataSize) {
    uint16_t totalSize = ceil(dataSize/4.0)*4 + 4;
    if (getFree() < totalSize) {
        return false;
    }
    if (tail + totalSize > maxSize) {
        wrap = tail;
        tail = 0;
    }
    byte* tailBytes = buffer + tail;

    ((uint16_t*)tailBytes)[0] = dataSize;
    ((uint16_t*)tailBytes)[1] = id;
    memcpy(tailBytes+4, data, dataSize);

    tail += totalSize;

    length++;
    size += totalSize;

    return true;
}

bool EventBuffer::push(uint16_t id, char* data) {
    return push(id, data, strlen(data)+1);
}
bool EventBuffer::push(uint16_t id) {
    return push(id, NULL, 0);
}
bool EventBuffer::push(Event event) {
    return push(event.id, event.data, event.size);
}

Event EventBuffer::next() {
    Event event = {0, 0, NULL};
    if (length == 0) {
        return event;
    }
    if (head == wrap) {
        head = 0;
    }
    byte* headBytes = buffer + head;
    event.size = ((uint16_t*)headBytes)[0];
    event.id = ((uint16_t*)headBytes)[1];
    event.data = (headBytes+4);
    return event;
}

Event EventBuffer::shift() {
    Event event = next();
    if (length == 0) {
        return event;
    }
    uint16_t totalSize = ceil(event.size/4.0)*4 + 4;
    head += totalSize;
    length--;
    size -= totalSize;
    byte* headBytes = buffer + head;
    if (length > 0 && ((int16_t*)headBytes)[0] < 0) {
        head = 0;
    }
    return event;
}

uint16_t EventBuffer::getSize() {
    return size;
}

uint16_t EventBuffer::getLength() {
    return length;
}

uint16_t EventBuffer::getFree() {
    if (tail > head || (tail == head && length == 0)) {
        uint16_t end = maxSize-tail;
        return head>end ? head : end;
    } else {
        return head - tail;
    }
}