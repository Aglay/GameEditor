#ifndef _STRING_UTILITY_H_
#define _STRING_UTILITY_H_

#include "Header.h"

class StringUtility
{
public:
	static std::string removeSuffix(std::string str);
	static void removeLastComma(std::string& stream);
	static std::string getFileName(std::string str);
	static std::string getFileNameNoSuffix(std::string str);
	static std::string getParentDir(std::string dir);
	static std::string getFileSuffix(std::string fileName);
	static void split(std::string str, const char * deli, std::vector<std::string>& vec, bool removeEmpty = true);
	static std::string boolToString(bool value) { return value ? "True" : "False"; }
	static bool stringToBool(std::string str) { return str == "True" || str == "true"; }
	static std::string strReplace(const std::string& str, const int& begin, const int& end, const std::string& reStr);
	static std::string strReplaceAll(std::string str, const std::string& key, const std::string& newWords);
	static std::string bufferToString(char* buffer, const int& bufferSize);
	static std::string intToString(int i, int limitLen = 0); // limitLen���ַ�������С����,���������λ��������С����,�����ǰ���0
	static int stringToInt(std::string str);
	static std::string checkString(const std::string& str, const std::string& valid);
	static std::string checkNumberString(const std::string& str, const std::string& valid = "");
	static float pow_f(float f, int p);
	static std::string floatToString(float f, int precision);	//precision -> ����,������С����λ��
	static float stringToFloat(std::string str);
	static bool endWith(std::string oriString, std::string pattern);// �ж�oriString�Ƿ���pattern��β
	static bool startWith(std::string oriString, std::string pattern);// �ж�oriString�Ƿ���pattern��ͷ
	static void rightToLeft(std::string& str);
	static void leftToRight(std::string& str);
	static int strFind(std::string res, std::string sub, int startPose = 0, bool fromStart = true);
	static bool findSubstr(std::string res, std::string dst, bool sensitive, int* pos = NULL, int startPose = 0, bool firstOrLast = true);
	static std::vector<std::string> findSubstr(std::vector<std::string>& res, std::string dst, bool sensitive);
	static void strToLower(std::string& str);
	static void strToUpper(std::string& str);
	static std::string charToHex(unsigned char byte);
	static std::string charArrayToHexString(unsigned char* data, const int& dataCount);
	static void jsonStartArray(std::string& str){ str += "["; }
	static void jsonEndArray(std::string& str)
	{
		removeLastComma(str);
		str += "],";
	}
	static void jsonStartStruct(std::string& str){ str += "{"; }
	static void jsonEndStruct(std::string& str)
	{
		removeLastComma(str);
		str += "},";
	}
	static void jsonAddPair(std::string& str, std::string name, std::string value)
	{
		str += "\"";
		str += name;
		str += "\":";
		str += "\"";
		str += value;
		str += "\",";
	}
};

#endif