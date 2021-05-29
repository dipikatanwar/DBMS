#include<iostream>
#include "file_manager.h"
#include "errors.h"
#include<cstring>
#include<math.h>
#include<fstream>
#include<limits.h>
#include<vector>

using namespace std;

int curr_page_num_out=-1;
int curr_offset_out=-1;
int N; // integers on one page

//keeps track of current page and offset to read from
int curr_read_page = -1;
int curr_read_offset = -1;

//to keep track of current page and offset to write to
int curr_write_page = -1;
int curr_write_offset = -1;

//count number of deletions
int count = 0;

int global_last_page_num;		


void printFile(FileHandler& fh)
{

	//int last_page_num = global_last_page_num;
	PageHandler fph;
	int curr_page_num;
	try{
		fph = fh.FirstPage ();	
	}
	catch(InvalidPageException& e){
	
		return;
	}

	curr_page_num = fph.GetPageNum();
	PageHandler lph = fh.LastPage();
	int last_page_num = lph.GetPageNum();
	//cout<<"Curr Page number: "<<curr_page_num<<" "<<"Last Page Number: "<<last_page_num<<endl;

	while(curr_page_num <= last_page_num)
	{	
		PageHandler cph = fh.PageAt(curr_page_num);
		char* data = cph.GetData ();
		int num;
		int index = 0;
		int offset=0;
		while(offset < N)
		{
			memcpy (&num, &data[index], sizeof(int));
			cout <<"PageNum: "<<curr_page_num<<" Offset: "<<offset<<" Number: " << num << endl;
			offset++;
			index = index+sizeof(int);
		}
		fh.UnpinPage(curr_page_num);
		curr_page_num++;
	}

}

int extractNum(string line)
{
	int i = 7;
	int value = 0;
	string str;
	while(line[i] != '\0')
	{
		str = str + line[i];
		i++;
	}
	value = stoi(str);
	return value;
}

void linear_search(FileHandler& in_fh,int search_num)
{
	int last_page_num = global_last_page_num;
	PageHandler fph;
	try{
		fph = in_fh.FirstPage ();
	}
	catch(InvalidPageException& e){
		return;
	}
	int curr_page_num = fph.GetPageNum();


	while(curr_page_num <= last_page_num)
	{
		PageHandler cph = in_fh.PageAt(curr_page_num);
		char *data = cph.GetData();
		int num;
		int index = 0;
		int offset=0;
		while(offset < N)
		{
			memcpy(&num, &data[index], sizeof(int));
			//cout<<curr_page_num<<" "<<offset<<" "<<num<<endl;
			if(search_num == num)
			{
				if(count == 0){
					curr_write_page = curr_page_num;
					curr_write_offset = offset;
				}

				count++;
				curr_read_page = curr_page_num;
				curr_read_offset = offset;
				//store(curr_page_num,offset,out_fh);
			}
			offset++;
			index = index + sizeof(int);
		}
		in_fh.UnpinPage(curr_page_num);
		curr_page_num++;
	}

	//store(-1,-1,out_fh);
}

vector<int> Read(FileHandler& in_fh){
	vector<int> values;
	PageHandler ph = in_fh.PageAt(curr_read_page);
	char* data = ph.GetData();
	int num;
	int temp = INT_MIN;

	while(curr_read_offset < N){
		int x = sizeof(int)*curr_read_offset;
		memcpy(&num,&data[x],sizeof(int));
		values.push_back(num);
		//cout<<num<<endl;
		memcpy(&data[x],&temp,sizeof(int));
		curr_read_offset ++;
	}

	in_fh.MarkDirty(curr_read_page);
	in_fh.FlushPage(curr_read_page);

	if(curr_read_offset == N){
		curr_read_offset = 0;
		curr_read_page++;
	}

	return values;

}


void Write(FileHandler& in_fh, vector<int> v){
	int last_page_num = global_last_page_num;
	PageHandler pg = in_fh.PageAt(curr_write_page);
	char *data = pg.GetData();

	for(int i=0;i<v.size();i++){
		int x = sizeof(int)*curr_write_offset;
		memcpy(&data[x],&v[i],sizeof(int));
		curr_write_offset++;
		if(curr_write_offset >= N){
			in_fh.MarkDirty(curr_write_page);
			in_fh.FlushPage(curr_write_page);
			curr_write_page++;
			curr_write_offset = 0;
			if(curr_write_page <= last_page_num){
				pg = in_fh.PageAt(curr_write_page);
				data = pg.GetData();
			}
		}
	}
	in_fh.MarkDirty(curr_write_page);
	in_fh.FlushPage(curr_write_page);
}


void Del_last_pages(FileHandler& in_fh){

	int num;
	PageHandler ph;
	try{
		ph = in_fh.PageAt(global_last_page_num);
	}
	catch(InvalidPageException& e){
		return;
	}

	char* data = ph.GetData();
	memcpy(&num,&data[0],sizeof(int));

	while(num == INT_MIN && global_last_page_num>=0){
			//cout<<"Deleting: "<<last_page_num<<" "<<global_last_page_num<<endl;
			in_fh.DisposePage(global_last_page_num);
			global_last_page_num --;
			if(global_last_page_num >= 0){
			PageHandler ph = in_fh.PageAt(global_last_page_num);
			char* data = ph.GetData();
			memcpy(&num,&data[0],sizeof(int));
			}			
	}

	if(global_last_page_num >= 0)
		in_fh.UnpinPage(global_last_page_num); 
}


int main(int argc, char* argv[]) {

	if(argc < 3)
	{
		cout<<"Error: Not enough arguments!!"<<endl;
		return 0;
	}

	FileManager fm;
    FileHandler in_fh = fm.OpenFile (argv[1]);
    char* filename = argv[1];
	PageHandler lph = in_fh.LastPage ();	
	global_last_page_num = lph.GetPageNum();

	in_fh.FlushPage(global_last_page_num);
	int last_page_num = global_last_page_num;

	N = floor(PAGE_CONTENT_SIZE / sizeof(int));

    ifstream fp;
    fp.open(argv[2]);
	string line;
	 while (fp) {
        getline(fp, line);
		int num = extractNum(line);
        if( num != -1 && line.length() != 0){
        	//reinitialize the variables for each new delete op
        	count = 0;
        	curr_read_page = -1;
			curr_read_offset = -1;
			curr_write_page = -1;
			curr_write_offset = -1;
			

			linear_search(in_fh,num);

			//cout<<"Current number: "<<num<<endl;
			//cout<<"delete from "<<curr_write_page<<" "<<curr_write_offset<<endl;
			//cout<<"delete upto "<<curr_read_page<<" "<<curr_read_offset<<endl;
			//cout<<"Number of deletions: "<<count<<endl<<endl;

			int curr_read_page1 = curr_read_page;
			int curr_write_page1 = curr_write_page;
			int curr_read_offset1 = curr_read_offset;
			int curr_write_offset1 = curr_write_offset;

			//write INT_MIN TO ALL THE LOCATIONS THAT WE ARE DELETING
			while(curr_write_page1<=curr_read_page1 && curr_write_page1!=-1 && curr_read_page1!=-1){
				if(curr_write_page1 == curr_read_page1 && curr_write_offset1 > curr_read_offset1){
					break;
				}
				int temp = INT_MIN;
				PageHandler ph = in_fh.PageAt(curr_write_page1);
				char* data = ph.GetData();
				int indx = curr_write_offset1*sizeof(int);
				memcpy(&data[indx],&temp,sizeof(int));
				curr_write_offset1++;
				if(curr_write_offset1 >= N){
					in_fh.MarkDirty(curr_write_page1);
					in_fh.FlushPage(curr_write_page1);
					curr_write_offset1 = 0;
					curr_write_page1++;
				}
			}
			//if we are not writing INT_MIN till the last offset, then we have to flush the page
			if(curr_write_page1 <= last_page_num && curr_write_page1!= -1){
				in_fh.MarkDirty(curr_write_page1);
				in_fh.FlushPage(curr_write_page1);	
			}
			
			//Reading should begin from the next line
			curr_read_offset ++;
			if(curr_read_offset >= N){
				curr_read_offset = 0;
				curr_read_page ++;
			}

			//check if the element to be deleted is present in the file
			//cout<<curr_read_page<<" "<<curr_write_page<<" "<<last_page_num<<endl;
        	if(!(curr_read_page == -1 && curr_write_page == -1)){
        			//values that are read are stored in the buffer
        		vector<int> values;
        	
        		while(curr_read_page <= last_page_num){
        			//cout<<"Values Read from Page: "<<curr_read_page<<" Offset: "<<curr_read_offset<<endl;
        			values = Read(in_fh);
        			/*for(int i=0;i<values.size();i++)
        				cout<<values[i]<<" ";
        			cout<<endl;*/
        			Write(in_fh,values);
        			
        			}	      
        		}
        		//printFile(in_fh);
        	//}
        }
  	}
  	Del_last_pages(in_fh);
  	//if all the pages are deleted
  	if(global_last_page_num == -1){
		fm.DestroyFile(filename);
		fm.CreateFile(filename);	
  	}

    // cout<<"The final contents of the file are: "<<endl;
    // printFile(in_fh);

	fp.close();
	fm.CloseFile (in_fh);
	//out_fh.FlushPage(curr_page_num_out);
	//fm.CloseFile (out_fh);
}
