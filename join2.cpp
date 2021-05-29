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

struct location 
{
	int pageNum;
	int offset;
	int count;
	location(int a,int b,int c)
	{
		pageNum = a;
		offset = b;
		count = c;
	}
};
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

void initializePage(PageHandler ph)
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

void store(int num,FileHandler & out)
{
    if(curr_offset_out >= N || curr_page_num_out == -1)
	{
		if(curr_page_num_out != -1) // THis is first page creating no previous page to flush.
			out.FlushPage(curr_page_num_out);
		
		if(num == INT_MIN) return;
		PageHandler ph = out.NewPage();
		initializePage(ph);
		curr_page_num_out = ph.GetPageNum();
		out.UnpinPage(curr_page_num_out);
		curr_offset_out=0;
	}
	PageHandler ph = out.PageAt(curr_page_num_out);
	int index = curr_offset_out*sizeof(int);
	char *data=ph.GetData();
	memcpy (&data[index], &num, sizeof(int));
	curr_offset_out++;
	index = curr_offset_out*sizeof(int);
}

struct location linear_search(FileHandler& in_fh,int search_num)
{
	PageHandler fph = in_fh.FirstPage ();
	int curr_page_num = fph.GetPageNum();
	in_fh.UnpinPage(curr_page_num);

	PageHandler lph = in_fh.LastPage ();
	int last_page_num = lph.GetPageNum();
	in_fh.UnpinPage(last_page_num);

	int located_page = -1;
	int located_offset = -1;
	int located_count = 0;
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
				if(located_page == -1)
				{
					located_page = curr_page_num;
					located_offset = offset;
				}
				located_count++;
			}
			offset++;
			index = index+sizeof(int);
		}
		in_fh.UnpinPage(curr_page_num);
		curr_page_num++;
	}
	return location(located_page,located_offset,located_count);
}

void join2(FileHandler & in1, FileHandler & in2,FileHandler & out)
{
	PageHandler fph_in1 = in1.FirstPage ();
	int in1_first_pgnum = fph_in1.GetPageNum();
	in1.UnpinPage(in1_first_pgnum);
	PageHandler lph_in1 = in1.LastPage ();
	int in1_last_pgnum = lph_in1.GetPageNum();
	in1.UnpinPage(in1_last_pgnum);

    for(int c_in1 = in1_first_pgnum; c_in1<= in1_last_pgnum; c_in1++)
    {
		PageHandler cph_in1 = in1.PageAt(c_in1);
		char* data1 = cph_in1.GetData ();
		int num1; 
		int index1=0;
		for(int i = 0; i < N; i++)
		{
			memcpy (&num1, &data1[index1], sizeof(int));
			index1 = index1+sizeof(int);
			location found = linear_search(in2,num1);
			if(found.pageNum == -1) continue;
			else
			{
				int j = 0;
				//cout<<"Page "<<found.pageNum<<"Off: "<<found.offset<<"NUmber: "<<num1<<"count "<<found.count<<endl;
				while(j < found.count)
				{
					store(num1,out);
					j++;
				}
			}
		}
        in1.UnpinPage(c_in1);
    }
}

int main(int argc, char* argv[]) {

	if(argc < 4)
	{
		cout<<"Error: Not enough arguments!!"<<endl;
		return 0;
	}
	FileManager fm;
    FileHandler in1_fh = fm.OpenFile (argv[1]);
    FileHandler in2_fh = fm.OpenFile (argv[2]);

   	fm.DestroyFile (argv[3]);
	FileHandler out_fh = fm.CreateFile (argv[3]);
	curr_page_num_out = -1;

    N = floor(PAGE_CONTENT_SIZE / sizeof(int));
   	join2(in1_fh,in2_fh,out_fh);
	//printFile(in2_fh);
    fm.CloseFile (in1_fh);
    fm.CloseFile (in2_fh);

	if(curr_page_num_out != -1)
	{
		out_fh.FlushPage(curr_page_num_out);
    	//printFile(out_fh);
	}
	fm.CloseFile (out_fh);
}