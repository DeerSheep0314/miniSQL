#pragma once
#ifndef _CATALOG_
#define _CATALOG_

#include"MiniSQL.h"
extern database dbinfo;
class CatalogManager {
private:
public:
	CatalogManager();
	//更新catalog
	int updateCat();
	//新表
	int new_table(table& new_tbl);
	//新索引
	//int new_index(index& idx);
	//退出时自动更新一次
	~CatalogManager() {
		updateCat();
	}
};

/*
*.db文件格式实例
<table>
table1 2 //table name | block size
cloumn1 0 10 1 1 //column name | data type | data size | unique flag | pk flag
column2 2 33 0 0
...
</table>
<table>
...
</table>
<indices>
...
</indices>
*/

//
//TODO:索引文件的catalog信息读写
//

CatalogManager::CatalogManager() {
	string file_name = (string)DBNAME + ".db";
	vector<string> temp_tbl;
	fstream fin(file_name,ios::in);
	if (!fin.is_open()) {
		string errmsg = "Error when loading file:" + file_name;
		throw errmsg;
	}
	enum mode { TBL , IDX, STANDBY };
	mode md= STANDBY;
	while (!fin.eof()) {
		string temp;
		getline(fin, temp);
		switch (md) {
		case STANDBY:
			if (temp == "<table>") {
				md = TBL;
			}
			else if(temp=="<index>"){
				md = IDX;
			}
			break;
		case TBL:
			if (temp == "</table>") {
				md = STANDBY;
				table new_tbl(temp_tbl);
				if (dbinfo.tables.count(new_tbl.tablename) != 0) {
					cout << "Warning::table named \"" + new_tbl.tablename + "\" already exisit. Continue? y/n \n";
					string usr_selection;
					getline(cin, usr_selection); 
					str_tolower(usr_selection);
					if(usr_selection=="y"||usr_selection==""){
					}
					else {
						continue;
					}
				}
				dbinfo.tables[new_tbl.tablename] = new_tbl;
				temp_tbl.clear();
			}
			else {
				temp_tbl.push_back(temp);
			}
			break;
		case IDX:
			if (temp == "</index>") {
				md = STANDBY;
			}
			else {
				//idxnames.push_back(temp);
			}
			break;
		}
		
	}
	
	if (md != STANDBY) {
		cout << "Warning::unsafe catalog file loading:some datas may have not been loaded successfully."<<endl;
	}
}

int CatalogManager::updateCat() {
	string file_name = (string)DBNAME + ".db";
	fstream fout(file_name,ios::out|ios::trunc);
	if (!fout.is_open()) {
		string errmsg = "Error when opening file to write:" + file_name;
		throw errmsg;
	}
	for (auto tbl : dbinfo.tables) {
		vector<string> tbl_info = tbl.second.Cataloginfo();
		for (auto line : tbl_info) {
			fout<<line<<endl;	
		}
	}
	fout.close();
	return 1;
}
int CatalogManager::new_table(table& new_tbl) {
	string file_name = (string)DBNAME + ".db";
	fstream fout(file_name, ios::out | ios::app);
	vector<string> tbl_info = new_tbl.Cataloginfo();
	for (auto line : tbl_info) {
		fout << line << endl;
	}
	fout.close();
	return 1;
}

#endif // !_CATALOG_
