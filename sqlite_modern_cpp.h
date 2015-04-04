#pragma once

#include <string>
#include <functional>
#include <stdexcept>

#include <sqlite3.h>

namespace sqlite {

#pragma region function_traits
	template <typename T>
	struct function_traits : public function_traits < decltype(&T::operator()) > {};

	template <typename ClassType, typename ReturnType, typename... Args>
	struct function_traits < ReturnType(ClassType::*)(Args...) const >
		// we specialize for pointers to member function
	{
		enum { arity = sizeof...(Args) };
		// arity is the number of arguments.

		typedef ReturnType result_type;

		template <size_t i>
		struct arg {
			typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
			// the i-th argument is equivalent to the i-th tuple element of a tuple
			// composed of those arguments.
		};
	};

#pragma endregion

	class database;
	class database_binder;

	template<std::size_t> class binder;

	class database_binder {
	private:
		sqlite3 * _db;
		std::u16string _sql;
		sqlite3_stmt* _stmt;
		int _inx;

		void _extract(std::function<void(void)> call_back) {
			int hresult;

			while ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
				call_back();
			}

			if (hresult != SQLITE_DONE) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			if (sqlite3_finalize(_stmt) != SQLITE_OK) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			_stmt = nullptr;
		}
		void _extract_single_value(std::function<void(void)> call_back) {
			int hresult;

			if ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
				call_back();
			}

			if ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) {
				throw std::runtime_error("not every row extracted");
			}

			if (hresult != SQLITE_DONE) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			if (sqlite3_finalize(_stmt) != SQLITE_OK) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			_stmt = nullptr;
		}
		void _prepare() {
			if (sqlite3_prepare16_v2(_db, _sql.data(), -1, &_stmt, nullptr) != SQLITE_OK) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}
		}
	protected:
		database_binder(sqlite3 * db, std::u16string const & sql) :
			_db(db),
			_sql(sql),
			_stmt(nullptr),
			_inx(1) {
			_prepare();
		}

		database_binder(sqlite3 * db, std::string const & sql) : database_binder(db, std::u16string(sql.begin(), sql.end())) { }

	public:
		friend class database;
		~database_binder() {
			/* Will be executed if no >>op is found */
			if (_stmt) {
				int hresult;
				while ((hresult = sqlite3_step(_stmt)) == SQLITE_ROW) { }

				if (hresult != SQLITE_DONE) {
					throw std::runtime_error(sqlite3_errmsg(_db));
				}

				if (sqlite3_finalize(_stmt) != SQLITE_OK) {
					throw std::runtime_error(sqlite3_errmsg(_db));
				}

				_stmt = nullptr;
			}
		}
#pragma region operator <<
		database_binder& operator <<(double val) {
			if (sqlite3_bind_double(_stmt, _inx, val) != SQLITE_OK) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			++_inx;
			return *this;
		}
		database_binder& operator <<(float val) {
			if (sqlite3_bind_double(_stmt, _inx, double(val)) != SQLITE_OK) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			++_inx;
			return *this;
		}
		database_binder& operator <<(int val) {
			if (sqlite3_bind_int(_stmt, _inx, val) != SQLITE_OK) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			++_inx;
			return *this;
		}
		database_binder& operator <<(sqlite_int64 val) {
			if (sqlite3_bind_int64(_stmt, _inx, val) != SQLITE_OK) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			++_inx;
			return *this;
		}
		database_binder& operator <<(std::string const& txt) {
			if (sqlite3_bind_text(_stmt, _inx, txt.data(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			++_inx;
			return *this;
		}
		database_binder& operator <<(std::u16string const& txt) {
			if (sqlite3_bind_text16(_stmt, _inx, txt.data(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
				throw std::runtime_error(sqlite3_errmsg(_db));
			}

			++_inx;
			return *this;
		}
#pragma endregion

#pragma region get_col_from_db
		void get_col_from_db(int inx, int& i) {
			if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
				i = 0;
			} else {
				i = sqlite3_column_int(_stmt, inx);
			}
		}
		void get_col_from_db(int inx, sqlite3_int64& i) {
			if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
				i = 0;
			} else {
				i = sqlite3_column_int64(_stmt, inx);
			}
		}
		void get_col_from_db(int inx, std::string& s) {
			if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
				s = std::string();
			} else {
				sqlite3_column_bytes(_stmt, inx);
				s = std::string((char*)sqlite3_column_text(_stmt, inx));
			}
		}
		void get_col_from_db(int inx, std::u16string& w) {
			if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
				w = std::u16string();
			} else {
				sqlite3_column_bytes16(_stmt, inx);
				w = std::u16string((char16_t *)sqlite3_column_text16(_stmt, inx));
			}
		}
		void get_col_from_db(int inx, double& d) {
			if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
				d = 0;
			} else {
				d = sqlite3_column_double(_stmt, inx);
			}
		}
		void get_col_from_db(int inx, float& f) {
			if (sqlite3_column_type(_stmt, inx) == SQLITE_NULL) {
				f = 0;
			} else {
				f = float(sqlite3_column_double(_stmt, inx));
			}
		}
#pragma endregion

#pragma region operator >>

		void operator>>(int & val) {
			_extract_single_value([&] {
				get_col_from_db(0, val);
			});
		}
		void operator>>(std::string& val) {
			_extract_single_value([&] {
				get_col_from_db(0, val);
			});
		}
		void operator>>(std::u16string& val) {
			_extract_single_value([&] {
				get_col_from_db(0, val);
			});
		}
		void operator>>(double & val) {
			_extract_single_value([&] {
				get_col_from_db(0, val);
			});
		}
		void operator>>(float & val) {
			_extract_single_value([&] {
				get_col_from_db(0, val);
			});
		}
		void operator>>(sqlite3_int64 & val) {
			_extract_single_value([&] {
				get_col_from_db(0, val);
			});
		}

		template<typename FUNC>
		void operator>>(FUNC l) {
			typedef function_traits<decltype(l)> traits;

			database_binder& dbb = *this;
			_extract([&]() {
				binder<traits::arity>::run(dbb, l);
			});

		}
#pragma endregion

	};

	class database {
	private:
		sqlite3 * _db;
		bool _connected;
		bool _ownes_db;
	public:


		database(std::u16string const & db_name) : _db(nullptr), _connected(false), _ownes_db(true) {
			_connected = sqlite3_open16(db_name.data(), &_db) == SQLITE_OK;
		}
		database(std::string const & db_name) : database(std::u16string(db_name.begin(), db_name.end())) { }

		database(sqlite3* db) {
			_db = db;
			_connected = SQLITE_OK;
			_ownes_db = false;
		}

		~database() {
			if (_db && _ownes_db) {
				sqlite3_close_v2(_db);
				_db = nullptr;
			}
		}

		database_binder operator<<(std::string const& sql) const {
			return database_binder(_db, sql);
		}
		database_binder operator<<(std::u16string const& sql) const {
			return database_binder(_db, sql);
		}

		operator bool() const {
			return _connected;
		}
		operator std::string() {
			return sqlite3_errmsg(_db);
		}
		operator std::u16string() {
			return (char16_t*)sqlite3_errmsg16(_db);
		}
	};

#pragma region binder specialization

	template<std::size_t Count>
	struct binder {
		template<
			typename    Function,
			typename... Values
		>
		static typename std::enable_if<(sizeof...(Values) < Count), void>::type run(
			database_binder& db,
			Function&        function,
			Values&&...      values
		) {
			typename function_traits<Function>::template arg<sizeof...(Values)>::type value{};
			db.get_col_from_db(sizeof...(Values), value);

			run<Function>(db, function, values..., value);
		}

		template<
			typename    Function,
			typename... Values
		>
		static typename std::enable_if<(sizeof...(Values) == Count), void>::type run(
			database_binder&,
			Function&        function,
			Values&&...      values
		) {
			function(std::move(values)...);
		}
	};

#pragma endregion

}
