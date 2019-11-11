#pragma once
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <string>
#include <cstdint>
#include <optional>
#include <nlohmann/json.hpp>
#include <ostream>
#include <sstream>

using json = nlohmann::json;

#define ANY_NUMERIC_CAL(op_name, op_val) template<typename T>	\
bool numeric_cal_##op_name(const T& other_value)				\
{																\
	auto& cur_any_value = *this;								\
	if (cur_any_value.is_int())								\
	{															\
		any_int_type& pre = std::get<any_int_type>(cur_any_value);\
		pre = static_cast<any_int_type>(pre op_val other_value);\
		return true;											\
	}															\
	else														\
	{															\
		double& pre = std::get<double>(cur_any_value);			\
		pre = static_cast<double>(pre op_val other_value);		\
		return true;											\
	}															\
	return false;												\
}																\

#define ANY_NUMERIC_ANY_CAL(op_name)							\
bool numeric_cal_##op_name(const any_value_type& other_value)	\
{																\
	auto& cur_any_value = *this;								\
	if (other_value.is_double())								\
	{															\
		return numeric_cal_##op_name(std::get<double>(other_value)); \
	}															\
	else if (other_value.is_int())							\
	{															\
		return numeric_cal_##op_name(std::get<any_int_type>(other_value));\
	}															\
	return false;												\
}																\

namespace behavior_tree::common
{
	using any_int_type = std::int64_t;
    using any_key_type = std::variant<std::string, any_int_type>;
    class any_value_type;
	using any_vector = std::vector<any_value_type>;
	using any_int_map = std::unordered_map<any_int_type, any_value_type>;
	using any_str_map = std::unordered_map<std::string, any_value_type>;
    class any_value_type: public std::variant<any_int_type, double, bool, std::string, any_vector, any_int_map, any_str_map>
    {
	public:
		using base = std::variant<any_int_type, double, bool, std::string, any_vector, any_int_map, any_str_map>;
        using base::base;
        using base::operator=;
		any_value_type() :
			base()
		{

		}
		any_value_type(const int& _in_value) :
			base(static_cast<any_int_type>(_in_value))
		{

		}
		any_value_type(const std::uint32_t& _in_value) :
			base(static_cast<any_int_type>(_in_value))
		{

		}
		any_value_type(const std::uint16_t& _in_value) :
			base(static_cast<any_int_type>(_in_value))
		{

		}
		any_value_type(const std::uint8_t& _in_value) :
			base(static_cast<any_int_type>(_in_value))
		{

		}
		any_value_type(const std::int8_t& _in_value) :
			base(static_cast<any_int_type>(_in_value))
		{

		}
		any_value_type(const std::int16_t& _in_value) :
			base(static_cast<any_int_type>(_in_value))
		{

		}
		any_value_type(const std::uint64_t& _in_value) :
			base(static_cast<any_int_type>(_in_value))
		{

		}
		any_value_type(const float& _in_value) :
			base(static_cast<double>(_in_value))
		{

		}
		bool is_int_map() const
		{
			return std::holds_alternative<any_int_map>(*this);
		}
		bool is_str_map() const
		{
			return std::holds_alternative<any_str_map>(*this);
		}
		bool is_int() const
		{
			return std::holds_alternative<any_int_type>(*this);
		}
		bool is_double() const
		{
			return std::holds_alternative<double>(*this);
		}
		bool is_bool() const
		{
			return std::holds_alternative<bool>(*this);
		}
		bool is_str() const
		{
			return std::holds_alternative<std::string>(*this);
		}
		bool is_vector() const
		{
			return std::holds_alternative<any_vector>(*this);
		}
		bool is_numeric() const
		{
			return is_int() || is_double();
		}
		template <typename T>
		bool numeric_value(T dst) const
		{
			if (is_int())
			{
				dst = static_cast<T>(std::get<any_int_type>(*this));
				return true;
			}
			if (is_double())
			{
				dst = static_cast<T>(std::get<double>(*this));
				return true;
			}
			return false;
		}
		template <typename T>
		std::optional<bool> numeric_larger_than_impl(const any_value_type& other_value) const
		{
			const auto& cur_any_value = *this;
			if (!cur_any_value.is_numeric() || !other_value.is_numeric())
			{
				return std::nullopt;
			}
			if (!std::holds_alternative<T>(cur_any_value))
			{
				return std::nullopt;
			}
			const auto& cur_raw_value = std::get<T>(cur_any_value);
			if (other_value.is_double())
			{
				return cur_raw_value > std::get<double>(other_value);
			}
			else if (other_value.is_int())
			{
				return cur_raw_value > std::get<any_int_type>(other_value);
			}
			return false;

		}

		std::optional<bool> numeric_larger_than(const any_value_type& other_value) const
		{
			const auto& cur_any_value = *this;
			if (!other_value.is_numeric())
			{
				return std::nullopt;
			}
			
			if(cur_any_value.is_int())
			{
				return cur_any_value.numeric_larger_than_impl<any_int_type>(other_value);
			}
			else if (cur_any_value.is_double())
			{
				return cur_any_value.numeric_larger_than_impl<double>(other_value);
			}
			else
			{
				return std::nullopt;
			}
			
		}
		std::optional<bool> numeric_less_than(const any_value_type& other_value) const
		{
			return other_value.numeric_larger_than(*this);

		}
		std::optional<bool> numeric_no_larger_than(const any_value_type& other_value) const
		{
			const auto& cur_any_value = *this;
			if (!cur_any_value.is_numeric() || !other_value.is_numeric())
			{
				return std::nullopt;
			}
			if (*this == other_value)
			{
				return true;
			}
			return numeric_less_than(other_value);
		}
		std::optional<bool> numeric_no_less_than(const any_value_type& other_value) const
		{
			const auto& cur_any_value = *this;
			if (!cur_any_value.is_numeric() || !other_value.is_numeric())
			{
				return std::nullopt;
			}
			if (*this == other_value)
			{
				return true;
			}
			return numeric_larger_than(other_value);
		}
ANY_NUMERIC_CAL(add, +)
ANY_NUMERIC_CAL(dec, -)
ANY_NUMERIC_CAL(multiply, *)
ANY_NUMERIC_CAL(div, /)
		
ANY_NUMERIC_ANY_CAL(add)
ANY_NUMERIC_ANY_CAL(dec)
ANY_NUMERIC_ANY_CAL(multiply)
ANY_NUMERIC_ANY_CAL(div)
	
public:
friend std::ostream& operator<<(std::ostream& buffer, const any_value_type& cur_value)
{
	if (cur_value.is_bool())
	{
		buffer << std::get<bool>(cur_value);
	}
	else if (cur_value.is_int())
	{
		buffer << std::get<any_int_type>(cur_value);
	}
	else if (cur_value.is_double())
	{
		buffer << std::get<double>(cur_value);
	}
	else if (cur_value.is_str())
	{
		buffer << std::get<std::string>(cur_value);
	}
	else if (cur_value.is_vector())
	{
		buffer << "[";
		for (const auto& one_item : std::get<any_vector>(cur_value))
		{
			buffer << one_item << ",";
		}
		buffer << "]";
	}
	else if (cur_value.is_str_map())
	{
		buffer << "{";
		for (const auto& one_item : std::get<any_str_map>(cur_value))
		{
			buffer <<"("<< one_item.first << ","<<one_item.second<<"), ";
		}
		buffer << "}";
	}
	else if (cur_value.is_int_map())
	{
		buffer << "{";
		for (const auto& one_item : std::get<any_int_map>(cur_value))
		{
			buffer << one_item.first << ":" << one_item.second << ", ";
		}
		buffer << "}";
	}
	return buffer;
}
std::string to_string() const
{
	std::ostringstream buffer;
	buffer<<*this;
	return buffer.str();
}
	};
	//template <typename T>
	//typename std::enable_if<std::is_integral_v<T> && !std::is_same_v<bool, T> && !std::is_same_v<any_int_type, T>, any_value_type>::type 
	//	any_encode(const T& _in_value)
	//{
	//	return any_value_type(static_cast<any_int_type>(_in_value));
	//}
	//static any_value_type any_encode(const float& _in_value)
	//{
	//	return any_value_type(static_cast<double>(_in_value));
	//}
	template <typename T>
	typename std::enable_if< std::is_constructible_v<any_value_type, T>, any_value_type>::type 
		any_encode(const T& _in_value)
	{
		return any_value_type(_in_value);
	}
	template <typename T>
	typename std::enable_if< std::is_constructible_v<any_value_type, T>, any_value_type>::type 
		any_encode(const std::vector<T>& _in_value)
	{
		any_vector result;
		for (const auto& one_item : _in_value)
		{
			result.emplace_back(one_item);
		}
		return result;
	}
	template <typename T>
	typename std::enable_if< !std::is_constructible_v<any_value_type, T>, any_value_type>::type 
		any_encode(const std::vector<T>& _in_value)
	{
		any_vector result;
		for (const auto& one_item : _in_value)
		{
			result.emplace_back(any_encode(one_item));
		}
		return result;
	}
	template <typename T>
	typename std::enable_if< std::is_constructible_v<any_value_type, T>, any_value_type>::type 
		any_encode(const std::list<T>& _in_value)
	{
		any_vector result;
		for (const auto& one_item : _in_value)
		{
			result.emplace_back(one_item);
		}
		return result;
	}
	template <typename T>
	typename std::enable_if< !std::is_constructible_v<any_value_type, T>, any_value_type>::type 
		any_encode(const std::list<T>& _in_value)
	{
		any_vector result;
		for (const auto& one_item : _in_value)
		{
			result.emplace_back(any_encode(one_item));
		}
		return result;
	}
	template <typename T>
	typename std::enable_if< std::is_constructible_v<any_value_type, T>, any_value_type>::type
		any_encode(const std::unordered_set<T>& _in_value)
	{
		any_vector result;
		for (const auto& one_item : _in_value)
		{
			result.emplace_back(one_item);
		}
		return result;
	}
	template <typename T>
	typename std::enable_if< !std::is_constructible_v<any_value_type, T>, any_value_type>::type
		any_encode(const std::unordered_set<T>& _in_value)
	{
		any_vector result;
		for (const auto& one_item : _in_value)
		{
			result.emplace_back(any_encode(one_item));
		}
		return result;
	}
	// 无法做下面的两个特化 只能统一采取encode
	//template <typename T1, typename T2>
	//typename std::enable_if< std::is_constructible_v<any_value_type, T2> && std::is_integral_v<T1>, any_value_type>::type 
	//	any_encode(const std::unordered_map<T1, T2>& _in_value)
	//{
	//	any_int_map result;
	//	for (const auto& one_item : _in_value)
	//	{
	//		result.emplace(static_cast<any_int_type>(one_item.first), one_item.second);
	//	}
	//	return result;
	//}
	//template <typename T1, typename T2>
	//typename std::enable_if< std::is_constructible_v<any_value_type, T2> && std::is_integral_v<T1>, any_value_type>::type
	//	any_encode(const std::unordered_map<T1, T2>& _in_value)
	//{
	//	any_int_map result;
	//	for (const auto& one_item : _in_value)
	//	{
	//		result.emplace(static_cast<any_int_type>(one_item.first), any_encode(one_item.second));
	//	}
	//	return result;
	//}
	template <typename T1, typename T2>
	typename std::enable_if<std::is_integral_v<T1>, any_value_type>::type
		any_encode(const std::unordered_map<T1, T2>& _in_value)
	{
		any_int_map result;
		for (const auto& one_item : _in_value)
		{
			result.emplace(static_cast<any_int_type>(one_item.first), any_encode(one_item.second));
		}
		return result;
	}

	template <typename T>
	typename std::enable_if< std::is_constructible_v<any_value_type, T>, any_value_type>::type 
		any_encode(const std::unordered_map<std::string, T>& _in_value)
	{
		any_str_map result;
		for (const auto& one_item : _in_value)
		{
			result.emplace(one_item.first, one_item.second);
		}
		return result;
	}
	template <typename T>
	typename std::enable_if< !std::is_constructible_v<any_value_type, T>, any_value_type>::type 
		any_encode(const std::unordered_map<std::string, T>& _in_value)
	{
		any_str_map result;
		for (const auto& one_item : _in_value)
		{
			result.emplace(one_item.first, any_encode(one_item.second));
		}
		return result;
	}
	template <typename T1, typename T2>
	any_value_type any_encode(const std::pair<T1, T2>& _in_value)
	{
		any_vector result;
		result.push_back(any_encode(_in_value.first));
		result.push_back(any_encode(_in_value.second));
		return result;
	}
	template <typename... Args, std::size_t... index>
	any_value_type any_encode_tuple(const std::tuple<Args...>& _in_value, std::index_sequence<index...>)
	{
		any_vector result;
		(result.push_back(any_encode(std::get<index>(_in_value))),...);
		return result;
	}
	template <typename... Args>
	any_value_type any_encode(const std::tuple<Args...>& _in_value)
	{
		return any_encode_tuple(_in_value, std::index_sequence_for<Args...>{});
	}

	static any_value_type any_encode(const json& data)
	{
		if (data.is_null())
		{
			return any_value_type();
		}
		else if (data.is_string())
		{
			return data.get<std::string>();
		}
		else if (data.is_boolean())
		{
			return data.get<bool>();
		}
		else if (data.is_number_float())
		{
			return double(data.get<float>());
		}
		else if (data.is_number_integer())
		{
			return static_cast<any_int_type>(data.get<int>());
		}
		else if (data.is_number_unsigned())
		{
			return data.get<any_int_type>();
		}
		else if (data.is_array())
		{
			auto result = any_vector();
			for (const auto& one_item : data)
			{
				result.push_back(any_encode(one_item));
			}
			return result;
		}
		else if (data.is_object())
		{
			any_str_map result;
			for (auto& one_item : data.items())
			{
				auto& item_value = one_item.value();
				result[one_item.key()] = any_encode(item_value);

			}
			return result;
		}
		return any_value_type();
	}
	static bool any_decode(const any_value_type& data, any_value_type& dst)
	{
		dst = data;
		return true;
	}
	template <typename T>
	typename std::enable_if<std::is_arithmetic_v<T>, bool>::type
		any_decode(const any_value_type& data, T& dst)
	{
		return data.numeric_value(dst);
	}
	template <typename T>
	typename std::enable_if<!std::is_arithmetic_v<T> &&
		std::is_constructible_v<any_value_type, T>, bool>::type
		any_decode(const any_value_type& data, T& dst)
	{
		if (!std::holds_alternative<T>(data))
		{
			return false;
		}
		dst = std::get<T>(data);
		return true;
	}
	template <typename T>
	bool any_decode(const any_value_type& data, std::vector<T>& dst)
	{
		if (!std::holds_alternative<any_vector>(data))
		{
			return false;
			
		}
		auto& cur_vec = std::get<any_vector>(data);
		for (auto& one_item : cur_vec)
		{
			T temp;
			if (any_decode(one_item, temp))
			{
				dst.push_back(temp);
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	template <typename T, std::size_t N>
	bool any_decode(const any_value_type& data, std::array<T, N>& dst)
	{
		if (!std::holds_alternative<any_vector>(data))
		{
			return false;
		}
		auto& cur_vec = std::get<any_vector>(data);
		if (cur_vec.size() != N)
		{
			return false;
		}
		for (auto& one_item : cur_vec)
		{
			T temp;
			if (any_decode(one_item, temp))
			{
				dst.push_back(temp);
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	template <typename T>
	bool any_decode(const any_value_type& data, std::unordered_map<std::string, T>& dst)
	{
		if (!std::holds_alternative<any_str_map>(data))
		{
			return false;
		}
		auto& cur_map = std::get<any_str_map>(data);
		for (auto& one_item : cur_map)
		{
			T temp;
			if (any_decode(one_item.second, temp))
			{
				dst[one_item.first] = temp;
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	template <typename T1, typename T2>
	typename std::enable_if<std::is_integral_v<T1>, bool>::type 
		any_decode(const any_value_type& data, std::unordered_map<T1, T2>& dst)
	{
		if (!std::holds_alternative<any_int_map>(data))
		{
			return false;
		}
		auto& cur_map = std::get<any_int_map>(data);
		for (auto& one_item : cur_map)
		{
			T2 temp;
			if (any_decode(one_item.second, temp))
			{
				dst[static_cast<T1>(one_item.first)] = temp;
			}
			else
			{
				return false;
			}
		}
		return true;
	}

	template <typename T1, typename T2>
	bool any_decode(const any_value_type& data, std::pair<T1, T2>& dst)
	{
		if (!std::holds_alternative<any_vector>(data))
		{
			return false;
		}
		auto& cur_vec = std::get<any_vector>(data);
		if (cur_vec.size() != 2)
		{
			return false;
		}
		if (!any_decode(cur_vec[0], dst.first))
		{
			return false;
		}
		if (!any_decode(cur_vec[1], dst.second))
		{
			return false;
		}
		return true;
	}
	template <typename... Args>
	bool any_decode(const any_value_type& data, std::tuple<Args...>& dst)
	{
		if (!std::holds_alternative<any_vector>(data))
		{
			return false;
		}
		auto& cur_vec = std::get<any_vector>(data);
		if (cur_vec.size() != std::tuple_size<std::tuple<Args...>>::value)
		{
			return false;
		}
	}
	template <typename... Args, std::size_t... index>
	bool decode_for_tuple(const any_vector& data, std::tuple<Args...>& dst, std::index_sequence<index...>)
	{
		return (any_decode(data[index], std::get<index>(dst)) && ...);
	}

	static bool any_decode(const any_value_type& data, json& dst)
	{
		if (data.is_bool())
		{
			dst = std::get<bool>(data);
			return true;
		}
		else if (data.is_str())
		{
			dst = std::get<std::string>(data);
			return true;
		}
		else if (data.is_double())
		{
			dst = std::get<double>(data);
			return true;
		}
		else if (data.is_int())
		{
			dst = std::get<any_int_type>(data);
			return true;
		}
		else if (data.is_vector())
		{
			const auto& cur_vec = std::get<any_vector>(data);
			for (auto& one_item : cur_vec)
			{
				json temp;
				any_decode(one_item, temp);
				dst.push_back(temp);
			}
			return true;
		}
		else if (data.is_str_map())
		{
			const auto& cur_vec = std::get<any_str_map>(data);
			for (auto& one_item : cur_vec)
			{
				json temp;
				any_decode(one_item.second, temp);
				dst[one_item.first] = temp;
			}
			return true;
		}
		else if (data.is_int_map())
		{
			const auto& cur_vec = std::get<any_int_map>(data);
			for (auto& one_item : cur_vec)
			{
				json temp;
				any_decode(one_item.second, temp);
				dst[static_cast<any_int_type>(one_item.first)] = temp;
			}
			return true;
		}
		else
		{
			return true;
		}
	}
}

