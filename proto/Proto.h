#ifndef ECO_PROTO_H
#define ECO_PROTO_H
/*******************************************************************************
@ ����

@ ����

@ �쳣

@ ��ע

--------------------------------------------------------------------------------
@ ��ʷ��¼ @
@ ujoy modifyed on 2016-11-02

--------------------------------------------------------------------------------
* ��Ȩ����(c) 2016 - 2019, siberia corp, ��������Ȩ����

*******************************************************************************/
#include <eco/net/Log.h>
#include "Object.pb.h"


////////////////////////////////////////////////////////////////////////////////
template<typename Stream>
Stream& operator<<(OUT Stream& stream, IN const ::proto::Property& p)
{
	return stream << p.user_id() << '-' << p.object_id()
		<< p.name() << p.value();
}

template<typename Stream>
Stream& operator<<(OUT Stream& stream, IN const ::proto::Error& e)
{
	return stream << e.message() << " #" << e.id();
}


ECO_NS_BEGIN(eco);
ECO_NS_END(eco);
#endif