
// cmake -S . -B build && cmake --build build && ./build/thread_memory

#include <thread>
#include <stdio.h>
#include <unistd.h>

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

int main() {

    Obj stackObj;
    Obj *heapObj = new Obj();

    printf("\n");

    printf("main: heapObj->x (%x) = %d\n", &heapObj, heapObj->state());
    printf("main: stackObj.x (%x) = %d\n", &stackObj, stackObj.state());

    thread ([&]() {

        heapObj->run();
        stackObj.run();
        sleep(1);

        printf("thread: heapObj->x (%x) = %d\n", &heapObj, heapObj->state());
        printf("thread: stackObj.x (%x) = %d\n", &stackObj, stackObj.state());

    }).detach();

    sleep(3);
    
    printf("main: heapObj->x (%x) = %d\n", &heapObj, heapObj->state());
    printf("main: stackObj.x (%x) = %d\n", &stackObj, stackObj.state());

    printf("main thread: close();\n");

}