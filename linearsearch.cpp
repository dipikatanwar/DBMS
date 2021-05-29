#include<iostream>
#include "file_manager.h"
#include "errors.h"
#include<cstring>
#include<math.h>
#include<fstream>
#include<bits/stdc++.h>

using namespace std;

int curr_page_num_out=-1;
int curr_offset_out=-1;
int N; // integers on one page

void printFile(FileHandler& fh)
{
	
	PageHandler fph = fh.FirstPage ();
	int curr_page_num = fph.GetPageNum();
	fh.UnpinPage(curr_page_num);
	PageHandler lph = fh.LastPage ();
	int last_page_num = lph.GetPageNum();
	fh.UnpinPage(last_page_num);

	while(curr_page_num <= last_page_num)
	{
		PageHandler cph = fh.PageAt(curr_page_num);
		char* data = cph.GetData ();
		int num;
		int index = 0;
		int offset=0;
		while( offset < N)
		{
			memcpy (&num, &data[index], sizeof(int));
			cout <<"PageNum: "<<curr_page_num<<" Offset: "<<offset<<" Number: " << num << endl;
			//if(num == INT_MIN) return;
			offset++;
			index = index+sizeof(int);
		}
		fh.UnpinPage(curr_page_num);
		curr_page_num++;
	}
}

int extractNum(string line)
{
	int digits = line.length() - 7;
	if(digits < 1)
	{
		//cout<<"Error:InValid Query"<<endl;
		return INT_MIN;
	}
	int i = 7;
	int value = 0;
	while(line[i] != '\0')
	{
		char a = line[i];
		value = (a - '0')*pow(10,(digits-1)) + value;
		i++;
		digits--;
	}
	return value;
}

void initializePage(PageHandler& ph)
{
	int curr_offset = 0;
	char *data=ph.GetData();
	for(int i=0;i < N; i++)
	{
		int index = curr_offset*sizeof(int);
		int num = INT_MIN;
		memcpy (&data[index], &num, sizeof(int));
		curr_offset++;
	}
}
void store(int pageNum,int offset,FileHandler & out_fh)
{
	if(curr_offset_out >= N)
	{
		out_fh.FlushPage(curr_page_num_out);
		if(pageNum == INT_MIN) return;
		PageHandler ph = out_fh.NewPage();
		initializePage(ph);
		curr_page_num_out = ph.GetPageNum();
		out_fh.UnpinPage(curr_page_num_out);
		curr_offset_out=0;
	}
	PageHandler ph = out_fh.PageAt(curr_page_num_out);
	int index = curr_offset_out*sizeof(int);
	char *data=ph.GetData();
	memcpy (&data[index], &pageNum, sizeof(int));
	curr_offset_out++;
	if(pageNum != INT_MIN)
	{
		index = curr_offset_out*sizeof(int);
		memcpy (&data[index], &offset, sizeof(int));
		curr_offset_out++;
	}
}

void linear_search(FileHandler& in_fh,int search_num,FileHandler& out_fh)
{
	PageHandler fph = in_fh.FirstPage ();
	int curr_page_num = fph.GetPageNum();
	in_fh.UnpinPage(curr_page_num);

	PageHandler lph = in_fh.LastPage ();
	int last_page_num = lph.GetPageNum();
	in_fh.UnpinPage(last_page_num);

	while(curr_page_num <= last_page_num)
	{
		PageHandler cph = in_fh.PageAt(curr_page_num);
		char* data = cph.GetData ();
		int num;
		int index = 0;
		int offset=0;
		while(offset < N)
		{
			memcpy (&num, &data[index], sizeof(int));
			if(search_num == num)
			{
				store(curr_page_num,offset,out_fh);
			}
			offset++;
			index = index+sizeof(int);
		}
		in_fh.UnpinPage(curr_page_num);
		curr_page_num++;
	}
	store(-1,-1,out_fh);
}

int main(int argc, char* argv[]) {

	if(argc < 4)
	{
		cout<<"Error: Not enough arguments!!"<<endl;
		return 0;
	}
	FileManager fm;
    FileHandler in_fh = fm.OpenFile (argv[1]);
	
	fm.DestroyFile (argv[3]);
	FileHandler out_fh = fm.CreateFile (argv[3]);
	PageHandler ph = out_fh.NewPage ();
	curr_page_num_out = ph.GetPageNum();
	curr_offset_out = 0;

	N = floor(PAGE_CONTENT_SIZE / sizeof(int));
	initializePage(ph);
	//printFile(in_fh);

     ifstream fp;
     fp.open(argv[2]);
	string line;
	 while (fp) {
        getline(fp, line);
		int num = extractNum(line);
        if( num != INT_MIN)
			linear_search(in_fh,num,out_fh);
    }
	
	fp.close();
	fm.CloseFile (in_fh);
	
	if(curr_page_num_out != -1)
	{
		out_fh.FlushPage(curr_page_num_out);
    	//printFile(out_fh);
	}
	fm.CloseFile (out_fh);
}
