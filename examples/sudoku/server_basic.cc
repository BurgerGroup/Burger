#include "sudoku.h"

#include "burger/net/CoTcpServer.h"
#include "burger/net/Scheduler.h"
#include "burger/net/InetAddress.h"
#include "burger/base/Timestamp.h"
#include "burger/net/CoTcpConnection.h"
#include "burger/net/RingBuffer.h"
#include "burger/base/Util.h"
#include "burger/base/Log.h"

using namespace burger;
using namespace burger::net;
using namespace std::placeholders;

// 只有一个IO线程，既负责listenfd, 也负责connfd
class SudokuServer {
public:
    SudokuServer(Scheduler* sched, const InetAddress& listenAddr)
        : server_(sched, listenAddr),
        startTime_(Timestamp::now()) {
        server_.setConnectionHandler(std::bind(&SudokuServer::connHanler, this, _1));
    }
    void start() { server_.start(); }
private:
    void connHanler(const CoTcpConnection::ptr& conn) {
        RingBuffer::ptr buf = std::make_shared<RingBuffer>();
        while(conn->recv(buf) > 0) {  
            size_t len = buf->getReadableBytes();
            while(len >= kCells+2) {  // 反复逐条读数据，2为\r\n
                const char* crlf = buf->findCRLF();
                if(crlf) {    // 如果找到一条完整请求
                    std::string request(buf->peek(), crlf);  // 取出请求
                    DEBUG("request is : {}", request);
                    buf->retrieveUntil(crlf+2);
                    len = buf->getReadableBytes();
                    if(!processRequest(conn, request)) {
                        conn->send("Bad request!\r\n");
                        conn->shutdown();
                        break;
                    }
                } else if(len > 100) {  // id+ ":" + kCells + "\r\n"
                    conn->send("Id too long!\r\n");
                    conn->shutdown();
                    break;
                } else {
                    break;
                }
            }
        }
    }
    bool processRequest(const CoTcpConnection::ptr& conn, const std::string& request) {
        std::string id;
        std::string puzzle;
        bool goodRequest = true;
        std::string::const_iterator colon = find(request.begin(), request.end(), ':');
        if(colon != request.end()) {
            id.assign(request.begin(), colon);
            puzzle.assign(colon+1, request.end());
        } else {
            puzzle  = request;
        }
        if(puzzle.size() == implicit_cast<size_t>(kCells)) {
            DEBUG("{}", conn->getName());
            std::string result = solveSudoku(puzzle);
            if(id.empty()) {
                conn->send(result+"\r\n");
            } else {
                conn->send(id+":"+result+"\r\n");
            }
        } else {
            goodRequest  = false;
        }
        return goodRequest;
    }
private:
    CoTcpServer server_;
    Timestamp startTime_;
};

int main() {
    LOGGER(); LOG_LEVEL_INFO;
    Scheduler sched;
    InetAddress listenAddr(8888);
    SudokuServer server(&sched, listenAddr);

    server.start();
    sched.wait();
}