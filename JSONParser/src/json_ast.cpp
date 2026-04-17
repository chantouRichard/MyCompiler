#include "json_ast.h"

void print_indent(std::ostream& os, int indent) {
    for (int i = 0; i < indent; i++) {
        os << "  ";
    }
}

void print_json(const JsonNode& node, std::ostream& os, int indent, bool pretty) {
    switch (node.type) {
        case JSON_NULL:
            os << "null";
            break;
            
        case JSON_BOOL:
            os << (node.bo ? "true" : "false");
            break;
            
        case JSON_NUMBER:
            os << node.num;
            break;
            
        case JSON_STRING:
            os << "\"" << node.str << "\"";
            break;
            
        case JSON_ARRAY:
            os << "[";
            if (pretty && !node.arr.empty()) os << "\n";
            for (size_t i = 0; i < node.arr.size(); i++) {
                if (pretty) print_indent(os, indent + 1);
                print_json(node.arr[i], os, indent + 1, pretty);
                if (i != node.arr.size() - 1) os << ",";
                if (pretty) os << "\n";
            }
            if (pretty && !node.arr.empty()) print_indent(os, indent);
            os << "]";
            break;
            
        case JSON_OBJECT:
            os << "{";
            if (pretty && !node.obj.empty()) os << "\n";
            bool first = true;
            for (const auto& pair : node.obj) {
                if (!first) {
                    os << ",";
                    if (pretty) os << "\n";
                }
                if (pretty) print_indent(os, indent + 1);
                os << "\"" << pair.first << "\":";
                if (pretty) os << " ";
                print_json(pair.second, os, indent + 1, pretty);
                first = false;
            }
            if (pretty && !node.obj.empty()) {
                os << "\n";
                print_indent(os, indent);
            }
            os << "}";
            break;
    }
}