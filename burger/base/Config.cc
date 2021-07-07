#include "Config.h"
#include <unistd.h>
#include <stdlib.h>
namespace burger {

Config& Config::Instance(const std::string& relativePath) {
    static Config conf(relativePath);
    return conf;
}

uint16_t Config::getUInt16(const std::string& section, const std::string& search, int defaultVal) {
    return static_cast<uint16_t>(getInt(section, search, defaultVal));
}

int Config::getInt(const std::string& section, const std::string& search, int defaultVal) {
    std::string expression = reader_->Get(section, search, std::to_string(defaultVal));
    // return reader_->GetInteger(section, search, defaultVal);
    return detail::getIntFromStringExpression(std::move(expression));
}

size_t Config::getSize(const std::string& section, const std::string& search, int defaultVal) {
    return static_cast<size_t>(getInt(section, search, defaultVal));
}

std::string Config::getString(const std::string& section, const std::string& search, const std::string& defaultVal) {
    return reader_->Get(section, search, defaultVal);
}

bool Config::getBool(const std::string& section, const std::string& search, bool defaultVal) {
    return reader_->GetBoolean(section, search, defaultVal);
}

double Config::getDouble(const std::string& section, const std::string& search, double defaultVal) {
    return reader_->GetDouble(section, search, defaultVal);
}

// todo: 这种方式不够好，修改灵活点，不要写死,或者写一个setConfigFile()
Config::Config(const std::string& relativePath)
    : relativePath_(relativePath) {
    // std::string filePath = detail::getFilePath() + relativePath_;
     
    std::string filePath(getenv("HOME"));
    // printf("%s\n", filePath.c_str());   // for test
    filePath += relativePath;
    // printf("%s\n", filePath.c_str());   // for test
    reader_ = util::make_unique<INIReader>(filePath);
    if(reader_->ParseError() != 0) {
        ERROR("Can't load config file at: {}", filePath);
    }
}

namespace detail {

int getPriority(const char& ch) {
    if(ch == '+' || ch == '-') return 1;
    if(ch == '*' || ch == '/') return 2;
    return -1;
}

bool isDigit(char ch) {
	if (ch >= '0' && ch <= '9') {
		return true;
	}
	return false;
}

bool isOperator(const std::string& str) {
	if (str[0] >= '0' && str[0] <= '9') {
		return false;
	}
	return true;
}

int calculate(int left, int right, const std::string& op) {
	int result = 0;
    const char& ch = op[0];
	switch (ch)
	{
	case '+':
		result = left + right;
		break;
	case '-':
		result = left - right;
		break;
	case '*':
		result = left * right;
		break;
	case '/':
		result = left / right;
		break;
	}
	return result;
}

int getIntFromStringExpression(const std::string& expression) {
    std::vector<std::string> suffix;
    std::stack<char> operators;

    size_t i = 0, len = expression.size();
	while(i < len) {
        char cur = expression[i];
        if(cur == ' ') {
            ++i;
            continue;
        }
		if(isDigit(cur)) {     // 数字直接入栈
            size_t start = i;
            while(isDigit(expression[i])) {
                ++i;
            }
            suffix.push_back(expression.substr(start, i - start));
        }
		else {
		    if(operators.empty() || cur == '(') {
		        operators.push(cur);
	        }
	        else {
		        if(cur != ')') {    // " + - * / "
			        while (!operators.empty() && getPriority(operators.top()) >= getPriority(cur)){
                        suffix.emplace_back(1, operators.top());
                        operators.pop();
			        }
			        operators.push(cur);
		        }
		        else { // 右括号：将栈中左括号之后入栈的运算符全部出栈到表达式结果，左括号出栈
                    while (operators.top() != '(') {
                        suffix.emplace_back(1, operators.top());
                        operators.pop();
                    }
			        operators.pop();
		        }
	        }
			++i;
		}
	}
    while(!operators.empty()) {
        suffix.emplace_back(1, operators.top());
	    operators.pop();
    }

    std::stack<int> nums;
    for(const auto& str : suffix) {
        if(isOperator(str)) {
            int num_1 = nums.top();
            nums.pop();
            int num_2 = nums.top();
            nums.pop();
        
            nums.push(calculate(num_2, num_1, str));
        }
        else {
            nums.push(atoi(str.c_str()));
        }       
    }
    return nums.top();
}


// std::string getFilePath() {
//     char buffer[1024];   
//     if(!getcwd(buffer, sizeof(buffer))) {
//         ERROR("Can't get current work directory.")
//         return "";
//     }

//     std::string path(buffer);
//     std::string mainPath("Burger");
//     size_t idx = path.rfind(mainPath);
//     if(idx == path.npos) {
//         ERROR("Can't get 'Burger' directory.")
//         return "";
//     }

//     path.resize(idx + mainPath.size());
//     return path;
// }
}  // detail
} // burger