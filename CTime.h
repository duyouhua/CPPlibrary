#include <iostream>
#include <cstring>
#include <ctime>
#include <sys/time.h>
#include <cstdio>
#include<signal.h>
#include <stddef.h>
#include <iomanip>
#include<signal.h>

//普通时间类，以后根据情况扩展
class CTime
{
public:
    CTime();
	CTime(std::string tim);
    virtual ~CTime();
	std::string getTime();
	std::string getShortTime();
friend std::ostream& operator<<(std::ostream& os, CTime& tim);
	void Start(){ m_start = currentMillions();}
	void Stop(){ m_end = currentMillions();}
	void Reset() {m_start = m_end = 0;}
	void costTime();
private:
	unsigned long currentMillions()
	{
		struct timeb stim;
		ftime(&stim);
		return stim.time*1000+stim.millitm;
	}

    void timeToStr(char* buf)
	{
		time_t ti = time(NULL);
		strftime(buf, 30, "%Y-%m-%d %H:%M:%S", localtime(&ti));
	}
	std::string m_time; //reserve the time
	unsigned long m_start, m_end;
};


CTime::CTime()
{
	char buf[30]={'\0'};
	timeToStr(buf);
	m_time = std::string(buf);	
}

CTime::CTime(std::string tim)
{
	if(tim.size() == 0)
	{
		char buf[30];
        timeToStr(buf);
		m_time = buf;
	}
	else
	{
		m_time = tim;
	}
}

CTime::~CTime()
{}
std::string CTime::getTime()
{
	return m_time;
}

std::string CTime::getShortTime()
{
	if(m_time.size())
	{
		return m_time.substr(0, m_time.find_first_of(' '));
	}
	else
	{
		char buf[30];
        timeToStr(buf);
		return std::string(buf);
	}
}

void CTime::costTime()
{
	unsigned long diffTime = m_end - m_start;
	if (diffTime < 1000)
	{
		cout << "It take you " << diffTime << " milliseconds" << endl;
	}
	else
	{
		unsigned long secs  = diffTime / 1000;
		unsigned long msecs = diffTime % 1000;
		cout << "It take you " << secs << " seconds, " << " " << msecs << " milliseconds." << endl;        
	}
}
std::ostream& operator<<(std::ostream& os, CTime& tim)
{
	os <<tim.m_time;
	return os;
}

//专门设置定时任务类
class CTimeTask
{
public:
	typedef void(*func)(int signo, siginfo_t* info, void* context);
	
    CTimeTask()
	{
		memset(&evp, 0, sizeof(evp));  
		evp.sigev_notify = SIGEV_SIGNAL;  
		evp.sigev_signo = SIGRTMAX;  
		evp.sigev_value.sival_ptr = &timeID;
		if(::timer_create(CLOCK_REALTIME, &evp, &timeID) ==-1 )  
		{  
			cout << "timer create err" << endl;  
			return;  
		}  
	}
    virtual ~CTimeTask()
	{}

    int excuteTask(int sec, int nsec, func onFunc)
	{
		struct sigaction sigac;  
		sigemptyset(&sigac.sa_mask);  
		sigac.sa_flags = SA_SIGINFO;  
		sigac.sa_sigaction = onFunc;  
		sigaction(SIGRTMAX, &sigac, NULL);  
     
		struct itimerspec it;    
		it.it_interval.tv_sec = sec;    
		it.it_interval.tv_nsec = nsec;    
		it.it_value.tv_sec = sec;    
		it.it_value.tv_nsec = nsec;  
  
		if(::timer_settime(timeID, 0, &it, NULL) == -1)  
		{  
			cout << "set time task err" << endl;  
			::timer_delete(timeID);  
			return -1;  
		}  
	}
	static CTimeTask* getInstance()
	{
		if(timeTask == NULL)
			timeTask = new CTimeTask();
		return timeTask;
	}
private:
    timer_t timeID;  
    struct sigevent evp;
	static CTimeTask *timeTask;
	CTimeTask(const CTimeTask&);
	CTimeTask& operator=(const CTimeTask&);
};
CTimeTask* CTimeTask::timeTask = NULL;
#define timerTask CTimeTask::getInstance()

//日期类处理年月日
class CDate
{
public:
	
	static string DateSpan(string strDate, long nSpan, char cDelimiter = '-');
	static void   GetCurrentMonth(string &strFrom, string &strTo);
	static int    GetWeekDay(string strDate);
	static int    GetWeek(string strDate);
	static long   MinuteDiff(string strDateFrom, string strDateTo);
	static long   DateDiff(string strDateFrom, string strDateTo);
	static int    YearDiff(string strDateFrom, string strDateTo);
	static string GetMonthStart(string strDate);
	static string GetMonthEnd(string strDate);
	static string GetYear(string strDate);
	static string GetYearMonth(string strDate);
	static long   days_from_8zeros(char *date8);  
	static string FilterNumber(const string &str);
	static string GetWeekStartEnd(int year,int weekorder);
	static int    GetMonthDays(int year,int month);
	static int    WhatTime();
private:
	CDate(const CDate&);
	CDate& operator=(const CDate&);
};

/************************************************************************
  为指定日期加减一定天数
  输入参数：
    strDate    - 指定日期(YYYY-MM-DD)
    nSpan      - 天数(正为加，负为减)
    cDelimiter - 年月日间的分隔符(新生成日期用)，缺省为“-”
  输出参数：
    string     - 新生成的日期
************************************************************************/
string CDate::DateSpan(string strDate, long nSpan, char cDelimiter /* = '-' */)
{
    struct tm day = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    char date[32];
    time_t now = time(NULL);

    if(sscanf(strDate.c_str(), "%04d-%02d-%02d", &day.tm_year, &day.tm_mon, &day.tm_mday) == 3)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        now = mktime(&day);
    }
    else if(sscanf(strDate.c_str(), "%04d/%02d/%02d", &day.tm_year, &day.tm_mon, &day.tm_mday) == 3)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        now = mktime(&day);
    }

    now += nSpan * 24 * 60 * 60;
    localtime_r(&now, &day);
    sprintf(date, "%04d%c%02d%c%02d", day.tm_year + 1900, cDelimiter, day.tm_mon + 1, cDelimiter, day.tm_mday);

    return date;
}

/************************************************************************
  取得当月起止日期
  输出参数：
    strFrom - 当月起始日期，如2006-05-01
    strTo   - 当月截止日期，如2006-05-31
************************************************************************/
void CDate::GetCurrentMonth(string &strFrom, string &strTo)
{
    struct tm tmDay = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    time_t now = time(NULL);
    char date[32];

    // 当天时间截去日，改为1日即为当月起始日期
    localtime_r(&now, &tmDay);
    sprintf(date, "%04d-%02d-%02d", tmDay.tm_year + 1900, tmDay.tm_mon + 1, 1);
    strFrom = date;

    // 将日期改为下月的第一日
    tmDay.tm_mday = 1;
    tmDay.tm_mon++;
    if(tmDay.tm_mon > 11)
    {
        tmDay.tm_mon = 0;
        tmDay.tm_year++;
    }

    // 下月的第一日减去一天即为当月的最后一日
    now = mktime(&tmDay) - 24 * 60 * 60;
    localtime_r(&now, &tmDay);
    sprintf(date, "%04d-%02d-%02d", tmDay.tm_year + 1900, tmDay.tm_mon + 1, tmDay.tm_mday);
    strTo = date;
}

/************************************************************************
  取得指定日期是周几
                                                                      
  输入参数：
    strDate - 指定日期(1980-01-01)
  输出参数：
    无
  返 回 值：
    int     - 周几(1为周一，7为周日)
************************************************************************/
int CDate::GetWeekDay(string strDate)
{
    struct tm day = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    time_t now = time(NULL);

    if(sscanf(strDate.c_str(), "%04d-%02d-%02d", &day.tm_year, &day.tm_mon, &day.tm_mday) == 3)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        now = mktime(&day);
    }
    else if(sscanf(strDate.c_str(), "%04d/%02d/%02d", &day.tm_year, &day.tm_mon, &day.tm_mday) == 3)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        now = mktime(&day);
    }

    localtime_r(&now, &day);

    return day.tm_wday == 0 ? 7 : day.tm_wday;
}

/************************************************************************
  取得指定日期是哪年的第几周
                                                                      
  输入参数：
    strDate - 指定日期(1980-01-01)
  输出参数：
    无
  返 回 值：
    int     - 年和周，形如 200603，表示2006年第3周
************************************************************************/
int CDate::GetWeek(string strDate)
{
    string strFirstWeek = strDate.substr(0, 4) + "-01-01";
    int nYear = atoi(strDate.substr(0, 4).c_str());

    return nYear * 100 + (DateDiff(strFirstWeek, strDate) + GetWeekDay(strFirstWeek) - 1) / 7 + 1;
}

/************************************************************************
  取得指定两个日期的差(分钟数)
                                                                      
  输入参数：
    strDateFrom - 开始日期(1980-01-01)
    strDateTo   - 截止日期(1980-12-31)
  输出参数：
    无
  返 回 值：
    int         - 两个日期的差(分钟)
************************************************************************/
long CDate::MinuteDiff(string strDateFrom, string strDateTo)
{
    struct tm day = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    time_t from = time(NULL);
    time_t to = time(NULL);

    if(sscanf(strDateFrom.c_str(), "%04d-%02d-%02d %d:%d:%d", &day.tm_year, &day.tm_mon, &day.tm_mday,&day.tm_hour, &day.tm_min, &day.tm_sec) == 6)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        from = mktime(&day);        
    }   
    else if(sscanf(strDateFrom.c_str(), "%04d/%02d/%02d  %d:%d:%d", &day.tm_year, &day.tm_mon, &day.tm_mday,&day.tm_hour, &day.tm_min, &day.tm_sec) == 6)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        from = mktime(&day);
    }

    if(sscanf(strDateTo.c_str(), "%04d-%02d-%02d %d:%d:%d", &day.tm_year, &day.tm_mon, &day.tm_mday,&day.tm_hour, &day.tm_min, &day.tm_sec) == 6)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        to = mktime(&day);
    }
    else if(sscanf(strDateTo.c_str(), "%04d/%02d/%02d  %d:%d:%d", &day.tm_year, &day.tm_mon, &day.tm_mday,&day.tm_hour, &day.tm_min, &day.tm_sec) == 6)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        to = mktime(&day);
    }
    return (to - from)/60;
}
/************************************************************************
  取得指定两个日期的差(天数)
                                                                      
  输入参数：
    strDateFrom - 开始日期(1980-01-01)
    strDateTo   - 截止日期(1980-12-31)
  输出参数：
    无
  返 回 值：
    int         - 两个日期的差(天数)
************************************************************************/
long CDate::DateDiff(string strDateFrom, string strDateTo)
{
    struct tm day = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    time_t from = time(NULL);
    time_t to = time(NULL);

    if(sscanf(strDateFrom.c_str(), "%04d-%02d-%02d", &day.tm_year, &day.tm_mon, &day.tm_mday) == 3)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        from = mktime(&day);
    }
    else if(sscanf(strDateFrom.c_str(), "%04d/%02d/%02d", &day.tm_year, &day.tm_mon, &day.tm_mday) == 3)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        from = mktime(&day);
    }

    if(sscanf(strDateTo.c_str(), "%04d-%02d-%02d", &day.tm_year, &day.tm_mon, &day.tm_mday) == 3)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        to = mktime(&day);
    }
    else if(sscanf(strDateTo.c_str(), "%04d/%02d/%02d", &day.tm_year, &day.tm_mon, &day.tm_mday) == 3)
    {
        day.tm_year -= 1900;
        day.tm_mon--;
        to = mktime(&day);
    }

    return (to - from) / (60 * 60 * 24);
}

/************************************************************************
  取得指定两个日期的差年数
                                                                      
  输入参数：
    strDateFrom - 开始日期(1980-01-01)
    strDateTo   - 截止日期(1980-12-31)
  输出参数：
    无
  返 回 值：
    int         - 两个日期的差(天数)
************************************************************************/
int CDate::YearDiff(string strDateFrom, string strDateTo)
{
    struct tm day = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int nYearFrom = 0,nYearTo = 0;

    if(strDateFrom > strDateTo)
    {
        string strTemp = strDateFrom;
        strDateFrom = strDateTo;
        strDateTo = strTemp;
    }    
    if(sscanf(strDateFrom.c_str(), "%04d-%02d-%02d", &day.tm_year, &day.tm_mon, &day.tm_mday) == 3)
    {
		nYearFrom = day.tm_year;
    }   
    if(sscanf(strDateTo.c_str(), "%04d-%02d-%02d", &day.tm_year, &day.tm_mon, &day.tm_mday) == 3)
    {
		nYearTo = day.tm_year;
    }
    int nCount = 0,temp = 0;
    char buff[9];
    if((nYearTo-nYearFrom)>0)
    {
        nCount = nYearTo-nYearFrom;
        for(int i=0 ;i <= nCount ;i++)
        {
            temp = day.tm_year+i;
            sprintf(buff,"%04d-12-31",temp);
        }
    }
    
    return nYearTo - nYearFrom;
}

/************************************************************************
  取得指定日期所在月的起始日期
                                                                      
  输入参数：
    strDate - 指定日期(1980-01-23)
  输出参数：
    无
  返 回 值：
    string  - 月起始日期(1980-01-01)
************************************************************************/
string CDate::GetMonthStart(string strDate)
{
    return strDate.substr(0, 8) + "01";
}

/************************************************************************
  取得指定日期所在的年份
                                                                      
  输入参数：
    strDate - 指定日期(1980-01-23)
  输出参数：
    无
  返 回 值：
    string  - 月起始日期(1980-01-01)
*********************************************************************/
string CDate::GetYear(string strDate)
{
    string strCur = strDate.substr(0, 4);
    int year = atoi(strCur.c_str());
    if(year < 2004)
    {
		strCur = CVariant().AsDateWithDefault().substr(0,4);
    }
    return strCur;
}

/************************************************************************
  取得指定日期所在的年份月份
                                                                      
  输入参数：
    strDate - 指定日期(1980-01-23)
  输出参数：
    无
  返 回 值：
    string  - 月起始日期(1980-01-01)
*********************************************************************/
string CDate::GetYearMonth(string strDate)
{
    int nYear, nMonth;
    string strCur;

    strCur = strDate.substr(0, 4);
    int year = atoi(strCur.c_str());
    if(year < 2003)
    {
		strCur = CVariant().AsDateWithDefault().substr(0,4)+"_"+CVariant().AsDateWithDefault().substr(5,2);
    }
    
    if(sscanf(strDate.c_str(), "%04d-%02d", &nYear, &nMonth) == 2)
    {
        strCur = strDate.substr(0, 4)+"_"+strDate.substr(5,2);
    }
    else
    {
        // 格式不正确，返回当前月截止日期
        strCur = CVariant().AsDateWithDefault().substr(0,4)+"_"+CVariant().AsDateWithDefault().substr(5,2);
    }   
    
    return strCur;
}

/************************************************************************
  取得指定日期所在月的截止日期
                                                                      
  输入参数：
    strDate - 指定日期(1980-01-23)
  输出参数：
    无
  返 回 值：
    string  - 月截止日期(1980-01-31)
************************************************************************/
string CDate::GetMonthEnd(string strDate)
{
    char buff[32];
    int nYear, nMonth;

    if(sscanf(strDate.c_str(), "%04d-%02d", &nYear, &nMonth) == 2)
    {
        // OK，得到月和年
    }
    else if(sscanf(strDate.c_str(), "%04d/%02d", &nYear, &nMonth) == 2)
    {
        // OK，得到月和年
    }
    else
    {
        // 格式不正确，返回当前月截止日期
        return GetMonthEnd(CVariant().AsDateWithDefault());
    }

    // 计算下月一日
    nMonth ++;
    if(nMonth > 12)
    {
        nMonth = 1;
        nYear ++;
    }
    sprintf(buff, "%04d-%02d-01", nYear, nMonth);

    // 下月一日减一日即为本月截止日期
    return CDate::DateSpan(buff, -1);
}

/************************************************************************
  日期距离公元00000000的天数
                                                                      
  输入参数：
      char *date8 - 日期
      
  输出参数：
    
  返 回 值：
    long        天数
                                          
************************************************************************/
long CDate::days_from_8zeros(char *date8)
{
	int  leap;       //0-平年 1-闰年
	long int days;
	int  vyear,vmonth,vday;
	int  c;
	char strtmp[10];

	vyear=vmonth=vday=0;
	memset(strtmp,0,sizeof(strtmp)); memcpy(strtmp,date8+0,4); sscanf(strtmp,"%d",&vyear);
	memset(strtmp,0,sizeof(strtmp)); memcpy(strtmp,date8+4,2); sscanf(strtmp,"%d",&vmonth);
	memset(strtmp,0,sizeof(strtmp)); memcpy(strtmp,date8+6,2); sscanf(strtmp,"%d",&vday);


	days=0;
	for (c=0;c<vyear;c++)
	{
		leap=0;
		if (((c%4)==0&&(c%100)!=0) || (c%400)==0)  leap=1;
		if (leap==0) days+=365;
		if (leap==1) days+=366;
	}

	c=vyear;
	leap=0;
	if (((c%4)==0&&(c%100)!=0) || (c%400)==0)  leap=1;

	for (c=1;c<vmonth;c++)
	{
		if (c==1) days+=31;
		if (c==2 && leap==0) days+=28;
		if (c==2 && leap==1) days+=29;
		if (c==3) days+=31;
		if (c==4) days+=30;
		if (c==5) days+=31;
		if (c==6) days+=30;
		if (c==7) days+=31;
		if (c==8) days+=31;
		if (c==9) days+=30;
		if (c==10) days+=31;
		if (c==11) days+=30;
		if (c==12) days+=31;
	}
	days+=vday;
	return(days);
}

string CDate::GetWeekStartEnd(int year,int weekorder)
{
	if(year < 1000 || 9999 <year)
	{
		return "";
	}
	if(weekorder <1 || 54 < weekorder )
	{
		return "";
	}
	char buff[5];
	sprintf(buff,"%d",year);
	string datFDayFirstWeek,datLDayFirstWeek;
	datFDayFirstWeek=string(buff)+"-01-01";//当年的第一天
	int intWeekday = GetWeekDay(datFDayFirstWeek)-1;//周几
	datLDayFirstWeek=DateSpan(datFDayFirstWeek,7-intWeekday);
	if(intWeekday==1)
	{
		return datFDayFirstWeek+"|"+datLDayFirstWeek;
	}
	else
	{
		string datFDayTheWeek,datLDayTheWeek;
		datLDayTheWeek = DateSpan(datLDayFirstWeek,(weekorder-1)*7-1);
		datFDayTheWeek = DateSpan(datLDayTheWeek, -6);
		if(weekorder>=1 && weekorder<=52)
		{
			return datFDayTheWeek+"|"+datLDayTheWeek;
		}
		else if(weekorder==53)
		{
			if(atoi(GetYear(datLDayTheWeek).c_str())==year+1)
			{
				sprintf(buff,"%d",year);
				datLDayTheWeek = string(buff)+"-12-31";
			}
			return datFDayTheWeek+"|"+datLDayTheWeek;
		}
		else if(weekorder ==54)
		{
			if(atoi(GetYear(datFDayTheWeek).c_str())==year)
			{
				sprintf(buff,"%d",year);
				datFDayTheWeek=datLDayTheWeek=string(buff)+"-12-31";
				return datFDayTheWeek+"|"+datLDayTheWeek;            
			}
			else
			{
				return "";
			}
		}  
	}  
	return "";
}

string CDate::FilterNumber(const string &str)
{
    char buff[256];
    sscanf(str.c_str(), "%[^0-9]", buff);  
    return string(buff);
}

int CDate::GetMonthDays(int year,int month)
{
	int days = 0;
	int leap = 0;
	switch (month)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		days = 31;
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		days = 30;
		break;
	case 2:
		if ( year % 400 == 0 )
		{
			leap = 1;
		}else if ( year % 400 == 0 && year % 100 !=0 )
		{
			leap = 1;
		}
		if ( leap)
		{
			days = 29;
		}else
		{
			days = 28;
		}
	default :
		break;
	}
	return days;
}

// 返回几点（0-23）
int CDate::WhatTime()
{
	time_t ltime;    
	time(&ltime);        
	struct tm *t = localtime(&ltime); 

	return t->tm_hour;
}

