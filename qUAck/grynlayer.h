
#ifndef __GRYNLAYER_H
#define __GRYNLAYER_H

#include <string>
#include "../Conn/EDFConn.h"

class GrynLayer
{
    public:
		GrynLayer();
    public:
        typedef enum SessionState
        {
            CLOSED = 0, 
            OPEN = 1, 
            CLOSING = 2, 
            LOST = 3,
            LOGGEDIN = 4
        };

        typedef enum UserState
        {
		   USER_UNSETSTATE = 0,
           USER_BUSY       = 1,
           USER_SILENT     = 2,
           USER_SHADOW     = 4,
		   USER_ISAGENT    = 8,
		   USER_ISGUEST    = 16,
		   USER_FORCELOGIN = 32
        };

    public:
        bool Connect(const char *szServer, int iPort, bool bSecure = false, const char *szCertFile = NULL);
        void Disconnect();
        void setTimeout(int timeout);
        int  getTimeout();
 
        bool IsSecure();
        bool IsConnected();

        void setBusyStatus(bool busy, const std::string &message="");
	void FolderSubscribe(int folderId);
	
	void GuestLogin();
	void Login(const std::string &username, const std::string &password, UserState status);
	
	void MessageProcessing();


	void setBanner(const std::string &newBanner);
	const std::string &getBanner();
	const std::string &getProtocol();
        const std::string getServerDetails();
        int getSystemTime();

        int getPort();
        const std::string &getHostname();

        const std::string getErrorMessage();


        EDFConn *getEDF()
        {
           return m_conn;
        }

    private:
        EDFConn *m_conn;
        std::string m_banner;
        std::string m_version;
        int m_systemTime;
        int m_port;
        std::string m_hostname;

        std::string m_servername; 
        int         m_buildnum; 
        std::string m_builddate;
        std::string m_protocol;
};


#endif
