#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include <time.h>
#include <vector>

struct TCPNode
{
	int				heart_beat_time;
	char 			cServerIP[32];
	int				iServerPort;//如果为TCP_SERVER_SOCKET的话，记录PORT

	int				socket;
	time_t 			time_out_time;
	int				read_msg_head_len;
	int				socket_node_state;

	char			*read_buffer;						
	int 			read_msg_len;
	int 			read_msg_pos;
	int 			total_read_bytes;
	
	char			*write_buffer;
	int				write_msg_len;
	int				total_write_bytes;
	
	int				iWaitOutTime;
	
	bool			bKill;
};

class TCPClient
{
public:
	TCPClient(int _readbuff,int _writebuff);
	virtual~ 				TCPClient();
	int 					AddTCPClientNode(const char* _ip,int _port);
	void 					Disconnect(int _index);
	
	void 					SetKillFlag(int iIndex,bool bFlag);
protected:
	virtual void 			CallbackAddSocketNode(int index){};
	virtual void 			CallbackDelSocketNode(int index){};
	virtual void 			CallbackNodeTimeOut(int index);
	virtual void 			CallbackTCPReadData(int index);
	virtual void			CallbackWriteAllNode(){};
	
	void 					Update();
	int 					WriteData(int index,void* pData,int len);
	TCPNode*				GetTCPNode(int index){return &all_tcp_nodes[index];};
	void					DelSocketNode(int index);
	void 					ResetSocketNode(int index);
private:
	int  					AddSocketNode(int socket);
	int 					TCPReadData(int socket, void *data, int data_size);
	int 					TCPWriteData(int socket, void *data, const int data_size);
private:
	std::vector<TCPNode> 	all_tcp_nodes;
	int 					socket_node_num;
	int 					max_node_index;
	int 					read_buff_size;
	int 					write_buff_size;
};



#endif