/*
 * LuaBase.cpp
 *
 *  Created on: Jan 31, 2019
 *      Author: chorm
 */

#include <LuaBase.hpp>

using namespace lua;

LuaValue::LuaValue(const LuaValue& rhs):info{rhs.info},target{}{
	if(info){
		if(info->use_sbo)
			target = memory;
		else
			target = operator new(info->size);
		info->copyctor(target,rhs.target);
	}
}
LuaValue::LuaValue(LuaValue&& rhs)noexcept:info{rhs.info},target{}{
	if(info){
		if(info->use_sbo){
			target = memory;
			info->movector(target,rhs.target);
		}else
			target = std::exchange(rhs.target,nullptr);
	}
}
LuaValue::~LuaValue(){
	if(target){
		info->dtor(target);
		if(!info->use_sbo)
			operator delete(target);
	}
}

LuaValue& LuaValue::operator=(const LuaValue& rhs){
	if(this==&rhs)
		return *this;
	if(!target){
		if(rhs.target){
			info = rhs.info;
			if(info->use_sbo)
				target = memory;
			else
				target = operator new(info->size);
			info->copyctor(target,rhs.target);
		}
	}else if(!rhs.target){
		info->dtor(target);
		if(info->use_sbo)
			operator delete(target);
		target = nullptr;
	}else if(info->type_id==rhs.info->type_id)
		info->copyassign(target,rhs.target);
	else{
		info->dtor(target);
		if(info->use_sbo)
			operator delete(target);
		info = rhs.info;
		if(info->use_sbo)
			target = memory;
		else
			target = operator new(info->size);
		info->copyctor(target,rhs.target);
	}
	return *this;
}

LuaValue& LuaValue::operator=(LuaValue&& v){
	if(!target&&!v.target)
		return *this;
	else if(!v.target){
		info->dtor(target);
		if(!info->use_sbo)
			operator delete(target);
		target = nullptr;
	}else{
		if(info->use_sbo&&v.info->type_id==info->type_id)
			info->moveassign(target,v.target);
		else{
			info->dtor(target);
			if(!info->use_sbo)
				operator delete(target);

			info = v.info;
			if(info->use_sbo){
				target = memory;
				info->movector(target,v.target);
			}else
				target = std::exchange(v.target,nullptr);

		}
	}
	return *this;
}

LuaValue::operator bool()const{
	if(!target)
		return false;
	else
		return info->cvt_bool(target);
}

LuaValue LuaValue::operator&&(const LuaValue& v)const{
	if(*this)
		return v;
	else
		return *this;
}
LuaValue LuaValue::operator||(const LuaValue& v)const{
	if(*this)
		return *this;
	else
		return v;
}


LuaVarargs::LuaVarargs(const LuaValue& v){
	arr.push_back(v);
}
LuaVarargs::LuaVarargs(LuaValue&& v){
	arr.push_back(std::move(v));
}
LuaVarargs::LuaVarargs(std::initializer_list<LuaValue> v):arr{v}{}
LuaVarargs::LuaVarargs(LuaVarargs args1,LuaVarargs args2):arr{std::move(args1.arr)}{
	for(LuaValue& v:args2)
		arr.push_back(std::move(v));
}
LuaVarargs::LuaVarargs(LuaVarargs args,LuaValue v2):arr{std::move(args.arr)}{
	arr.push_back(std::move(v2));
}
LuaVarargs::LuaVarargs(LuaValue v,LuaVarargs args){
	arr.push_back(std::move(v));
	for(LuaValue& v:args)
		arr.push_back(std::move(v));
}
LuaVarargs::operator LuaValue()const{
	if(empty)
		return LuaValue{};
	else
		return *begin();
}
LuaVarargs::operator bool()const{
	return static_cast<bool>(static_cast<LuaValue>(*this));
}
LuaVarargs::size_type LuaVarargs::size()const{
	return arr.size();
}
bool LuaVarargs::empty()const{
	return arr.empty();
}
LuaVarargs::reference LuaVarargs::operator[](size_type idx){
	return arr[idx];
}
LuaVarargs::const_reference LuaVarargs::operator[](size_type idx)const{
	return arr[idx];
}
LuaVarargs::iterator LuaVarargs::begin(){
	return arr.begin();
}
LuaVarargs::const_iterator LuaVarargs::begin()const{
	return arr.begin();
}
LuaVarargs::const_iterator LuaVarargs::cbegin()const{
	return arr.cbegin();
}
LuaVarargs::iterator LuaVarargs::end(){
	return arr.end();
}
LuaVarargs::const_iterator LuaVarargs::end()const{
	return arr.end();
}
LuaVarargs::const_iterator LuaVarargs::cend()const{
	return arr.cend();
}
LuaVarargs::reverse_iterator LuaVarargs::rbegin(){
	return arr.rbegin();
}
LuaVarargs::const_reverse_iterator LuaVarargs::rbegin()const{
	return arr.rbegin();
}
LuaVarargs::const_reverse_iterator LuaVarargs::crbegin()const{
	return arr.crbegin();
}
LuaVarargs::reverse_iterator LuaVarargs::rend(){
	return arr.rend();
}
LuaVarargs::const_reverse_iterator LuaVarargs::rend()const{
	return arr.rend();
}
LuaVarargs::const_reverse_iterator LuaVarargs::crend()const{
	return arr.crend();
}

