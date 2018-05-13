#ifndef ECO_ECO_H
#define ECO_ECO_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2017-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2017 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/log/Log.h>
#include <eco/thread/State.h>
#include <eco/thread/TaskServer.h>
#include <eco/thread/Timer.h>
#include <list>

ECO_NS_BEGIN(eco);
class Being;
class Btask;
typedef std::auto_ptr<Btask> BtaskAptr;
////////////////////////////////////////////////////////////////////////////////
class Eco
{
	ECO_IMPL_INIT(Eco);
public:
	// ���췽��
	Eco();
	~Eco();

	// ���л
	void start();

	// ֹͣ�
	void stop();

	// ����ִ������
	void post_task(IN const Btask& task);

	// ����ȴ�����
	void post_wait(IN const Btask& task);

	// ֪ͨ�ȴ�����
	void move_wait();

	// �����������
	void add_being(IN Being* be);
	// �Ƴ���������
	void remove_being(IN Being* be);

	// �����Ƶ��
	void set_unit_live_tick_seconds(
		IN const uint32_t unit_live_tick_secs);
	const uint32_t get_unit_live_tick_seconds() const;
	
	// ��������߳�
	void set_task_server_thread_size(
		IN const uint32_t thread_size);
	const uint32_t get_task_server_thread_size() const;

	// ��ʱ��
	Timer& timer();
	
private:
	// ����������Ӧ������ִ�м�����޸�ϵͳ��������
	// ��ʼ�������ࡣ�������ڿ�ʼ������
	void start_live_timer();
	void on_live_timer();
	bool is_time_to_live(IN const uint32_t interval_tick) const;
	// ��ȡ��һ������ִ�еĻ��
	Being* get_next_being();
	// ��ǰ��������
	void reset_current_being();
	Being* get_current_being() const;

private:
	// ϵͳ�������������ࣨÿx��1�Σ�������άϵϵͳ�������С�
	uint32_t m_unit_live_tick_sec;
	uint32_t m_tick_count;
	eco::Timer m_timer;

	// ���������б�
	std::list<Being*> m_be_list;
	// ��ǰ�������е���������
	std::list<Being*>::iterator m_cur_it;
	eco::atomic::State m_running;
	mutable eco::ConditionVariable m_cond_var;

	// �������
	uint32_t m_task_server_thread_size;
	std::list<BtaskAptr> m_wait_task_list;
	eco::TaskServer<BtaskAptr> m_task_server;
	mutable eco::Mutex m_wait_task_list_mutex;
};


////////////////////////////////////////////////////////////////////////////////
ECO_API Eco* get_eco();
ECO_NS_END(eco);
#endif