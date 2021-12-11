# TaskSchedulerEvents

An object-oriented, event-driven construct for loosely coupling Arduino TaskScheduler tasks together. I am building this for my personal use. It is not yet stable.

---

## Preamble

Arduino has a simple model of specifying `setup()` and `loop()` functions that the runtime runs at the start and on infinite repeat respectively. I typically define some helper functions and global variables to track state. I enjoy the simplicity of this but for non-trivial projects I find it gets messy. The loop function ends up doing a lot of low-level work checking for various changes and the core logic of the application is lost under a pile of low-level details.

I want to add the minimal extra structure on top of the out-of-the box Arduino experience, so that my Arduino projects scale in a neater way without losing the simplicity.

### Goals

My goals are:
1. Independent modules with their own `setup` and `loop` functionality and containing all variables needed to handle one concern of the application.
2. Each module can start and stop and determine its own loop interval
3. Modules are coupled together by dispatching events on a single, global event bus. A third function is defined for each module that handles an incoming event.
4. It works on any Arduino micro-controller. I will otherwise just write normal Arduino code.

---

## Components

### Task Scheduler

[arkhipenko/TaskScheduler](https://github.com/arkhipenko/TaskScheduler) is an Arduino library for cooperative multitasking that already meets requirements 1, 2, and 4 above, particularly when in its [OOP configuration](https://github.com/arkhipenko/TaskScheduler/tree/master/examples/Scheduler_example21_OO_Callbacks). This project is an event bus for goal 3 that uses and works alongside OOP TaskScheduler.

### Event Bus

A single event bus is created. You could have multiple but I like to have one.

```cpp
#include <EventBus.h>
EventBus e;
```

This event bus is passed to each of the modules, which in turn passes it to its superclass constructor of either `EventEmitter` or `EventHandler`.

### EventEmitter

An `EventEmitter` subclass gets a `dispatch` function that dispatches events to an event bus

```cpp
class MyEmitterTask : public Task, public EventEmitter {
    MyEmitterTask(Scheduler* s, EventBus* e)
        : Task(50 * TASK_MILLISECOND, TASK_FOREVER, s, false),
          EventEmitter(e) {}

    bool OnEnable() {
        dispatch(MY_EMITTER_TASK_ENABLED);
        return true;
    }
```

### Events

Events have a numeric id and an optional payload which can be of any type and any size (up to the size of the event bus buffer). I like to use a single Enum for all my event ids. 

```cpp
enum EventType {
    BUTTON_PRESSED = 1,
    BUTTON_RELEASED,
    BUTTON_LONG_PRESSED,
    BUTTON_DOUBLE_PRESSED,

    WIFI_CONNECTED,
    WIFI_DISCONNECTED,
    // etc...
};
```

### dispatch

`dispatch` has three signatures:

```cpp
// Event type but no payload
bool dispatch(uint16_t id);
// Event type with a text (null-terminated char array) payload
bool dispatch(uint16_t id, char* data);
// Event type with an arbitrary payload
bool dispatch(uint16_t id, void* data, uint16_t data_size);
```

### EventHandler

An `EventHandler` subclass is like an `EventEmitter` but it must also specify a `HandleEvent` method, which will receive every event sent to the event bus.

```cpp
class MainHandler : public EventHandler {
    MainHandler(Scheduler &s, EventBus &e) : EventHandler(&s, &e) {}

    void HandleEvent(Event event) {
        Serial.println(event.id);
        if (event.id === MY_CUSTOM_EVENT) {
            MyCustomEventPayload* payload = (MyCustomEventPayload*)event.data;
            // etc...
        }
    }
}
```

---

## Configuration

Two aspects of the event bus can be configured with defines

```cpp
#define EVENT_MAX_LISTENERS 64 // The maximum number of EventHandlers that can be attached to a single bus
#define EVENT_BUFFER_SIZE 256 // The number of bytes allocated by the event bus for queued events
```

---

## Examples

### Event-Emitting Task

This is a TaskScheduler OOP Task that also emits events onto an event bus. It can do this because it extends `EventEmitter`. It contains all the state and logic for watching a button and dispatching events when it is pressed, released, double pressed and long pressed.

```cpp
class ButtonTask : public Task, public EventEmitter {
   public:
    ButtonTask(Scheduler* s, EventBus* e, uint8_t _pin)
        : Task(50 * TASK_MILLISECOND, TASK_FOREVER, s, false),
          EventEmitter(e) {
        pin = _pin;
    }

    bool OnEnable() {
        pinMode(pin, INPUT_PULLUP);
        return true;
    }

    bool Callback() {
        bool newPressed = digitalRead(pin) == LOW;

        if (newPressed != pressed) {
            if (newPressed) {
                dispatch(BUTTON_PRESSED, &pin, sizeof(pin));
                if (millis() - lastPress < 1000) {
                    dispatch(BUTTON_DOUBLE_PRESSED, &pin, sizeof(pin));
                }
                lastPress = millis();
            } else {
                dispatch(BUTTON_RELEASED, &pin, sizeof(pin));
            }
        } else {
            if (pressed && !longPressed && millis() - lastPress > 2000) {
                dispatch(BUTTON_LONG_PRESSED, &pin, sizeof(pin));
            }
        }
        pressed = newPressed;
        longPressed = pressed && millis() - lastPress > 2000;
        return true;
    }

   private:
    unsigned long lastPress;
    bool pressed;
    bool longPressed;
    uint8_t pin;
};
```

### Event-Handling Task

By extending `EventHandler`, a task can dispatch events but also define a new method `HandleEvent` that will receive all events dispatched onto the event bus. This example manages an MQTT connection and automatically connects after a `WIFI_CONNECTED` event is received.

```cpp
class MQTTTask : public Task, public EventHandler {
   public:
    MQTTTask(Scheduler &s, EventBus &e, IPAddress ip, int port, char* _id, char* _topic)
        : Task(1000 * TASK_MILLISECOND, TASK_FOREVER, &s, false),
          EventHandler(&s, &e) {
        wifiClient = new WiFiClient();
        client = new PubSubClient(*wifiClient);
        // ...etc
        state = DISCONNECTED;
    }

    bool OnEnable() {
        return WiFi.status() == WL_CONNECTED;
    }

    bool Callback() {
        switch (state) {
            case CONNECTED:
                if (client->connected()) {
                    client->loop();
                } else {
                    // connection lost
                    state = DISCONNECTED;
                    dispatch(MQTT_SERVER_DISCONNECTED);
                    if (WiFi.status() == WL_CONNECTED) {
                        connect();
                    }
                }
                break;
            case DISCONNECTED:
                if (WiFi.status() == WL_CONNECTED) {
                    connect();
                }
        }
        return true;
    }

    void HandleEvent(Event event) {
        switch (event.id) {
            case WIFI_CONNECTED:
                enable();
                connect();
                break;
            case WIFI_DISCONNECTED:
                disable();
                if (state == CONNECTED) {
                    state = DISCONNECTED;
                    dispatch(MQTT_SERVER_DISCONNECTED);
                }
                break;
        }
    }

    bool connect() {
        //Do connecting etc...
    }

   private:
    enum State {
        CONNECTED,
        DISCONNECTED,
    };
    WiFiClient *wifiClient;
    PubSubClient *client;
    State state;
};

```

### Only Event Handling

Event handlers do not need to be tasks. I like to have a single event handler that orchestrates all other tasks purely in response to events.

```cpp
class MainHandler : public EventHandler {
   public:
    MainHandler(Scheduler &s, EventBus &e) : EventHandler(&s, &e) {}

    void HandleEvent(Event event) {
        switch (event.id) {
            case BUTTON_SINGLE_PRESSED:
                relayTask.turnOn(3600);
                break;

            case MQTT_MESSAGE_RECEIVED: {
                mqtt_message_received_event mqttEvent = event.data;
                if (strcmp(mqttEvent.topic, MQTT_TOPIC_OFF) == 0) {
                    relayTask.turnOff();
                } else if (strcmp(mqttEvent.topic, MQTT_TOPIC_ON) == 0) {
                    bool error = deserializeJson(payload, mqttEvent.payload);
                    if (error) {
                        break;
                    }
                    relayTask.turnOn(payload["forSeconds"] | 3600);
                }
                break;
            }

            case WIFI_CONNECTING:
                ledTask.blink(250);
                break;

            case MQTT_SERVER_CONNECTED:
                ledTask.turnOff();
                break;
        }
    }

   private:
    StaticJsonDocument<1024> payload;
};
```

### main arduino file

My main arduino file ends up looking something like this. Note that the only extra setup on top of TaskScheduler is creating an `EventBus`.

```cpp
#include <Arduino.h>
#define _TASK_OO_CALLBACKS
#define _TASK_STATUS_REQUEST
#include <EventBus.h>
#include <TaskScheduler.h>
#include <TaskSchedulerDeclarations.h>

#include "ButtonTask.cpp"
#include "LedTask.cpp"
#include "MqttTask.cpp"
#include "RelayTask.cpp"
#include "WifiTask.cpp"

Scheduler ts;
EventBus e;

ButtonTask buttonTask(ts, e, BUTTON_PIN);
LEDTask ledTask(ts, e, LED_PIN);
RelayTask relayTask(ts, e, RELAY_PIN);
WifiTask wifiTask(ts, e, WIFI_SSID, WIFI_PASS);
MQTTTask mqttTask(ts, e, IPAddress(MQTT_IP), MQTT_PORT, MQTT_ID, MQTT_SUB);

class MainHandler : public EventHandler {
    // see above
}

MainHandler mainHandler(ts, e);

void setup() {
    Serial.begin(921600);

    buttonTask.enable();
    relayTask.enable();
    wifiTask.enable();
    mqttTask.enable();
    ledTask.enable();

    ts.startNow();
}

void loop() {
    ts.execute();
    yield();
}
```