
// cmake -S . -B build && cmake --build build && ./build/thread_memory

#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace std;

class Obj {
public:
    Obj(){
        m_state = 1;
    }
    int state () { return m_state; }
    void run() {

        thread ([&]() {   
            m_state = 2;
        }).detach();

    }
private:
    int m_state = 0;
};

void test_shared_memory() {
    Obj stackObj;
    Obj *heapObj = new Obj();

    printf("\n");

    printf("main: heapObj->x = %d\n", heapObj->state());
    printf("main: stackObj.x = %d\n", stackObj.state());

    thread ([&]() {

        heapObj->run();
        stackObj.run();
        sleep(1);

        printf("thread: heapObj->x = %d\n", heapObj->state());
        printf("thread: stackObj.x = %d\n", stackObj.state());

    }).detach();

    sleep(3);
    
    printf("main: heapObj->x = %d\n", heapObj->state());
    printf("main: stackObj.x = %d\n", stackObj.state());

    printf("main thread: close();\n");
}

int main() {

    // test_shared_memory();


    return 0;


}