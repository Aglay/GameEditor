﻿#include "SQLiteWeaponFrame.h"
#include "Utility.h"
#include "SQLiteDataReader.h"
#include "SQLite.h"

void SQLiteWeaponFrame::query(int weaponID, txVector<WeaponFrameData>& dataList)
{
	std::string conditionString;
	appendConditionInt(conditionString, COL_ID, weaponID, "");
	std::string queryStr = "SELECT * FROM " + mTableName + " WHERE " + conditionString;
	parseReader(mSQLite->executeQuery(queryStr), dataList);
}
void SQLiteWeaponFrame::query(int weaponID, int direction, txVector<WeaponFrameData>& dataList)
{
	std::string conditionString;
	appendConditionInt(conditionString, COL_ID, weaponID, " and ");
	appendConditionInt(conditionString, COL_DIRECTION, direction, "");
	std::string queryStr = "SELECT * FROM " + mTableName + " WHERE " + conditionString;
	parseReader(mSQLite->executeQuery(queryStr), dataList);
}
void SQLiteWeaponFrame::query(int weaponID, int direction, const std::string& action, txVector<WeaponFrameData>& dataList)
{
	std::string conditionString;
	appendConditionInt(conditionString, COL_ID, weaponID, " and ");
	appendConditionInt(conditionString, COL_DIRECTION, direction, " and ");
	appendConditionString(conditionString, COL_ACTION, action, "");
	std::string queryStr = "SELECT * FROM " + mTableName + " WHERE " + conditionString;
	parseReader(mSQLite->executeQuery(queryStr), dataList);
}
bool SQLiteWeaponFrame::updateData(const WeaponFrameData& data)
{
	std::string updateString;
	appendUpdateInt(updateString, COL_FRAME_COUNT, data.mFrameCount);
	appendUpdateIntArray(updateString, COL_POSX, data.mPosX);
	appendUpdateIntArray(updateString, COL_POSY, data.mPosY, true);
	std::string conditionString;
	appendConditionInt(conditionString, COL_ID, data.mID, " and ");
	appendConditionInt(conditionString, COL_DIRECTION, data.mDirection, " and ");
	appendConditionString(conditionString, COL_ACTION, data.mAction, "");
	return doUpdate(updateString, conditionString);
}

bool SQLiteWeaponFrame::insert(const WeaponFrameData& data)
{
	std::string valueString;
	appendValueInt(valueString, data.mID);
	appendValueString(valueString, data.mLabel);
	appendValueInt(valueString, data.mDirection);
	appendValueString(valueString, data.mAction);
	appendValueInt(valueString, data.mFrameCount);
	appendValueIntArray(valueString, data.mPosX);
	appendValueIntArray(valueString, data.mPosY, true);
	return doInsert(valueString);
}

bool SQLiteWeaponFrame::insertOrUpdate(const WeaponFrameData& data)
{
	bool ret = false;
	txVector<WeaponFrameData> tempList;
	query(data.mID, data.mDirection, data.mAction, tempList);
	if (tempList.size() == 0)
	{
		ret = insert(data);
	}
	else
	{
		ret = updateData(data);
	}
	return ret;
}

void SQLiteWeaponFrame::parseReader(SQLiteDataReader* reader, txVector<WeaponFrameData>& dataList)
{
	while (reader->read())
	{
		WeaponFrameData data;
		data.mID = reader->getInt(getCol(COL_ID));
		data.mLabel = reader->getString(getCol(COL_LABEL));
		data.mDirection = reader->getInt(getCol(COL_DIRECTION));
		data.mAction = reader->getString(getCol(COL_ACTION));
		data.mFrameCount = reader->getInt(getCol(COL_FRAME_COUNT));
		StringUtility::stringToIntArray(reader->getString(getCol(COL_POSX)), data.mPosX);
		StringUtility::stringToIntArray(reader->getString(getCol(COL_POSY)), data.mPosY);
		dataList.push_back(data);
	}
	mSQLite->releaseReader(reader);
}