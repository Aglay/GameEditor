﻿using Mono.Data.Sqlite;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

public class EffectData : TableData
{
	public static string COL_LABEL = "Label";
	public static string COL_ID = "ID";
	public static string COL_DESC = "Desc";
	public static string COL_RESOURCE = "Resource";
	public string mLabel;
	public int mID;
	public string mDesc;
	public string mResource;
	public override void parse(SqliteDataReader reader)
	{
		mLabel = reader[COL_LABEL].ToString();
		mID = stringToInt(reader[COL_ID].ToString());
		mDesc = reader[COL_DESC].ToString();
		mResource = reader[COL_RESOURCE].ToString();
	}
}

public class SQLiteEffect : SQLiteTable
{
	public SQLiteEffect()
		:base("Effect")
	{}
	public void query(int id, out EffectData data)
	{
		string condition = "";
		appendConditionInt(ref condition, EffectData.COL_ID, id, "");
		parseReader(doQuery(condition), out data);
	}
	public void queryAll(out List<EffectData> dataList)
	{
		parseReader(doQuery(), out dataList);
	}
	public void insert(EffectData data)
	{
		string valueString = "";
		appendValueString(ref valueString, data.mLabel);
		appendValueInt(ref valueString, data.mID);
		appendValueString(ref valueString, data.mDesc);
		appendValueString(ref valueString, data.mResource, true);
		doInsert(valueString);
	}
};