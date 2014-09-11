/*
# ===========
# SAP Dissector Plugin for Wireshark
#
# Copyright (C) 2014 Core Security Technologies
#
# The plugin was designed and developed by Martin Gallo from the Security
# Consulting Services team of Core Security Technologies.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# ==============
*/

#include "config.h"

#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/expert.h>

/* Define default ports */
#define SAPMS_PORT_RANGE "3600-3699,3900-3999"

/* MS Flag values */
static const value_string hf_sapms_flag_vals[] = {
	{  1, "MS_ONE_WAY" },
	{  2, "MS_REQUEST" },
	{  3, "MS_REPLY" },
	{  4, "MS_ADMIN" },
	{  0, NULL }
};	

/* MS IFlag values */
static const value_string hf_sapms_iflag_vals[] = {
	{  1, "MS_SEND_NAME" },
	{  2, "MS_SEND_TYPE" },
	{  3, "MS_LOGIN" },
	{  4, "MS_LOGOUT" },
	{  5, "MS_ADM_OPCODES" },
	{  6, "MS_MOD_MSGTYPES" },
	{  7, "MS_SEND_TYPE_ONCE" },
	{  8, "MS_LOGIN_2" },
	{  9, "MS_MOD_STATE" },
	{ 10, "MS_J2EE_LOGIN" },
	{ 12, "MS_J2EE_SEND_TO_CLUSTERID" },
	{ 13, "MS_J2EE_SEND_BROADCAST" },
	{ 14, "MS_SEND_TYPE_ACK" },
	{ 15, "MS_J2EE_LOGIN_2" },
	{ 16, "MS_SEND_SERVICE" },
	{ 17, "MS_J2EE_LOGIN_3" },
	{ 18, "MS_J2EE_LOGIN_4" },
	{  0, NULL }
};

/* MS Error number values */
static const value_string hf_sapms_errorno_vals[] = {
    { 0, "MSERECONNECTION" },
    { 1, "MSENISELWRITE" },
    { 2, "MSENISELREAD" },
    { 3, "MSENIQUEUE" },
    { 4, "MSENILAYER" },
    { 71, "MSETESTSOFTSHUTDOWN" },
    { 72, "MSENOTINIT" },
    { 73, "MSEALREADYINIT" },
    { 74, "MSEINVHDL" },
    { 75, "MSEWRONGSERVER" },
    { 76, "MSEMOREDATA" },
    { 77, "MSESNDTYPEFAILED" },
    { 78, "MSEDUPKEY" },
    { 79, "MSESOFTSHUTDOWN" },
    { 81, "MSENOMEM" },
    { 82, "MSEHEADERINCOMPLETE" },
    { 83, "MSETYPESNOTALLOWED" },
    { 84, "MSEACCESSDENIED" },
    { 85, "MSEWRITEINCOMPLETE" },
    { 86, "MSEREADINCOMPLETE" },
    { 87, "MSEADMIN" },
    { 88, "MSESOCKTOOLARGE" },
    { 89, "MSENOSLOTFREE" },
    { 90, "MSESHUTDOWN" },
    { 91, "MSEREPLYTOOLARGE" },
    { 92, "MSEWRONGVERSION" },
    { 93, "MSEMSGDESTROYED" },
    { 94, "MSENOTUNIQUE" },
    { 95, "MSEPARTNERUNKNOWN" },
    { 96, "MSEPARTNERDIED" },
    { 97, "MSEWRONGTYPE" },
    { 98, "MSEWRONGNAME" },
    { 99, "MSEWAKEUP" },
    { 100, "MSENOTFOUND" },
    { 101, "MSEINVAL" },
    { 102, "MSENOMSG" },
    { 103, "MSEINTERN" },
};

/* MS Adm Message Types values */
static const value_string hf_sapms_adm_msgtype_vals[] = {
    {  1, "ADM_REQUEST" },
    {  2, "ADM_REPLY" },
    {  0, NULL }
};

/* MS Adm Record Opcode values */
static const value_string hf_sapms_adm_record_opcode_vals[] = {
    { 0x00, "AD_GENERAL" }, 
    { 0x01, "AD_PROFILE" },
    { 0x02, "AD_WPSTAT" },
    { 0x03, "AD_QUEUE" },
    { 0x04, "AD_STARTSTOP" },
    { 0x05, "AD_WPCONF" },
    { 0x06, "AD_USRLST" },
    { 0x07, "AD_WPKILL" },
    { 0x08, "AD_TIMEINFO" },
    { 0x09, "AD_TM_RECONNECT" },
    { 0x0a, "AD_ALRT_GET_STATE" },
    { 0x0b, "AD_ALRT_OPERATION" },
    { 0x0c, "AD_ALRT_SET_PARAM" },
    { 0x0d, "AD_DB_RECONNECT" },
    { 0x0e, "AD_ECHO" },
    { 0x0f, "AD_MSGSERVER" },
    { 0x10, "AD_WPCONF2" },
    { 0x11, "AD_GENERAL2" },
    { 0x12, "AD_SET_LIST_PARAM" },
    { 0x13, "AD_DUMP_STATUS" },
    { 0x14, "AD_RZL" },
    { 0x15, "AD_RZL_STRG" },
    { 0x16, "AD_COUNT_WPS" },
    { 0x17, "AD_QUEUE2" },
    { 0x19, "AD_EM" },
    { 0x1a, "AD_ES" },
    { 0x1b, "AD_SHUTDOWN_STATE" },
    { 0x1c, "AD_SHUTDOWN_INFO" },
    { 0x1d, "AD_SHUTDOWN_ERROR" },
    { 0x1f, "AD_DISPLACE" },
    { 0x20, "AD_GET_TIMETAB" },
    { 0x21, "AD_SET_TIMETAB" },
    { 0x28, "AD_MSBUF" },
    { 0x29, "AD_ARFC_NOREQ" },
    { 0x2a, "AD_ENQID_INFO" },
    { 0x2b, "AD_DEL_USER" },
    { 0x2c, "AD_SPO_ADM" },
    { 0x2d, "AD_NTAB_SYNC" },
    { 0x2e, "AD_SHARED_PARAMETER" },
    { 0x2f, "AD_RESET_TRACE" },
    { 0x30, "AD_RESET_USR02" },
    { 0x31, "AD_WALL_CREATE" },
    { 0x32, "AD_WALL_DELETE" },
    { 0x33, "AD_WALL_MODIFY" },
    { 0x34, "AD_SERVER_STATE" },
    { 0x3c, "AD_SELFIDENT" },
    { 0x3d, "AD_DP_TRACE_CHANGE" },
    { 0x3e, "AD_DP_DUMP_NIHDL" },
    { 0x3f, "AD_DP_CALL_DELAYED" },
    { 0x40, "AD_GW_ADM" },
    { 0x41, "AD_DP_WAKEUP_MODE" },
    { 0x42, "AD_VMC_SYS_EVENT" },
    { 0x43, "AD_SHARED_PARAM_ALL_WPS" },
    { 0x44, "AD_SECSESSION_UPDATE" },
    { 0x45, "AD_SECSESSION_TERMINATE" },
    { 0x46, "AD_ASRF_REQUEST" },
    { 0x47, "AD_GET_NILIST" },
    { 0x48, "AD_LOAD_INFO" },
    { 0x49, "AD_TEST" },
    { 0x4a, "AD_HANDLE_ACL" },
    { 0x4b, "AD_ENQ_LOG_RESET" },
    {  0, NULL }
};

/* MS Adm AD_RZL_STRG Type values */
static const value_string hf_sapms_adm_rzl_strg_type_vals[] = {
    { 10, "STRG_TYPE_READALL" },
    { 11, "STRG_TYPE_READALL_I" },
    { 12, "STRG_TYPE_READALL_C" },
    { 13, "STRG_TYPE_READALL_LC" },
    { 15, "STRG_TYPE_READALL_OFFSET_I" },
    { 16, "STRG_TYPE_READALL_OFFSET_C" },
    { 17, "STRG_TYPE_READALL_OFFSET_LC" },
    { 20, "STRG_TYPE_READALL_OFFSET" },
    { 21, "STRG_TYPE_READ_I" },
    { 22, "STRG_TYPE_READ_C" },
    { 23, "STRG_TYPE_READ_LC" },
    { 31, "STRG_TYPE_WRITE_I" },
    { 32, "STRG_TYPE_WRITE_C" },
    { 33, "STRG_TYPE_WRITE_LC" },
    { 41, "STRG_TYPE_DEL_I" },
    { 42, "STRG_TYPE_DEL_C" },
    { 43, "STRG_TYPE_DEL_LC" },
    { 51, "STRG_TYPE_CREATE_I" },
    { 52, "STRG_TYPE_CREATE_C" },
    { 53, "STRG_TYPE_CREATE_LC" },
    { 90, "STRG_TYPE_DUMP" },
};

/* MS OP Code values */
static const value_string hf_sapms_opcode_vals[] = {
	{  1, "MS_SERVER_CHG" },
	{  2, "MS_SERVER_ADD" },
	{  3, "MS_SERVER_SUB" },
	{  4, "MS_SERVER_MOD" },
	{  5, "MS_SERVER_LST" },
	{  6, "MS_CHANGE_IP" },
	{  7, "MS_SET_SECURITY_KEY" },
	{  8, "MS_GET_SECURITY_KEY" },
	{  9, "MS_GET_SECURITY_KEY2" },
	{ 10, "MS_GET_HWID" },
	{ 11, "MS_INCRE_TRACE" },
	{ 12, "MS_DECRE_TRACE" },
	{ 13, "MS_RESET_TRACE" },
	{ 14, "MS_ACT_STATISTIC" },
	{ 15, "MS_DEACT_STATISTIC" },
	{ 16, "MS_RESET_STATISTIC" },
	{ 17, "MS_GET_STATISTIC" },
	{ 18, "MS_DUMP_NIBUFFER" },
	{ 19, "MS_RESET_NIBUFFER" },
	{ 20, "MS_OPEN_REQ_LST" },
	{ 21, "MS_SERVER_INFO" },
	{ 22, "MS_SERVER_LIST" },
	{ 23, "MS_SERVER_ENTRY" },
	{ 24, "MS_DOMAIN_INFO" },
	{ 25, "MS_DOMAIN_LIST" },
	{ 26, "MS_DOMAIN_ENTRY" },
	{ 27, "MS_MAP_URL_TO_ADDR" },
	{ 28, "MS_GET_CODEPAGE" },
	{ 29, "MS_SOFT_SHUTDOWN" },
	{ 30, "MS_DUMP_INFO" },
	{ 31, "MS_FILE_RELOAD" },
	{ 32, "MS_RESET_DOMAIN_CONN" },
	{ 33, "MS_NOOP" },
	{ 34, "MS_SET_TXT" },
	{ 35, "MS_GET_TXT" },
	{ 36, "MS_COUNTER_CREATE" },
	{ 37, "MS_COUNTER_DELETE" },
	{ 38, "MS_COUNTER_INCREMENT" },
	{ 39, "MS_COUNTER_DECREMENT" },
	{ 40, "MS_COUNTER_REGISTER" },
	{ 41, "MS_COUNTER_GET" },
	{ 42, "MS_COUNTER_LST" },
	{ 43, "MS_SET_LOGON" },
	{ 44, "MS_GET_LOGON" },
	{ 45, "MS_DEL_LOGON" },
	{ 46, "MS_SERVER_DISC" },
	{ 47, "MS_SERVER_SHUTDOWN" },
	{ 48, "MS_SERVER_SOFT_SHUTDOWN" },
	{ 49, "MS_J2EE_CLUSTERNODE_CHG" },
	{ 50, "MS_J2EE_CLUSTERNODE_ADD" },
	{ 51, "MS_J2EE_CLUSTERNODE_SUB" },
	{ 52, "MS_J2EE_CLUSTERNODE_MOD" },
	{ 53, "MS_J2EE_CLUSTERNODE_LST" },
	{ 54, "MS_J2EE_SERVICE_REG" },
	{ 55, "MS_J2EE_SERVICE_UNREG" },
	{ 56, "MS_J2EE_SERVICE_LST" },
	{ 57, "MS_J2EE_SERVICE_ADD" },
	{ 58, "MS_J2EE_SERVICE_SUB" },
	{ 59, "MS_J2EE_SERVICE_MOD" },
	{ 60, "MS_J2EE_MOD_STATE" },
	{ 61, "MS_J2EE_SERVICE_GET" },
	{ 62, "MS_J2EE_SERVICE_REG2" },
	{ 63, "MS_NITRACE_SETGET" },
	{ 64, "MS_SERVER_LONG_LIST" },
	{ 65, "MS_J2EE_DEBUG_ENABLE" },
	{ 66, "MS_J2EE_DEBUG_DISABLE" },
	{ 67, "MS_SET_PROPERTY" },
	{ 68, "MS_GET_PROPERTY" },
	{ 69, "MS_DEL_PROPERTY" },
	{ 70, "MS_IP_PORT_TO_NAME" },
	{ 71, "MS_CHECK_ACL" },
	{ 72, "MS_LICENSE_SRV" },
	{ 74, "MS_SERVER_TEST_SOFT_SHUTDOWN" },
	{ 75, "MS_J2EE_RECONNECT_P1" },
	{ 76, "MS_J2EE_RECONNECT_P2" },
	{  0, NULL },
};	

/* MS OP Code Error values */
static const value_string hf_sapms_opcode_error_vals[] = {
	{  0, "MSOP_OK" },
	{  1, "MSOP_UNKNOWN_OPCODE" },
	{  2, "MSOP_NOMEM" },
	{  3, "MSOP_SECURITY_KEY_NOTSET" },
	{  4, "MSOP_UNKNOWN_CLIENT" },
	{  5, "MSOP_ACCESS_DENIED" },
	{  6, "MSOP_REQUEST_REQUIRED" },
	{  7, "MSOP_NAME_REQUIRED" },
	{  8, "MSOP_GET_HWID_FAILED" },
	{  9, "MSOP_SEND_FAILED" },
	{ 10, "MSOP_UNKNOWN_DOMAIN" },
	{ 11, "MSOP_UNKNOWN_SERVER" },
	{ 12, "MSOP_NO_DOMAIN_SERVER" },
	{ 13, "MSOP_INVALID_URL" },
	{ 14, "MSOP_UNKNOWN_DUMP_REQ" },
	{ 15, "MSOP_FILENOTFOUND" },
	{ 16, "MSOP_UNKNOWN_RELOAD_REQ" },
	{ 17, "MSOP_FILENOTDEFINED" },
	{ 18, "MSOP_CONVERT_FAILED" },
	{ 19, "MSOP_NOTSET" },
	{ 20, "MSOP_COUNTER_EXCEEDED" },
	{ 21, "MSOP_COUNTER_NOTFOUND" },
	{ 22, "MSOP_COUNTER_DELETED" },
	{ 23, "MSOP_COUNTER_EXISTS" },
	{ 24, "MSOP_EINVAL" },
	{ 25, "MSOP_NO_J2EE_CLUSTERNODE" },
	{ 26, "MSOP_UNKNOWN_PROPERTY" },
	{ 27, "MSOP_UNKNOWN_VERSION" },
	{ 28, "MSOP_ICTERROR" },
	{ 29, "MSOP_KERNEL_INCOMPATIBLE" },
	{ 30, "MSOP_NIACLCREATE_FAILED" },
	{ 31, "MSOP_NIACLSYNTAX_ERROR" },
};

/* MS Set/Get Property ID values */
static const value_string hf_sapms_property_id_vals[] = {
	{  1, "MS_PROPERTY_TEXT" },
	{  2, "MS_PROPERTY_VHOST" },
	{  3, "MS_PROPERTY_IPADR" },
	{  4, "MS_PROPERTY_PARAM" },
	{  5, "MS_PROPERTY_SERVICE" },
	{  6, "MS_PROPERTY_DELALT" },
	{  7, "Release information" },
	{  0, NULL },
};

/* MS Dump Info Dump values */
static const value_string hf_sapms_dump_command_vals[] = {
	{  1, "MS_DUMP_MSADM" },
	{  2, "MS_DUMP_CON" },
	{  3, "MS_DUMP_PARAMS" },
	{  4, "MS_DUMP_ALL_CLIENTS" },
	{  5, "MS_DUMP_ALL_SERVER" },
	{  6, "MS_DUMP_ALL_DOMAIN" },
	{  7, "MS_DUMP_DOMAIN_CONN" },
	{  8, "MS_DUMP_RELEASE" },
	{  9, "MS_DUMP_SIZEOF" },
	{ 10, "MS_DUMP_FIADM" },
	{ 11, "MS_DUMP_FICON" },
	{ 12, "MS_DUMP_COUNTER" },
	{ 13, "MS_DUMP_STATISTIC" },
	{ 14, "MS_DUMP_NIBUF" },
	{ 15, "MS_DUMP_URLMAP" },
	{ 16, "MS_DUMP_URLPREFIX" },
	{ 17, "MS_DUMP_URLHANDLER" },
	{ 18, "MS_DUMP_NOSERVER" },
	{ 19, "MS_DUMP_ACLINFO" },
	{ 20, "MS_DUMP_PERMISSION_TABLE" },
	{ 21, "MS_DUMP_J2EE_CLUSTER_STAT" },
	{ 22, "MS_DUMP_ACL_FILE_EXT" },
	{ 23, "MS_DUMP_ACL_FILE_INT" },
	{ 24, "MS_DUMP_ACL_FILE_ADMIN" },
	{ 25, "MS_DUMP_ACL_FILE_EXTBND" },
	{ 26, "MS_DUMP_ACL_FILE_HTTP" },
	{ 27, "MS_DUMP_ACL_FILE_HTTPS" },
	{  0, NULL },
};

/* MS Reload file values */
static const value_string hf_sapms_file_reload_vals[] = {
    { 1, "MS_RELOAD_CLIENT_TAB" },
    { 2, "MS_RELOAD_SERVER_TAB" },
    { 3, "MS_RELOAD_DOMAIN_TAB" },
    { 4, "MS_RELOAD_URLMAP" },
    { 5, "MS_RELOAD_URLPREFIX" },
    { 6, "MS_RELOAD_ACL_INFO" },
    { 7, "MS_RELOAD_PERMISSION_TABLE" },
    { 8, "MS_RELOAD_STOC" },
    { 9, "MS_RELOAD_ACL_FILE_EXT" },
    { 10, "MS_RELOAD_ACL_FILE_INT" },
    { 11, "MS_RELOAD_ACL_FILE_ADMIN" },
    { 12, "MS_RELOAD_ACL_FILE_EXTBND" },
    { 13, "MS_RELOAD_ACL_FILE_HTTP" },
    { 14, "MS_RELOAD_ACL_FILE_HTTPS" },
    { 0, NULL }
};


/* MS Logon Type values */
static const value_string hf_sapms_logon_type_vals[] = {
    { 0, "MS_LOGON_DIAG_LB" },
    { 1, "MS_LOGON_DIAG_LBS" },
    { 2, "MS_LOGON_DIAG" },
    { 3, "MS_LOGON_DIAGS" },
    { 4, "MS_LOGON_RFC" },
    { 5, "MS_LOGON_RFCS" },
    { 6, "MS_LOGON_HTTP" },
    { 7, "MS_LOGON_HTTPS" },
    { 8, "MS_LOGON_FTP" },
    { 9, "MS_LOGON_SMTP" },
    { 10, "MS_LOGON_NNTP" },
    { 11, "MS_LOGON_DIAG_E" },
    { 12, "MS_LOGON_DIAGS_E" },
    { 13, "MS_LOGON_RFC_E" },
    { 14, "MS_LOGON_RFCS_E" },
    { 15, "MS_LOGON_HTTP_E" },
    { 16, "MS_LOGON_HTTPS_E" },
    { 17, "MS_LOGON_FTP_E" },
    { 18, "MS_LOGON_SMTP_E" },
    { 19, "MS_LOGON_NNTP_E" },
    { 20, "MS_LOGON_J2EE" },
    { 21, "MS_LOGON_J2EES" },
    { 22, "MS_LOGON_J2EE_E" },
    { 23, "MS_LOGON_J2EES_E" },
    { 24, "MS_LOGON_P4" },
    { 25, "MS_LOGON_P4S" },
    { 26, "MS_LOGON_IIOP" },
    { 27, "MS_LOGON_IIOPS" },
    { 28, "MS_LOGON_SDM" },
    { 29, "MS_LOGON_TELNET" },
    { 30, "MS_LOGON_DEBUG" },
    { 31, "MS_LOGON_DPROXY" },
    { 32, "MS_LOGON_P4HTTP" },
    { 33, "MS_LOGON_HTTPRI" },
    { 34, "MS_LOGON_HTTPSRI" },
    { 35, "MS_LOGON_J2EERI" },
    { 36, "MS_LOGON_J2EESRI" },
    { 37, "MS_LOGON_TRXNS" },
};


/* MS Client Status values */
static const value_string hf_sapms_server_lst_status_vals[] = {
    { 0, "MS_STATE_UNKNOWN" },
    { 1, "ACTIVE" },
    { 2, "INACTIVE" },
    { 3, "MS_STATE_SHUTDOWN" },
    { 4, "MS_STATE_STOP" },
    { 5, "MS_STATE_STARTING" },
    { 6, "MS_STATE_INIT" },
};


/* Message Type values */
#define SAPMS_MSG_TYPE_DIA   0x01
#define SAPMS_MSG_TYPE_UPD   0x02
#define SAPMS_MSG_TYPE_ENQ   0x04
#define SAPMS_MSG_TYPE_BTC   0x08
#define SAPMS_MSG_TYPE_SPO   0x10
#define SAPMS_MSG_TYPE_UP2   0x20
#define SAPMS_MSG_TYPE_ATP   0x40
#define SAPMS_MSG_TYPE_ICM   0x80

static int proto_sapms = -1;

static int hf_sapms_eyecatcher = -1;
static int hf_sapms_version = -1;
static int hf_sapms_errorno = -1;
static int hf_sapms_toname = -1;
static int hf_sapms_msgtypes = -1;
static int hf_sapms_msgtypes_dia = -1;
static int hf_sapms_msgtypes_upd = -1;
static int hf_sapms_msgtypes_enq = -1;
static int hf_sapms_msgtypes_btc = -1;
static int hf_sapms_msgtypes_spo = -1;
static int hf_sapms_msgtypes_up2 = -1;
static int hf_sapms_msgtypes_atp = -1;
static int hf_sapms_msgtypes_icm = -1;
static int hf_sapms_reserved = -1;
static int hf_sapms_key = -1;
static int hf_sapms_flag = -1;
static int hf_sapms_iflag = -1;
static int hf_sapms_fromname = -1;
static int hf_sapms_fromhost = -1;
static int hf_sapms_fromserv = -1;
static int hf_sapms_message = -1;

static int hf_sapms_adm_eyecatcher = -1;
static int hf_sapms_adm_version = -1;
static int hf_sapms_adm_msgtype = -1;
static int hf_sapms_adm_recsize = -1;
static int hf_sapms_adm_recno = -1;
static int hf_sapms_adm_records = -1;

static int hf_sapms_adm_record = -1;
static int hf_sapms_adm_record_opcode = -1;
static int hf_sapms_adm_record_serial_number = -1;
static int hf_sapms_adm_record_executed = -1;
static int hf_sapms_adm_record_errorno = -1;
static int hf_sapms_adm_record_value = -1;

static int hf_sapms_adm_parameter = -1;

static int hf_sapms_adm_rzl_strg_type = -1;
static int hf_sapms_adm_rzl_strg_name = -1;
static int hf_sapms_adm_rzl_strg_value = -1;
static int hf_sapms_adm_rzl_strg_value_integer = -1;

static int hf_sapms_opcode = -1;
static int hf_sapms_opcode_error = -1;
static int hf_sapms_opcode_version = -1;
static int hf_sapms_opcode_charset = -1;
static int hf_sapms_opcode_value = -1;

static int hf_sapms_property_client = -1;
static int hf_sapms_property_id = -1;
static int hf_sapms_property_length = -1;
static int hf_sapms_property_value = -1;

static int hf_sapms_property_ip_address = -1;
static int hf_sapms_property_ip_address6 = -1;

static int hf_sapms_property_service = -1;

static int hf_sapms_property_release = -1;
static int hf_sapms_property_release_patchno = -1;
static int hf_sapms_property_release_supplvl = -1;
static int hf_sapms_property_release_platform = -1;


static int hf_sapms_text_name = -1;
static int hf_sapms_text_length = -1;
static int hf_sapms_text_value = -1;

static int hf_sapms_counter_uuid = -1;
static int hf_sapms_counter_count = -1;
static int hf_sapms_counter_no = -1;

static int hf_sapms_change_ip_address = -1;
static int hf_sapms_change_ip_address6 = -1;

static int hf_sapms_security_name = -1;
static int hf_sapms_security_key = -1;
static int hf_sapms_security_port = -1;
static int hf_sapms_security_address = -1;
static int hf_sapms_security_address6 = -1;

static int hf_sapms_file_reload = -1;
static int hf_sapms_file_filler = -1;

static int hf_sapms_logon_type = -1;
static int hf_sapms_logon_port = -1;
static int hf_sapms_logon_address = -1;
static int hf_sapms_logon_name_length = -1;
static int hf_sapms_logon_name = -1;
static int hf_sapms_logon_prot_length = -1;
static int hf_sapms_logon_prot = -1;
static int hf_sapms_logon_host_length = -1;
static int hf_sapms_logon_host = -1;
static int hf_sapms_logon_misc_length = -1;
static int hf_sapms_logon_misc = -1;
static int hf_sapms_logon_address6_length = -1;
static int hf_sapms_logon_address6 = -1;

static int hf_sapms_shutdown_reason_length = -1;
static int hf_sapms_shutdown_reason = -1;

static int hf_sapms_ip_to_name_address4 = -1;
static int hf_sapms_ip_to_name_address6 = -1;
static int hf_sapms_ip_to_name_port = -1;
static int hf_sapms_ip_to_name_length = -1;
static int hf_sapms_ip_to_name = -1;

static int hf_sapms_check_acl_error_code = -1;
static int hf_sapms_check_acl_acl = -1;

static int hf_sapms_codepage = -1;

static int hf_sapms_dump_dest = -1;
static int hf_sapms_dump_filler = -1;
static int hf_sapms_dump_index = -1;
static int hf_sapms_dump_command = -1;
static int hf_sapms_dump_name = -1;

static int hf_sapms_server_lst_client = -1;
static int hf_sapms_server_lst_name = -1;
static int hf_sapms_server_lst_host = -1;
static int hf_sapms_server_lst_service = -1;
static int hf_sapms_server_lst_msgtypes = -1;
static int hf_sapms_server_lst_msgtypes_dia = -1;
static int hf_sapms_server_lst_msgtypes_upd = -1;
static int hf_sapms_server_lst_msgtypes_enq = -1;
static int hf_sapms_server_lst_msgtypes_btc = -1;
static int hf_sapms_server_lst_msgtypes_spo = -1;
static int hf_sapms_server_lst_msgtypes_up2 = -1;
static int hf_sapms_server_lst_msgtypes_atp = -1;
static int hf_sapms_server_lst_msgtypes_icm = -1;
static int hf_sapms_server_lst_hostaddr = -1;
static int hf_sapms_server_lst_hostaddrv4 = -1;
static int hf_sapms_server_lst_servno = -1;
static int hf_sapms_server_lst_status = -1;
static int hf_sapms_server_lst_nitrc = -1;
static int hf_sapms_server_lst_sys_service = -1;

static gint ett_sapms = -1;

/* Global port preference */
static range_t *global_sapms_port_range;

/* Global highlight preference */
static gboolean global_sapms_highlight_items = TRUE;

/* Protocol handle */
static dissector_handle_t sapms_handle;

void proto_reg_handoff_sapms(void);


static void
dissect_sapms_adm_record(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, guint32 offset, guint32 length){

    proto_item *record = NULL, *value = NULL;
    proto_tree *record_tree = NULL, *value_tree = NULL;
    guint8 adm_opcode;

    while (length>=104){
        
        record = proto_tree_add_item(tree, hf_sapms_adm_record, tvb, offset, 104, FALSE);
        record_tree = proto_item_add_subtree(record, ett_sapms);

        adm_opcode = tvb_get_guint8(tvb, offset);
        proto_tree_add_item(record_tree, hf_sapms_adm_record_opcode, tvb, offset, 1, FALSE);
        proto_item_append_text(record_tree, ", Adm Opcode=%s", val_to_str(adm_opcode, hf_sapms_adm_record_opcode_vals, "Unknown")); offset+=1; length-=1;

        proto_tree_add_item(record_tree, hf_sapms_adm_record_serial_number, tvb, offset, 1, FALSE); offset+=1; length-=1;
        proto_tree_add_item(record_tree, hf_sapms_adm_record_executed, tvb, offset, 1, FALSE); offset+=1; length-=1;
        proto_tree_add_item(record_tree, hf_sapms_adm_record_errorno, tvb, offset, 1, FALSE); offset+=1; length-=1;

        /* Dissect the value in function of the opcode */
        switch (adm_opcode){
        	case 0x01: 		/* AD_PROFILE */
        	case 0x2e:{		/* AD_SHARED_PARAMETER */
        		proto_tree_add_item(record_tree, hf_sapms_adm_parameter, tvb, offset, 100, FALSE); offset+=100; length-=100;
        		break;
        	}
            case 0x15:{ 	/* AD_RZL_STRG */
            	guint8 strg_type = 0;
            	strg_type = tvb_get_guint8(tvb, offset);
                proto_tree_add_item(record_tree, hf_sapms_adm_rzl_strg_type, tvb, offset, 1, FALSE); offset+=1; length-=1;
                offset+=3; length-=3; /* Skip 3 bytes */
                proto_tree_add_item(record_tree, hf_sapms_adm_rzl_strg_name, tvb, offset, 20, FALSE); offset+=20; length-=20;

        		value = proto_tree_add_item(record_tree, hf_sapms_adm_rzl_strg_value, tvb, offset, 40, FALSE);
                value_tree = proto_item_add_subtree(value, ett_sapms);
                switch (strg_type){
                	case 11:		/* STRG_TYPE_READALL_I */
                	case 15:		/* STRG_TYPE_READALL_OFFSET_I */
                	case 21:		/* STRG_TYPE_READ_I */
                	case 31:		/* STRG_TYPE_WRITE_I */
                	case 41:		/* STRG_TYPE_DEL_I */
                	case 51:{		/* STRG_TYPE_CREATE_I */
                		proto_tree_add_item(value_tree, hf_sapms_adm_rzl_strg_value_integer, tvb, offset, 4, FALSE); offset+=4; length-=4;
                		proto_tree_add_item(value_tree, hf_sapms_adm_rzl_strg_value_integer, tvb, offset, 4, FALSE); offset+=4; length-=4;
                		proto_tree_add_item(value_tree, hf_sapms_adm_rzl_strg_value_integer, tvb, offset, 4, FALSE); offset+=4; length-=4;
                		proto_tree_add_item(value_tree, hf_sapms_adm_rzl_strg_value_integer, tvb, offset, 4, FALSE); offset+=4; length-=4;
                		proto_tree_add_item(value_tree, hf_sapms_adm_rzl_strg_value_integer, tvb, offset, 4, FALSE); offset+=4; length-=4;
                		proto_tree_add_item(value_tree, hf_sapms_adm_rzl_strg_value_integer, tvb, offset, 4, FALSE); offset+=4; length-=4;
                		proto_tree_add_item(value_tree, hf_sapms_adm_rzl_strg_value_integer, tvb, offset, 4, FALSE); offset+=4; length-=4;
                		proto_tree_add_item(value_tree, hf_sapms_adm_rzl_strg_value_integer, tvb, offset, 4, FALSE); offset+=4; length-=4;
                		proto_tree_add_item(value_tree, hf_sapms_adm_rzl_strg_value_integer, tvb, offset, 4, FALSE); offset+=4; length-=4;
                		proto_tree_add_item(value_tree, hf_sapms_adm_rzl_strg_value_integer, tvb, offset, 4, FALSE); offset+=4; length-=4;
                		break;
                	}
                	default:{
                		offset+=40; length-=40;
                		break;
                	}
                }

                offset+=36; length-=36; /* Skip the last 36 bytes */
                break;
            }
            default:{
                proto_tree_add_item(record_tree, hf_sapms_adm_record_value, tvb, offset, 100, FALSE); offset+=100; length-=100;
                if (global_sapms_highlight_items){
                    expert_add_info_format(pinfo, record_tree, PI_UNDECODED, PI_WARN, "The ADM opcode is dissected partially (0x%.2x)", adm_opcode);
                }
                break;
            }
        }

    }

}

static gint
dissect_sapms_client(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, guint32 offset, guint8 opcode_version){
    proto_item *client = NULL, *msg_types = NULL;
    proto_tree *client_tree = NULL, *msg_types_tree = NULL;
    struct e_in6_addr address_ipv6;
    guint32 address_ipv4 = 0;
    gint client_length = 0;

    /* Chose the client length according to the version number */
    if (opcode_version == 0x01){  			/* This version was seen in the older releases (6.40) */
    	client_length = 67;
    } else if (opcode_version == 0x02){ 	/* This version was seen in the older releases (6.40) */
    	client_length = 115;
    } else if (opcode_version == 0x03){
    	client_length = 150;
    } else if (opcode_version == 0x04){ 	/* Version 4 was seen in releases >7.0 */
    	client_length = 160;
    } else {								/* Default to version 4 */
    	client_length = 160;
        if (global_sapms_highlight_items){
            expert_add_info_format(pinfo, tree, PI_UNDECODED, PI_WARN, "This version has not been seen, dissection of this packet could be wrong for this version (0x%.2x)", opcode_version);
        }
	}

    if (tvb_length_remaining(tvb, offset) < client_length){
        expert_add_info_format(pinfo, tree, PI_MALFORMED, PI_WARN, "Invalid client length (expected=%d, actual=%d)", client_length, tvb_length_remaining(tvb, offset));
    	return (client_length);
    }

    /* Add the client tree */
    client = proto_tree_add_item(tree, hf_sapms_server_lst_client, tvb, offset, client_length, FALSE);
	client_tree = proto_item_add_subtree(client, ett_sapms);

	/* Client name field */
	if (opcode_version==0x01){
		proto_tree_add_item(client_tree, hf_sapms_server_lst_name, tvb, offset, 20, FALSE); offset+=20;
	} else {
		proto_tree_add_item(client_tree, hf_sapms_server_lst_name, tvb, offset, 40, FALSE); offset+=40;
	}

	/* Host field (version 1 is 20 bytes, 2 is 32 bytes and version 3/4 is 64 bytes) */
	if (opcode_version==0x01){
		proto_tree_add_item(client_tree, hf_sapms_server_lst_host, tvb, offset, 20, FALSE); offset+=20;
	} else if (opcode_version==0x02){
		proto_tree_add_item(client_tree, hf_sapms_server_lst_host, tvb, offset, 32, FALSE); offset+=32;
	} else {
		proto_tree_add_item(client_tree, hf_sapms_server_lst_host, tvb, offset, 64, FALSE); offset+=64;
	}
	/* Service field */
	proto_tree_add_item(client_tree, hf_sapms_server_lst_service, tvb, offset, 20, FALSE); offset+=20;

	/* Message type flags */
	msg_types = proto_tree_add_item(client_tree, hf_sapms_server_lst_msgtypes, tvb, offset, 1, FALSE);
	msg_types_tree = proto_item_add_subtree(msg_types, ett_sapms);
	proto_tree_add_item(msg_types_tree, hf_sapms_server_lst_msgtypes_dia, tvb, offset, 1, FALSE);
	proto_tree_add_item(msg_types_tree, hf_sapms_server_lst_msgtypes_upd, tvb, offset, 1, FALSE);
	proto_tree_add_item(msg_types_tree, hf_sapms_server_lst_msgtypes_enq, tvb, offset, 1, FALSE);
	proto_tree_add_item(msg_types_tree, hf_sapms_server_lst_msgtypes_btc, tvb, offset, 1, FALSE);
	proto_tree_add_item(msg_types_tree, hf_sapms_server_lst_msgtypes_spo, tvb, offset, 1, FALSE);
	proto_tree_add_item(msg_types_tree, hf_sapms_server_lst_msgtypes_up2, tvb, offset, 1, FALSE);
	proto_tree_add_item(msg_types_tree, hf_sapms_server_lst_msgtypes_atp, tvb, offset, 1, FALSE);
	proto_tree_add_item(msg_types_tree, hf_sapms_server_lst_msgtypes_icm, tvb, offset, 1, FALSE); offset+=1;

	/* Add the IPv6 address (only for version 3/4) */
	if (opcode_version >= 0x03){
		tvb_get_ipv6(tvb, offset, &address_ipv6);
		proto_tree_add_ipv6(client_tree, hf_sapms_server_lst_hostaddr, tvb, offset, 16, (guint8 *)&address_ipv6); offset+=16;
	}

	/* Add the IPv4 address */
	address_ipv4 = tvb_get_ipv4(tvb, offset);
	proto_tree_add_ipv4(client_tree, hf_sapms_server_lst_hostaddrv4, tvb, offset, 4, address_ipv4); offset+=4;

	/* Service Number field */
	proto_tree_add_item(client_tree, hf_sapms_server_lst_servno, tvb, offset, 2, FALSE); offset+=2;

	/* Other fields only on version 2/3/4 */
	if (opcode_version >= 0x02){
		proto_tree_add_item(client_tree, hf_sapms_server_lst_status, tvb, offset, 1, FALSE); offset+=1;
		proto_tree_add_item(client_tree, hf_sapms_server_lst_nitrc, tvb, offset, 1, FALSE); offset+=1;
	}

	/* Add the Sys Server field (only for v4) */
	if (opcode_version == 0x04){
		proto_tree_add_item(client_tree, hf_sapms_server_lst_sys_service, tvb, offset, 4, FALSE); offset+=4;
	}

	/* Return the client length according to the version */
	return (client_length);
}


static void
dissect_sapms_counter(tvbuff_t *tvb, proto_tree *tree, guint32 offset){
	proto_tree_add_item(tree, hf_sapms_counter_uuid, tvb, offset, 40, FALSE); offset+=40;
	proto_tree_add_item(tree, hf_sapms_counter_count, tvb, offset, 4, FALSE); offset+=4;
	proto_tree_add_item(tree, hf_sapms_counter_no, tvb, offset, 4, FALSE); offset+=4;
}

static void
dissect_sapms_property(tvbuff_t *tvb, proto_tree *tree, guint32 offset){
	guint32 property_id = 0;
    proto_item *value= NULL;
    proto_tree *value_tree = NULL;

	proto_tree_add_item(tree, hf_sapms_property_client, tvb, offset, 40, FALSE); offset+=40;

	property_id = tvb_get_ntohl(tvb, offset);
	proto_tree_add_item(tree, hf_sapms_property_id, tvb, offset, 4, FALSE); offset+=4;

	/* Check if the property item has a value */
	if (!tvb_offset_exists(tvb, offset)){
		return;
	}

	value = proto_tree_add_item(tree, hf_sapms_property_value, tvb, offset, -1, FALSE);
	value_tree = proto_item_add_subtree(value, ett_sapms);

	switch (property_id){

		case 0x03:{			/* MS_PROPERTY_IPADR */
			proto_tree_add_item(value_tree, hf_sapms_property_ip_address, tvb, offset, 4, FALSE); offset+=4;
			proto_tree_add_item(value_tree, hf_sapms_property_ip_address6, tvb, offset, 16, FALSE); offset+=16;
			break;
		}
		case 0x05:{			/* MS_PROPERTY_SERVICE */
			proto_tree_add_item(value_tree, hf_sapms_property_service, tvb, offset, 2, FALSE); offset+=2;
			break;
		}
		case 0x07:{			/* Release Information */
			proto_tree_add_item(value_tree, hf_sapms_property_release, tvb, offset, 10, FALSE); offset+=10;
			proto_tree_add_item(value_tree, hf_sapms_property_release_patchno, tvb, offset, 4, FALSE); offset+=4;
			proto_tree_add_item(value_tree, hf_sapms_property_release_supplvl, tvb, offset, 4, FALSE); offset+=4;
			proto_tree_add_item(value_tree, hf_sapms_property_release_platform, tvb, offset, 4, FALSE); offset+=4;
			break;
		}
	}
}

static void
dissect_sapms_opcode(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, guint32 offset, guint8 flag, guint8 opcode, guint8 opcode_version, guint32 length){
    gint client_length = 0;

    switch (opcode){
		case 0x02:			/* MS_SERVER_ADD */
		case 0x03:			/* MS_SERVER_SUB */
		case 0x04:{			/* MS_SERVER_MOD */
			client_length = dissect_sapms_client(tvb, pinfo, tree, offset, opcode_version);
			break;
		}
        case 0x05:{         /* MS_SERVER_LST */
            if (flag == 0x03){ /* If it's a reply (flag=MS_REPLY) */
				while (tvb_length_remaining(tvb, offset) > 0){
					client_length = dissect_sapms_client(tvb, pinfo, tree, offset, opcode_version); offset += client_length;
				}
            }            
            break;
        }
        case 0x06:{			 /* MS_CHANGE_IP */
            proto_tree_add_item(tree, hf_sapms_change_ip_address, tvb, offset, 4, FALSE); offset+=4; length-=4;

            if (opcode_version >= 0x02){
            	proto_tree_add_item(tree, hf_sapms_change_ip_address6, tvb, offset, 16, FALSE); offset+=16; length-=16;
            }
        	break;
        }
        case 0x07:          /* MS_SET_SECURITY_KEY */
        case 0x08:{         /* MS_GET_SECURITY_KEY */
            proto_tree_add_item(tree, hf_sapms_security_name, tvb, offset, 40, FALSE); offset+=40; length-=40;
            proto_tree_add_item(tree, hf_sapms_security_key, tvb, offset, 256, FALSE); offset+=256; length-=256;
            break;
        }
        case 0x09:{         /* MS_GET_SECURITY_KEY2 */
            guint32 address_ipv4;
            struct e_in6_addr address_ipv6;

            address_ipv4 = tvb_get_ipv4(tvb, offset);
            proto_tree_add_ipv4(tree, hf_sapms_security_address, tvb, offset, 4, address_ipv4); offset+=4; length-=4;
            proto_tree_add_item(tree, hf_sapms_security_port, tvb, offset, 2, FALSE); offset+=2; length-=2;
            proto_tree_add_item(tree, hf_sapms_security_key, tvb, offset, 256, FALSE); offset+=256; length-=256;
            tvb_get_ipv6(tvb, offset, &address_ipv6);
            proto_tree_add_ipv6(tree, hf_sapms_security_address6, tvb, offset, 16, (guint8 *)&address_ipv6); offset+=16; length-=16;
            break;
        }
        case 0x0a:{         /* MS_GET_HWID */
	        proto_tree_add_none_format(tree, hf_sapms_opcode_value, tvb, offset, length, "Hardware ID: %s", tvb_get_ephemeral_string(tvb, offset, length));
            break;
        }
        case 0x11:{			/* MS_GET_STATISTIC */
        	// XXX: Fill fields for statistics
        	break;
        }
        case 0x1C:{			/* MS_GET_CODEPAGE */
            if (flag == 0x03)
            	proto_tree_add_item(tree, hf_sapms_codepage, tvb, offset, 4, FALSE); offset+=4; length-=4;
        	break;
        }
        case 0x1E:{         /* MS_DUMP_INFO */

        	if (flag == 0x02) { /* If it's a request (flag=MS_REQUEST) */
                proto_tree_add_item(tree, hf_sapms_dump_dest, tvb, offset, 1, FALSE); offset+=1;
                proto_tree_add_item(tree, hf_sapms_dump_filler, tvb, offset, 3, FALSE); offset+=3;
                proto_tree_add_item(tree, hf_sapms_dump_index, tvb, offset, 2, FALSE); offset+=2;
                proto_tree_add_item(tree, hf_sapms_dump_command, tvb, offset, 2, FALSE); offset+=2;
                if (length>=48)
                    proto_tree_add_item(tree, hf_sapms_dump_name, tvb, offset, 40, FALSE); offset+=40;

            } else if (flag == 0x03) { /* If it's a reply (flag=MS_REPLY) */
            	guint32 string_length = 0;
            	length = tvb_strsize(tvb, offset);
            	/* Add each string in a different item */
            	while (length>1){
            		string_length = tvb_find_line_end(tvb, offset, -1, NULL, FALSE);
            		if (string_length>0){
            			proto_tree_add_none_format(tree, hf_sapms_opcode_value, tvb, offset, string_length, "%s", tvb_get_ephemeral_string(tvb, offset, string_length));
            			offset+=string_length; length-=string_length;
            		}
            		offset+=1; length-=1;
            	}
            }
            break;
        }
        case 0x1f:{			/* MS_FILE_RELOAD */
            proto_tree_add_item(tree, hf_sapms_file_reload, tvb, offset, 1, FALSE); offset+=1;
            proto_tree_add_item(tree, hf_sapms_file_filler, tvb, offset, 3, FALSE); offset+=3;
            break;
        }
        case 0x22:          /* MS_SET_TXT */
        case 0x23:{         /* MS_GET_TXT */
        	guint32 text_length = 0;

            proto_tree_add_item(tree, hf_sapms_text_name, tvb, offset, 40, FALSE); offset+=40; length-=40;
            text_length = tvb_get_ntohl(tvb, offset);
            proto_tree_add_item(tree, hf_sapms_text_length, tvb, offset, 4, FALSE); offset+=4; length-=4;
            /* Check length */
            if (text_length != length ){
            	expert_add_info_format(pinfo, tree, PI_MALFORMED, PI_WARN, "Invalid length (expected=%d, actual=%d)", text_length, length);
            }
            proto_tree_add_item(tree, hf_sapms_text_value, tvb, offset, length, FALSE); offset+=length;        
            break;
        }
        case 0x24:			/* MS_COUNTER_CREATE */
        case 0x25:			/* MS_COUNTER_DELETE */
        case 0x26:			/* MS_COUNTER_INCREMENT */
        case 0x27:			/* MS_COUNTER_DECREMENT */
        case 0x28:			/* MS_COUNTER_REGISTER */
        case 0x29:{			/* MS_COUNTER_GET */
        	if (tvb_length_remaining(tvb, offset) >= 48){
        		dissect_sapms_counter(tvb, tree, offset); offset+=48;
        	}
        	break;
        }
        case 0x2a:{			/* MS_COUNTER_LST */
        	while (tvb_length_remaining(tvb, offset) >= 48){
        		dissect_sapms_counter(tvb, tree, offset); offset+=48;
        	}
        	break;
        }
        case 0x2b:          /* MS_SET_LOGON */
        case 0x2c:          /* MS_GET_LOGON */
        case 0x2d:{         /* MS_DEL_LOGON */
            guint16 name_length = 0, prot_length = 0, host_length = 0, misc_length = 0, address6_length = 0;
            guint32 address_ipv4;
            struct e_in6_addr address_ipv6;

            proto_tree_add_item(tree, hf_sapms_logon_type, tvb, offset, 2, FALSE); offset+=2; length-=2;
            proto_tree_add_item(tree, hf_sapms_logon_port, tvb, offset, 2, FALSE); offset+=2; length-=2;

            address_ipv4 = tvb_get_ipv4(tvb, offset);
            proto_tree_add_ipv4(tree, hf_sapms_logon_address, tvb, offset, 4, address_ipv4); offset+=4; length-=4;

            name_length = tvb_get_ntohs(tvb, offset);
            proto_tree_add_item(tree, hf_sapms_logon_name_length, tvb, offset, 2, FALSE); offset+=2; length-=2;
            if (name_length > 0 && length >= name_length){
                proto_tree_add_item(tree, hf_sapms_logon_name, tvb, offset, name_length, FALSE); offset+=name_length; length-=name_length;
            }

            prot_length = tvb_get_ntohs(tvb, offset);
            proto_tree_add_item(tree, hf_sapms_logon_prot_length, tvb, offset, 2, FALSE); offset+=2; length-=2;
            if (prot_length > 0 && length >= prot_length){
                proto_tree_add_item(tree, hf_sapms_logon_prot, tvb, offset, prot_length, FALSE); offset+=prot_length; length-=prot_length;
            }

            host_length = tvb_get_ntohs(tvb, offset);
            proto_tree_add_item(tree, hf_sapms_logon_host_length, tvb, offset, 2, FALSE); offset+=2; length-=2;
            if (host_length > 0 && length >= host_length){
                proto_tree_add_item(tree, hf_sapms_logon_host, tvb, offset, host_length, FALSE); offset+=host_length; length-=host_length;
            }

            misc_length = tvb_get_ntohs(tvb, offset);
            proto_tree_add_item(tree, hf_sapms_logon_misc_length, tvb, offset, 2, FALSE); offset+=2; length-=2;
            if (misc_length > 0 && length >= misc_length){
                proto_tree_add_item(tree, hf_sapms_logon_misc, tvb, offset, misc_length, FALSE); offset+=misc_length; length-=misc_length;
            }

            address6_length = tvb_get_ntohs(tvb, offset);
            proto_tree_add_item(tree, hf_sapms_logon_address6_length, tvb, offset, 2, FALSE); offset+=2; length-=2;
            if (address6_length==16 && length >= address6_length){
                tvb_get_ipv6(tvb, offset, &address_ipv6);
                proto_tree_add_ipv6(tree, hf_sapms_logon_address6, tvb, offset, 16, (guint8 *)&address_ipv6); offset+=16; length-=16;
            } else { /* Add expert info if wrong IPv6 address length */
            	expert_add_info_format(pinfo, tree, PI_MALFORMED, PI_WARN, "Invalid length (%d) or data", address6_length);
            }

            break;
        }
        case 0x2e:			/* MS_SERVER_DISC */
        case 0x2f:			/* MS_SERVER_SHUTDOWN */
        case 0x30:			/* MS_SERVER_SOFT_SHUTDOWN */
        case 0x4a:{			/* MS_SERVER_TEST_SOFT_SHUTDOWN */
        	guint16 reason_length = 0;

			client_length = dissect_sapms_client(tvb, pinfo, tree, offset, opcode_version); offset += client_length; length -= client_length;
            reason_length = tvb_get_ntohs(tvb, offset);
            proto_tree_add_item(tree, hf_sapms_shutdown_reason_length, tvb, offset, 2, FALSE); offset+=2; length-=2;

            if (reason_length > 0 && length > 0){
            	reason_length = length < reason_length? length : reason_length;
                proto_tree_add_item(tree, hf_sapms_shutdown_reason, tvb, offset, reason_length, FALSE); offset+=reason_length; length-=reason_length;
            }
        	break;
        }
        case 0x43:          /* MS_SET_PROPERTY */
        case 0x44:          /* MS_GET_PROPERTY */
        case 0x45:{         /* MS_DEL_PROPERTY */
       		dissect_sapms_property(tvb, tree, offset);
            break;
        }
        case 0x46:{			/* MS_IP_PORT_TO_NAME */
            guint32 address_ipv4 = 0, name_length = 0;
            struct e_in6_addr address_ipv6;

            if (opcode_version == 0x01){
                address_ipv4 = tvb_get_ipv4(tvb, offset);
                proto_tree_add_ipv4(tree, hf_sapms_ip_to_name_address4, tvb, offset, 4, address_ipv4); offset+=4; length-=4;
        	} else if (opcode_version == 0x02){
                tvb_get_ipv6(tvb, offset, &address_ipv6);
                proto_tree_add_ipv6(tree, hf_sapms_ip_to_name_address6, tvb, offset, 16, (guint8 *)&address_ipv6); offset+=16; length-=16;
        	}

        	proto_tree_add_item(tree, hf_sapms_ip_to_name_port, tvb, offset, 2, FALSE); offset+=2; length-=2;

            name_length = tvb_get_ntohl(tvb, offset);
            proto_tree_add_item(tree, hf_sapms_ip_to_name_length, tvb, offset, 4, FALSE); offset+=4; length-=4;
            if (name_length > 0 && length >= name_length){
                proto_tree_add_item(tree, hf_sapms_ip_to_name, tvb, offset, name_length, FALSE); offset+=name_length; length-=name_length;
            }

        	break;
        }
        case 0x47:{			/* MS_CHECK_ACL */
        	guint32 string_length = 0;
        	proto_tree_add_item(tree, hf_sapms_check_acl_error_code, tvb, offset, 2, FALSE); offset+=2; length-=2;
			string_length = tvb_strnlen(tvb, offset, length) + 1;
			proto_tree_add_item(tree, hf_sapms_check_acl_acl, tvb, offset, string_length, FALSE); offset+=string_length; length-=string_length;
			string_length = tvb_strnlen(tvb, offset, length) + 1;
			proto_tree_add_item(tree, hf_sapms_check_acl_acl, tvb, offset, string_length, FALSE); offset+=string_length; length-=string_length;
        	break;
        }
        default:{
            if (global_sapms_highlight_items){
                expert_add_info_format(pinfo, tree, PI_UNDECODED, PI_WARN, "The opcode is dissected partially (0x%.2x)", opcode);
            }
            break;
        }

    }

}


static void
dissect_sapms(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
    guint8 flag=0, iflag = 0, opcode = 0, opcode_version = 0;
    guint32 offset = 0, remaining_length = 0;

	/* Add the protocol to the column */
	col_add_str(pinfo->cinfo, COL_PROTOCOL, "SAPMS");
	/* Clear out stuff in the info column */
	col_clear(pinfo->cinfo,COL_INFO);

	if (tree) { /* we are being asked for details */

		proto_item *ti = NULL, *oi = NULL, *msg_types = NULL;
		proto_tree *sapms_tree = NULL, *sapms_opcode_tree = NULL, *msg_types_tree = NULL;

		/* Add the main sapms subtree */
		ti = proto_tree_add_item(tree, proto_sapms, tvb, 0, -1, FALSE);
		sapms_tree = proto_item_add_subtree(ti, ett_sapms);

        /* Check for the eye catcher string */
        if (tvb_strneql(tvb, offset, "**MESSAGE**\00", 12) == 0){
            proto_tree_add_item(sapms_tree, hf_sapms_eyecatcher, tvb, offset, 12, FALSE); offset+=12;
            proto_tree_add_item(sapms_tree, hf_sapms_version, tvb, offset, 1, FALSE); offset+=1;
            proto_tree_add_item(sapms_tree, hf_sapms_errorno, tvb, offset, 1, FALSE); offset+=1;
            proto_tree_add_item(sapms_tree, hf_sapms_toname, tvb, offset, 40, FALSE); offset+=40;
            msg_types = proto_tree_add_item(sapms_tree, hf_sapms_msgtypes, tvb, offset, 1, FALSE);
            msg_types_tree = proto_item_add_subtree(msg_types, ett_sapms);
            proto_tree_add_item(msg_types_tree, hf_sapms_msgtypes_dia, tvb, offset, 1, FALSE);
            proto_tree_add_item(msg_types_tree, hf_sapms_msgtypes_upd, tvb, offset, 1, FALSE);
            proto_tree_add_item(msg_types_tree, hf_sapms_msgtypes_enq, tvb, offset, 1, FALSE);
            proto_tree_add_item(msg_types_tree, hf_sapms_msgtypes_btc, tvb, offset, 1, FALSE);
            proto_tree_add_item(msg_types_tree, hf_sapms_msgtypes_spo, tvb, offset, 1, FALSE);
            proto_tree_add_item(msg_types_tree, hf_sapms_msgtypes_up2, tvb, offset, 1, FALSE);
            proto_tree_add_item(msg_types_tree, hf_sapms_msgtypes_atp, tvb, offset, 1, FALSE);
            proto_tree_add_item(msg_types_tree, hf_sapms_msgtypes_icm, tvb, offset, 1, FALSE); offset+=1;
            proto_tree_add_item(sapms_tree, hf_sapms_reserved, tvb, offset, 3, FALSE); offset+=3;
            proto_tree_add_item(sapms_tree, hf_sapms_key, tvb, offset, 8, FALSE); offset+=8;

            flag = tvb_get_guint8(tvb, offset);
            proto_tree_add_item(sapms_tree, hf_sapms_flag, tvb, offset, 1, FALSE); offset+=1;
            proto_item_append_text(sapms_tree, ", Flag=%s", val_to_str(flag, hf_sapms_flag_vals, "Unknown"));

            iflag = tvb_get_guint8(tvb, offset);
            proto_tree_add_item(sapms_tree, hf_sapms_iflag, tvb, offset, 1, FALSE); offset+=1;
            proto_item_append_text(sapms_tree, ", IFlag=%s", val_to_str(iflag, hf_sapms_iflag_vals, "Unknown"));

            col_append_fstr(pinfo->cinfo, COL_INFO, "Flag=%s,IFlag=%s", val_to_str(flag, hf_sapms_flag_vals, "Unknown"), val_to_str(iflag, hf_sapms_iflag_vals, "Unknown"));

            proto_tree_add_item(sapms_tree, hf_sapms_fromname, tvb, offset, 40, FALSE); offset+=40;

            offset+=2; /* Skip 2 bytes */

            if (!tvb_offset_exists(tvb, offset)){
            	return;
            }

            /* The remaining of the packet is dissected based on the flag */
            switch (iflag){

                /* MS_SEND_NAME or unknown (forwarded messages) */
                case 0x00:
                case 0x01:{
                    opcode = tvb_get_guint8(tvb, offset);
                    proto_tree_add_item(sapms_tree, hf_sapms_opcode, tvb, offset, 1, FALSE); offset+=1;
                    proto_tree_add_item(sapms_tree, hf_sapms_opcode_error, tvb, offset, 1, FALSE); offset+=1;
                    opcode_version = tvb_get_guint8(tvb, offset);
                    proto_tree_add_item(sapms_tree, hf_sapms_opcode_version, tvb, offset, 1, FALSE); offset+=1;
                    proto_tree_add_item(sapms_tree, hf_sapms_opcode_charset, tvb, offset, 1, FALSE); offset+=1;

                    proto_item_append_text(sapms_tree, ", Opcode=%s", val_to_str(opcode, hf_sapms_opcode_vals, "Unknown"));
                    col_append_fstr(pinfo->cinfo, COL_INFO, ", Opcode=%s", val_to_str(opcode, hf_sapms_opcode_vals, "Unknown"));

		            /* Add the opcode value subtree */
                    remaining_length = (guint32)tvb_length_remaining(tvb, offset);
                    if (remaining_length > 0){
		                oi = proto_tree_add_item(sapms_tree, hf_sapms_opcode_value, tvb, offset, remaining_length, FALSE);
		                sapms_opcode_tree = proto_item_add_subtree(oi, ett_sapms);
                        dissect_sapms_opcode(tvb, pinfo, sapms_opcode_tree, offset, flag, opcode, opcode_version, remaining_length);
                    }
                    break;

                /* MS_ADM_OPCODES */
                } case 0x05:{
                    proto_tree_add_item(sapms_tree, hf_sapms_adm_eyecatcher, tvb, offset, 12, FALSE); offset+=12;
                    proto_tree_add_item(sapms_tree, hf_sapms_adm_version, tvb, offset, 1, FALSE); offset+=1;
                    proto_tree_add_item(sapms_tree, hf_sapms_adm_msgtype, tvb, offset, 1, FALSE); offset+=1;
                    proto_tree_add_item(sapms_tree, hf_sapms_adm_recsize, tvb, offset, 11, FALSE); offset+=11;
                    proto_tree_add_item(sapms_tree, hf_sapms_adm_recno, tvb, offset, 11, FALSE); offset+=11;

		            /* Add the records subtree */
                    remaining_length = (guint32)tvb_length_remaining(tvb, offset);
                    if (remaining_length > 0){
                        dissect_sapms_adm_record(tvb, pinfo, sapms_tree, offset, remaining_length);
                    }
                    break;
                }

            }

        }
	}
}

void
proto_register_sapms(void)
{
	static hf_register_info hf[] = {
        /* General Header fields */
        { &hf_sapms_eyecatcher,
            { "Eye Catcher", "sapms.eyecatcher", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Eye Catcher", HFILL }},
        { &hf_sapms_version,
            { "Version", "sapms.version", FT_UINT8, BASE_HEX, NULL, 0x0, "SAP MS Version", HFILL }},
        { &hf_sapms_errorno,
            { "Error Number", "sapms.errorno", FT_UINT8, BASE_HEX, hf_sapms_errorno_vals, 0x0, "SAP MS Error Number", HFILL }},
        { &hf_sapms_toname,
            { "To Name", "sapms.toname", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS To Name", HFILL }},
        { &hf_sapms_msgtypes,
            { "Message Type", "sapms.msgtype", FT_UINT8, BASE_HEX, NULL, 0x0, "SAP MS Message Type", HFILL }},
        { &hf_sapms_msgtypes_dia,
			{ "DIA", "sapms.msgtype.dia", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_DIA, "SAP MS Message Type DIA",
			HFILL }},
        { &hf_sapms_msgtypes_upd,
			{ "UPD", "sapms.msgtype.upd", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_UPD, "SAP MS Message Type UPD",
			HFILL }},
        { &hf_sapms_msgtypes_enq,
			{ "ENQ", "sapms.msgtype.enq", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_ENQ, "SAP MS Message Type ENQ",
			HFILL }},
        { &hf_sapms_msgtypes_btc,
			{ "BTC", "sapms.msgtype.btc", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_BTC, "SAP MS Message Type BTC",
			HFILL }},
        { &hf_sapms_msgtypes_spo,
			{ "SPO", "sapms.msgtype.spo", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_SPO, "SAP MS Message Type SPO",
			HFILL }},
        { &hf_sapms_msgtypes_up2,
			{ "UP2", "sapms.msgtype.up2", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_UP2, "SAP MS Message Type UP2",
			HFILL }},
        { &hf_sapms_msgtypes_atp,
			{ "ATP", "sapms.msgtype.atp", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_ATP, "SAP MS Message Type ATP",
			HFILL }},
        { &hf_sapms_msgtypes_icm,
			{ "ICM", "sapms.msgtype.icm", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_ICM, "SAP MS Message Type ICM",
			HFILL }},
        { &hf_sapms_reserved,
            { "Reserved", "sapms.reserved", FT_NONE, BASE_NONE, NULL, 0x0, "SAP MS Reserved", HFILL }},
        { &hf_sapms_key,
            { "Key", "sapms.key", FT_NONE, BASE_NONE, NULL, 0x0, "SAP MS Key", HFILL }},
        { &hf_sapms_flag,
            { "Flag", "sapms.flag", FT_UINT8, BASE_HEX, hf_sapms_flag_vals, 0x0, "SAP MS Flag", HFILL }},
        { &hf_sapms_iflag,
            { "IFlag", "sapms.iflag", FT_UINT8, BASE_HEX, hf_sapms_iflag_vals, 0x0, "SAP MS IFlag", HFILL }},
        { &hf_sapms_fromname,
            { "From Name", "sapms.fromname", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS From Name", HFILL }},
        { &hf_sapms_fromhost,
            { "From Host", "sapms.fromhost", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS From Host", HFILL }},
        { &hf_sapms_fromserv,
            { "From Service", "sapms.fromserv", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS From Service", HFILL }},
        { &hf_sapms_message,
            { "Message", "sapms.message", FT_NONE, BASE_NONE, NULL, 0x0, "SAP MS Message", HFILL }},

        /* ADM Message fields */
        { &hf_sapms_adm_eyecatcher,
            { "Adm Eye Catcher", "sapms.adm.eyecatcher", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Adm Eye Catcher", HFILL }},
        { &hf_sapms_adm_version,
            { "Adm Version", "sapms.adm.version", FT_UINT8, BASE_HEX, NULL, 0x0, "SAP MS Adm Version", HFILL }},
        { &hf_sapms_adm_msgtype,
            { "Adm Message Type", "sapms.adm.msgtype", FT_UINT8, BASE_HEX, hf_sapms_adm_msgtype_vals, 0x0, "SAP MS Adm Message Type", HFILL }},
        { &hf_sapms_adm_recsize,
            { "Adm Record Size", "sapms.adm.recsize", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Adm Record Size", HFILL }},
        { &hf_sapms_adm_recno,
            { "Adm Records Number", "sapms.adm.recno", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Adm Records Number", HFILL }},
        { &hf_sapms_adm_records,
            { "Adm Records", "sapms.adm.records", FT_NONE, BASE_NONE, NULL, 0x0, "SAP MS Adm Records", HFILL }},

        { &hf_sapms_adm_record,
            { "Adm Record", "sapms.adm.records", FT_NONE, BASE_NONE, NULL, 0x0, "SAP MS Adm Record", HFILL }},
        { &hf_sapms_adm_record_opcode,
            { "Adm Record Opcode", "sapms.adm.record.opcode", FT_UINT8, BASE_HEX, hf_sapms_adm_record_opcode_vals, 0x0, "SAP MS Adm Record Opcode", HFILL }},
        { &hf_sapms_adm_record_serial_number,
            { "Adm Record Serial Number", "sapms.adm.record.serial_number", FT_UINT8, BASE_HEX, NULL, 0x0, "SAP MS Adm Record Serial Number", HFILL }},
        { &hf_sapms_adm_record_executed,
            { "Adm Record Executed", "sapms.adm.record.executed", FT_UINT8, BASE_HEX, NULL, 0x0, "SAP MS Adm Record Executed", HFILL }},
        { &hf_sapms_adm_record_errorno,
            { "Adm Record Error Number", "sapms.adm.record.errorno", FT_UINT8, BASE_HEX, NULL, 0x0, "SAP MS Adm Record Error Number", HFILL }},
        { &hf_sapms_adm_record_value,
            { "Adm Record Value", "sapms.adm.record.value", FT_NONE, BASE_NONE, NULL, 0x0, "SAP MS Adm Record Value", HFILL }},

		{ &hf_sapms_adm_parameter,
			{ "Adm Profile Parameter", "sapms.adm.parameter", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Adm Profile Parameter", HFILL }},

        { &hf_sapms_adm_rzl_strg_type,
            { "Adm RZL String Type", "sapms.adm.rzl_strg.type", FT_UINT8, BASE_HEX, hf_sapms_adm_rzl_strg_type_vals, 0x0, "SAP MS Adm RZL String Type", HFILL }},
        { &hf_sapms_adm_rzl_strg_name,
            { "Adm RZL String Name", "sapms.adm.rzl_strg.name", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Adm RZL String Name", HFILL }},
        { &hf_sapms_adm_rzl_strg_value,
            { "Adm RZL String Value", "sapms.adm.rzl_strg.value", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Adm RZL String Value", HFILL }},
		{ &hf_sapms_adm_rzl_strg_value_integer,
			{ "Adm RZL String Integer Value", "sapms.adm.rzl_strg.value", FT_INT32, BASE_DEC, NULL, 0x0, "SAP MS Adm RZL String Integer Value", HFILL }},

        /* OPCODE fields */
        { &hf_sapms_opcode,
            { "Opcode", "sapms.opcode", FT_UINT8, BASE_HEX, hf_sapms_opcode_vals, 0x0, "SAP MS Opcode", HFILL }},
        { &hf_sapms_opcode_error,
            { "Opcode Error", "sapms.opcode.error", FT_UINT8, BASE_HEX, hf_sapms_opcode_error_vals, 0x0, "SAP MS Opcode Error", HFILL }},
        { &hf_sapms_opcode_version,
            { "Opcode Version", "sapms.opcode.version", FT_UINT8, BASE_DEC, NULL, 0x0, "SAP MS Opcode Version", HFILL }},
        { &hf_sapms_opcode_charset,
            { "Opcode Character Set", "sapms.opcode.charset", FT_UINT8, BASE_DEC, NULL, 0x0, "SAP MS Opcode Character Set", HFILL }},
        { &hf_sapms_opcode_value,
            { "Opcode Value", "sapms.opcode.value", FT_NONE, BASE_NONE, NULL, 0x0, "SAP MS Opcode Value", HFILL }},

        /* MS_SET/GET/DEL_PROPERTY opcode fields */
        { &hf_sapms_property_client,
            { "Property Client", "sapms.property.client", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Property Client", HFILL }},
        { &hf_sapms_property_id,
            { "Property ID", "sapms.property.id", FT_UINT32, BASE_HEX, hf_sapms_property_id_vals, 0x0, "SAP MS Property ID", HFILL }},
        { &hf_sapms_property_length,
            { "Property Length", "sapms.property.length", FT_UINT32, BASE_DEC, NULL, 0x0, "SAP MS Property Length", HFILL }},
        { &hf_sapms_property_value,
            { "Property", "sapms.property.value", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Property Value", HFILL }},
		{ &hf_sapms_property_ip_address,
			{ "Property IP Address v4", "sapms.property.ipaddr4", FT_IPv4, BASE_NONE, NULL, 0x0, "SAP MS Property IP Address IPv4", HFILL }},
		{ &hf_sapms_property_ip_address6,
			{ "Property IP Address v6", "sapms.property.ipaddr6", FT_IPv6, BASE_NONE, NULL, 0x0, "SAP MS Property IP Address IPv6", HFILL }},
		{ &hf_sapms_property_service,
			{ "Property Service Number", "sapms.property.servno", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Property Service Number", HFILL }},
		{ &hf_sapms_property_release,
			{ "Property Release", "sapms.property.release", FT_STRINGZ, BASE_NONE, NULL, 0x0, "SAP MS Property Release", HFILL }},
		{ &hf_sapms_property_release_patchno,
			{ "Property Patch Number", "sapms.property.patchno", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Property Patch Number", HFILL }},
		{ &hf_sapms_property_release_supplvl,
			{ "Property Support Level", "sapms.property.supplvl", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Property Support Level", HFILL }},
		{ &hf_sapms_property_release_platform,
			{ "Property Platform", "sapms.property.platform", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Property Platform", HFILL }},

        /* MS_GET_CODEPAGE */
		{ &hf_sapms_codepage,
			{ "Codepage", "sapms.codepage", FT_UINT32, BASE_DEC, NULL, 0x0, "SAP MS Codepage", HFILL }},

        /* MS_DUMP fields */
        { &hf_sapms_dump_dest,
            { "Dump Dest", "sapms.dump.dest", FT_UINT8, BASE_HEX, NULL, 0x0, "SAP MS Dump Dest", HFILL }},
        { &hf_sapms_dump_filler,
            { "Dump Filler", "sapms.dump.filler", FT_NONE, BASE_NONE, NULL, 0x0, "SAP MS Dump Filler", HFILL }},
        { &hf_sapms_dump_index,
            { "Dump Index", "sapms.dump.index", FT_UINT16, BASE_HEX, NULL, 0x0, "SAP MS Dump Index", HFILL }},
        { &hf_sapms_dump_command,
            { "Dump Command", "sapms.dump.command", FT_UINT16, BASE_HEX, hf_sapms_dump_command_vals, 0x0, "SAP MS Dump Command", HFILL }},
        { &hf_sapms_dump_name,
            { "Dump Name", "sapms.dump.name", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Dump Name", HFILL }},

        /* MS_SERVER_LIST fields */
        { &hf_sapms_server_lst_client,
            { "Client", "sapms.serverlst.client", FT_NONE, BASE_NONE, NULL, 0x0, "SAP MS Client", HFILL }},
        { &hf_sapms_server_lst_name,
            { "Client Name", "sapms.serverlst.name", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Client Name", HFILL }},
        { &hf_sapms_server_lst_host,
            { "Host", "sapms.serverlst.host", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Host", HFILL }},
        { &hf_sapms_server_lst_service,
            { "Service", "sapms.serverlst.service", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Service", HFILL }},
        { &hf_sapms_server_lst_msgtypes,
            { "Message Types", "sapms.serverlst.msgtypes", FT_UINT8, BASE_HEX, NULL, 0x0, "SAP MS Message Types", HFILL }},
        { &hf_sapms_server_lst_msgtypes_dia,
			{ "DIA", "sapms.serverlst.msgtype.dia", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_DIA, "SAP MS Message Type DIA",
			HFILL }},
        { &hf_sapms_server_lst_msgtypes_upd,
			{ "UPD", "sapms.serverlst.msgtype.upd", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_UPD, "SAP MS Message Type UPD",
			HFILL }},
        { &hf_sapms_server_lst_msgtypes_enq,
			{ "ENQ", "sapms.serverlst.msgtype.enq", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_ENQ, "SAP MS Message Type ENQ",
			HFILL }},
        { &hf_sapms_server_lst_msgtypes_btc,
			{ "BTC", "sapms.serverlst.msgtype.btc", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_BTC, "SAP MS Message Type BTC",
			HFILL }},
        { &hf_sapms_server_lst_msgtypes_spo,
			{ "SPO", "sapms.serverlst.msgtype.spo", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_SPO, "SAP MS Message Type SPO",
			HFILL }},
        { &hf_sapms_server_lst_msgtypes_up2,
			{ "UP2", "sapms.serverlst.msgtype.up2", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_UP2, "SAP MS Message Type UP2",
			HFILL }},
        { &hf_sapms_server_lst_msgtypes_atp,
			{ "ATP", "sapms.serverlst.msgtype.atp", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_ATP, "SAP MS Message Type ATP",
			HFILL }},
        { &hf_sapms_server_lst_msgtypes_icm,
			{ "ICM", "sapms.serverlst.msgtype.icm", FT_BOOLEAN, 8, NULL, SAPMS_MSG_TYPE_ICM, "SAP MS Message Type ICM",
			HFILL }},
        { &hf_sapms_server_lst_hostaddr,
            { "Host Address v6", "sapms.serverlst.hostaddr", FT_IPv6, BASE_NONE, NULL, 0x0, "SAP MS Host Address IPv6", HFILL }},
        { &hf_sapms_server_lst_hostaddrv4,
            { "Host Address v4", "sapms.serverlst.hostaddr4", FT_IPv4, BASE_NONE, NULL, 0x0, "SAP MS Host Address IPv4", HFILL }},
        { &hf_sapms_server_lst_servno,
            { "Service Number", "sapms.serverlst.servno", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Service Number", HFILL }},
        { &hf_sapms_server_lst_status,
            { "Status", "sapms.serverlst.status", FT_UINT8, BASE_HEX, hf_sapms_server_lst_status_vals, 0x0, "SAP MS Status", HFILL }},
        { &hf_sapms_server_lst_nitrc,
            { "NI Trace", "sapms.serverlst.nitrc", FT_UINT8, BASE_HEX, NULL, 0x0, "SAP MS NI Trace", HFILL }},
        { &hf_sapms_server_lst_sys_service,
            { "Sys Service", "sapms.serverlst.sysservice", FT_UINT32, BASE_HEX, NULL, 0x0, "SAP MS Sys Service", HFILL }},

        /* MS_SET_SECURITY_KEY, MS_GET_SECURITY_KEY and MS_GET_SECURITY_KEY2 fields */
        { &hf_sapms_security_name,
            { "Security Name", "sapms.security.name", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Security Name", HFILL }},
        { &hf_sapms_security_key,
            { "Security Key", "sapms.security.key", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Security Key", HFILL }},
        { &hf_sapms_security_port,
            { "Security Port", "sapms.security.port", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Security Port", HFILL }},
        { &hf_sapms_security_address,
            { "Security Address v4", "sapms.security.addr", FT_IPv4, BASE_NONE, NULL, 0x0, "SAP MS Security Address IPv4", HFILL }},
        { &hf_sapms_security_address6,
            { "Security Address v6", "sapms.security.addr6", FT_IPv6, BASE_NONE, NULL, 0x0, "SAP MS Security Address IPv6", HFILL }},

        /* MS_SET_TEXT, MS_GET_TEXT */
        { &hf_sapms_text_name,
            { "Text Name", "sapms.text.name", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Text Name", HFILL }},
        { &hf_sapms_text_length,
            { "Text Length", "sapms.text.length", FT_UINT32, BASE_DEC, NULL, 0x0, "SAP MS Text Length", HFILL }},
        { &hf_sapms_text_value,
            { "Text Value", "sapms.text.value", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Text Value", HFILL }},

        /* COUNTER fields */
		{ &hf_sapms_counter_uuid,
			{ "Counter UUID", "sapms.counter.uuid", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Counter UUID", HFILL }},
		{ &hf_sapms_counter_count,
			{ "Counter Count", "sapms.counter.count", FT_UINT32, BASE_DEC, NULL, 0x0, "SAP MS Counter Count", HFILL }},
		{ &hf_sapms_counter_no,
			{ "Counter Number", "sapms.counter.no", FT_UINT32, BASE_DEC, NULL, 0x0, "SAP MS Counter Number", HFILL }},

		/* CHANGE_IP fields */
		{ &hf_sapms_change_ip_address,
			{ "Change IP Address IPv4", "sapms.change.addr", FT_IPv4, BASE_NONE, NULL, 0x0, "SAP MS Change IP Address IPv4", HFILL }},
		{ &hf_sapms_change_ip_address6,
			{ "Change IP Address IPv6", "sapms.change.addr6", FT_IPv6, BASE_NONE, NULL, 0x0, "SAP MS Change IP Address IPv6", HFILL }},

        /* FILE RELOAD fields */
        { &hf_sapms_file_reload,
            { "File Reload Name", "sapms.filereload.name", FT_UINT8, BASE_HEX, hf_sapms_file_reload_vals, 0x0, "SAP MS File Reload Name", HFILL }},
        { &hf_sapms_file_filler,
            { "File Reload Filler", "sapms.filereload.filler", FT_NONE, BASE_NONE, NULL, 0x0, "SAP MS File Reload Filler", HFILL }},

        /* MS_GET_LOGON, MS_SET_LOGON and MS_DEL_LOGON fields */
        { &hf_sapms_logon_type,
            { "Logon Type", "sapms.logon.type", FT_UINT16, BASE_HEX, hf_sapms_logon_type_vals, 0x0, "SAP MS Logon Type", HFILL }},
        { &hf_sapms_logon_port,
            { "Logon Port", "sapms.logon.port", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Logon Port", HFILL }},
        { &hf_sapms_logon_address,
            { "Logon Address IPv4", "sapms.logon.addr", FT_IPv4, BASE_NONE, NULL, 0x0, "SAP MS Logon Address IPv4", HFILL }},
        { &hf_sapms_logon_name_length,
            { "Logon Name Length", "sapms.logon.name_length", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Logon Name Length", HFILL }},
        { &hf_sapms_logon_name,
            { "Logon Name", "sapms.logon.name", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Logon Name", HFILL }},
        { &hf_sapms_logon_prot_length,
            { "Logon Protocol Length", "sapms.logon.prot_length", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Logon Protocol Length", HFILL }},
        { &hf_sapms_logon_prot,
            { "Logon Protocol", "sapms.logon.prot", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Logon Protocol", HFILL }},
        { &hf_sapms_logon_host_length,
            { "Logon Host Length", "sapms.logon.host_length", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Logon Host Length", HFILL }},
        { &hf_sapms_logon_host,
            { "Logon Host", "sapms.logon.host", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Logon Host", HFILL }},
        { &hf_sapms_logon_misc_length,
            { "Logon Misc Length", "sapms.logon.misc_length", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Logon Misc Length", HFILL }},
        { &hf_sapms_logon_misc,
            { "Logon Misc", "sapms.logon.misc", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Logon Misc", HFILL }},
        { &hf_sapms_logon_address6_length,
            { "Logon Address IPv6 Length", "sapms.logon.addr6_length", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Logon Address IPv6 Length", HFILL }},
        { &hf_sapms_logon_address6,
            { "Logon Address IPv6", "sapms.logon.address6", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Logon Address IPv6", HFILL }},

		{ &hf_sapms_shutdown_reason_length,
			{ "Shutdown Reason Length", "sapms.shutdown.reason_length", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Shutdown Reason Length", HFILL }},
		{ &hf_sapms_shutdown_reason,
			{ "Shutdown Reason", "sapms.shutdown.reason", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS Shutdown Reason", HFILL }},

		/* MS_IP_PORT_TO_NAME fields */
		{ &hf_sapms_ip_to_name_address4,
			{ "IP to Name Address IPv4", "sapms.ip_to_name.addr4", FT_IPv4, BASE_NONE, NULL, 0x0, "SAP MS IP to Name Address IPv4", HFILL }},
		{ &hf_sapms_ip_to_name_address6,
			{ "IP to Name Address IPv6", "sapms.ip_to_name.addr6", FT_IPv6, BASE_NONE, NULL, 0x0, "SAP MS IP to Name Address IPv6", HFILL }},
		{ &hf_sapms_ip_to_name_port,
			{ "IP to Name Port", "sapms.ip_to_name.port", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS IP to Name Port", HFILL }},
		{ &hf_sapms_ip_to_name_length,
			{ "IP to Name Length", "sapms.ip_to_name.name_length", FT_UINT32, BASE_DEC, NULL, 0x0, "SAP MS IP to Name Length", HFILL }},
		{ &hf_sapms_ip_to_name,
			{ "IP to Name", "sapms.ip_to_name.name", FT_STRING, BASE_NONE, NULL, 0x0, "SAP MS IP to Name", HFILL }},

		/* MS_CHECK_ACL fields */
		{ &hf_sapms_check_acl_error_code,
			{ "Check ACL Error Code", "sapms.check_acl.error_code", FT_UINT16, BASE_DEC, NULL, 0x0, "SAP MS Check ACL Error Code", HFILL }},
		{ &hf_sapms_check_acl_acl,
			{ "Check ACL ACL Entry", "sapms.check_acl.acl", FT_STRINGZ, BASE_NONE, NULL, 0x0, "SAP MS Check ACL ACL Entry", HFILL }},
	};

	/* Setup protocol subtree array */
	static gint *ett[] = {
		&ett_sapms
	};

	module_t *sapms_module;

	/* Register the protocol */
	proto_sapms = proto_register_protocol (
		"SAP Message Server Protocol",	/* name       */
		"SAPMS",		/* short name */
		"sapms"		/* abbrev     */
	);

	register_dissector("sapms", dissect_sapms, proto_sapms);

	proto_register_field_array(proto_sapms, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));

	/* Register the preferences */
	sapms_module = prefs_register_protocol(proto_sapms, proto_reg_handoff_sapms);

	range_convert_str(&global_sapms_port_range, SAPMS_PORT_RANGE, MAX_TCP_PORT);
	prefs_register_range_preference(sapms_module, "tcp_ports", "SAP MS Protocol TCP port numbers", "Port numbers used for SAP MS Protocol (default " SAPMS_PORT_RANGE ")", &global_sapms_port_range, MAX_TCP_PORT);

    prefs_register_bool_preference(sapms_module, "highlight_unknown_items", "Highlight unknown SAP MS messages", "Whether the SAP MS Protocol dissector should highlight unknown MS messages (migth be noise and generate a lot of expert warnings)", &global_sapms_highlight_items);

}

/**
 * Helpers for dealing with the port range
 */
static void range_delete_callback (guint32 port)
{
	dissector_delete_uint("sapni.port", port, sapms_handle);
}

static void range_add_callback (guint32 port)
{
	dissector_add_uint("sapni.port", port, sapms_handle);
}

/**
 * Register Hand off for the SAP MS Protocol
 */
void
proto_reg_handoff_sapms(void)
{
	static range_t *sapms_port_range;
	static gboolean initialized = FALSE;

	if (!initialized) {
		sapms_handle = create_dissector_handle(dissect_sapms, proto_sapms);
		initialized = TRUE;
	} else {
		range_foreach(sapms_port_range, range_delete_callback);
		g_free(sapms_port_range);
	}

	sapms_port_range = range_copy(global_sapms_port_range);
	range_foreach(sapms_port_range, range_add_callback);
}
