#ifndef __PROTO_TYPE_H__
#define __PROTO_TYPE_H__

#define MESSAGE_VERSION 3
#define MAX_ROOM_NUM 10

///1
#define THR_SOCKET_CONNECT		0x01
#define THR_SOCKET_DISCONNECT	0x02
#define THR_KILL_PLAYER_MSG		0x03 //主动断开玩家的连接
#define THR_ADD_RADIUS_MSG		0x04 //添加radius连接

///2
#define		GET_SERVER_INFO_MSG				0xDD//获得服务器的消息
#define		SEND_INFO_CENTER_SERVER_MSG		0xDE//游戏服发送自己的IP信息到中心服务器
#define		UPDATA_CENTER_SERVER_PNUM_MSG	0xDF//更新服务的人数
#define		KEEP_ALIVE_MSG					0xE0//保持连接消息
#define		AUTHEN_REQ_MSG					0xE1//玩家登陆验证请求
#define		AUTHEN_RES_MSG					0xE8//登陆验证响应
#define	  	UPDATE_CENTER_SERVER_INFO_MSG 	0xFD//更新中心服务器信息
#define		GET_CENTER_SERVER_MSG			0x10//定时向common_room获取中心服IP
#define		AUTHEN_REQ_RADIUS_MSG 			0x51//服务器登陆Radius请求
#define		USERINFO_REQ_RADIUS_MSG 		0x52//向Radius发送新登陆用户信息请求
#define		GAME_ROOM_INFO_REQ_RADIUS_MSG 	0x54//游戏服务器请求房间信息，结构体中附带房间人数
#define   	AUTHEN_RES_RADIUS_MSG 			0x61//服务器登陆Radius请求回应
#define   	USERINFO_RES_RADIUS_MSG 		0x62//用户信息回应
#define		GAME_ROOM_INFO_RES_RADIUS_MSG 	0x64//游戏服务器房间信息请求回应，附带赠送金币数目一起吧

///3
#define RADIUS_LOG_MAIN_INFO_REQ	0x70//游戏日志消息类型
#define CHILD_GET_PROP_LOG_REQ 		0x02//道具获得日志

#endif