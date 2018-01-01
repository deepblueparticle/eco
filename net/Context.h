#ifndef ECO_NET_CONTEXT_H
#define ECO_NET_CONTEXT_H
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Project.h>
#include <eco/net/protocol/Protocol.h>
#include <eco/net/TcpSession.h>
#include <eco/net/TcpConnection.h>


namespace eco{;
namespace net{;
class TcpPeer;
typedef std::weak_ptr<TcpPeer> TcpPeerWptr;
////////////////////////////////////////////////////////////////////////////////
class DataContext
{
	ECO_OBJECT(DataContext);
public:
	inline DataContext(IN TcpSessionHost* host = nullptr)
		: m_prot(nullptr)
		, m_category(0)
	{
		if (host != nullptr)
		{
			m_session_host = *host;
		}
	}

	inline DataContext(IN DataContext&& v)
		: m_data(std::move(v.m_data))
		, m_category(v.m_category)
		, m_prot(v.m_prot)
		, m_session_host(v.m_session_host)
		, m_peer_wptr(std::move(v.m_peer_wptr))
	{}

	inline DataContext& operator=(IN DataContext&& v)
	{
		m_data = std::move(v.m_data);
		m_category = v.m_category;
		m_prot = v.m_prot;
		m_session_host = v.m_session_host;
		m_peer_wptr = std::move(v.m_peer_wptr);
		return *this;
	}

	// message bytes size.
	inline uint32_t size() const
	{
		return m_data.size();
	}
	// message bytes.
	inline const char* c_str(IN uint32_t pos = 0) const
	{
		return &m_data[pos];
	}

public:
	// message data.
	eco::String			m_data;
	MessageCategory		m_category;

	// protocol and tcp peer.
	Protocol*			m_prot;
	TcpPeerWptr			m_peer_wptr;
	TcpSessionHost		m_session_host;
};


////////////////////////////////////////////////////////////////////////////////
class Context
{
public:
	eco::String			m_data;
	eco::Bytes			m_message;
	MessageMeta			m_meta;
	TcpSession			m_session;
	TcpConnection		m_connection;
	
public:
	inline Context() : m_session(eco::null)
	{}

	inline void release_data()
	{
		m_data.release();
		m_message.clear();
	}

	inline Context& operator=(IN Context&& c)
	{
		m_data = std::move(m_data);
		m_message = c.m_message;
		m_meta = c.m_meta;
		m_session = c.m_session;
		m_connection = c.m_connection;
		return *this;
	}
};


}}
////////////////////////////////////////////////////////////////////////////////
#endif