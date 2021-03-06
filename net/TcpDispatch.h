#ifndef ECO_NET_TCP_DISPATCH_H
#define ECO_NET_TCP_DISPATCH_H
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
#include <eco/Type.h>
#include <eco/net/RequestHandler.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
// message handled by "handler", suguest using in server.
template<typename Handler>
inline void handle_context(IN Context& c)
{
	const TcpConnection& conn = c.m_session.get_connection();

	// 1.filter request
	const char* e = nullptr;
	// sessesion auth or session request after authed.
	if (Handler::auth_flag() > 0)
	{
		if (eco::has(c.m_meta.m_category, category_session))
		{
			if (Handler::auth_flag() && !c.m_session.authorized())
				e = "session isn't authed when recv";
			if (!Handler::auth_flag() && c.m_session.authorized())
				e = "session is authed when recv";
		}
		else
		{
			if (Handler::auth_flag() && !conn.authorized())
				e = "connection isn't authed when recv";
			if (!Handler::auth_flag() && conn.authorized())
				e = "connection is authed when recv";
		}
	}
	
	if (e != nullptr)
	{
		EcoError(eco::net::req) << Log(c.m_session,
			c.m_meta.m_message_type, Handler::name()) <= e;
		return;
	}
	
	try
	{
		// 2.decode message by a newer handler.
		// heap is used to be passed by deriving from "enable_shared_from_this".
		std::shared_ptr<Handler> hdl(new Handler);
		if (!hdl->on_decode(c.m_message.m_data, c.m_message.m_size))
		{
			EcoError(eco::net::req) << Log(c.m_session, c.m_meta.m_message_type,
				Handler::name()) <= "decode message fail";
			return;
		}
		// release io raw data to save memory.
		c.release_data();
		hdl->context() = std::move(c);

		// 3.handle request.
		hdl->on_request();
	} 
	catch (eco::Error& e) {
		EcoError << e;
	}
	catch (std::exception& e) {
		EcoLog(error) << e.what();
	}
}


////////////////////////////////////////////////////////////////////////////////
// message handled by "functor", suguest using in client.
template<typename Message>
inline void* handle_context_make(std::shared_ptr<Message>& obj)
{
	obj.reset(new Message());
	return obj.get();
}
template<typename Message>
inline void* handle_context_make(Message& obj)
{
	return &obj;
}

template<typename Message, typename Codec>
inline void handle_context(
	IN std::function<void(IN Message&, IN Context&)>& func,
	IN Context& c)
{
	Codec codec;
	Message object;
	codec.set_message(handle_context_make(object));
	codec.decode(c.m_message.m_data, c.m_message.m_size);
	func(object, c);
}
inline void handle_context_default(
	IN std::function<void(IN eco::Bytes&, IN Context&)>& func,
	IN Context& c)
{
	func(c.m_message, c);
}


////////////////////////////////////////////////////////////////////////////////
// message array handled by "functor", suguest using in client.
#define ECO_HANDLE_CONTEXT_ARRAY_FUNC(Message) \
std::function<void(std::vector<std::shared_ptr<Message>>&, Context&)>

template<typename Message, typename Codec>
inline void handle_context_array(
	IN ECO_HANDLE_CONTEXT_ARRAY_FUNC(Message)& func,
	IN Context& c)
{
	static std::vector<Message*> s_message_set;

	// 1.deccode message.
	Codec codec;
	Message* msg(new Message);
	codec.set_message(msg);
	codec.decode(&c.m_message);
	s_message_set.push_back(msg);

	// 2.handle message.
	if (eco::has(c.m_option, opt_last))
	{
		// 3.release message object.
		func(s_message_set, c);
		auto it = s_message_set.begin();
		for (; it != s_message_set.end(); ++it){
			delete *it;
		}
		s_message_set.clear();
	}
}


////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpDispatch
{
public:
	typedef std::function<void(IN Context&)> HandlerFunc;

	virtual ~TcpDispatch() {}

	/*@ register message and message handler.*/
	virtual void register_handler(IN uint64_t id, IN HandlerFunc hdl) = 0;

	/*@ register message default handler.*/
	virtual void register_default_handler(IN HandlerFunc hdl) = 0;

public:
	/*@ register message and message handler, and message dedicated by typeid.
	* @ tmpl.message: message type to be registered.
	* @ tmpl.Handler: to handle "tmpl.message" type.
	*/
	template<typename Handler>
	inline void dispatch()
	{
		register_handler(Handler::request_type(),
			std::bind(&handle_context<Handler>, std::placeholders::_1));
	}

	/*@ register default message handler to process unregistered message type.
	* @ tmpl.Handler: default message handler.
	*/
	template<typename Handler>
	inline void dispatch_default()
	{
		register_default_handler(
			std::bind(&handle_context<Handler>, std::placeholders::_1));
	}

	/*@ register message and message handler function.*/
#ifndef ECO_NO_FUNCTION_TEMPLATE_DEFAULT
	template<typename Message, typename Codec = net::ProtobufCodec>
	inline void dispatch(
		IN uint64_t id,
		IN std::function<void(IN Message&, IN Context&)> func)
	{
		register_handler(id, std::bind(&handle_context<Message, Codec>,
			func, std::placeholders::_1));
	}
	template<typename Message, typename Codec = eco::net::ProtobufCodec,
		typename Object, typename Func>
	inline void dispatch(IN uint64_t id, IN Object& obj, IN Func func)
	{
		dispatch<Message, Codec>(id, std::bind(
			func, &obj, std::placeholders::_1, std::placeholders::_2));
	}
#else
	template<typename Message, typename Codec>
	inline void dispatch(
		IN uint64_t id,
		IN std::function<void(IN Message&, IN Context&)> func)
	{
		register_handler(id, std::bind(&handle_context<Message, Codec>,
			func, std::placeholders::_1));
	}
#endif

	/*@ register message default handler func.*/
	inline void dispatch_default(
		IN std::function<void(IN eco::Bytes&, IN Context&)> func)
	{
		register_default_handler(std::bind(&handle_context_default, 
			func, std::placeholders::_1));
	}

	/*@ register message and message handler function.*/
	template<typename Message, typename Codec>
	inline void dispatch_array(
		IN uint64_t id,
		IN ECO_HANDLE_CONTEXT_ARRAY_FUNC(Message) func)
	{
		register_handler(id, std::bind(&handle_context_array<Message, Codec>,
			func, std::placeholders::_1));
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif