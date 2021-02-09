#pragma once
#include<string>
#include<iostream>
#include<vector>
#include<sstream>
/*���ܺ�������Ҫ����һЩ�鷳���ַ�������*/
using namespace std;
void str_filter(string& str, char ch);//������β�ո�
void str_filter(string& str,string& filter);//���˶���ո�
string str_extract(string& str, char left, char right);//��ȡ���������е����ݣ�����string������ʹ�ã�����ʵ��ʹ�������ֻ�Ƿ��ص�һ�������һ���м������
string str_extract_first(string& str, char left, char right);//��ȡ��ӽ�һ������ڵ����ݣ�������ҷ�����ͬ�����Զ�Ϊ�ڶ���
string str_sub(string& str, const char* filter);//tokenizer ���ַ���������filter�ָ����ֵΪ�и��µĲ���
string str_sub(string& str, size_t pos);//����λ�ö��ַ��������и�,��������token
vector<string> str_tokenizer(string str,const char* filter);//��ȫ��tokenizer������ֵΪ�и��µķǿ�token����
void str_tolower(string& str);//ת��ΪСд
//�Ƿ��Ǹ�����
bool is_float(string str);
bool is_int(string str);//�Ƿ�������
//ĳλ���ַ�,�Ƿ����������÷�Χ
bool is_in_quotation(string str, size_t pos);
//��ָ��λ���и�ָ�������ַ��������ر����µ��ַ���
string str_sub(string& str, size_t pos, size_t offs);
//������������ַ����зָ��tokenizer,���ϸ�ƥ��
vector<string> str_tokenizer_quot_strict(string str, const char* filter);
//����Ӵ��Ƿ��������Ϊ�����ĵ���
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
	if (pos != string::npos) {//�ҵ��Ӵ�
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
		if (pos == string::npos) {//δ�ҵ�ָ��Ԫ��
			ret_vec.push_back(str);
			break;
		}
		if (!is_in_quotation(str, pos)) {//���������ڣ��ָ�
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

//������������ַ����зָ��tokenizer,���ϸ�ƥ��
vector<string> str_tokenizer_quot_strict(string str, const char* filter) {
	size_t last_pos = 0, pos = 0;
	vector<string> ret_vec;
	pos = str.find(filter, last_pos);
	while (!str.empty()) {
		if (pos == string::npos) {//δ�ҵ�ָ��Ԫ��
			ret_vec.push_back(str);
			break;
		}

		if (!is_in_quotation(str, pos) && is_independ(str, pos, strlen(filter))) {//���������ڣ���ƥ��ɹ�,�ָ�
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
	if (str.find_first_of("0x") == 0) {//ʮ������
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
	if (str.find_first_of("0x") == 0) {//ʮ������
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

int stoi_h(string str) {//��ת��16���Ƶ�stoi
	int ret_val;
	if (str.find("0x") != string::npos) {//���Ϊ16���Ʊ��
		stringstream ss;
		ss << hex << str;
		ss >> ret_val;
	}
	else {
		ret_val = stoi(str);
	}
	return ret_val;
}
float stof_h(string str) {//��ת��16���Ƶ�stof
	float ret_val;
	if (str.find("0x") != string::npos) {//���Ϊ16���Ʊ��
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
	int is_in_sin = 0, is_in_dbl = 0;//binary, ������Ϊ0��1��˫����flagΪ0��2
	for (int i = 0; i < str.size(); i++) {
		switch (is_in_sin | is_in_dbl) {
		case 0://����������
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
		case 1://�ڵ�������
			if (i == pos) {
				find = 1;
				break;
			}
			if (str[i] == '\'') {//�ǵ�����
				if (str[i - 1] == '\\') {//ת�嵥����
					break;
				}
				else {//��ת�嵥����
					sin_quo_count++;
					is_in_sin = 0;
				}
			}
			break;
		case 2://��˫������
			if (i == pos) {
				find = 1;
				break;
			}
			if (str[i] == '"') {//��˫����
				if (str[i - 1] == '\\') {//ת��˫����
					break;
				}
				else {//��ת��˫����
					dbl_quo_count++;
					is_in_dbl = 0;
				}
			}
			break;
		}
		if (find == 1)
			break;
	}
	if ((is_in_sin | is_in_dbl) == 0) {//����������
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
	//��⿪ͷ,��Ҫ���ڵ�һλ��ǰһλ�ǿո�
	if (start == 0 || str[start - 1] == ' ' || str[start - 1] == '\t') {//����Ҫ��
		//���󲿣���Ҫ�ڽ�β���һλ�ǿո�
		if (start + len - 1 == str.size() || str[len + start] == ' ' || str[len + start] == '\t') {
			return true;
		}
	}
	return false;
}