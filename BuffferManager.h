#pragma once
#ifndef __buffer_h__
#define __buffer_h__
#include "MiniSQL.h"
#include "MacroDefines.h"
#include <string> 
#include <fstream>
#include <vector>

class buffer
{
public:
	
	buffer() {
		memset(bufferBlock, 0, 1000 * sizeof(block));
		//for(int i = 0; i< MAXBLOCKNUMBER; i++) bufferBlock[i].initialize();
	}
	~buffer() {
		for (int i = 0; i < MAXBLOCKNUMBER; i++)
			flashBack(i);
	}
private:
public:
	block bufferBlock[MAXBLOCKNUMBER];
	void flashBack(int bufferNum) {
		if (!bufferBlock[bufferNum].isWritten) return;
		string filename = bufferBlock[bufferNum].filename;
		fstream fout(filename.c_str(), ios::in | ios::out);
		fout.seekp(BLOCKSIZE*bufferBlock[bufferNum].blockOffset, fout.beg);
		fout.write(bufferBlock[bufferNum].values, BLOCKSIZE);
		bufferBlock[bufferNum].initialize();
		fout.close();
	}

	int getbufferNum(string filename, int blockOffset) {
		int bufferNum = getIfIsInBuffer(filename, blockOffset);
		if (bufferNum == -1) {
			bufferNum = getEmptyBufferExcept(filename);
			readBlock(filename, blockOffset, bufferNum);
		}
		return bufferNum;
	}

	void readBlock(string filename, int blockOffset, int bufferNum) {
		bufferBlock[bufferNum].isValid = 1;
		bufferBlock[bufferNum].isWritten = 0;
		bufferBlock[bufferNum].filename = filename;
		bufferBlock[bufferNum].blockOffset = blockOffset;
		fstream  fin(filename.c_str(), ios::in);
		fin.seekp(BLOCKSIZE*blockOffset, fin.beg);
		fin.read(bufferBlock[bufferNum].values, BLOCKSIZE);
		fin.close();
	}

	void writeBlock(int bufferNum) {
		bufferBlock[bufferNum].isWritten = 1;
		fstream  fout(bufferBlock[bufferNum].filename.c_str(), ios::out | ios::binary);
		fout.seekp(BLOCKSIZE*bufferBlock[bufferNum].blockOffset, fout.beg);
		fout.write(bufferBlock[bufferNum].values, BLOCKSIZE);
		useBlock(bufferNum);
	}

	void useBlock(int bufferNum) {//this LRU algorithm is quite expensive
		for (int i = 1; i < MAXBLOCKNUMBER; i++)
		{
			if (i == bufferNum) {
				bufferBlock[bufferNum].LRUvalue = 0;
				bufferBlock[i].isValid = 1;
			}
			else bufferBlock[bufferNum].LRUvalue++;	//The greater is LRUvalue, the less useful the block is
		}
	}

	int getEmptyBuffer() {
		int bufferNum = 0;
		int highestLRUvalue = bufferBlock[0].LRUvalue;
		for (int i = 0; i < MAXBLOCKNUMBER; i++)
		{
			if (!bufferBlock[i].isValid) {
				bufferBlock[i].initialize();
				bufferBlock[i].isValid = 1;
				return i;
			}
			else if (highestLRUvalue < bufferBlock[i].LRUvalue)
			{
				highestLRUvalue = bufferBlock[i].LRUvalue;
				bufferNum = i;
			}
		}
		flashBack(bufferNum);
		bufferBlock[bufferNum].isValid = 1;
		return bufferNum;
	}

	int getEmptyBufferExcept(string filename) {	//buffer with the destine filename is not suppose to be flashback
		int bufferNum = -1;
		int highestLRUvalue = bufferBlock[0].LRUvalue;
		for (int i = 0; i < MAXBLOCKNUMBER; i++)
		{
			if (!bufferBlock[i].isValid) {
				bufferBlock[i].isValid = 1;
				return i;
			}
			else if (highestLRUvalue < bufferBlock[i].LRUvalue && bufferBlock[i].filename != filename)
			{
				highestLRUvalue = bufferBlock[i].LRUvalue;
				bufferNum = i;
			}
		}
		if (bufferNum == -1) {
			cout << "No more unused buffer!" << endl;
			exit(0);
		}
		flashBack(bufferNum);
		bufferBlock[bufferNum].isValid = 1;
		return bufferNum;
	}

	insertPos getInsertPosition(table& thetable) {//To increase efficiency, we always insert values from the back of the file
		insertPos ipos;
		if (thetable.block_num == 0) {//the *.table file is empty and the data is firstly inserted
			ipos.bufferNUM = addBlockInFile(thetable);
			ipos.position = 0;
			return ipos;
		}
		string filename = thetable.tablename + ".table";
		int length = thetable.getRecordLen() + 1;
		int blockOffset = thetable.block_num - 1;//get the block offset in file of the last block
		int bufferNum = getIfIsInBuffer(filename, blockOffset);
		if (bufferNum == -1) {//indicate that the data is not read in buffer yet
			bufferNum = getEmptyBuffer();
			readBlock(filename, blockOffset, bufferNum);
		}
		const int recordNum = BLOCKSIZE / length;
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * length;
			char isEmpty = bufferBlock[bufferNum].values[position];
			if (isEmpty == EMPTY) {//find an empty space
				ipos.bufferNUM = bufferNum;
				ipos.position = position;
				return ipos;
			}
		}
		//if the program run till here, the last block is full, therefor one more block is added
		ipos.bufferNUM = addBlockInFile(thetable);
		ipos.position = 0;
		return ipos;
	}

	int addBlockInFile(table& thetable) {//add one more block in file for the table
		int bufferNum = getEmptyBuffer();
		bufferBlock[bufferNum].initialize();
		bufferBlock[bufferNum].isValid = 1;
		bufferBlock[bufferNum].isWritten = 1;
		bufferBlock[bufferNum].filename = thetable.tablename + ".table";
		bufferBlock[bufferNum].blockOffset = thetable.block_num++;
		return bufferNum;
	}

	int addBlockInFile(index& indexinfor) {//add one more block in file for the index
		string filename = indexinfor.index_name + ".index";
		int bufferNum = getEmptyBufferExcept(filename);
		bufferBlock[bufferNum].initialize();
		bufferBlock[bufferNum].isValid = 1;
		bufferBlock[bufferNum].isWritten = 1;
		bufferBlock[bufferNum].filename = filename;
		bufferBlock[bufferNum].blockOffset = indexinfor.block_num++;
		return bufferNum;
	}

	int getIfIsInBuffer(string filename, int blockOffset) {
		for (int bufferNum = 0; bufferNum < MAXBLOCKNUMBER; bufferNum++)
			if (bufferBlock[bufferNum].filename == filename && bufferBlock[bufferNum].blockOffset == blockOffset)	return bufferNum;
		return -1;	//indicate that the data is not read in buffer yet
	}

	void scanIn(table thetable) {//this is dangerous because
		string filename = thetable.tablename + ".table";
		fstream  fin(filename.c_str(), ios::in);
		for (int blockOffset = 0; blockOffset < thetable.block_num; blockOffset++) {
			if (getIfIsInBuffer(filename, blockOffset) == -1) {	//indicate that the data is not read in buffer yet
				int bufferNum = getEmptyBufferExcept(filename);
				readBlock(filename, blockOffset, bufferNum);
			}
		}
		fin.close();
	}

	void setInvalid(string filename) {//when a file is deleted, a table or an index, all the value in buffer should be set invalid
		for (int i = 0; i < MAXBLOCKNUMBER; i++)
		{
			if (bufferBlock[i].filename == filename) {
				bufferBlock[i].isValid = 0;
				bufferBlock[i].isWritten = 0;
			}
		}
	}
public:
	/*This function can show the values in the buffer, which is for debug only
	Take care when using. If the size of buffer is very big, your computer may
	crash down*/
	void ShowBuffer(int from, int to) {
		const int max = 30;
		if (!(0 <= from && from <= to && to < MAXBLOCKNUMBER)) {
			cout << "No Such buffer" << endl;
			return;
		}
		if ((to - from) > max) {
			cout << "Only" << max << " buffer(s)" << " can be displayed at one time." << endl;
			return;
		}
		for (int i = from; i <= to; i++) {
			ShowBuffer(i);
		}
	}
	void ShowBuffer(int bufferNum) {
		cout << "BlockNumber: " << bufferNum << endl;
		cout << "IsWritten: " << bufferBlock[bufferNum].isWritten << endl;
		cout << "IsValid: " << bufferBlock[bufferNum].isValid << endl;
		cout << "Filename: " << bufferBlock[bufferNum].filename << endl;
		cout << "blockOffset: " << bufferBlock[bufferNum].blockOffset << endl;
		cout << bufferBlock[bufferNum].values << endl;
	}
};

#endif
