#ifndef TDMS_CHANNEL_H
#define TDMS_CHANNEL_H

#include "TdmsObject.h"
#include "TdmsGroup.h"
#include "endianfstream.hh"

#include <map>
#include <string>
#include <vector>

class TdmsChannel
{
public:
	typedef enum {
		tdsTypeVoid,
		tdsTypeI8,
		tdsTypeI16,
		tdsTypeI32,
		tdsTypeI64,
		tdsTypeU8,
		tdsTypeU16,
		tdsTypeU32,
		tdsTypeU64,
		tdsTypeSingleFloat,
		tdsTypeDoubleFloat,
		tdsTypeExtendedFloat,
		tdsTypeSingleFloatWithUnit = 0x19,
		tdsTypeDoubleFloatWithUnit,
		tdsTypeExtendedFloatWithUnit,
		tdsTypeString = 0x20,
		tdsTypeBoolean = 0x21,
		tdsTypeTimeStamp = 0x44,
		tdsTypeFixedPoint = 0x4F,
		tdsTypeComplexSingleFloat = 0x08000c,
		tdsTypeComplexDoubleFloat = 0x10000d,
		tdsTypeDAQmxRawData = 0xFFFFFFFF
	} tdsDataType;

	TdmsChannel(const std::string&, std::iendianfstream&);
	~TdmsChannel();

	const std::string& getName() const {return d_name;}

	std::string getUnit() const;
	std::string getDescription() const;
	std::string getProperty(const std::string&) const;
	std::string propertiesToString() const;

	unsigned long long getChannelSize() const;
	unsigned long long getDataCount() const {return d_data_cnt;}
	unsigned long long getStringCount() const {return d_string_cnt;}
	unsigned long long getMaxVectorSize() const;
	unsigned int getPropertyCount() const {return d_properties.size();}

	unsigned int getTypeSize() const {return typeSize;}
	void setTypeSize(unsigned int);

	unsigned int getDataType() const {return dataType;}
	void setDataType(unsigned int);

	void appendValue(double);
	const std::vector<double>& getDataVector() {return dataVector;}
	double getValue(unsigned int) const;

	void appendImaginaryValue(double val){if (d_store_values) imagDataVector.push_back(val);}
	std::vector<double> getImaginaryDataVector() {return imagDataVector;}
	double getImaginaryValue(unsigned int) const;

	void appendString(std::string);
	std::vector<std::string> getStringVector() {return stringVector;}
	std::string getString(unsigned int) const;

	void addProperties(std::map<std::string, std::string>);
	void setProperties(std::map<std::string, std::string> props){d_properties = props;}
	std::map<std::string, std::string> getProperties(){return d_properties;}

	unsigned int getDimension() const {return dimension;}
	void setDimension(unsigned int d){dimension = d;}

	unsigned long long getValuesCount() const {return nvalues;}
	void setValuesCount(unsigned long long);

	bool readRawData(unsigned long long, bool);
	bool hasRawData() const {return flagHasRawData;}
	void setRawDataFlag(bool flag) {flagHasRawData = flag;}

	void readValue(unsigned int, bool = false);

	bool readDAQmxData(std::vector<FormatChangingScaler>, std::vector<unsigned int>, bool = false);
	void readDAQmxValue(unsigned int, double, double, unsigned int, unsigned int, unsigned int, bool = false);

	bool getInterleavedFlag(){return flagIsInterleaved;}
	void setInterleavedFlag(bool val){flagIsInterleaved = val;}

	double getMinDataValue() const {return d_min_value;}
	double getMaxDataValue() const {return d_max_value;}
	double getDataSum() const {return d_data_sum;}
	double getDataMean() const {return d_data_cnt ? d_data_sum/d_data_cnt : .0;}

	TdmsGroup *getGroup(){return d_group;}
	void setGroup(TdmsGroup *g){d_group = g;}

	bool getStoreValues(){return d_store_values;}
	void setStoreValues(bool on){d_store_values = on;}

	unsigned int getIndexWithinGroup(){return d_group ? d_group->getChannelIndex(this) : 0;}

	//! Reserve memory for the data vectors.
	//! Calling this function only makes sense if the file was first parsed without storing data values.
	//!
	bool reserveMemory();
	void freeMemory();

private:
	void readStrings();
	//! Reserve memory for the data vectors
	bool reserveDataMemory(unsigned long long);

	const std::string d_name;
	std::iendianfstream& file;
	unsigned int dataType;
	unsigned int typeSize;
	unsigned int dimension;
	unsigned long long nvalues;
	unsigned long long d_data_cnt;
	unsigned long long d_string_cnt;
	bool flagHasRawData;
	bool flagIsInterleaved;
	bool d_store_values;
	double d_min_value;
	double d_max_value;
	double d_data_sum;

	std::map<std::string, std::string> d_properties;
	std::vector<double> dataVector;
	std::vector<double> imagDataVector;
	std::vector<std::string> stringVector;
	TdmsGroup *d_group;
};
#endif
