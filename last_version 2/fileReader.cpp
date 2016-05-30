#include "fileReader.h"
FileReader::FileReader(string name):file_name(name)
{
	index["Info"]=0;
	index["DataServer"]=1;
	index["SYSTEM"]=2;
	index["PollTime"]=3;
}

bool ReadFile()
{
	ifstream file(file_name);
	int count=-1;
	if(!file.is_open()){
		cout<<"Can't open file "<<file_name<<" !"<<endl;
		return false;
	}

	while(!file.eof())
	{
		string raw;
		getline(file,raw);
		if(raw.substr(0,1)=="["){
			count++;
			continue;
		}
		else{
			vector<string> ret;
			Split(raw,"=",ret);
		}
	}
	file.close();
	return true;
}

void FileReader::Split(string s, string delim,vector<string>& ret)
{
	ret.clear();
	size_t last = 0;
	size_t index=s.find_first_of(delim,last);
	while (index!=std::string::npos)
	{
		ret->push_back(s.substr(last,index-last));
		last=index+1;
		index=s.find_first_of(delim,last);
	}
	if (index-last>0)
	{
		ret->push_back(s.substr(last,index-last));
	}
}