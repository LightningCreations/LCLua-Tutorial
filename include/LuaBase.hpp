/*
 * LuaBase.hpp
 *
 *  Created on: Jan 31, 2019
 *      Author: chorm
 */

#ifndef INCLUDE_LUABASE_HPP__2019_01_31_10_03_03
#define INCLUDE_LUABASE_HPP__2019_01_31_10_03_03

#include <utility>
#include <functional>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <typeinfo>
#include <lclib/TypeTraits.hpp>
#include <lclib/DynamicArray.hpp>

namespace lua{
	struct lua_value{
	public:
		static constexpr std::uint32_t TYPE{0};
		static constexpr char TYPE_NAME[]{""};
		lua_value();
		lua_value(const lua_value&);
		lua_value(lua_value&&);
		lua_value& operator=(const lua_value&);
		lua_value& operator=(lua_value&&);
		explicit operator bool()const;
	};

	template<typename T> using lua_value_type = std::void_t<
				decltype(T::TYPE),
				decltype(T::TYPE_NAME),
				std::enable_if_t<
					std::is_default_constructible_v<T>&&
					std::is_copy_constructible_v<T>&&
					std::is_move_constructible_v<T>&&
					std::is_copy_assignable_v<T>&&
					std::is_move_assignable_v<T>&&
					std::is_constructible_v<bool,T>&&
					(alignof(T)<=alignof(std::max_align_t))
				>,
				decltype(std::declval<const T>()==std::declval<const T>()),
				decltype(std::declval<const T>().hashcode())
			>;

	struct lua_value_info{
	private:
		using default_constructor_t = void(void*);
		using copy_constructor_t = void(void*,const void*);
		using move_constructor_t = void(void*,void*);
		using destructor_t = void(void*)noexcept;
		using hashcode_t = std::size_t(const void*);
		using equals_t = bool(const void*,const void*);
		template<typename T> using conversion_fn_t = T(const void*);

		template<typename T> static void default_construct(void* memory){
			new(memory) T{};
		}
		template<typename T> static void copy_construct(void* memory,const void* obj){
			new(memory) T{*static_cast<const T*>(obj)};
		}
		template<typename T> static void move_construct(void* memory,void* obj){
			new(memory) T{std::move(*static_cast<T*>(obj))};
		}
		template<typename T> static void copy_assign(void* lhs,const void* rhs){
			*static_cast<T*>(lhs) = *static_cast<const T*>(rhs);
		}
		template<typename T> static void move_assign(void* lhs,void* rhs){
			*static_cast<T*>(lhs) = std::move(*static_cast<T*>(rhs));
		}
		template<typename T> static bool convert_to_bool(const void* obj){
			return static_cast<bool>(*static_cast<const T*>(obj));
		}
		template<typename T> static void destroy(void* obj){
			static_cast<T*>(obj)->~T();
		}
		template<typename T> static std::size_t hashcode(const void* obj){
			return static_cast<const T*>(obj)->hashcode();
		}
		template<typename T> static bool equals(const void* v1,const void* v2){
			return *static_cast<const T*>(v1)==*static_cast<const T*>(v2);
		}
	public:
		std::size_t size;
		std::uint32_t type_id;
		const char* type_name;
		default_constructor_t* defaultctor;
		copy_constructor_t* copyctor;
		move_constructor_t* movector;
		copy_constructor_t* copyassign;
		move_constructor_t* moveassign;
		conversion_fn_t<bool>* cvt_bool;
		destructor_t* dtor;
		bool use_sbo;
		hashcode_t* hash;
		equals_t* eq;
		template<typename T,typename=lua_value_type<T>> lua_value_info(std::in_place_type_t<T>)
			:size{sizeof(T)},
			 type_id{T::TYPE},type_name{T::TYPE_NAME},
			 defaultctor{&default_construct<T>},
			 copyctor{&copy_construct<T>},
			 movector{&move_construct<T>},
			 copyassign{&copy_assign<T>},
			 moveassign{&move_assign<T>},
			 cvt_bool{&convert_to_bool<T>},
			 dtor{&destroy<T>},
			 use_sbo{sizeof(T)<64&&std::is_nothrow_move_constructible_v<T>},
			 hash{&hashcode<T>},
			 eq{&equals<T>}{}
	};
	template<typename T,typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>>> const lua_value_info& get_info(){
		const static lua_value_info info{std::in_place_type<T>};
		return info;
	}
	struct LuaValue{
	private:
		alignas(alignof(std::max_align_t)) unsigned char memory[64];
		void* target;
		const lua_value_info* info;
	public:
		template<typename T,typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>>>
			LuaValue(T&& t)noexcept(std::is_nothrow_move_constructible_v<T>):info{&get_info<T>()}{
				if(info->use_sbo)
					target = memory;
				else
					target = operator new(sizeof(T));
				info->movector(target,&t);
			}
		template<typename T,typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>>>
			LuaValue(const T& t)noexcept(std::is_nothrow_copy_constructible_v<T>):info{&get_info<T>()},memory{}{
				if(info->use_sbo)
					target = memory;
				else
					target = operator new(sizeof(T));
				info->copyctor(target,&t);
			}
		constexpr LuaValue()noexcept:info{},memory{},target{}{}

		template<typename T,typename... Args,
		typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>&&std::is_constructible_v<T,Args&&...>>>
		LuaValue(std::in_place_type_t<T>,Args&&... args)noexcept(std::is_nothrow_constructible_v<T,Args&&...>):info{&get_info<T>()},memory{},target{}{
			if(info->use_sbo)
				target = memory;
			else
				target = operator new(sizeof(T));
			new(target) T(std::forward<Args>(args)...);
		}
		LuaValue(const lua_value_info&);
		LuaValue(const LuaValue&);
		LuaValue(LuaValue&&)noexcept;
		~LuaValue();
		LuaValue& operator=(const LuaValue&);
		LuaValue& operator=(LuaValue&&);
		explicit operator bool()const;

		LuaValue operator&&(const LuaValue&)const;
		LuaValue operator||(const LuaValue&)const;

		template<typename T,typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>>>
			explicit operator T*()noexcept{
				if(info==&get_info<T>())
					return static_cast<T*>(target);
				else
					return nullptr;
			}
		template<typename T,typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>>>
			explicit operator const T*()const noexcept{
				if(info==&get_info<T>())
					return static_cast<const T*>(target);
				else
					return nullptr;
			}
		template<typename T,typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>>>
			explicit operator T&(){
				if(target&&info==&get_info<T>())
					return *static_cast<T*>(target);
				else
					throw std::bad_cast{};
			}
		template<typename T,typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>>>
			explicit operator const T&()const{
				if(target&&info==&get_info<T>())
					return *static_cast<const T*>(target);
				else
					throw std::bad_cast{};
			}
		bool operator==(const LuaValue&)const;
		std::size_t hashcode()const;
	};

	struct LuaVarargs{
	private:
		std::vector<LuaValue> arr;
	public:
		using element_type = LuaValue;
		using pointer = LuaValue*;
		using const_pointer = const LuaValue*;
		using reference = LuaValue&;
		using const_reference = const LuaValue&;
		using iterator = std::vector<LuaValue>::iterator;
		using const_iterator = std::vector<LuaValue>::const_iterator;
		using reverse_iterator = std::vector<LuaValue>::reverse_iterator;
		using const_reverse_iterator = std::vector<LuaValue>::const_reverse_iterator;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		LuaVarargs()=default;
		LuaVarargs(const LuaValue&);
		LuaVarargs(LuaValue&&);
		LuaVarargs(std::initializer_list<LuaValue>);
		LuaVarargs(LuaVarargs,LuaVarargs);
		LuaVarargs(LuaVarargs,LuaValue);
		LuaVarargs(LuaValue,LuaVarargs);
		LuaVarargs(const LuaVarargs&)=default;
		LuaVarargs(LuaVarargs&&)=default;
		LuaVarargs& operator=(const LuaVarargs&)=default;
		LuaVarargs& operator=(LuaVarargs&&)=default;
		explicit operator LuaValue()const;
		explicit operator bool()const;
		size_type size()const;
		bool empty()const;
		reference operator[](size_type);
		const_reference operator[](size_type)const;
		iterator begin();
		const_iterator begin()const;
		const_iterator cbegin()const;
		iterator end();
		const_iterator end()const;
		const_iterator cend()const;
		reverse_iterator rbegin();
		const_reverse_iterator rbegin()const;
		const_reverse_iterator crbegin()const;
		reverse_iterator rend();
		const_reverse_iterator rend()const;
		const_reverse_iterator crend()const;
		template<typename T,typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>>>
			explicit operator T*()noexcept{
				if(empty())
					return nullptr;
				else
					return static_cast<T*>(*begin());
			}
		template<typename T,typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>>>
			explicit operator const T*()const noexcept{
				if(empty())
					return nullptr;
				else
					return static_cast<const T*>(*begin());
			}
		template<typename T,typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>>>
			T* get_value(size_type idx){
			if(size()<=idx)
				return nullptr;
			else
				return static_cast<T*>((*this)[idx]);
		}
		template<typename T,typename=std::enable_if_t<std::is_constructible_v<lua_value_info,std::in_place_type_t<T>>>>
			const T* get_value(size_type idx)const{
			if(size()<=idx)
				return nullptr;
			else
				return static_cast<const T*>((*this)[idx]);
		}
	};
}

namespace std{
	template<> struct hash<lua::LuaValue>{
		public:
			constexpr hash()=default;
			constexpr hash(const hash&)=default;
			constexpr hash(hash&&)=default;
			constexpr hash& operator=(const hash&)=default;
			constexpr hash& operator=(hash&&)=default;
			inline std::size_t operator()(const lua::LuaValue& v){
				return v.hashcode();
			}
	};
}


#endif /* INCLUDE_LUABASE_HPP__2019_01_31_10_03_03 */
