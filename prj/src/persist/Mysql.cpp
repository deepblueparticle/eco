#include "PrecHeader.h"
#include <eco/persist/MySql.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Error.h>
#include <eco/thread/Atomic.h>
#include <eco/thread/Mutex.h>
#include <vector>
#include <mysql/mysql.h>


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
__declspec(thread) uint32_t	t_trans_level = 0;
inline bool has_transaction()
{
	return t_trans_level > 0;
}


////////////////////////////////////////////////////////////////////////////////
class MySqlConfig : public eco::DatabaseConfig
{
public:
	virtual void get_field_type_sql(
		OUT std::string& field_sql,
		IN  const uint32_t field_type,
		IN  const uint32_t field_size) override
	{
		switch (field_type)
		{
		case type_date_time:
			field_sql = "DATETIME";
			break;
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
class MySql::Impl
{
	ECO_IMPL_INIT(MySql);
public:
	// connection info.
	int m_port;
	std::string m_server_ip;
	std::string m_db_name;
	std::string m_user_id;
	std::string m_password;
	std::string m_char_set;
	// mysql instance.
	MYSQL* m_mysql;
	eco::Mutex m_mysql_mutex;

	// mysql config.
	MySqlConfig m_config;

public:
	inline Impl() : m_mysql(nullptr), m_port(0)
	{}

	inline ~Impl()
	{
		close();
	}

	void set_charset(IN const Database::CharSet v)
	{
		switch (v)
		{
		case Database::char_set_gbk :
			m_char_set = "gbk";
			return ;
		case Database::char_set_gb2312 :
			m_char_set = "gb2312";
			return ;
		case Database::char_set_utf8 :
			m_char_set = "utf8";
			return ;
		case Database::char_set_utf16 :
			m_char_set = "utf16";
			return ;
		case Database::char_set_utf32 :
			m_char_set = "utf32";
			return ;
		}
		m_char_set = "gbk";	// default is gbk.
	}

	void open()
	{
		// init mysql instance.
		m_mysql = mysql_init(nullptr);
		if (nullptr == m_mysql)
		{
			throw_error();
		}

		// connect to mysql server fail.
		if (nullptr == mysql_real_connect(m_mysql,
			m_server_ip.c_str(), 
			m_user_id.c_str(),
			m_password.c_str(),
			m_db_name.c_str(),
			m_port, nullptr, 0))
		{
			throw_error();
		}

		// set autocommit=0 transaction.
		if (mysql_autocommit(m_mysql, false) != 0)
		{
			throw_error();
		}

		// set mysql client options.
		// set mysql client character set.
		if (mysql_set_character_set(m_mysql, m_char_set.c_str()) != 0)
		{
			throw_error();
		}

		// connect to mysql server success.
		// connect to dedicated server.
		if (0 != mysql_select_db(m_mysql, m_db_name.c_str()))
		{
			close_and_throw_error();
		}
	}

	// connect to server once.
	inline void open_once()
	{
		if (m_mysql == nullptr)
		{
			open();
		}
	}

	// close mysql.
	inline void close()
	{
		if (nullptr != m_mysql)
		{
			mysql_close(m_mysql);
			m_mysql = nullptr;
		}
	}

	// ping mysql server: if success return true, else false.
	inline bool ping()
	{
		return (mysql_ping(m_mysql) == 0) ? true : false;
	}

	// throw error.
	inline void close_and_throw_error()
	{
		std::string msg("mysql connect database fail: ");
		msg += mysql_error(m_mysql);
		eco::Error e(mysql_errno(m_mysql), msg);
		close();
		throw e;
	}

	// throw error.
	inline void throw_error(
		IN const std::string& sql = std::string(),
		IN const int err_no = 0)
	{
		std::string msg("mysql connect database fail: ");
		msg += mysql_error(m_mysql);
		msg += " sql:";
		msg += sql;
		throw std::logic_error(msg);
	}

	// reconnect mysql.
	inline void reconnect()
	{
		// reconnect 3 times.
		for (int i=0; i<3; ++i)
		{
			try
			{
				close();
				open();
				break;
			}
			catch (...)
			{}
		}// end for.
	}

	// query sql.
	inline void query_sql(IN const char* sql)
	{
		if (0 != mysql_query(m_mysql, sql))
		{
			uint32_t eno = mysql_errno(m_mysql);
			// 1064: sql syntax error.
			// 1146: table doesn't exist.
			if (eno == 1064 || eno == 1146)
			{
				throw_error(sql, eno);
			}

			reconnect();
			if (0 != mysql_query(m_mysql, sql))
			{
				throw_error(sql);
			}
		}// end if.
	}
};
ECO_SHARED_IMPL(MySql);


////////////////////////////////////////////////////////////////////////////////
void MySql::open(
	IN const char* server_ip,
	IN const int port,
	IN const char* db_name,
	IN const char* user_id,
	IN const char* password,
	IN const CharSet char_set)
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex);
	impl().m_port = port;
	impl().m_db_name = db_name;
	impl().m_user_id = user_id;
	impl().m_password = password;
	impl().m_server_ip = server_ip;
	impl().set_charset(char_set);
	impl().open();
}


////////////////////////////////////////////////////////////////////////////////
void MySql::close()
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex);
	impl().close();
}


////////////////////////////////////////////////////////////////////////////////
bool MySql::is_open()
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex, !has_transaction());
	return (impl().m_mysql != nullptr);
}


////////////////////////////////////////////////////////////////////////////////
void MySql::select(OUT Record& obj, IN  const char* sql)
{
	Recordset data_sheet;
	select(data_sheet, sql);
	if (data_sheet.size() > 0)
	{
		obj = data_sheet[0];
	}
}


////////////////////////////////////////////////////////////////////////////////
void MySql::select(
	OUT Recordset& obj_sheet,
	IN  const char* sql)
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex, !has_transaction());
	impl().open_once();

	// query sql fail, reconnect it.
	impl().query_sql(sql);

	// query success, get query result.
	MYSQL_RES* res = mysql_store_result(impl().m_mysql);
	if (nullptr == res)
	{
		impl().throw_error();
	}

	// get data row size.(object size.)
	uint64_t row_size = mysql_num_rows(res);
	if (row_size == 0)
	{
		return ;
	}

	// iterate the object row.
	MYSQL_ROW row = mysql_fetch_row(res);
	uint64_t field_size = mysql_num_fields(res);
	obj_sheet.reserve((size_t)(obj_sheet.size() + row_size));
	for (uint32_t r=0; row!=nullptr && r<row_size; ++r)
	{
		Record& item = obj_sheet.add_item();
		item.reserve((size_t)field_size);
		for (uint32_t f=0; f<field_size; ++f)
		{
			const char* v = row[f];
			item.add(v != nullptr ? v : "");
		}
		row = mysql_fetch_row(res);
	}
	// free query result.
	mysql_free_result(res);
}


////////////////////////////////////////////////////////////////////////////////
uint64_t MySql::execute_sql(IN const char* sql)
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex, !has_transaction());
	impl().open_once();

	// query sql fail, reconnect it.
	impl().query_sql(sql);

	return mysql_affected_rows(impl().m_mysql);
}


////////////////////////////////////////////////////////////////////////////////
bool MySql::ping()
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex, !has_transaction());
	impl().open_once();
	return impl().ping();
}


////////////////////////////////////////////////////////////////////////////////
uint64_t MySql::get_system_time()
{
	const char* sql = "select now()";
	Record rd;
	select(rd, sql);

	// system time format: 2016-03-10 17:41:40.
	if (rd.size() != 0)		// parse system time.
	{
		std::string temp = rd[0];
		temp.erase(16, 1);
		temp.erase(13, 1);
		temp.erase(10, 1);
		temp.erase(7, 1);
		temp.erase(4, 1);
		return _atoi64(temp.c_str());
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
bool MySql::has_table(IN const char* table_name)
{
	std::string cond_sql("where table_name='"); 
	cond_sql += table_name;
	cond_sql += "' and TABLE_SCHEMA='";
	cond_sql += impl().m_db_name;
	cond_sql += "'";
	return has_record("information_schema.TABLES", cond_sql.c_str()) > 0;
}


////////////////////////////////////////////////////////////////////////////////
void MySql::get_tables(OUT Recordset& tables, IN  const char* db)
{
	char cond_sql[128] = { 0 };
	const char* name = eco::empty(db) ? impl().m_db_name.c_str() : db;
	sprintf(cond_sql, "select TABLE_NAME from information_schema.TABLES"
		" where TABLE_SCHEMA='%s'", name);
	return select(tables, cond_sql);
}


////////////////////////////////////////////////////////////////////////////////
DatabaseConfig& MySql::config()
{
	return impl().m_config;
}


////////////////////////////////////////////////////////////////////////////////
void MySql::begin()
{
	impl().m_mysql_mutex.lock();
	execute_sql("BEGIN;");
}


////////////////////////////////////////////////////////////////////////////////
void MySql::commit()
{
	execute_sql("COMMIT;");
	impl().m_mysql_mutex.unlock();
}


////////////////////////////////////////////////////////////////////////////////
void MySql::rollback()
{
	execute_sql("ROLLBACK;");
	impl().m_mysql_mutex.unlock();
}


////////////////////////////////////////////////////////////////////////////////
uint32_t& MySql::transaction_level()
{
	return t_trans_level;
}


////////////////////////////////////////////////////////////////////////////////
}// ns