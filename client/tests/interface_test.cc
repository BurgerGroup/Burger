#include "interface.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <set>
InterFace ie;
std::set<std::string> circle_friend = {"ysy", "sgwf", "skyu", "pig"};

void headerRun() {
    int y, x;
    std::string message = "welcome to chat sys";
    int step = 0;
    while (1) {
        ie.createHeader();
        ie.Refresh(ie.header_);
        getmaxyx(ie.header_, y, x);
        int startY = y * 4 / 10;
        int startX = x;
        ie.putStrToInterFace(ie.header_, startY, step++, message);
        ie.Refresh(ie.header_);
        usleep(200000);
        step %= (startX - 1);
        ie.clearInterFace(ie.header_, startY, 1);
        ie.Refresh(ie.header_);
    }
}
void inputRun() {
    std::string point = "Please Enter: ";
    while (1) {
        // 创建聊天输入窗口
        ie.createInput();
        // 刷新窗口
        ie.Refresh(ie.input_);
        // 向聊天输入框中放置消息
        ie.putStrToInterFace(ie.input_, 1, 1, point);
        // 刷新窗口
        ie.Refresh(ie.input_);
        // 将聊天输入框中的消息保存至 std::string 中
        std::string message = ie.getStrFromInterFace(ie.input_);
    }

}


void outputRun() {
    int step = 1;
    int y = 0;
    int x = 0;
    // 创建聊天输出窗口
    ie.createOutput();
    // 刷新窗口
    ie.Refresh(ie.output_);
    while (1) {
        std::string message = "test1111";
        // 将从消息队列中获取到的数据打印到聊天输出框
        // ie.putStrToInterFace(ie.output_, step++, 1, message);
        // 若屏幕沾满，则 clear
        // ie.Refresh(ie.output_);
        getmaxyx(ie.output_, y, x);
        // 处理输出框内文字格式，防止超出边界
        int startY = y;
        step %= startY;
        if (step == 0) {
            ie.createOutput();
            ie.Refresh(ie.output_);
            step = 0; // 用于再0这
            ie.putStrToInterFace(ie.output_, step++, 1, message);
            
            ie.Refresh(ie.output_);
        }
    }

}


void onlineListRun() {
    ie.createOnlineList();
    ie.Refresh(ie.onlinelist_);
    int x, y;
    int step = 1;
    while (1) {
        std::string message;
        std::set<std::string>::iterator it = circle_friend.begin();
        ie.createOnlineList();
        ie.Refresh(ie.onlinelist_);
        step = 1;
        for (; it != circle_friend.end(); it++) {
            message = *it;
            ie.putStrToInterFace(ie.onlinelist_, step++, 1, message);
            ie.Refresh(ie.onlinelist_);
            getmaxyx(ie.onlinelist_, y, x);
            int startY = y;
            step %= startY;
            if (step == 0) {
                sleep(3);
                ie.createOnlineList();
                ie.Refresh(ie.onlinelist_);
                step = 1;
                ie.putStrToInterFace(ie.onlinelist_, step++, 1, message);
                ie.Refresh(ie.onlinelist_);
            }
        }
        sleep(1);
    }
}

int main() {

    std::thread t1(headerRun);
    std::thread t2(inputRun);
    std::thread t3(outputRun); 
    std::thread t4(onlineListRun); 
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}




