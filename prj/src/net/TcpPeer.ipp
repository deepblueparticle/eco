#ifndef ECO_NET_PEER_IPP
#define ECO_NET_PEER_IPP
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
#include <eco/net/TcpPeer.h>
#include <eco/net/TcpConnector.h>
#include <eco/net/Context.h>
#include <eco/log/Log.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpPeer::Impl : public TcpConnectorHandler
{
public:
	TcpState m_state;
	TcpPeer::wptr m_wptr;
	TcpPeerHandler* m_handler;
	std::auto_ptr<TcpConnector> m_connector;
	// the session of tcp peer.
	//std::vector<uint32_t> m_session_id;
	//eco::Mutex m_session_id_mutex;

public:
	Impl() : m_handler(nullptr)
	{}

	inline void init(IN TcpPeer&)
	{}

	inline void make(IN IoService* io, IN TcpPeerHandler* handler)
	{
		// init io service and set tcp server.
		m_connector.reset(new TcpConnector(io));
		m_connector->register_handler(*this);
		set_handler(handler);

		// init state: prepare for first heartbeat.
		m_state.self_live(false);
	}

	// tcp peer callback handler.
	inline void set_handler(IN TcpPeerHandler* v)
	{
		m_handler = v;
	}
	inline TcpPeerHandler& handler()
	{
		return *m_handler;
	}

	// raw io socket related by the net framework: exp boost::asio\libevent.
	inline TcpSocket* socket()
	{
		return m_connector->socket();
	}

	// tcp peer identity which is the address of connector.
	inline int64_t get_id() const
	{
		return m_connector->get_id();
	}

	// tcp peer connection state.
	inline TcpState& state()
	{
		return m_state;
	}
	inline const TcpState& get_state() const
	{
		return m_state;
	}
	// set tcp peer state to connected.
	inline void set_connected()
	{
		m_state.connected();
		m_state.peer_live(true);
	}

	// ready to receive data head.
	inline void async_connect(IN const Address& addr)
	{
		m_connector->async_connect(addr);
	}
	inline void async_recv();

	// add session to peer. todo.
	inline void add_session(IN const SessionId id)
	{
	}

	// send response to client.
	inline void async_send(IN eco::String& data)
	{
		m_state.self_live(true);
		m_connector->async_write(data);
	}

	inline void async_send(
		IN MessageMeta& meta, 
		IN Protocol& prot,
		IN ProtocolHead& prot_head)
	{
		eco::Error e;
		eco::String data;
		if (!prot.encode(data, meta, prot_head, e))
		{
			EcoNet(EcoError,*this, "async_send", e);
			return;
		}
		async_send(data);
	}

	// send heartbeat.
	inline void async_send_heartbeat(IN ProtocolHead& prot_head)
	{
		eco::String data;
		prot_head.encode_heartbeat(data);
		async_send(data);
	}

	// close peer.
	inline void close()
	{
		if (!m_state.is_close())
		{
			// 1.close state.
			m_state.close();

			// 2.close socket.
			m_connector->close();
		}
	}

	// close peer and notify peer handler.
	inline void notify_close(IN const eco::Error* e)
	{
		close();
		m_handler->on_close(get_id());
	}

	// get data context that to be handled by dispatch server.
	inline void get_data_context(
		OUT DataContext& dc,
		IN  MessageCategory category,
		IN  eco::String& data,
		IN  Protocol& prot,
		IN  ProtocolHead& prot_head)
	{
		dc.m_category = category;
		dc.m_data = std::move(data);
		dc.m_peer_wptr = m_wptr;
		dc.m_prot = &prot;
		dc.m_prot_head = &prot_head;
	}

public:
	// when peer has connected to server.
	virtual void on_connect(
		IN bool is_connected,
		IN const eco::Error* error) override;

	// when the peer has received data head.
	virtual void on_read_head(
		IN char* data,
		IN const uint32_t head_size,
		IN const eco::Error* error) override;

	// when the peer has received data.
	virtual void on_read_data(
		IN eco::String& data,
		IN const eco::Error* error) override;

	// the peer has send data.
	virtual void on_write(
		IN const uint32_t write_size,
		IN const eco::Error* error) override;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif