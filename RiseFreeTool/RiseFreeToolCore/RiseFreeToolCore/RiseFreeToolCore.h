#ifndef _RISE_FREE_TOOL_CORE_H_
#define _RISE_FREE_TOOL_CORE_H_

#include "CommonDefine.h"
#include "txSingleton.h"

#define DECLEAR_SYSTEM(className)\
public:\
	className* get##className() { return m##className; }\
protected:\
	className* m##className;

class EventSystem;
class CommandSystem;
class SpeedDataManager;
class ComManager;
class RiseFreeToolCore : public txSingleton<RiseFreeToolCore>
{
public:
	RiseFreeToolCore();
	virtual ~RiseFreeToolCore(){ destroy(); }
	virtual void init();
	virtual void update(float elapsedTime);
	virtual void destroy();
	void notifyInitDone() { mInitFlag = true; }
	void checkErrorInfoBuffer();
	void sendEvent(const CORE_EVENT_TYPE& type, const std::string& param, bool sendImmediately = true);
	void sendEvent(const CORE_EVENT_TYPE& type, const txVector<std::string>& params = txVector<std::string>(), bool sendImmediately = true);
	void sendDelayEvent(const CORE_EVENT_TYPE& type, const std::string& param);
	void sendDelayEvent(const CORE_EVENT_TYPE& type, const txVector<std::string>& params = txVector<std::string>());
	void logError(const std::string& info, const bool& delay = false);
	void logInfo(const std::string& info, const bool& delay = false);
	DECLEAR_SYSTEM(EventSystem);
	DECLEAR_SYSTEM(CommandSystem);
	DECLEAR_SYSTEM(SpeedDataManager);
	DECLEAR_SYSTEM(ComManager);
protected:
	txVector<std::string> mErrorInfoBuffer;		// ������Ϣ����,���ڱ����¼�ϵͳ����֮ǰ�����Ĵ�����Ϣ
	bool mInitFlag;								// ��ʼ�����,Ϊfalse��ʾ��δ��ʼ�����,���ⲿ���б��
};

#endif