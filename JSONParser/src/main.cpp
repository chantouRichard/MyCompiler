#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include "json_ast.h"

using namespace std;

extern FILE* yyin;
extern int yyparse(unique_ptr<JsonNode>& ast);

void print_usage(const char* prog_name) {
    cerr << "Usage: " << prog_name << " <input.json> [-o <output.json>] [--pretty]" << endl;
    cerr << "Options:" << endl;
    cerr << "  -o <file>   输出到指定文件（默认输出到标准输出）" << endl;
    cerr << "  --pretty    美化输出（带缩进）" << endl;
}

int main(int argc, char* argv[]) {
    // 默认值
    string input_file;
    string output_file;
    bool pretty = false;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-o") {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                cerr << "Error: -o requires a filename" << endl;
                return 1;
            }
        } else if (arg == "--pretty") {
            pretty = true;
        } else if (input_file.empty()) {
            input_file = arg;
        } else {
            cerr << "Error: Unknown argument: " << arg << endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // 检查输入文件
    if (input_file.empty()) {
        cerr << "Error: No input file specified" << endl;
        print_usage(argv[0]);
        return 1;
    }
    
    // 打开输入文件
    FILE* fp = fopen(input_file.c_str(), "r");
    if (!fp) {
        cerr << "Error: Cannot open file " << input_file << endl;
        return 1;
    }
    
    yyin = fp;
    unique_ptr<JsonNode> ast;
    int parse_result = yyparse(ast);
    fclose(fp);
    
    if (parse_result != 0 || !ast) {
        cerr << "Error: Failed to parse JSON" << endl;
        return 1;
    }
    
    // 输出结果
    if (output_file.empty()) {
        // 输出到标准输出
        print_json(*ast, cout, 0, pretty);
        cout << endl;
    } else {
        // 输出到文件
        ofstream out(output_file);
        if (!out) {
            cerr << "Error: Cannot create output file " << output_file << endl;
            return 1;
        }
        print_json(*ast, out, 0, pretty);
        out << endl;
        out.close();
        cout << "Output written to " << output_file << endl;
    }
    
    return 0;
}