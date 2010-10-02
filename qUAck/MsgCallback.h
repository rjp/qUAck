

class MsgCallback
{
      virtual void Page(std::string &from, std::string message) = 0;

      virtual void UserEvent_Login(const std::string &user, const std::string &status);
      virtual void UserEvent_Logout(const std::string &user, const std::string &status);
      virtual void UserEvent_Status(const std::string &user, const std::string &status);

      virtual void MsgEvent_Added(const std::string &user, const std::string &status);
      virtual void MsgEvent_Deleted(const std::string &user, const std::string &status);
      virtual void MsgEvent_Moved(const std::string &user, const std::string &status);
      virtual void MsgEvent_Edited(const std::string &user, const std::string &status);

      virtual void FolderEvent_Added(const std::string &user, const std::string &status);
      virtual void FolderEvent_Deleted(const std::string &user, const std::string &status);

      virtual void Shutdown();
      virtual void SystemMessage();
      virtual void 
};
