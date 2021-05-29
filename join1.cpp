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
		//	if(num == INT_MIN) return;
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

void join1(FileHandler & in1, FileHandler & in2,FileHandler & out)
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
		    
		PageHandler fph_in2 = in2.FirstPage ();
		int in2_first_pgnum = fph_in2.GetPageNum();
		in2.UnpinPage(in2_first_pgnum);
		PageHandler lph_in2 = in2.LastPage ();
		int in2_last_pgnum = lph_in2.GetPageNum();
		in2.UnpinPage(in2_last_pgnum);
        for(int c_in2 = in2_first_pgnum; c_in2<= in2_last_pgnum; c_in2++)
        {
            PageHandler cph_in2 = in2.PageAt(c_in2);
            char* data2 = cph_in2.GetData ();
            int num1,num2; 
		    int index1=0;
            for(int i = 0; i < N; i++)
            {
                memcpy (&num1, &data1[index1], sizeof(int));
                int index2=0;
                for(int j = 0; j < N; j++)
                {
                    memcpy (&num2, &data2[index2], sizeof(int));
                    if(num1 == num2)
                    {
                        store(num1,out);
                    }
                    index2 = index2+sizeof(int);
                }
                 index1 = index1+sizeof(int);
            }
            in2.UnpinPage(c_in2);
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
//	printFile(in1_fh);
//	printFile(in2_fh);
    join1(in1_fh,in2_fh,out_fh);

    fm.CloseFile (in1_fh);
    fm.CloseFile (in2_fh);
	if(curr_page_num_out != -1)
	{
		out_fh.FlushPage(curr_page_num_out);
    	//printFile(out_fh);
	}
	fm.CloseFile (out_fh);
}