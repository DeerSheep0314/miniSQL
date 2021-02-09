#pragma once
#include"MiniSQL.h"
#include"Interpreter.h"
#include"BuffferManager.h"
#include"Index.h"
#include"Catalog.h"

buffer buf;
CatalogManager cat;
//语句解释,执行
int sqlhandle(string sqlstmt);

int sqlhandle(string sqlstmt) {
	string key1, key2;
	string filter = "  ";
	string filtered_sqlstmt = sqlstmt;//为了方便，考虑到合法sql语句关键字部分不会有合法的连续空格，故使用过滤的sqlstmt进行类型判断。但传入构造器的仍然是原语句
	str_filter(filtered_sqlstmt, filter);
	int retv=1;
	res_set res;
	if (filtered_sqlstmt.empty()) {
		return -2;//检测到空语句
	}
	char delim = ' ';
	char* tokens = strtok((char*)(filtered_sqlstmt.c_str()), &delim);
	key1 = tokens;
	if (key1 == "quit") {
		sql_quit stmt;
		retv = stmt.execute();
	}
	else if (key1 == "execfile") {
		sql_execfile stmt(sqlstmt);
		retv = stmt.execute();
	}
	else {
		tokens = strtok(NULL, &delim);
		key2 = tokens;
		string key = key1 + " " + key2;
		str_tolower(key);
		if (key == "create table") {
			sql_create_table stmt(sqlstmt);
			retv = stmt.execute();
		}
		if (key == "drop table") {
			sql_drop_table stmt(sqlstmt);
			retv = stmt.execute();
		}
		if (key == "select *") {
			sql_select stmt(sqlstmt);
			res = stmt.execute();
			cout << res << endl;
		}
		if (key == "create index") {
			sql_create_index stmt(sqlstmt);
			retv = stmt.execute();
		}
		if (key == "drop index") {
			sql_drop_index stmt(sqlstmt);
			retv = stmt.execute();
		}
		if (key == "insert into") {
			sql_insert stmt(sqlstmt);
			retv = stmt.execute();
		}
	}
	
	return retv;
}

/*实际执行成员函数*/


int  sql_create_table::execute() {
	if (dbinfo.tables.count(this->table_name) != 0) {
		string errmsg = "Table name \"" + this->table_name + "\" already exist.";
		throw errmsg;
	}
	string file_name = this->table_name + ".table";
	fstream fout(file_name,ios::out);
	if (!fout.is_open()) {
		string errmsg = "Error when creating file:" + file_name + ".";
		throw errmsg;
	}
	table new_table(this->table_name, this->columns);
	dbinfo.tables[this->table_name] = new_table;
	cat.new_table(new_table);
	cout << "Table " + this->table_name + " has been successfully created. The columns are:"<<endl;
	for (auto col : this->columns) {
		cout << setw(15) << col.second.name << "|";
	}
	cout << endl;
	return 1;
}

int  sql_drop_table::execute() {
	if (dbinfo.tables.count(this->table_name) == 0) {
		string errmsg = "Table name \"" + this->table_name + "\" does not exist.";
		throw errmsg;
	}
	string file_name = this->table_name + ".table";
	if (!remove(file_name.c_str())) {
		string errmsg = "Error when deleting file:" + file_name + ".";
		throw errmsg;
	}
	else {
		for (auto idx : dbinfo.indices) {
			if (idx.second.table_name == this->table_name) {
				dbinfo.indices.erase(idx.first);
			}
		}
		dbinfo.tables.erase(this->table_name);
		cat.updateCat();
	}
	cout << "Table " + this->table_name + " has been successfully dropped."<<endl;
	return 1;
}
int sql_quit::execute() {
	cout << "Returning to main menu"<<endl;
	//确保信息更新到了文件中
	for (int i = 0; i < MAXBLOCKNUMBER; i++)
		buf.flashBack(i);
	cat.updateCat();
	//exit(0);//如果没有错误，则退出
	dbinfo.status = _Start;
	return -1;
}
int sql_create_index::execute() {
	if (dbinfo.tables.count(this->table_name) == 0) {
		string errmsg = "Table \"" + this->table_name + "\" does not exist.";
		throw errmsg;
	}
	if (dbinfo.tables[this->table_name].columns.count(this->column_name) == 0) {
		string errmsg = "Column \"" + this->column_name + "\" does not exist.";
		throw errmsg;
	}
	if (dbinfo.indices.count(this->index_name) != 0) {
		string errmsg = "Index named \"" + this->index_name + "\" already exist.";
		throw errmsg;
	}
	if (dbinfo.tables[this->table_name].columns[this->column_name].is_unique == false) {
		string errmsg = "Column \"" + this->column_name + "\" is not unique.";
		throw errmsg;
	}
	//index new_idx(*this);
	//dbinfo.indices[new_idx.index_name] = new_idx;
	//cat.new_index(new_idx);
	cout << "Index " + this->index_name + " has been successfully created."<<endl;
	return 1;
}
int sql_drop_index::execute() {
	if (dbinfo.indices.count(this->index_name) == 0) {
		string errmsg = "Index named \"" + this->index_name + "\" does not exist.";
		throw errmsg;
	}
	dbinfo.indices.erase(this->index_name);
	cat.updateCat();
	cout << "Index " + this->index_name + " has been successfully dropped." << endl;
	return 1;
}
res_set sql_select::execute() {
	if (dbinfo.tables.count(this->table_name) == 0) {
		string errmsg = "Table \"" + this->table_name + "\" does not exist.";
		throw errmsg;
	}

	table &thetable = dbinfo.tables[this->table_name];
	string filename = thetable.tablename + ".table";

	int length = thetable.getRecordLen() + 1;
	int recordNum = BLOCKSIZE / length;//单个block里最大记录条数
	res_set res(thetable);
	for (int blockOffset = 0; blockOffset < thetable.block_num; blockOffset++) {//所有block
		int bufferNum = buf.getIfIsInBuffer(filename, blockOffset);
		if (bufferNum == -1) {
			bufferNum = buf.getEmptyBuffer();
			buf.readBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++) {//block中的每一条记录
			int position = offset * length;
			string stringrow;
			stringrow = buf.bufferBlock[bufferNum].getvalues(position, position + length);

			if (stringrow.c_str()[0] == EMPTY) continue;//inticate that this row of record have been deleted
			stringrow.erase(stringrow.begin());//把第一位去掉
			sql_tuple tmptup(thetable.tablename, stringrow);
			if (tmptup.compare(this->predicates, thetable)) {
				res.tuples.push_back(tmptup);
			}

		}
	}
	cout << to_string(res.tuples.size()) << " record(s) fetched."<<endl;
	return res;
}

int sql_delete_from::execute() {
	if (dbinfo.tables.count(this->table_name) == 0) {
		string errmsg = "Table \"" + this->table_name + "\" does not exist.";
		throw errmsg;
	}
	int count = 0;
	table &thetable = dbinfo.tables[this->table_name];
	string filename = thetable.tablename + ".table";

	int length = thetable.getRecordLen() + 1;
	int recordNum = BLOCKSIZE / length;//单个block里最大记录条数
	for (int blockOffset = 0; blockOffset < thetable.block_num; blockOffset++) {//所有block
		int bufferNum = buf.getIfIsInBuffer(filename, blockOffset);
		if (bufferNum == -1) {
			bufferNum = buf.getEmptyBuffer();
			buf.readBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++) {//block中的每一条记录
			int position = offset * length;
			string stringrow;
			stringrow = buf.bufferBlock[bufferNum].getvalues(position, position + length);

			if (stringrow.c_str()[0] == EMPTY) continue;//inticate that this row of record have been deleted
			stringrow.erase(stringrow.begin());//把第一位去掉
			sql_tuple tmptup(thetable.tablename, stringrow);
			if (tmptup.compare(this->predicates, thetable)) {//所有条件都满足
				buf.bufferBlock[bufferNum].values[position] = DELETED;
				count++;
			}
			buf.bufferBlock[bufferNum].isWritten = 1;
		}

	}
	cout << to_string(count) << " record(s) deleted." << endl;
	return count;
}
int sql_insert::execute() {
	if (dbinfo.tables.count(this->table_name) == 0) {
		string errmsg = "Table \"" + this->table_name + "\" does not exist.";
		throw errmsg;
	}
	int count = 0;
	table &thetable = dbinfo.tables[this->table_name];
	string filename = thetable.tablename + ".table";

	int length = thetable.getRecordLen() + 1;
	int recordNum = BLOCKSIZE / length;//单个block里最大记录条数
	map<string, column>::iterator it = thetable.columns.begin();
	for (int i = 0; i < thetable.columns.size(); i++) {//检验unique和primary key是否重复
		if (it->second.is_pk || it->second.is_unique) {
			string stmt = "select * from " + thetable.tablename + " where " + it->second.name + "=" + this->values[i].value;
			sql_select sqlstmt(stmt);
			res_set res = sqlstmt.execute();
			if (res.tuples.size() > 0) {
				string errmsg = "Unique or primary key column duplicated. Name:\"" + it->second.name + "\"";
				throw errmsg;
			}
		}
	}
	string insert_val = this->connect();
	if (insert_val.size() != thetable.getRecordLen()) {
		string errmsg = "Incorrect record length, recieved " + to_string(insert_val.size()) + " expecting " + to_string(thetable.getRecordLen()) + ".";
		throw errmsg;
	}
	//一切ok，进行插入
	insertPos iPos = buf.getInsertPosition(thetable);
	buf.bufferBlock[iPos.bufferNUM].values[iPos.position] = NOTEMPTY;
	for (int i = 0; i < thetable.getRecordLen(); i++) {
		buf.bufferBlock[iPos.bufferNUM].values[iPos.position + i + 1] = insert_val.c_str()[i];
	}
	buf.bufferBlock[iPos.bufferNUM].isWritten = true;
	cout << "Successfully insert a record to " + this->table_name + "." << endl;
	return 1;
}
int sql_execfile::execute() {
	fstream fin(this->filename);
	if (!fin.is_open()) {
		string errmsg = "Error when loading file:" + this->filename;
		throw errmsg;
	}
	string buffer = "";
	while (fin.eof()) {
		string line;
		getline(fin, line);
		size_t last_pos = 0;
		while (line.find(';', last_pos) != string::npos) {//找到分号
			size_t pos = line.find_first_of(';', last_pos);
			if (!is_in_quotation(line, pos)) {//第一个分号不在括号内
				string addstr = str_sub(line, pos);
				buffer += " " + addstr;
				str_filter(buffer, ' ');
				//cout << buffer << endl;
				try {
					int ret_v = sqlhandle(buffer);
					if (ret_v = -1)//有退出命令
						return -1;
				}
				catch (string e) {
					cout << e << endl;
				}
				
				buffer.clear();
			}
			else {//第一个分号在括号内
				last_pos = pos + 1;
			}
		}
		buffer += " " + line;
	}
}