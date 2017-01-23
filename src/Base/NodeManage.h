#ifndef __CLIENT_MANAGE_H__
#define __CLIENT_MANAGE_H__

#include <unordered_map>
#include <list>

/*
 *Warming: It is not muti thread safe.
 *
 */

template<class T>
class NodeManage
{
public:
  NodeManage();
  ~NodeManage();
  
  T* GetFreeNode();
  void ReturnNode(int _id);
  
  T* GetNode(int _id);
  void RegNode(int _id,T* _node);
  
  inline std::size_t FreeSize(){return m_lisFreeNodes.size();}
  inline std::size_t NodeSize(){return m_mapNodes.size();}
private:
  std::list<T*> m_lisFreeNodes;
  std::unordered_map<int,T*> m_mapNodes;
};

template<class T>
NodeManage<T>::NodeManage()
{
	
}
template<class T>
NodeManage<T>::~NodeManage()
{
	typename std::list<T*>::iterator it=m_lisFreeNodes.begin();
	for(;it!=m_lisFreeNodes.end();++it)
	{
		delete *it;
	}
	typename std::unordered_map<int,T*>::iterator it2 = m_mapNodes.begin();
	for(;it2!=m_mapNodes.end();++it2)
	{
		delete (it2->second);
	}
}
template<class T>
T* NodeManage<T>::GetFreeNode()
{
	T* res;
	if(m_lisFreeNodes.empty())
	{
		res = new T();
	}
	else
	{
		res = m_lisFreeNodes.back();
		m_lisFreeNodes.pop_back();
	}
	res->Reset();
	return res;
}
template<class T>
T* NodeManage<T>::GetNode(int _id)
{
	typename std::unordered_map<int,T*>::iterator it = m_mapNodes.find(_id);
	if(it==m_mapNodes.end())
	{
		return NULL;
	}
	return it->second;
}
template<class T>
void NodeManage<T>::ReturnNode(int _id)
{
	typename std::unordered_map<int,T*>::iterator it = m_mapNodes.find(_id);
	if(it!=m_mapNodes.end())
	{
		T* pNode=it->second;
		m_lisFreeNodes.push_back(pNode);
		m_mapNodes.erase(it);
	}
}
template<class T>
void NodeManage<T>::RegNode(int _id,T* _node)
{
	m_mapNodes[_id] = _node;
}

#endif
