/**
 * 聊天室界面化
 * 将客户端发送的数据打印到聊天输出框内
 * 绘制用户输入框
 * 用户展示在线好友列表
 */

#ifndef INCLUDE_INTERFACE_H
#define INCLUDE_INTERFACE_H

#include <iostream>
#include <stdlib.h>
#include <ncurses.h>
#include <string>

class InterFace {
public:
    InterFace();
    ~InterFace();
    // 标题窗口绘制
    void createHeader();
    // 消息输出框绘制
    void createOutput();
    // 用户输入框绘制
    void createInput();
    // 在线用户列表图形化绘制
    void createOnlineList();
    // 从出口中获取数据，并保存在缓冲 buffer 中
    std::string getStrFromInterFace(WINDOW* w);
    // 向窗口中放置数据
    void putStrToInterFace(WINDOW* w, int startY, int startX, const std::string& message);
    void Refresh(WINDOW* w) { wrefresh(w); }
    WINDOW* createNewInterFace(int h, int w, int startY, int startX);
    void clearInterFace(WINDOW* w, int begin, int line);
public:
    // 聊天窗口标题句柄
    WINDOW* header_;
    // 消息输出框句柄
    WINDOW* output_;
    // 用户输入框句柄
    WINDOW* input_;
    // 在线列表绘制句柄
    WINDOW* onlinelist_;
}; // InterFace

#endif // INCLUDE_INTERFACE_H
