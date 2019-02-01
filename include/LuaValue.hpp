/*
 * LuaValue.hpp
 *
 *  Created on: Jan 31, 2019
 *      Author: chorm
 */

#ifndef INCLUDE_LUAVALUE_HPP__2019_01_31_11_04_25
#define INCLUDE_LUAVALUE_HPP__2019_01_31_11_04_25

#include <utility>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <typeinfo>
#include <lclib/TypeTraits.hpp>
#include <LuaBase.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <limits>
#include <functional>
#include <lclib/Hash.hpp>

namespace lua{
	struct LuaNil{
	public:
		constexpr static LuaNil NIL{};
		constexpr static std::uint32_t TYPE{0};
		constexpr static char TYPE_NAME[]{"nil"};
		constexpr LuaNil()=default;
		constexpr LuaNil(const LuaNil&)=default;
		constexpr LuaNil(LuaNil&&)=default;
		constexpr LuaNil& operator=(const LuaNil&)=default;
		constexpr LuaNil& operator=(LuaNil&&)=default;
		constexpr explicit operator bool()const{
			return false;
		}
		constexpr std::size_t hashcode()const{
			return 0;
		}
		constexpr friend bool operator==(const LuaNil&,const LuaNil&){
			return true;
		}
	};

	struct LuaNumber{
	private:
		double value;
	public:
		constexpr static std::uint32_t TYPE{1};
		constexpr static char TYPE_NAME[]{"number"};
		constexpr LuaNumber():value{}{}
		constexpr LuaNumber(double val):value{val}{}
		constexpr LuaNumber(const LuaNumber&)=default;
		constexpr LuaNumber(LuaNumber&&)=default;
		constexpr LuaNumber& operator=(const LuaNumber&)=default;
		constexpr LuaNumber& operator=(LuaNumber&&)=default;
		constexpr explicit operator bool()const{
			return true;
		}
		constexpr friend bool operator==(const LuaNumber& n1,const LuaNumber& n2){
			return n1.value==n2.value;
		}
		constexpr std::size_t hashcode(){
			return ::hashcode(value);
		}
		constexpr static LuaNumber ZERO{};
		constexpr static LuaNumber ONE{1};
		constexpr static LuaNumber HUGE{std::numeric_limits<double>::infinity()};
	};

	struct LuaBoolean{
	private:
		bool val;
	public:
		constexpr static std::uint32_t TYPE{2};
		constexpr static char TYPE_NAME[]{"boolean"};
		constexpr LuaBoolean():val{}{}
		constexpr LuaBoolean(const LuaBoolean&)=default;
		constexpr LuaBoolean(LuaBoolean&&)=default;
		constexpr LuaBoolean& operator=(const LuaBoolean&)=default;
		constexpr LuaBoolean& operator=(LuaBoolean&&)=default;
		constexpr explicit operator bool()const{
			return val;
		}
		constexpr friend bool operator==(const LuaBoolean& v1,const LuaBoolean& v2){
			return v1.val==v2.val;
		}
		constexpr std::size_t hashcode()const{
			return val?1337:1331;
		}
		constexpr static LuaNumber TRUE{true};
		constexpr static LuaNumber FALSE{false};
	};

	struct LuaString{
	private:
		std::reference_wrapper<const std::string> value;
	public:
		LuaString();
		LuaString(const LuaString&)=default;
		LuaString(LuaString&&)noexcept=default;
		LuaString& operator=(const LuaString&)=default;
		LuaString& operator=(LuaString&&)=default;
		LuaString(const std::string&);
		LuaString(std::string&&);
		explicit operator bool()const;
		explicit operator const std::string&()const;
		bool operator==(const LuaString&)const;
		std::size_t hashcode()const;
		LuaString concat(const LuaString&)const;
	};
}

#endif /* INCLUDE_LUAVALUE_HPP__2019_01_31_11_04_25 */
