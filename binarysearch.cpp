#include<iostream>
#include "file_manager.h"
#include "errors.h"
#include<cstring>
#include<math.h>
#include<fstream>
#include<limits.h>
using namespace std;

int curr_page_num_out=-1;
int curr_offset_out=-1;
int N; // integers on one page
int global_last_page_num = -1;

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
	if( line.length() == 0)
	{
		return -1;
	}
	int i = 7;
	int value = -1;
	string str;
	while(line[i] != '\0')
	{
		str = str + line[i];
		i++;
	}
	value = stoi(str);
	return value;
}


void store(int pageNum,int offset,FileHandler & out_fh)
{
	if(curr_offset_out > N-1)
	{
		out_fh.FlushPage(curr_page_num_out);
		PageHandler ph = out_fh.NewPage();
		curr_page_num_out = ph.GetPageNum();
		curr_offset_out=0;
	}
	PageHandler ph = out_fh.PageAt(curr_page_num_out);
	int index = curr_offset_out*sizeof(int);
	char *data=ph.GetData();
	memcpy (&data[index], &pageNum, sizeof(int));
	curr_offset_out++;
	index = curr_offset_out*sizeof(int);
	memcpy (&data[index], &offset, sizeof(int));
	curr_offset_out++;
}


void binary_search(FileHandler& in_fh,int search_num,FileHandler& out_fh,int first_page_num,int last_page_num)
{
	int	curr_page_num=(first_page_num+last_page_num)/2;

	PageHandler cph = in_fh.PageAt(curr_page_num);
	int* data = (int *)cph.GetData ();
	int num;
	int first_offset=*data;
	int last_offset=*(data+N-1);

	int index = 0;
	int offset=0;

	// condition forlast page:
	if(global_last_page_num==curr_page_num and (last_offset==INT_MIN))
	{

			int index=0;

			while(*(data+(++index))!=INT_MIN );
			last_offset=*(data+index-1);
			// cout<<"last_offset : "<<last_offset<<"\n";		

	}

	
	//binary search logic :
	if(search_num<first_offset)
	{
		in_fh.UnpinPage(curr_page_num);
		if(curr_page_num!=first_page_num)
		{binary_search(in_fh,search_num,out_fh,first_page_num,curr_page_num-1);}
	}
	else if(search_num>first_offset)
	{
		if(search_num>last_offset)
		{
			in_fh.UnpinPage(curr_page_num);
			if(curr_page_num!=last_page_num)
			{binary_search(in_fh,search_num,out_fh,curr_page_num+1,last_page_num);}
		}
		else if(search_num==last_offset)
		{
			while(offset<N && *data!=INT_MIN)
			{

				memcpy (&num, data, sizeof(int));
				if(search_num == num)
				{
					// cout<<"p : "<<curr_page_num<<" o : "<<offset<<"\n";
					store(curr_page_num,offset,out_fh);
				}
				offset++;
				data++;
			}
			in_fh.UnpinPage(curr_page_num);
			if(curr_page_num!=last_page_num)
			{binary_search(in_fh,search_num,out_fh,curr_page_num+1,last_page_num);}
		}
		else if(search_num<last_offset)
		{
			while(offset<N && *data!=INT_MIN)
			{
				memcpy (&num, data, sizeof(int));
				
				if(search_num == num)
				{
					// cout<<"p : "<<curr_page_num<<" o : "<<offset<<"\n";
					
					store(curr_page_num,offset,out_fh);

				}
				offset++;
				data++;
			}
			in_fh.UnpinPage(curr_page_num);
		}
		
	}
	else if(search_num==first_offset)
	{
		
		if(search_num==last_offset )
		{
			while(offset<N && *data!=INT_MIN)
			{
				memcpy (&num, data, sizeof(int));
				// cout<<"p : "<<curr_page_num<<" o : "<<offset<<"\n";
					
				store(curr_page_num,offset,out_fh);
				offset++;
				data++;
			}
			in_fh.UnpinPage(curr_page_num);
			if(curr_page_num!=first_page_num)
			{binary_search(in_fh,search_num,out_fh,first_page_num,curr_page_num-1);}
			if(curr_page_num!=last_page_num)
			{binary_search(in_fh,search_num,out_fh,curr_page_num+1,last_page_num);}	
		}
		else if(search_num<last_offset)
		{
			while(offset<N && *data!=INT_MIN)
			{
				memcpy (&num, data, sizeof(int));
				
				if(search_num == num)
				{
					// cout<<"p : "<<curr_page_num<<" o : "<<offset<<"\n";
					
					store(curr_page_num,offset,out_fh);

				}
				offset++;
				data++;
			}
			in_fh.UnpinPage(curr_page_num);
			if(curr_page_num!=first_page_num)
			{binary_search(in_fh,search_num,out_fh,first_page_num,curr_page_num-1);}
		}
	}

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

	PageHandler lph = in_fh.LastPage ();
	global_last_page_num = lph.GetPageNum();
	in_fh.UnpinPage(global_last_page_num);


	N = floor(PAGE_CONTENT_SIZE / sizeof(int));
	
    ifstream fp;
    fp.open(argv[2]);
	string line;
	while (fp) 
	{
        getline(fp, line);
		int num = extractNum(line);
		// cout<<"NUMBER : "<<num<<"\n";
        if( num != -1 )
        {
        	
			binary_search(in_fh,num,out_fh,0,global_last_page_num);
					
			store(-1,-1,out_fh);
		}

    }
 	while(curr_offset_out<N)
	{
		store(INT_MIN,INT_MIN,out_fh);
	}
	// store()

	fp.close();
	out_fh.FlushPage(curr_page_num_out);
	// printFile(in_fh);
	// cout<<"\n\n11111111111111111111111111111111\n\n";
//	 printFile(out_fh);
	fm.CloseFile (in_fh);
	fm.CloseFile (out_fh);
}