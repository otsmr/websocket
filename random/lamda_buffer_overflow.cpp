
// cmake -S . -B build && cmake --build build && ./build/lamda_buffer_overflow

#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <iostream>

typedef std::function<void()> fkt_string;

class WebSocket {
public:
    void on_message(fkt_string f) { m_on_message = f; };
    void call();
    void listen();
private:
    fkt_string m_on_message = nullptr;

};

typedef std::function<void(WebSocket *)> fkt_ws;

class Socket {
public:
    void on_open(fkt_ws f) { m_on_open = f; };
    void call();
private:
    fkt_ws m_on_open = nullptr;
};

void WebSocket::listen() {

    char buffer[1000];
    std::cout << "&buffer" << &buffer << std::endl;
    m_on_message();
        
}


void Socket::call() {

    WebSocket ws;

    std::cout << "&ws=" << &ws << std::endl;

    m_on_open(&ws);

    ws.listen();

}

int main() {

    Socket socket;
    socket.on_open([](auto * ws) {

        std::cout << "&ws=" << &ws << std::endl;

        ws->on_message([&](){
            std::cout << "&ws=" << &ws << std::endl;
        });

    });

    std::cout << "socket.call();" << std::endl;
    socket.call();

    std::cout << "success" << std::endl;

    return 0;

}