#include <EventBuffer.cpp>
#include <unity.h>

typedef struct test {
    int a;
    char b[10];
};

void test_event_buffer(void) {
    EventBuffer eb(256);
    Event e;

    TEST_ASSERT_EQUAL_MESSAGE(256, eb.getFree(), "free of event buffer at start");
    TEST_ASSERT_EQUAL_MESSAGE(0, eb.getLength(), "length of event buffer at start");
    TEST_ASSERT_EQUAL_MESSAGE(0, eb.getSize(), "size of event buffer at start");

    // Event 1
    eb.push(1, "test");

    TEST_ASSERT_EQUAL_MESSAGE(12, eb.getSize(), "size of event buffer after 1 event");

    // Event 2
    eb.push(2);

    TEST_ASSERT_EQUAL_MESSAGE(12+4, eb.getSize(), "size of event buffer after 2 events");

    // Event 3
    struct test t;
    t.a = 99;
    strcpy(t.b, "testing");
    eb.push(3, &t, sizeof(test));

    TEST_ASSERT_EQUAL_MESSAGE(12+4+20, eb.getSize(), "size of event buffer after 3 events");

    TEST_ASSERT_EQUAL_MESSAGE(3, eb.getLength(), "in length of event buffer");

    e = eb.next();

    TEST_ASSERT_EQUAL_UINT16_MESSAGE(1, e.id, "id of first event");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(5, e.size, "size of first event");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("test", (char*)e.data, "data in first event");

    eb.shift();
    e = eb.next();

    TEST_ASSERT_EQUAL_MESSAGE(2, eb.getLength(), "length of event buffer after shift");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(2, e.id, "id of second event");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0, e.size, "size of second event");

    eb.shift();
    e = eb.next();

    TEST_ASSERT_EQUAL_UINT16_MESSAGE(3, e.id, "id of second event");
    struct test* t2 = (struct test*)e.data;
    TEST_ASSERT_EQUAL_INT16_MESSAGE(99, t2->a, "contents of struct");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("testing", t2->b, "contents of struct");

    eb.shift();
    TEST_ASSERT_EQUAL_MESSAGE(0, eb.getLength(), "length of event buffer with nothing in it");
    TEST_ASSERT_EQUAL_MESSAGE(0, eb.getSize(), "size of event buffer with nothing in it");
}

void test_event_buffer_wrap(void) {
    EventBuffer eb(16);

    eb.push(1);
    eb.shift();
    eb.push(1);
    eb.shift();

    // HEAD=8 TAIl=8
    TEST_ASSERT_EQUAL_MESSAGE(0, eb.getSize(), "size of fragmented event buffer");
    TEST_ASSERT_EQUAL_MESSAGE(8, eb.getFree(), "free of fragmented event buffer");

    eb.push(1);

    // HEAD=8 TAIL=12
    TEST_ASSERT_EQUAL_MESSAGE(4, eb.getSize(), "size of fragmented event buffer 2");
    TEST_ASSERT_EQUAL_MESSAGE(8, eb.getFree(), "free of fragmented event buffer 2");

    eb.push(99, "a"); // 8 bytes

    // TAIL=8 HEAD=8 (WRAP=12)
    TEST_ASSERT_EQUAL_MESSAGE(12, eb.getSize(), "size of wrapped event buffer");
    TEST_ASSERT_EQUAL_MESSAGE(0, eb.getFree(), "free of wrapped event buffer");

    eb.shift();

    Event e = eb.next();

    // HEAD=0 TAIL=8
    TEST_ASSERT_EQUAL_MESSAGE(99, e.id, "id of wrapped event");
    TEST_ASSERT_EQUAL_MESSAGE(2, e.size, "size of wrapped event");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("a", (char*)e.data, "data of wrapped event");

    eb.push(2);
    eb.push(3);

    // HEAD=0 TAIL=16
    TEST_ASSERT_EQUAL_MESSAGE(16, eb.getSize(), "size of full event buffer");
    TEST_ASSERT_EQUAL_MESSAGE(0, eb.getFree(), "free of full event buffer");
    TEST_ASSERT_FALSE_MESSAGE(eb.push(4), "trying to add to full buffer");

    eb.shift(); // 8 bytes

    // HEAD=8 TAIL=16
    TEST_ASSERT_EQUAL_MESSAGE(8, eb.getSize(), "size of space at start");
    TEST_ASSERT_EQUAL_MESSAGE(8, eb.getFree(), "free of space at start");
    TEST_ASSERT_TRUE_MESSAGE(eb.push(5), "trying to add to buffer with space");

    // TAIL=4 HEAD=8 (WRAP=16)
    TEST_ASSERT_EQUAL_MESSAGE(2, eb.shift().id, "id of first event");
    TEST_ASSERT_EQUAL_MESSAGE(3, eb.shift().id, "id of second event");
    TEST_ASSERT_EQUAL_MESSAGE(5, eb.shift().id, "id of third event");
    TEST_ASSERT_EQUAL_MESSAGE(0, eb.shift().id, "id of no event");
}

void process() {
    UNITY_BEGIN();
    RUN_TEST(test_event_buffer);
    RUN_TEST(test_event_buffer_wrap);
    UNITY_END();
}

#ifdef ARDUINO

#include <Arduino.h>
void setup() {
    delay(2000);
    process();
}

void loop() {
    delay(100);
}

#else

int main(int argc, char **argv) {
    process();
    return 0;
}

#endif