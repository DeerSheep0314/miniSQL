#pragma once
#include<string>
#include<iostream>
#include<vector>
#include<sstream>
/*功能函数，主要进行一些麻烦的字符串处理*/
using namespace std;
void str_filter(string& str, char ch);//过滤首尾空格
void str_filter(string& str,string& filter);//过滤多余空格
string str_extract(string& str, char left, char right);//提取两个符号中的内容，返回string，谨慎使用，鉴于实际使用情况，只是返回第一个和最后一个中间的内容
string str_extract_first(string& str, char left, char right);//提取最接近一组符号内的内容，如果左右符号相同，则自动为第二个
string str_sub(string& str, const char* filter);//tokenizer 将字符串按传入filter分割，返回值为切割下的部分
string str_sub(string& str, size_t pos);//按照位置对字符串进行切割,返回切下token
vector<string> str_tokenizer(string str,const char* filter);//完全体tokenizer，返回值为切割下的非空token容器
void str_tolower(string& str);//转换为小写
//是否是浮点数
bool is_float(string str);
bool is_int(string str);//是否是整数
//某位置字符,是否在引号作用范围
bool is_in_quotation(string str, size_t pos);
//从指定位置切割指定长度字符串，返回被切下的字符串
string str_sub(string& str, size_t pos, size_t offs);
//不会对引号内字符进行分割的tokenizer,且严格匹配
vector<string> str_tokenizer_quot_strict(string str, const char* filter);
//检测子串是否独立，即为单独的单词
bool is_independ(string& str, size_t start, size_t len);

bool is_digit(string& str);
void str_filter(string& str, char ch) {
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == '\t')
			str[i] = ' ';
	}
	if (str.empty()) {
		//string errmsg = "In function str_filter: parameter \"str\" can't be empty!";
		//throw errmsg;
	}
	else {

		str.erase(0, str.find_first_not_of(ch));
		str.erase(str.find_last_not_of(ch) + 1);

	}
}

void str_filter(string& str, string& filter) {
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == '\t')
			str[i] = ' ';
	}
	while (str.find(filter) != string::npos) {
		str.erase(str.find(filter), 1);
	}
}

string str_extract(string& str, char left, char right) {
	int first = str.find_first_of(left);
	int last = str.find_last_of(right);
	string ret_str = "";
	if (first >= last) {
		string errmsg = "Incorrect symbol match in string \"" + str + "\" with "+left;
		errmsg += " and " + right;
		errmsg += ".";
		throw errmsg;
	}
	else {
		ret_str = str.substr(first + 1, last - first - 1);
		
	}
	return ret_str;
}
string str_sub(string& str, const char* filter) {
	string token;
	size_t pos;
	if (strcmp(filter, " ") == 0)
		pos = str.find_first_of(filter) < str.find_first_of('\t') ? str.find_first_of(filter) : str.find_first_of('\t');
	else
		pos = str.find(filter);
	if (pos != string::npos) {//找到子串
		token = str.substr(0, pos);
		str = str.substr(pos + strlen(filter));
	}
	else {
		token = str;
		str = "";
	}
	return token;
}

string str_sub(string& str, size_t pos, size_t offs) {
	if (pos >= str.size()) {
		string ret_str = str;
		str = "";
		return str;
	}
	string ret_str = str.substr(0, pos);
	if (pos + offs >= str.size()) {
		str = "";
	}
	else
		str = str.substr(pos + offs);
	return ret_str;
}


bool is_digit(string& str) {
	int dotcount = 0;
	for (int i = 0; i < str.length(); i++) {
		if ((str[i] < 48 || str[i]>57)&&(str[i]!='.'||str[i]=='.'&&dotcount>=1)&&(i!=0||str[i]!='-'))
			return false;
		if (str[i] == '.')
			dotcount++;
	}
	return true;
}

string str_extract_first(string& str, char left, char right) {
	int first = str.find_first_of(left);
	int second = str.find_first_of(right, first + 1);
	string ret_str = str.substr(first + 1, second - first - 1);
	return ret_str;
}

vector<string> str_tokenizer(string str, const char* filter) {
	vector<string> ret_vec;
	while (!str.empty()) {
		string token = str_sub(str, filter);
		if (!token.empty()) {
			ret_vec.push_back(token);
		}
	}
	return ret_vec;
}

vector<string> str_tokenizer_quot(string str, const char* filter) {
	size_t last_pos = 0, pos = 0;
	vector<string> ret_vec;
	pos = str.find(filter, last_pos);
	while (!str.empty()) {
		if (pos == string::npos) {//未找到指定元素
			ret_vec.push_back(str);
			break;
		}
		if (!is_in_quotation(str, pos)) {//不在引号内，分割
			string token = str_sub(str, pos, strlen(filter));
			ret_vec.push_back(token);
			last_pos = 0;
			pos = str.find(filter, last_pos);
		}
		else {
			last_pos = pos + strlen(filter);
			pos = str.find(filter, last_pos);
		}

	}
	return ret_vec;
}

//不会对引号内字符进行分割的tokenizer,且严格匹配
vector<string> str_tokenizer_quot_strict(string str, const char* filter) {
	size_t last_pos = 0, pos = 0;
	vector<string> ret_vec;
	pos = str.find(filter, last_pos);
	while (!str.empty()) {
		if (pos == string::npos) {//未找到指定元素
			ret_vec.push_back(str);
			break;
		}

		if (!is_in_quotation(str, pos) && is_independ(str, pos, strlen(filter))) {//不在引号内，且匹配成功,分割
			string token = str_sub(str, pos, strlen(filter));
			ret_vec.push_back(token);
			last_pos = 0;
			pos = str.find(filter, last_pos);
		}
		else {
			last_pos = pos + strlen(filter);
			pos = str.find(filter, last_pos);
		}

	}
	return ret_vec;
}


void str_tolower(string& str) {
	for (int i = 0; i < str.length(); i++) {
		if (str[i] >= 'A'&&str[i] <= 'Z') {
			str[i] = str[i] + 32;
		}
	}
}

bool is_int(string str) {
	str_tolower(str);
	if (str.find_first_of("0x") == 0) {//十六进制
		str_sub(str, "0x");
		for (int i = 0; i < str.length(); i++) {
			if ((str[i]<'0' || str[i]>'9') && (str[i]<'a' || str[i]>'f'))
				return false;
		}
		return true;
	}
	else {
		for (int i = 0; i < str.length(); i++) {

			if (i == 0 && str[i] == '-')
				continue;

			if (str[i]<'0' || str[i]>'9')
				return false;
		}
		return true;
	}
}
bool is_float(string str) {
	if (str.find_first_of("0x") == 0) {//十六进制
		str_sub(str, "0x");
		for (int i = 0; i < str.length(); i++) {
			if ((str[i]<'0' || str[i]>'9') && (str[i]<'a' || str[i]>'f'))
				return false;
		}
		return true;
	}
	else {
		int dot_count = 0;
		for (int i = 0; i < str.length(); i++) {

			if (i == 0 && str[i] == '-')
				continue;

			if (str[i]<'0' || str[i]>'9') {
				if (str[i] == '.'&&dot_count==0) {
					dot_count++;
					continue;
				}
				else {
					return false;
				}
			}
				
		}
		return true;
	}
}

int stoi_h(string str) {//可转换16进制的stoi
	int ret_val;
	if (str.find("0x") != string::npos) {//检测为16进制表达
		stringstream ss;
		ss << hex << str;
		ss >> ret_val;
	}
	else {
		ret_val = stoi(str);
	}
	return ret_val;
}
float stof_h(string str) {//可转换16进制的stof
	float ret_val;
	if (str.find("0x") != string::npos) {//检测为16进制表达
		stringstream ss;
		ss << hex << str;
		ss >> ret_val;
	}
	else {
		ret_val = stof(str);
	}
	return ret_val;
}
bool is_in_quotation(string str, size_t pos) {
	if (str.find('\'') == string::npos&&str.find('"') == string::npos)
		return false;
	int find = 0;
	int sin_quo_count = 0, dbl_quo_count = 0;
	int is_in_sin = 0, is_in_dbl = 0;//binary, 单引号为0，1，双引号flag为0，2
	for (int i = 0; i < str.size(); i++) {
		switch (is_in_sin | is_in_dbl) {
		case 0://不在引号中
			if (i == pos) {
				find = 1;
				break;
			}
			if (str[i] == '\'') {
				is_in_sin = 1;
				sin_quo_count++;

			}
			else if (str[i] == '"') {
				is_in_dbl = 2;
				dbl_quo_count++;
			}
			break;
		case 1://在单引号中
			if (i == pos) {
				find = 1;
				break;
			}
			if (str[i] == '\'') {//是单引号
				if (str[i - 1] == '\\') {//转义单引号
					break;
				}
				else {//非转义单引号
					sin_quo_count++;
					is_in_sin = 0;
				}
			}
			break;
		case 2://在双引号中
			if (i == pos) {
				find = 1;
				break;
			}
			if (str[i] == '"') {//是双引号
				if (str[i - 1] == '\\') {//转义双引号
					break;
				}
				else {//非转义双引号
					dbl_quo_count++;
					is_in_dbl = 0;
				}
			}
			break;
		}
		if (find == 1)
			break;
	}
	if ((is_in_sin | is_in_dbl) == 0) {//不在引号中
		return false;
	}
	else {
		return true;
	}
}
string str_sub(string& str, size_t pos) {
	string ret_str = str.substr(0, pos);
	str = str.substr(pos + 1);
	return ret_str;
}
bool is_independ(string& str, size_t start, size_t len) {
	//检测开头,需要是在第一位或前一位是空格
	if (start == 0 || str[start - 1] == ' ' || str[start - 1] == '\t') {//符合要求
		//检测后部，需要在结尾或后一位是空格
		if (start + len - 1 == str.size() || str[len + start] == ' ' || str[len + start] == '\t') {
			return true;
		}
	}
	return false;
}