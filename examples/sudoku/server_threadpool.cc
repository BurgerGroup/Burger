#include "burger/net/CoTcpServer.h"
#include "burger/net/Scheduler.h"
#include "burger/base/ThreadPool.h"
#include "burger/base/Timestamp.h"
#include "sudoku.h"
#include "burger/base/Util.h"
#include "burger/base/Log.h"
#include "burger/net/RingBuffer.h"
using namespace burger;
using namespace burger::net;
using namespace std::placeholders;

// todo : need test
class SudokuServer {
public:
    // 注意，这里的numThreads是计算线程池的线程数，而不是IO线程的
    SudokuServer(Scheduler* sched, const InetAddress& listenAddr, int numThreads)
        : server_(sched, listenAddr, "SudokuServer"),
        numThreads_(numThreads),
        startTime_(Timestamp::now()) {
        server_.setConnectionHandler(std::bind(&SudokuServer::connHandler, this, _1));
        // server_.setThreadNum(xxx);  todo : need to implement 多线程IO
    }
    void start() {
        INFO("starting {} calculate threads", numThreads_);
        threadPool_.start(numThreads_);   // 计算线程池
        server_.start();
    }
private: 
    void connHandler(const CoTcpConnection::ptr& conn) {
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
            threadPool_.run(std::bind(&solve, conn, puzzle, id));  // 加到线程池的队列中运行
        } else {
            goodRequest  = false;
        }
        return goodRequest;
    }
    static void solve(const CoTcpConnection::ptr& conn, 
            const std::string& puzzle, const std::string& id) {
        std::string result = solveSudoku(puzzle);
        if(id.empty()) {
            conn->send(result+"\r\n");  // 这里发送回去还是IO线程发送，而不是计算线程池发送
            // todo: 考虑不在当前线程调用send
        } else {
            conn->send(id+":"+result+"\r\n");
        }
    }

private:
    CoTcpServer server_;
    Threadpool threadPool_;   // 计算线程池
    int numThreads_;
    Timestamp startTime_;        
};

int main(int argc, char* argv[]) {
    LOGGER(); LOG_LEVEL_INFO;
    int numThreads = 0;
    if(argc > 1) {
        numThreads = atoi(argv[1]);
    }
    Scheduler sched;
    InetAddress listenAddr(8888);
    SudokuServer server(&sched, listenAddr, numThreads);
    server.start();
    sched.wait();
}

