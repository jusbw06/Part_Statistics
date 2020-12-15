#ifndef TDMS_PARSER_H
#define TDMS_PARSER_H

#include "endianfstream.hh"
#include <vector>
#include <map>

using namespace std;

class TdmsGroup;
class TdmsObject;
class TdmsChannel;
class TdmsSegment;

class TdmsParser
{
public:
	typedef vector<TdmsGroup*> TdmsGroupSet;
	typedef vector<TdmsSegment*> TdmsSegmentSet;

	TdmsParser(const std::string &, bool = true);
	TdmsParser(char *, bool = true);
	~TdmsParser();

	int fileOpeningError(){return file.is_open() ? 0 : 1;}

	void read(const bool = false);
	bool readChannelData(TdmsChannel*, const bool = false);

	std::iendianfstream& fileStream(){return file;}
	unsigned long long fileSize() const {return d_file_size;}

	TdmsGroup* getGroup(unsigned int) const;
	TdmsGroup* getGroup(const std::string &);
	void addGroup(TdmsGroup* group){d_groups.push_back(group);}
	unsigned int getGroupCount() const {return d_groups.size();}

	TdmsSegment *getSegment(unsigned int) const;
	TdmsSegment *getCurrentSegment() const;
	unsigned int getSegmentCount() const {return d_segments.size();}
	bool currentSegmentInterleaved() const;

	void setProperties(std::map<std::string, std::string> props){d_properties = props;}
	std::map<std::string, std::string> getProperties(){return d_properties;}
	string propertiesToString() const;

	//! Initializes the reading of the given file
	void init();
	//! Releases memory used to store data values.
	void freeMemory();
	//! Releases all memory in use.
	void clear();
	//! Closes and clears the file.
	void close();
	//! Reopens the file stream.
	bool reopen();

	bool getStoreValues(){return d_store_values;}
	void setStoreValues(bool);

	std::string getFileName() const {return d_file_name;}

private:
	unsigned long long readSegment(const bool, bool*);
	unsigned long long readChannelDataFromSegment(TdmsChannel*, TdmsSegment*, bool = false);
	bool readChannelRawDataFromSegment(TdmsChannel *, TdmsSegment *, unsigned long long, const bool = false);
	bool readRawData(unsigned long long, const bool);

	std::iendianfstream file;
	const std::string d_file_name;
	bool d_store_values;
	TdmsGroupSet d_groups;
	TdmsSegmentSet d_segments;
	TdmsObject *d_prev_object;
	unsigned long long d_file_size;
	std::map<std::string, std::string> d_properties;
};

#endif
