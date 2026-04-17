#ifndef JSON_AST_H
#define JSON_AST_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

// 类型枚举
enum JsonType {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOL,
    JSON_NULL
};

struct JsonNode {
    JsonType type;
    
    // 值（根据 type 使用对应的字段）
    std::map<std::string, JsonNode> obj;
    std::vector<JsonNode> arr;
    std::string str;
    int num;
    bool bo;
    
    // 构造函数
    JsonNode() : type(JSON_NULL), num(0), bo(false) {}
    explicit JsonNode(const std::map<std::string, JsonNode>& o) : type(JSON_OBJECT), obj(o), num(0), bo(false) {}
    explicit JsonNode(const std::vector<JsonNode>& a) : type(JSON_ARRAY), arr(a), num(0), bo(false) {}
    explicit JsonNode(const std::string& s) : type(JSON_STRING), str(s), num(0), bo(false) {}
    explicit JsonNode(int n) : type(JSON_NUMBER), num(n), bo(false) {}
    explicit JsonNode(bool b) : type(JSON_BOOL), bo(b), num(0) {}
};

void print_json(const JsonNode& node, std::ostream& os, int indent = 0, bool pretty = false);

#endif