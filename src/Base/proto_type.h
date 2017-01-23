#ifndef __PROTO_TYPE_H__
#define __PROTO_TYPE_H__

#define MESSAGE_VERSION 3
#define MAX_ROOM_NUM 10


enum
{
	///1
	THR_SOCKET_CONNECT = 0x01,
	THR_SOCKET_DISCONNECT = 0x02,
	THR_KILL_PLAYER_MSG = 0x03, //主动断开玩家的连接
	THR_ADD_RADIUS_MSG = 0x04, //添加radius连接

	///2
	GET_SERVER_INFO_MSG = 0xDD,//获得服务器的消息
	SEND_INFO_CENTER_SERVER_MSG = 0xDE,//游戏服发送自己的IP信息到中心服务器
	UPDATA_CENTER_SERVER_PNUM_MSG = 0xDF,//更新服务的人数
	KEEP_ALIVE_MSG = 0xE0,//保持连接消息
	AUTHEN_REQ_MSG = 0xE1,//玩家登陆验证请求
	AUTHEN_RES_MSG = 0xE8,//登陆验证响应
	UPDATE_CENTER_SERVER_INFO_MSG = 0xFD,//更新中心服务器信息
	GET_CENTER_SERVER_MSG = 0x10,//定时向common_room获取中心服IP
	AUTHEN_REQ_RADIUS_MSG = 0x51,//服务器登陆Radius请求
	USERINFO_REQ_RADIUS_MSG = 0x52,//向Radius发送新登陆用户信息请求
	GAME_ROOM_INFO_REQ_RADIUS_MSG = 0x54,//游戏服务器请求房间信息，结构体中附带房间人数
	AUTHEN_RES_RADIUS_MSG = 0x61,//服务器登陆Radius请求回应
	USERINFO_RES_RADIUS_MSG = 0x62,//用户信息回应
	GAME_ROOM_INFO_RES_RADIUS_MSG = 0x64,//游戏服务器房间信息请求回应，附带赠送金币数目一起吧

	///3
	RADIUS_LOG_MAIN_INFO_REQ = 0x70,//游戏日志消息类型
	CHILD_GET_PROP_LOG_REQ = 0x02,//道具获得日志
};

#endif