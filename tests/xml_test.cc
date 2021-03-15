#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include <iostream>

using namespace rapidxml;


int CreateXml() {
    xml_document<> doc;
    xml_node<>* rot = doc.allocate_node(node_pi, doc.allocate_string("xml version='1.0' encoding='utf-8'"));
    doc.append_node(rot);
    xml_node<>* node = doc.allocate_node(node_element, "config", "information");
    doc.append_node(node);
    xml_node<>* color = doc.allocate_node(node_element, "color", NULL);
    node->append_node(color);
    color->append_node(doc.allocate_node(node_element, "red", "0.1"));
    color->append_node(doc.allocate_node(node_element, "green", "0.1"));
    color->append_node(doc.allocate_node(node_element, "blue", "0.1"));
    color->append_node(doc.allocate_node(node_element, "alpha", "1.0"));
    xml_node<>* size = doc.allocate_node(node_element, "size", NULL);
    size->append_node(doc.allocate_node(node_element, "x", "640"));
    size->append_node(doc.allocate_node(node_element, "y", "480"));
    node->append_node(size);
    xml_node<>* mode = doc.allocate_node(node_element, "mode", "screen mode");
    mode->append_attribute(doc.allocate_attribute("fullscreen", "false"));
    node->append_node(mode);
    std::string text;
    print(std::back_inserter(text), doc, 0);
    std::cout << text << std::endl;
    std::ofstream out("config2.xml");
    out << doc;

    return 0;
}


//读取并修改config3.xml
int ReadAndChangeXml() {
    file<> fdoc("config2.xml");
    std::cout << fdoc.data() << std::endl;
    xml_document<> doc;
    doc.parse<0>(fdoc.data());
    std::cout << doc.name() << std::endl;
    //! 获取根节点
    xml_node<>* root = doc.first_node();
    std::cout << root->name() << std::endl;
    //! 获取根节点第一个节点
    xml_node<>* node1 = root->first_node();
    std::cout << node1->name() << std::endl;
    xml_node<>* node11 = node1->first_node();
    std::cout << node11->name() << std::endl;
    std::cout << node11->value() << std::endl;
    //! 修改之后再次保存
    xml_node<>* size = root->first_node("size");
    size->append_node(doc.allocate_node(node_element, "w", "1"));
    size->append_node(doc.allocate_node(node_element, "h", "1"));
    std::string text;
    print(std::back_inserter(text), doc, 0);
    std::cout << text << std::endl;
    std::ofstream out("../config/config2.xml");
    out << doc;

    return 0;
}


void createxml() {
    xml_document<> doc;
    xml_node<>* root = doc.allocate_node(node_pi, doc.allocate_string("xml version='1.0' encoding='utf-8'"));
    doc.append_node(root);

    xml_node<>* node = doc.allocate_node(node_element, "config", "information");
    doc.append_node(node);

    xml_node<>* color = doc.allocate_node(node_element, "color", NULL);
    node->append_node(color);
    color->append_node(doc.allocate_node(node_element, "red", "0.1"));
    color->append_node(doc.allocate_node(node_element, "green", "0.1"));
    color->append_node(doc.allocate_node(node_element, "blue", "0.1"));
    color->append_node(doc.allocate_node(node_element, "alpha", "0.1"));

    xml_node<>* size = doc.allocate_node(node_element, "size",NULL);
    node->append_node(size);
    size->append_node(doc.allocate_node(node_element, "x", "640"));
    size->append_node(doc.allocate_node(node_element, "y", "500"));
    xml_node<>* mode = doc.allocate_node(node_element, "mode", "screen mode");
    mode->append_attribute(doc.allocate_attribute("fullscreen", "false"));
    node->append_node(mode);
    std::string text;
    print(std::back_inserter(text), doc, 0);
    std::cout << text << std::endl;
    std::ofstream out("config.xml");
    out << doc;
}


void readAndChangexml() {
    file<> fdoc("config.xml");

    xml_document<> doc;
    std::cout << fdoc.data() << std::endl;
    //doc.parse<0>(fdoc.data());//修改节点值不会保存到文件中
    doc.parse<parse_no_data_nodes>(fdoc.data());//修改节点值会保存到文件中
    std::cout << "///" << std::endl;
    std::cout << doc.name() << std::endl;

    //获取根节点
    xml_node<>* root = doc.first_node();
    std::cout << root->name() << std::endl;

    //! 获取根节点第一个节点
    xml_node<>* node1 = root->first_node();
    std::cout << node1->name() << std::endl;

    xml_node<>* size = root->first_node("size");
    
    xml_node<>* x =  size->first_node("x");
    x->value("540");

    std::cout << size->first_node("x")->value() << std::endl;
    
    std::string text;
    print(std::back_inserter(text), doc, 0);
    std::cout << text << std::endl;
    std::ofstream out("config.xml");
    out << doc;
}

int main() {
    //createxml();
    readAndChangexml();
    //CreateXml();
    //system("pause");
    return 0;
}