/*
 * LuaString.cpp
 *
 *  Created on: Jan 31, 2019
 *      Author: chorm
 */

#include <LuaValue.hpp>

#include <set>
#include <tuple>

using namespace lua;

static std::set<std::string>& get_intern_set(){
	static std::set<std::string> intern;
	return intern;
}

static const std::string& intern(const std::string& str){
	std::set<std::string>::iterator element;
	std::tie(element,std::ignore) = get_intern_set().insert(str);
	return *element;
}

static const std::string& intern(std::string&& str){
	std::set<std::string>::iterator element;
	std::tie(element,std::ignore) = get_intern_set().insert(std::move(str));
	return *element;
}

LuaString::LuaString(const std::string& str):value{intern(str)}{}
LuaString::LuaString(std::string&& str):value{intern(std::move(str))}{}

LuaString::operator const std::string&()const{
	return value.get();
}

bool LuaString::operator==(const LuaString& str)const{
	return value.get()==str.value.get();
}

std::size_t LuaString::hashcode()const{
	return ::hashcode(value.get());
}

LuaString LuaString::concat(const LuaString& str)const{
	return static_cast<const std::string&>(*this)+static_cast<const std::string&>(str);
}
