#include "TCPClient.h"
#include "net.h"
#include <arpa/inet.h>
#include "log.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include<stdlib.h>

TCPClient::TCPClient(int _readbuff,int _writebuff)
{
	read_buff_size = _readbuff;
	write_buff_size = _writebuff;
	max_node_index = -1;
}
TCPClient::~TCPClient()
{
	for(size_t i=0;i<all_tcp_nodes.size();++i)
	{
		if(all_tcp_nodes[i].socket > 0)
		{
			close(all_tcp_nodes[i].socket);
		}
		
		free(all_tcp_nodes[i].read_buffer);
		free(all_tcp_nodes[i].write_buffer);
	}
}

int TCPClient::AddTCPClientNode(const char* _ip, int _port)
{
	int socket = SetupTCPClient(inet_addr(_ip), _port, 0);
	if(socket > 0)
	{
		Unblock(socket);
		return AddSocketNode(socket);
	}
	else
	{
		_log(_ERROR,"TCPClient","AddTCPClientNode Add Node Error ip=%s port=%d",_ip,_port);
		return -1;
	}
}

int TCPClient::AddSocketNode(int socket)
{
	//printf("=== AddSocketNode socket[%d]\n",socket);
	TCPNode* pNode = NULL;
	int index = 0;
	for (size_t i=0;i<all_tcp_nodes.size();++i)
	{
		if(all_tcp_nodes[i].socket == 0 && all_tcp_nodes[i].iWaitOutTime < time(0)) 
		{
			pNode = &(all_tcp_nodes[i]);
			index = i;
			break;
		}
	}
	if(pNode == NULL)
	{
		index = all_tcp_nodes.size();
		TCPNode one;
		memset(&one,0,sizeof(TCPNode));
		one.read_buffer = (char*)malloc(read_buff_size);
		one.write_buffer = (char*)malloc(write_buff_size);
		all_tcp_nodes.push_back(one);
		pNode = &(all_tcp_nodes[index]);
	}
	
	socket_node_num ++;

	if (max_node_index < index) 
		max_node_index = index;

	pNode->socket = socket;
	pNode->read_msg_head_len = 4;
	pNode->bKill = false;
	pNode->read_msg_pos = 0;
	pNode->write_msg_len = 0;
	pNode->total_write_bytes = 0;
	pNode->socket_node_state = 0;
	pNode->bKill = false;

	ResetSocketNode(index);

	CallbackAddSocketNode(index);
	
	return index;
}
void TCPClient::ResetSocketNode(int index)
{
	TCPNode* node = &(all_tcp_nodes[index]);
	memset(node->read_buffer, 0, read_buff_size);
	node->read_msg_len = node->read_msg_head_len;
	node->total_read_bytes = 0;
	node->read_msg_pos = 0;
	
	if (node->heart_beat_time)
	{
		time(&node->time_out_time);
		node->time_out_time += node->heart_beat_time;
	}
	else
	{
		node->time_out_time = 0;
	}	
}
void TCPClient::DelSocketNode(int index)
{
	TCPNode* node = &(all_tcp_nodes[index]);
	node->bKill = false;
	
	if(node->socket <= 0)
	{
		_log(_ERROR, "TCPClient", "DelSocketNode socket node socket error, socket[%d]index[%d]",node->socket,index);
		CallbackDelSocketNode(index);
		return;
	}
	close(node->socket);
	
	_log(_ERROR,"TCPClient","Close Socket[%d]",node->socket);

	socket_node_num --;
	CallbackDelSocketNode(index);
	node->socket = 0;
	node->iServerPort = 0;
	
	node->iWaitOutTime = time(NULL)+3;

	int i;
	for (i = max_node_index; i>=0; i--)
		if (all_tcp_nodes[i].socket > 0) break;
	max_node_index = i;
}
int TCPClient::TCPReadData(int socket, void *data, int data_size)
{
	int read_bytes;
	read_bytes = read(socket, data, data_size);
	return read_bytes;
}
int TCPClient::TCPWriteData(int socket, void *data, const int data_size)
{
	int write_bytes;
	write_bytes = write(socket, data, data_size);
	if(write_bytes < 0)
		_log(_ERROR, "TCPClient", "tcp write error:[%d]socket[%d]", errno,socket);
	return write_bytes;
}
void TCPClient::CallbackTCPReadData(int index)
{
	ResetSocketNode(index);
}
void TCPClient::CallbackNodeTimeOut(int index)
{
	//TCPNode* node = &all_tcp_nodes[index];
	DelSocketNode(index);
}

void TCPClient::Update()
{
	int 			max_socket = 0;
	fd_set 			rfds;
	fd_set 			wfds;
	struct timeval	tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
		
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	
	for(int i=0;i<=max_node_index;i++)
	{
		TCPNode* node = &all_tcp_nodes[i];	
		if(node->bKill == true && node->socket > 0)//crystal add 
		{
			printf("del socket node 1\n");
			DelSocketNode(i);
		}
		if(node->socket > 0)
		{
			if (node->read_msg_head_len > 0)
				FD_SET(node->socket, &rfds);
			if (node->write_msg_len > 0)
				FD_SET(node->socket, &wfds);
			if (node->socket > max_socket) 
				max_socket = node->socket;
		}
	}
	CallbackWriteAllNode();//处理群发
	//write_buffer_mutex.unlock();
		
	int retval = select(max_socket+1, &rfds, &wfds, 0, &tv);
	if (retval<0)
	{
		_log(_ERROR, "TCPClient", "select error:%d", errno);
		exit(0);
	}
	
	time_t cur_time = time(0);
	for(int i=0;i<=max_node_index;i++)
	{
		TCPNode* node = &all_tcp_nodes[i];
		if (node->socket <= 0) continue;
		if ((node->heart_beat_time != 0)&&(cur_time >= node->time_out_time))
			CallbackNodeTimeOut(i);
	}
			
	for (int i=0;i<=max_node_index;i++)
	{
		TCPNode* node = &all_tcp_nodes[i];
		if (FD_ISSET(node->socket, &wfds))
		{
			int write_bytes = TCPWriteData(node->socket, &((node->write_buffer)[node->total_write_bytes]),node->write_msg_len - node->total_write_bytes);	
			if(write_bytes <= 0)
			{
				if(write_bytes < 0)
				{
					_log(_ERROR,"TCPClient","Run:TCPWriteData error index[%d],socket[%d],max_node_index[%d]",i,node->socket,max_node_index);
				}
				printf("del socket node 2\n");
				DelSocketNode(i);
				//write_buffer_mutex.unlock();
				continue;
			}
			//printf("===write msg fd[%d] write[%d] total[%d] msg_len[%d]  buff[%d] err:%d\n",
			//	   node->socket,write_bytes,node->total_write_bytes,node->write_msg_len,write_buff_size,errno);
			node->total_write_bytes += write_bytes;
			if (node->total_write_bytes == node->write_msg_len)
			{
				node->total_write_bytes = 0;
				node->write_msg_len		= 0;
			}
		}
	}
			
	for (int i=0;i<=max_node_index;i++)
	{
		TCPNode* node = &all_tcp_nodes[i];
		if (FD_ISSET(node->socket, &rfds))
		{
			//printf("FD_ISSET node index[%d] fd[%d]\n",i,node->socket);
			if (read_buff_size - node->read_msg_pos < node->read_msg_len)
			{
				_log(_ERROR,"TCPClient","update error!!! read_buffer_size[%d] read_msg_pos[%d] read_head_len[%d],index[%d],socket[%d],max_node_index[%d]",
					 read_buff_size,node->read_msg_pos,node->read_msg_head_len,i,node->socket,max_node_index);
				DelSocketNode(i);
				printf("del socket node 3\n");
				continue;
			}		
			int read_bytes = TCPReadData(node->socket, &((node->read_buffer)[node->read_msg_pos + node->total_read_bytes]),node->read_msg_len - node->total_read_bytes);
			if(read_bytes <= 0)
			{
				printf("del socket node 4 socket[%d] index[%d] max_socket[%d] read_bytes[%d] error[%d]\n",node->socket,i,max_socket,read_bytes,errno);
				DelSocketNode(i);				
				continue;
			}
			node->total_read_bytes += read_bytes;
			if(node->total_read_bytes < node->read_msg_len)
				continue;

			CallbackTCPReadData(i);
		}
	}
}

void TCPClient::SetKillFlag(int iIndex,bool bFlag)
{
	TCPNode* node = &all_tcp_nodes[iIndex];	
	if(node->socket > 0)
	{
		node->bKill = bFlag;
	}
}

int TCPClient::WriteData(int index,void* pData,int len)
{
	if (index < 0 || index > max_node_index)
	{
		return -1;
	}
	
	TCPNode* node = &all_tcp_nodes[index];
	
	if(node->socket == 0 || node->bKill == true)
	{
		_log(_ERROR, "TCPClient", "WriteData:node->socket == 0 index:%d max_node_index:%d", index, max_node_index);
		return -1;
	}
	
	if (len > write_buff_size - node->write_msg_len)
	{
		_log(_ERROR, "TCPClient", "write buffer error, index[%d] data_size[%d] buff[%d] pos[%d] max_node[%d]", index, len, write_buff_size,node->write_msg_len,max_node_index);
		SetKillFlag(index,true);
		return -1;
	}
	
	memcpy( &((node->write_buffer)[node->write_msg_len]), pData, len);
	node->write_msg_len += len;
	
	return len;
}



