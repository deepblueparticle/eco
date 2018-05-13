#ifndef ECO_TEST_RESULT_H
#define ECO_TEST_RESULT_H
/*******************************************************************************
@ name

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-06-18.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Export.h>
#include <list>


namespace eco{;
namespace test{;
////////////////////////////////////////////////////////////////////////////////
class Result
{
public:
	enum State
	{
		success,		// �ɹ�
		failed,			// ʧ��
		timeout,		// ��ʱ
		unknown,		// δ֪����
	};

	Result();

	inline void Success()
	{
		m_state = success;
	}

	inline void Failed()
	{
		m_state = failed;
	}

	inline void Timeout()
	{
		m_state = timeout;
	}

	inline void Unknown()
	{
		m_state = unknown;
	}

private:
	uint32_t m_state;		// ִ�н��״̬
	std::string m_msg;		// ִ�н����ϸ��Ϣ
};


////////////////////////////////////////////////////////////////////////////////
class ResultSet
{
////////////////////////////////////////////////////////////////////////////////
public:
	ResultSet()
	{
		Reset();
	}

	inline void Reset()
	{
		ResetCase();
		ResetScene();
		ResetTest();
	}

	inline void ResetTest()
	{
		m_total_test = 0;
		m_failed_test = 0;
		m_checked_test = 0;
	}

	inline void ResetCase()
	{
		m_total_case = 0;
		m_failed_case = 0;
		m_checked_case = 0;
	}

	inline void ResetScene()
	{
		m_total_scene = 0;
		m_failed_scene = 0;
		m_checked_scene = 0;
	}

////////////////////////////////////////////////////////////////////////////////
public:
	inline void Fail(uint32_t failed_test = 1)
	{
		m_checked_test += failed_test;
		m_failed_test += failed_test;
		m_total_test += failed_test;
	}

	inline void Success(uint32_t success_test = 1)
	{
		m_checked_test += success_test;
		m_total_test += success_test;
	}
	
	inline bool IsOk() const
	{
		return m_failed_test == 0;
	}

	inline uint32_t GetTotalTest() const
	{
		return m_total_test;
	}

	inline uint32_t GetCheckedTest() const
	{
		return m_checked_test;
	}

	inline uint32_t GetFailedTest() const
	{
		return m_failed_test;
	}

	inline uint32_t GetSuccessTest() const
	{
		return m_total_test - m_failed_test;
	}

////////////////////////////////////////////////////////////////////////////////
public:
	inline void SetTotalCase(uint32_t total_case)
	{
		m_total_case = total_case;
	}

	inline void SetCheckedCase(uint32_t checked_case)
	{
		m_checked_case = checked_case;
	}

	inline void SetFailedCase(uint32_t failed_case)
	{
		m_failed_case = failed_case;
	}

	inline uint32_t GetTotalCase() const
	{
		return m_total_case;
	}

	inline uint32_t GetCheckedCase() const
	{
		return m_checked_case;
	}

	inline uint32_t GetFailedCase() const
	{
		return m_failed_case;
	}

	inline uint32_t GetSuccessCase() const
	{
		return m_total_case - m_failed_case;
	}

////////////////////////////////////////////////////////////////////////////////
public:
	inline void SetTotalScene(uint32_t total_scene)
	{
		m_total_scene = total_scene;
	}

	inline void SetCheckedScene(uint32_t checked_scene)
	{
		m_checked_scene = checked_scene;
	}

	inline void SetFailedScene(uint32_t failed_scene)
	{
		m_failed_scene = failed_scene;
	}

	inline uint32_t GetCheckedScene() const
	{
		return m_checked_scene;
	}

	inline uint32_t GetFailedChildScene() const
	{
		return (m_failed_scene == 0) ? 0 : m_failed_scene - 1;
	}

	inline uint32_t GetTotalChildScene() const
	{
		return (m_total_scene == 0) ? 0 : m_total_scene - 1;
	}

	inline uint32_t GetCheckedChildScene() const
	{
		return (m_checked_scene == 0) ? 0 : m_checked_scene - 1;
	}

	inline uint32_t GetSuccessChildScene() const
	{
		return GetTotalChildScene() - GetFailedChildScene();
	}
	
////////////////////////////////////////////////////////////////////////////////
public:
	// �ܳ�������
	uint32_t m_total_scene;
	// ִ�м���������������
	uint32_t m_checked_scene;
	// ִ��ʧ�ܵ�������
	uint32_t m_failed_scene;

	// ����������
	uint32_t m_total_case;
	// ִ�м���������������
	uint32_t m_checked_case;
	// ִ��ʧ�ܵ�������
	uint32_t m_failed_case;

	// ����������
	uint32_t m_total_test;
	// ִ�м����Ĳ���������
	uint32_t m_checked_test;
	// ִ��ʧ�ܵĲ��ԡ�
	uint32_t m_failed_test;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif