#include "grynlayer.h"
#include "qUAck.h"
#include "../ua.h"

GrynLayer::GrynLayer()
{
  m_conn = new EDFConn();
}

void GrynLayer::setBanner(const std::string &newBanner)
{
    if(newBanner.length()>0)
    {
       EDF *pRequest = new EDF();
       int  iTime    = CmdLineNum("Run time (seconds)");
       pRequest->AddChild("time", iTime);
       pRequest->Add("request", MSG_SYSTEM_EDIT);
       pRequest->AddChild("banner", newBanner.c_str());
       pRequest->Parent();

       //CmdEDFPrint("TaskMenu banner change", pRequest);
       EDF *pReply = NULL;
       if(CmdRequest(MSG_TASK_ADD, pRequest, &pReply) == false)
       {
          //CmdEDFPrint("Task banner change:", pReply);
       }
       delete pRequest;
       delete pReply;
    }
}



const std::string &GrynLayer::getProtocol()
{
   return m_protocol;
}

const std::string &GrynLayer::getBanner()
{
   return m_banner;
}


void GrynLayer::Disconnect()
{
   if (m_conn != NULL)
   {
      m_conn->Disconnect();
   }
}

int EDFChildInt(EDF *pData, const char *fieldName)
{
    int i=0;
    pData->GetChild(fieldName, &i);
    return i;
}

std::string EDFChildString(EDF *pData, const char *fieldName)
{
  char *szMsg = NULL;
  pData->GetChild(fieldName, &szMsg);
  if (szMsg != NULL)
  {
     std::string fieldData = szMsg; 
     delete[] szMsg;
     return fieldData;
  }
  return "";
}

bool GrynLayer::Connect(const char *szServer, int iPort, bool bSecure, const char *szCertFile)
{
  double dTick = gettick();

  m_hostname = szServer;
  m_port     = iPort;

  bool status = m_conn->Connect(m_hostname.c_str(), iPort, bSecure, szCertFile);

  if (!status)
     return false;

  debug("qUAck mem1: %ld bytes\n", memusage());

  int iLoop = time(NULL);
  while(m_conn->State() == Conn::OPEN && m_conn->Connected() == false)
  {
     EDF *pRead = m_conn->Read();
     int iGiveUp = 10;

     if(time(NULL) > iLoop + iGiveUp)
     {
        char szWrite[100];
        sprintf(szWrite, "Could not enable EDF in %d seconds, giving up", iGiveUp);
        CmdShutdown(szWrite);
     }
     if (pRead!=NULL)
        delete pRead;
  }

  debug("qUAck mem2: %ld bytes\n", memusage());

  if(!IsConnected())
  {
     CmdShutdown("Disconnected");
     return false;
  }
  debug(DEBUGLEVEL_DEBUG, "Diff after connection %ld ms\n", tickdiff(dTick));

  EDF *pSystem = NULL;
  CmdRequest(MSG_SYSTEM_LIST, &pSystem);

  
  pSystem->GetChild("systemtime", &m_systemTime);


  m_banner     = EDFChildString(pSystem, "banner"); 
  m_version    = EDFChildString(pSystem, "version"); 
  m_servername = EDFChildString(pSystem, "name");   
  m_buildnum   = EDFChildInt(pSystem, "buildnum"); 
  m_builddate  = EDFChildString(pSystem, "builddate"); 
  m_protocol   = EDFChildString(pSystem, "protocol"); 

  return true;

}


int GrynLayer::getTimeout()
{
   return m_conn->Timeout();
}

void GrynLayer::setTimeout(int timeout)
{
   m_conn->Timeout(timeout);
}

int GrynLayer::getSystemTime()
{
  return m_systemTime;
}

int GrynLayer::getPort()
{
  return m_port;
}

const std::string &GrynLayer::getHostname()
{
   return m_hostname;
}

const std::string GrynLayer::getServerDetails()
{
   // SGD pSystem->GetChild("name", &szServerName);
   // SGD pSystem->GetChild("buildnum", &iBuildNum);
   // SGD pSystem->GetChild("builddate", &szBuildDate);
   // SGD 

   char szWrite[100];
   memset(szWrite, 0, sizeof(szWrite));
   sprintf(szWrite, "\0370%s Version \0373%s\0370 Build \0373%d\0370, \0373%s\0370\n\n", m_servername.c_str(), m_version.c_str(), m_buildnum, m_builddate.c_str());
   return szWrite;
}


bool GrynLayer::IsSecure()
{
   return m_conn->GetSecure();
}

bool GrynLayer::IsConnected()
{
   if (m_conn != NULL && (m_conn->State()== Conn::OPEN))
   {
      return true;
   }
   else
   {
      return false;
   }
}

const std::string GrynLayer::getErrorMessage()
{
   return m_conn->Error();
}



void GrynLayer::setBusyStatus(bool busy, const std::string &message)
{
    int newStatus = -1;
    if (busy)
    {
        newStatus = LOGIN_BUSY;
    }
    else
    {
        newStatus = LOGIN_OFF;
    }

    EDF *pRequest = new EDF();
    pRequest->Add("login");
    pRequest->AddChild("status", newStatus);
    if (message.length()>0)
    {
       printf("statusmsg: %s\n", message.c_str());
       pRequest->AddChild("statusmsg", message.c_str());
    }
    pRequest->Parent();

    CmdRequest(MSG_USER_EDIT, pRequest);
    CmdUserReset();

}


void GrynLayer::FolderSubscribe(int folderId)
{
	EDF* pRequest = new EDF();
	pRequest->SetChild("folderid", folderId);
	CmdRequest(MSG_FOLDER_SUBSCRIBE, pRequest, false);
	
	delete pRequest;
}


void GrynLayer::GuestLogin()
{
	
	Login("", "", USER_ISGUEST);
}

void GrynLayer::Login(const std::string &username, const std::string &password, UserState status)
{
	EDF* pRequest = new EDF();
	if(username.length()>0)
	{
		pRequest->AddChild("name", username.c_str());
	}
	if(password.length()>0)
	{
		pRequest->AddChild("password", password.c_str());
	}
	pRequest->AddChild("client", CLIENT_NAME());
	pRequest->AddChild("clientbase", CLIENT_BASE);
	pRequest->AddChild("protocol", PROTOCOL);
	
	if (status & USER_ISAGENT)
	{
		pRequest->AddChild("usertype", USERTYPE_AGENT);
	} else if (status & USER_ISGUEST)
	{
		pRequest->AddChild("usertype", USERTYPE_TEMP);
	}
	
	if (status & USER_FORCELOGIN)
	{
		pRequest->AddChild("force", true);
	}
	
	int iStatus = 0;
	
	if(status & USER_BUSY)
	{
		iStatus += LOGIN_BUSY;
	}
	if(status & USER_SILENT)
	{
		iStatus += LOGIN_SILENT;
	}
	if(status & USER_SHADOW)
	{
		iStatus += LOGIN_SHADOW;
	}
}




void GrynLayer::MessageProcessing()
{
	/*EDF *pRead = m_conn->Read();
	
	if (pRead!=NULL)
	{
		char *szType = NULL;
		pRead->Get(&szType);
		if(stricmp(szType, "announce") == 0)
		{
			iMenuStatus = -1;
			int iAnnounce = CmdAnnounce(pRead, iMenuStatus == -1, !mask(pInput->Type(), CMD_MENU_SILENT));
			if(iMenuStatus != -1)
			{
				delete pRead;
			}
			if(iMenuStatus != -1)
			{
				if(mask(iAnnounce, CMD_RESET) == true)
				{
					pInput = CmdInputSetup(iMenuStatus);
					bShow = true;
				}
				else if(mask(iAnnounce, CMD_REDRAW) == true)
				{
					// pInput->Show();
					bShow = true;
				}
			}
		}
		
	}
	else if(!m_pGrynLayer->IsConnected())
	{
		CmdShutdown("Connection to host lost");
	}
	*/

}