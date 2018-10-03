#include "Utility.h"
#include "txSerializer.h"
#include "HumanImage.h"
#include "WeaponImage.h"
#include "SQLite.h"
#include "SQLiteCloth.h"
#include "SQLiteClothFrame.h"
#include "SQLiteMonster.h"
#include "SQLiteMonsterFrame.h"
#include "SQLiteWeapon.h"
#include "SQLiteWeaponFrame.h"
#include "HumanAction.h"
#include "WeaponAction.h"

void ImageUtility::encodePNG(const std::string& path, char* color, int width, int height, FREE_IMAGE_FORMAT format)
{
	std::string dir = StringUtility::getFilePath(path);
	FileUtility::createFolder(dir);
	FreeImage_Initialise();
	FIBITMAP* bitmap = FreeImage_Allocate(width, height, 32);
	BYTE* bits = FreeImage_GetBits(bitmap);
	memcpy(bits, color, width * height * 4);
	FreeImage_Save(format, bitmap, path.c_str());
	FreeImage_Unload(bitmap);
	FreeImage_DeInitialise();
}

bool ImageUtility::readWixFile(const std::string& filePath, WIXFileImageInfo& info)
{
	int fileSize = 0;
	char* fileBuffer = FileUtility::openBinaryFile(filePath, &fileSize);
	if (fileBuffer == NULL)
	{
		return false;
	}
	txSerializer serializer(fileBuffer, fileSize);
	serializer.readBuffer(info.mStrHeader, 44);
	serializer.read(info.mIndexCount);
	for (int i = 0; i < info.mIndexCount; ++i)
	{
		int curStartPos = 0;
		if (serializer.read(curStartPos))
		{
			info.mPositionList.push_back(curStartPos);
		}
	}
	TRACE_DELETE_ARRAY(fileBuffer);
	return true;
}

bool ImageUtility::readWilHeader(const std::string& filePath, WILFileHeader& header)
{
	int fileSize = 0;
	char* fileBuffer = FileUtility::openBinaryFile(filePath, &fileSize);
	if (fileBuffer == NULL)
	{
		return false;
	}
	txSerializer serializer(fileBuffer, fileSize);
	serializer.readBuffer(header.mInfo, 44);
	serializer.readBuffer(header.mPlayInfo, 12);
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			serializer.read(header.mColor[i][j]);
		}
	}
	TRACE_DELETE_ARRAY(fileBuffer);
	return true;
}

void ImageUtility::wixWilToPNG(const std::string& wixFileName, const std::string& wilFileName, const std::string& outputPath)
{
	// 打开wix文件
	WIXFileImageInfo wixFileHeader;
	if (!readWixFile(wixFileName, wixFileHeader))
	{
		std::cout << "找不到wix文件" << std::endl;
		return;
	}

	// 打开wil文件
	WILFileHeader wilHeader;
	if(!readWilHeader(wilFileName, wilHeader))
	{
		std::cout << "找不到wil文件" << std::endl;
		return;
	}

	POINT* posList = TRACE_NEW_ARRAY(POINT, wixFileHeader.mPositionList.size(), posList);
	int fileSize = 0;
	char* fileBuffer = FileUtility::openBinaryFile(wilFileName, &fileSize);
	txSerializer serializer(fileBuffer, fileSize);
	for (int i = 0; i < wixFileHeader.mPositionList.size(); ++i)
	{
		// 将下标设置到当前图片起始位置,并且读取图片信息
		int startPos = wixFileHeader.mPositionList[i];
		if (startPos == 0)
		{
			continue;
		}
		serializer.setIndex(startPos);
		WILFileImageInfo curInfo;
		// 宽高,位置偏移
		serializer.read(curInfo.mWidth);
		serializer.read(curInfo.mHeight);
		serializer.read(curInfo.mPosX);
		serializer.read(curInfo.mPosY);
		// 根据颜色索引在调色板中获取颜色数据
		int pixelCount = curInfo.mWidth * curInfo.mHeight;
		TRACE_NEW_ARRAY(char, pixelCount * 4, curInfo.mColor);
		for (int j = 0; j < pixelCount; ++j)
		{
			if (startPos + ImageHeaderLength + j >= fileSize)
			{
				std::cout << "error" <<std::endl;
				break;
			}
			unsigned char pixelIndex = fileBuffer[startPos + ImageHeaderLength + j];
			// 0蓝,1绿,2红
			curInfo.mColor[j * 4 + 0] = wilHeader.mColor[pixelIndex][0];// 蓝
			curInfo.mColor[j * 4 + 1] = wilHeader.mColor[pixelIndex][1];// 绿
			curInfo.mColor[j * 4 + 2] = wilHeader.mColor[pixelIndex][2];// 红
			curInfo.mColor[j * 4 + 3] = pixelIndex == 0 ? 0 : (char)255;
		}
		posList[i].x = curInfo.mPosX;
		posList[i].y = curInfo.mPosY;
		// 将图片转换为png
		encodePNG(outputPath + StringUtility::intToString(i) + ".png", curInfo.mColor, curInfo.mWidth, curInfo.mHeight, FIF_PNG);
		TRACE_DELETE_ARRAY(curInfo.mColor);
	}
	TRACE_DELETE_ARRAY(fileBuffer);

	writePositionFile(outputPath + "position.txt", posList, wixFileHeader.mPositionList.size());
	TRACE_DELETE_ARRAY(posList);
}

void ImageUtility::writePositionFile(const std::string& positionFile, POINT* posList, int posCount)
{
	std::string posStr;
	for (int i = 0; i < posCount; ++i)
	{
		posStr += StringUtility::intToString(posList[i].x);
		posStr += ",";
		posStr += StringUtility::intToString(posList[i].y);
		posStr += "\n";
	}
	FileUtility::writeFile(positionFile, posStr);
}

POINT* ImageUtility::readPositionFile(const std::string& positionFile, int& posCount)
{
	std::string posStr = FileUtility::openTxtFile(positionFile);
	txVector<std::string> posStrList;
	StringUtility::split(posStr, "\n", posStrList);
	posCount = posStrList.size();
	POINT* posList = TRACE_NEW_ARRAY(POINT, posCount, posList);
	for (int i = 0; i < posCount; ++i)
	{
		txVector<std::string> pointList;
		StringUtility::split(posStrList[i], ",", pointList);
		if (pointList.size() != 2)
		{
			continue;
		}
		posList[i].x = StringUtility::stringToInt(pointList[0]);
		posList[i].y = StringUtility::stringToInt(pointList[1]);
	}
	return posList;
}

void ImageUtility::autoGroupHumanImage(const std::string& path)
{
	// 先拆分位置文件
	splitPositionFile(path);
	// 按照600个文件一组,放入单独的文件夹中
	autoMoveFile(path, HUMAN_GROUP_SIZE);

	txVector<std::string> folderList;
	FileUtility::findFolders(path, folderList);
	int folderCount = folderList.size();
	for (int i = 0; i < folderCount; ++i)
	{
		// 按照动作重命名
		txVector<std::string> fileList;
		FileUtility::findFiles(folderList[i], fileList, ".png", false);
		sortByFileNumber(fileList);
		int fileCount = fileList.size();
		for (int j = 0; j < fileCount; ++j)
		{
			std::string actionName;
			int direction;
			int frameIndex;
			bool isValid = getHumanActionInfo(j, actionName, direction, frameIndex);
			// 如果是无效图片则需要删除
			if (!isValid)
			{
				deleteImageWithPosition(fileList[j]);
			}
			std::string actionFolderName = actionName + "_dir" + StringUtility::intToString(direction);
			std::string destPath = StringUtility::getFilePath(fileList[j]) + "/" + actionFolderName + "/";
			moveImageWithPosition(fileList[j], destPath + actionFolderName + "_" + StringUtility::intToString(frameIndex) + StringUtility::getFileSuffix(fileList[j], true));
		}
	}
}

void ImageUtility::autoGroupWeaponImage(const std::string& path)
{
	// 先拆分位置文件
	splitPositionFile(path);
	// 按照600个文件一组,放入单独的文件夹中
	autoMoveFile(path, WEAPON_GROUP_SIZE);
	// 因为武器文件是角色文件的2倍,可能是左右手的区别,所以暂时只解析前一半的文件,后面的删除
	txVector<std::string> folderList;
	FileUtility::findFolders(path, folderList);
	int folderCount = folderList.size();
	for (int i = 0; i < folderCount; ++i)
	{
		// 按照动作重命名
		txVector<std::string> fileList;
		FileUtility::findFiles(folderList[i], fileList, ".png", false);
		sortByFileNumber(fileList);
		int fileCount = fileList.size();
		for (int j = 0; j < fileCount; ++j)
		{
			if (WEAPON_GROUP_SIZE == 1200 && j >= WEAPON_GROUP_SIZE / 2)
			{
				deleteImageWithPosition(fileList[j]);
			}
			else
			{
				std::string actionName;
				int direction;
				int frameIndex;
				bool isValid = getHumanActionInfo(j, actionName, direction, frameIndex);
				// 如果是无效图片则需要删除
				if (!isValid)
				{
					deleteImageWithPosition(fileList[j]);
				}
				std::string actionFolderName = actionName + "_dir" + StringUtility::intToString(direction);
				std::string destPath = StringUtility::getFilePath(fileList[j]) + "/" + actionFolderName + "/";
				moveImageWithPosition(fileList[j], destPath + actionFolderName + "_" + StringUtility::intToString(frameIndex) + StringUtility::getFileSuffix(fileList[j], true));
			}
		}
	}
}

void ImageUtility::autoGroupMonsterImage0(const std::string& path)
{
	// 拆分位置文件
	ImageUtility::splitPositionFile(path);
	// 然后按360个文件一组,移动到单独的文件夹
	ImageUtility::autoMoveFile(path, MONSTER_GROUP_SIZE);
	// 怪物图片不像角色和武器图片那样会有规律排列,所以只能在大致分组后手动对每个文件夹进行动作分组
}

void ImageUtility::autoGroupMonsterImage1(const std::string& path)
{
	// 手动对动作进行分组后,就可以对每组动作文件进行方向分组
	// 重命名文件,将每个文件夹中的图片都重命名为该文件夹中的位置序号
	ImageUtility::renameImage(path);
	// 自动计算方向并分组
	ImageUtility::renameByDirection(path);
}

void ImageUtility::saveFrameInfo(const std::string& path, IMAGE_TYPE imageType, SQLite* sqlite)
{
	if (imageType == IT_MONSTER)
	{
		// 查找文件夹中的所有图片
		txVector<std::string> folderList;
		FileUtility::findFolders(path, folderList);
		int folderCount = folderList.size();
		for (int i = 0; i < folderCount; ++i)
		{
			MonsterImageGroup imageGroup;
			txVector<std::string> fileList;
			FileUtility::findFiles(folderList[i], fileList, ".png");
			int fileCount = fileList.size();
			for (int j = 0; j < fileCount; ++j)
			{
				std::string posFileName = StringUtility::getFilePath(fileList[j]) + "/" + StringUtility::getFileNameNoSuffix(fileList[j]) + ".txt";
				std::string posFile = FileUtility::openTxtFile(posFileName);
				txVector<int> posValue;
				StringUtility::stringToIntArray(posFile, posValue);
				if (posValue.size() != 2)
				{
					std::cout << "位置文件内容错误 : " << fileList[j] << std::endl;
					break;
				}
				MonsterImage monsterImage;
				monsterImage.mLabel = StringUtility::getFileName(folderList[i]);
				monsterImage.mPosX = posValue[0];
				monsterImage.mPosY = posValue[1];
				monsterImage.mMonsterID = i + 1;
				monsterImage.setFileName(StringUtility::getFileNameNoSuffix(fileList[j]));
				imageGroup.addImage(monsterImage);
			}
			// 按序列帧来分组,整理数据后写入数据库
			writeSQLite(imageGroup.mAllAction, sqlite);
		}
	}
	else if (imageType == IT_HUMAN)
	{
		// 查找文件夹中的所有图片
		txVector<std::string> folderList;
		FileUtility::findFolders(path, folderList);
		int folderCount = folderList.size();
		for (int i = 0; i < folderCount; ++i)
		{
			HumanImageGroup imageGroup;
			txVector<std::string> fileList;
			FileUtility::findFiles(folderList[i], fileList, ".png");
			int fileCount = fileList.size();
			for (int j = 0; j < fileCount; ++j)
			{
				std::string posFileName = StringUtility::getFilePath(fileList[j]) + "/" + StringUtility::getFileNameNoSuffix(fileList[j]) + ".txt";
				std::string posFile = FileUtility::openTxtFile(posFileName);
				txVector<int> posValue;
				StringUtility::stringToIntArray(posFile, posValue);
				if (posValue.size() != 2)
				{
					std::cout << "位置文件内容错误 : " << fileList[j] << std::endl;
					break;
				}
				HumanImage monsterImage;
				monsterImage.mLabel = StringUtility::getFileName(folderList[i]);
				monsterImage.mPosX = posValue[0];
				monsterImage.mPosY = posValue[1];
				monsterImage.mClothID = i + 1;
				monsterImage.setFileName(StringUtility::getFileNameNoSuffix(fileList[j]));
				imageGroup.addImage(monsterImage);
			}
			// 按序列帧来分组,整理数据后写入数据库
			writeSQLite(imageGroup.mAllAction, sqlite);
		}
	}
	else if (imageType == IT_WEAPON)
	{
		// 查找文件夹中的所有图片
		txVector<std::string> folderList;
		FileUtility::findFolders(path, folderList);
		int folderCount = folderList.size();
		for (int i = 0; i < folderCount; ++i)
		{
			WeaponImageGroup imageGroup;
			txVector<std::string> fileList;
			FileUtility::findFiles(folderList[i], fileList, ".png");
			int fileCount = fileList.size();
			for (int j = 0; j < fileCount; ++j)
			{
				std::string posFileName = StringUtility::getFilePath(fileList[j]) + "/" + StringUtility::getFileNameNoSuffix(fileList[j]) + ".txt";
				std::string posFile = FileUtility::openTxtFile(posFileName);
				txVector<int> posValue;
				StringUtility::stringToIntArray(posFile, posValue);
				if (posValue.size() != 2)
				{
					std::cout << "位置文件内容错误 : " << fileList[j] << std::endl;
					break;
				}
				WeaponImage monsterImage;
				monsterImage.mLabel = StringUtility::getFileName(folderList[i]);
				monsterImage.mPosX = posValue[0];
				monsterImage.mPosY = posValue[1];
				monsterImage.mWeaponID = i + 1;
				monsterImage.setFileName(StringUtility::getFileNameNoSuffix(fileList[j]));
				imageGroup.addImage(monsterImage);
			}
			// 按序列帧来分组,整理数据后写入数据库
			writeSQLite(imageGroup.mAllAction, sqlite);
		}
	}
}

void ImageUtility::writeSQLite(txMap<std::string, WeaponActionSet>& actionSetList, SQLite* sqlite)
{
	// 按序列帧来分组,整理数据后写入数据库
	// 遍历所有动作
	auto iter = actionSetList.begin();
	auto iterEnd = actionSetList.end();
	for (; iter != iterEnd; ++iter)
	{
		// 遍历该动作的所有方向
		for (int j = 0; j < DIRECTION_COUNT; ++j)
		{
			WeaponActionAnim& actionAnim = iter->second.mDirectionAction[j];
			WeaponFrameData data;
			data.mID = actionAnim.mImageFrame[0].mWeaponID;
			data.mLabel = StringUtility::ANSIToUTF8(actionAnim.mImageFrame[0].mLabel);
			data.mDirection = j;
			data.mAction = iter->first;
			data.mFrameCount = actionAnim.mImageFrame.size();
			// 遍历该动作的所有帧数
			for (int kk = 0; kk < data.mFrameCount; ++kk)
			{
				data.mPosX.push_back(actionAnim.mImageFrame[kk].mPosX);
				data.mPosY.push_back(actionAnim.mImageFrame[kk].mPosY);
			}
			bool ret = sqlite->mSQLiteWeaponFrame->insertOrUpdate(data);
			if (!ret)
			{
				break;
			}
		}
	}
}

void ImageUtility::writeSQLite(txMap<std::string, HumanActionSet>& actionSetList, SQLite* sqlite)
{
	auto iter = actionSetList.begin();
	auto iterEnd = actionSetList.end();
	for (; iter != iterEnd; ++iter)
	{
		// 遍历该动作的所有方向
		for (int j = 0; j < DIRECTION_COUNT; ++j)
		{
			HumanActionAnim& actionAnim = iter->second.mDirectionAction[j];
			ClothFrameData data;
			data.mID = actionAnim.mImageFrame[0].mClothID;
			data.mLabel = StringUtility::ANSIToUTF8(actionAnim.mImageFrame[0].mLabel);
			data.mDirection = j;
			data.mAction = iter->first;
			data.mFrameCount = actionAnim.mImageFrame.size();
			// 遍历该动作的所有帧数
			for (int kk = 0; kk < data.mFrameCount; ++kk)
			{
				data.mPosX.push_back(actionAnim.mImageFrame[kk].mPosX);
				data.mPosY.push_back(actionAnim.mImageFrame[kk].mPosY);
			}
			bool ret = sqlite->mSQLiteClothFrame->insertOrUpdate(data);
			if (!ret)
			{
				break;
			}
		}
	}
}

void ImageUtility::writeSQLite(txMap<std::string, MonsterActionSet>& actionSetList, SQLite* sqlite)
{
	auto iter = actionSetList.begin();
	auto iterEnd = actionSetList.end();
	for (; iter != iterEnd; ++iter)
	{
		// 遍历该动作的所有方向
		for (int j = 0; j < DIRECTION_COUNT; ++j)
		{
			MonsterActionAnim& actionAnim = iter->second.mDirectionAction[j];
			MonsterFrameData data;
			data.mID = actionAnim.mImageFrame[0].mMonsterID;
			data.mLabel = StringUtility::ANSIToUTF8(actionAnim.mImageFrame[0].mLabel);
			data.mDirection = j;
			data.mAction = iter->first;
			data.mFrameCount = actionAnim.mImageFrame.size();
			// 遍历该动作的所有帧数
			for (int kk = 0; kk < data.mFrameCount; ++kk)
			{
				data.mPosX.push_back(actionAnim.mImageFrame[kk].mPosX);
				data.mPosY.push_back(actionAnim.mImageFrame[kk].mPosY);
			}
			bool ret = sqlite->mSQLiteMonsterFrame->insertOrUpdate(data);
			if (!ret)
			{
				break;
			}
		}
	}
}

void ImageUtility::renameImage(const std::string& path)
{
	// 将目录中的文件按文件名排序后,重命名为从0开始的数字
	txVector<std::string> folderList;
	FileUtility::findFolders(path, folderList, true);
	int folderCount = folderList.size();
	for (int i = 0; i < folderCount; ++i)
	{
		txVector<std::string> fileList;
		FileUtility::findFiles(folderList[i], fileList, ".png");
		// 先根据文件名重新排列
		sortByFileNumber(fileList);
		int count = fileList.size();
		for (int j = 0; j < count; ++j)
		{
			std::string curFilePath = StringUtility::getFilePath(fileList[j]) + "/";
			std::string suffix = StringUtility::getFileSuffix(fileList[j]);
			renameImageWithPosition(fileList[j], curFilePath + StringUtility::intToString(j) + "." + suffix);
		}
	}
}

void ImageUtility::splitPositionFile(const std::string& path)
{
	// 将position.txt文件拆分为单个的txt文件,每个txt文件中只包含一个坐标
	int posCount = 0;
	POINT* posList = readPositionFile(path + "/position.txt", posCount);
	for (int i = 0; i < posCount; ++i)
	{
		std::string posStr = StringUtility::intToString(posList[i].x) + "," + StringUtility::intToString(posList[i].y);
		FileUtility::writeFile(path + "/" + StringUtility::intToString(i) + ".txt", posStr);
	}
	TRACE_DELETE_ARRAY(posList);
}

void ImageUtility::renameByDirection(const std::string& path)
{
	// 将目录中的所有文件先按照文件名排序,然后按照顺序分组为8个方向,再对每个方向的文件重命名
	txVector<std::string> folderList;
	FileUtility::findFolders(path, folderList, true);
	int folderCount = folderList.size();
	for (int i = 0; i < folderCount; ++i)
	{
		txVector<std::string> fileList;
		FileUtility::findFiles(folderList[i], fileList, ".png", false);
		sortByFileNumber(fileList);
		int fileCount = fileList.size();
		int actionFrameCount = fileCount / DIRECTION_COUNT;
		for (int j = 0; j < fileCount; ++j)
		{
			if (fileCount % DIRECTION_COUNT != 0)
			{
				std::cout << "图片数量错误,必须为方向的整数倍" << std::endl;
				break;
			}
			int imageDir = j / actionFrameCount;
			int index = j % actionFrameCount;
			// 把文件移动到一个新建文件夹中
			std::string curPath = StringUtility::getFilePath(fileList[j]) + "/";
			std::string destFolderName = StringUtility::getFolderName(fileList[j]) + "_dir" + StringUtility::intToString(imageDir);
			std::string destPath = StringUtility::getFilePath(curPath) + "/" + destFolderName + "/";
			moveImageWithPosition(fileList[j], destPath + destFolderName + "_" + StringUtility::intToString(index) + "." + StringUtility::getFileSuffix(fileList[j]));
		}
	}
	// 删除空的目录
	FileUtility::deleteEmptyFolder(path);
}

void ImageUtility::sortByFileNumber(txVector<std::string>& fileList)
{
	// 根据文件名的数字进行排序
	txMap<int, std::string> sortedList;
	int count = fileList.size();
	for (int i = 0; i < count; ++i)
	{
		sortedList.insert(StringUtility::stringToInt(StringUtility::getFileNameNoSuffix(fileList[i])), fileList[i]);
	}
	if (sortedList.size() != fileList.size())
	{
		return;
	}
	fileList.clear();
	auto iter = sortedList.begin();
	auto iterEnd = sortedList.end();
	for (; iter != iterEnd; ++iter)
	{
		fileList.push_back(iter->second);
	}
}

void ImageUtility::autoMoveFile(const std::string& path, int groupSize)
{
	txVector<std::string> fileList;
	FileUtility::findFiles(path, fileList, ".png");
	sortByFileNumber(fileList);
	int fileCount = fileList.size();
	for (int i = 0; i < fileCount; ++i)
	{
		int groupIndex = i / groupSize;
		std::string destFolderName = StringUtility::intToString(groupIndex);
		std::string destPath = StringUtility::getFilePath(fileList[i]) + "/" + destFolderName + "/";
		moveImageWithPosition(fileList[i], destPath + StringUtility::getFileName(fileList[i]));
	}
}

bool ImageUtility::getHumanActionInfo(int index, std::string& actionName, int& dir, int& frameIndex)
{
	int i = 0;
	while (true)
	{
		if (index - HUMAN_ACTION[i].mMaxFrame * DIRECTION_COUNT < 0)
		{
			break;
		}
		index -= HUMAN_ACTION[i].mMaxFrame * DIRECTION_COUNT;
		++i;
	}
	// 因为一组动作资源包含了8个方向上的所有动作,所以可以根据下标计算出方向和序列帧下标,前提是保留了空图片作为填充位置
	dir = index / HUMAN_ACTION[i].mMaxFrame;
	frameIndex = index % HUMAN_ACTION[i].mMaxFrame;
	actionName = HUMAN_ACTION[i].mName;
	return frameIndex < HUMAN_ACTION[i].mFrameCount;
}

void ImageUtility::moveImageWithPosition(const std::string& fullFileName, const std::string& destFullFileName)
{
	std::string sourceFileNameNoSuffix = StringUtility::getFileNameNoSuffix(fullFileName);
	std::string destFileNameNoSuffix = StringUtility::getFileNameNoSuffix(destFullFileName);
	std::string sourcePath = StringUtility::getFilePath(fullFileName) + "/";
	std::string destPath = StringUtility::getFilePath(destFullFileName) + "/";
	FileUtility::moveFile(fullFileName, destFullFileName);
	std::string positionFileName = sourcePath + sourceFileNameNoSuffix + ".txt";
	if (FileUtility::isFileExist(positionFileName))
	{
		FileUtility::moveFile(positionFileName, destPath + destFileNameNoSuffix + ".txt");
	}
}

void ImageUtility::renameImageWithPosition(const std::string& fullFileName, const std::string& destFullFileName)
{
	std::string sourceFileNameNoSuffix = StringUtility::getFileNameNoSuffix(fullFileName);
	std::string destFileNameNoSuffix = StringUtility::getFileNameNoSuffix(destFullFileName);
	std::string sourcePath = StringUtility::getFilePath(fullFileName) + "/";
	std::string destPath = StringUtility::getFilePath(destFullFileName) + "/";
	FileUtility::renameFile(fullFileName, destFullFileName);
	// 如果有同名位置文件,则也需要一起重命名
	std::string positionFileName = sourcePath + sourceFileNameNoSuffix + ".txt";
	if (FileUtility::isFileExist(positionFileName))
	{
		FileUtility::renameFile(positionFileName, destPath + destFileNameNoSuffix  + ".txt");
	}
}

void ImageUtility::deleteImageWithPosition(const std::string& fullFileName)
{
	std::string sourceFileNameNoSuffix = StringUtility::getFileNameNoSuffix(fullFileName);
	std::string sourcePath = StringUtility::getFilePath(fullFileName) + "/";
	FileUtility::deleteFile(fullFileName);
	// 如果有同名位置文件,则也需要一起重命名
	std::string positionFileName = sourcePath + sourceFileNameNoSuffix + ".txt";
	if (FileUtility::isFileExist(positionFileName))
	{
		FileUtility::deleteFile(positionFileName);
	}
}