/*
 * Copyright 2019 Broadcom Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HOSTAPDMGR_H_
#define _HOSTAPDMGR_H_

#include <swss/dbconnector.h>
#include <swss/schema.h>
#include <swss/table.h>
#include <swss/macaddress.h>
#include <swss/notificationproducer.h>
#include <swss/subscriberstatetable.h>
#include <swss/producerstatetable.h>
#include <swss/table.h>
#include <swss/select.h>
#include <swss/timestamp.h>
#include <swss/debugsh.h>
#include <map>
#include <string>
#include "netmsg.h"
#include "redisapi.h"

using namespace swss;
using namespace std;

#define HOSTAPDMGR_HOSTAPD_USER_CONFIG_FILE_PATH      "/etc/hostapd/hostapd.eap_user" /*CG_PAC*/
#define HOSTAPDMGR_HOSTAPD_USER_AUTH_TYPE_DEF         "MD5" /*CG_PAC*/
#define HOSTAPDMGR_HOSTAPD_USER_PASSWORD_DEF          "" /*CG_PAC*/
#define HOSTAPDMGR_HOSTAPD_USER_VLAN_ID_DEF           0 /*CG_PAC*/
#define HOSTAPDMGR_HOSTAPD_USER_SESSION_TIMEOUT_DEF   60 /*CG_PAC*/

void hostapdHandleDumpError(void *cbData);

typedef struct hostapd_glbl_info_s {
  unsigned int enable_auth;
  std::string auth_order_list; /*CG_PAC*/
}hostapd_glbl_info_t;
  
typedef struct hostapd_intf_info_s {
  std::string capabilities;
  std::string control_mode;
  unsigned int admin_status;
  unsigned int link_status;
  bool config_created;
}hostapd_intf_info_t;

typedef struct radius_server_info_s {
  std::string server_port;
  std::string server_key;
  std::string server_priority;
  std::string server_ip;
  bool config_ok;
}radius_server_info_t;

typedef std::map<std::string, radius_server_info_t> radius_server_info_map_t;

typedef struct radius_info_s {
  string m_radiusGlobalKey;
  radius_server_info_map_t radius_auth_server_list;  
}radius_info_t;

typedef std::map<std::string, hostapd_intf_info_t> hostapd_intf_info_map_t;

/*CG_PAC*/
/* Hostapd User config table param cache Info */
typedef struct hostapdUserConfigCacheParams_t {
    std::string password;    /*Password of the user/supplicant*/
    std::string auth_type;   /*Supported Authentication Type. Default: eap-md5*/
    int vlan_id;             /*VLAN to be associated with the authorized client*/
    int session_timeout;     /*Client session time*/
} hostapdUserConfigCacheParams_t;

/*MAP to store Hostapd user configuration table parameters
 * Key: MAC Address of Supplicant
 * Value: hostapdUserConfigCacheParams_t
 * */
typedef std::map<std::string, hostapdUserConfigCacheParams_t> hostapdUserConfigTableMap;

class HostapdMgr : public NetMsg
{ 
public:
  HostapdMgr(DBConnector *configDb, DBConnector *appDb);
  std::vector<Selectable*> getSelectables();
  bool processDbEvent(Selectable *source);
  virtual void onMsg(int nlmsg_type, struct nl_object *obj);
  void killHostapd(void);
  void showDebugInfo(DebugShCmd *cmd, string intf);
  string getStdIfFormat(string intf);

private:
  //tables this component listens to
  SubscriberStateTable m_confHostapdPortTbl;
  SubscriberStateTable m_confHostapdGlobalTbl;
  SubscriberStateTable m_confRadiusServerTable;
  SubscriberStateTable m_confRadiusGlobalTable;
  SubscriberStateTable m_confHostapdUserCfgTbl; /*CG_PAC*/

  hostapdUserConfigTableMap     m_hostapdUserConfigMap; //CG_PAC
  hostapd_glbl_info_t m_glbl_info;
  hostapd_intf_info_map_t m_intf_info;
  radius_info_t m_radius_info;
  string m_radiusServerInUse;
  radius_server_info_t m_radiusServerInUseInfo;
  unsigned int active_intf_cnt;

  bool start_hostapd;
  bool stop_hostapd;

  void setPort(const string & alias, const hostapd_intf_info_t &intf_info);
  void delPort(const string & alias);
    
  // DB Event handler functions
  bool processHostapdConfigPortTblEvent(Selectable *tbl);
  bool processHostapdConfigGlobalTblEvent(Selectable *tbl);
  bool processRadiusServerTblEvent(Selectable *tbl);
  bool processRadiusGlobalTblEvent(Selectable *tbl);

  void writeToFile(const string& filename, const string& value);
  void informHostapd(const string& type, const vector<string> & interfaces);
  void createConfFile(const string& intf);
  void createLocalConfFile(const string& intf); //CG_PAC
  void deleteConfFile(const string& intf);
  pid_t getHostapdPid(void);
  int waitForHostapdInit(pid_t hostapd_pid);
  void sendSignal(void);
  void updateRadiusServer();
};

#endif // _HOSTAPDMGR_H_
