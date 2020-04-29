//
//  clips.hpp
//  CLIPS_unicode
//
//  Created by chunlei.xiong on 13-11-12.
//  Copyright (c) 2013年 熊春雷. All rights reserved.
//
//  MUST be compiled by C++11 and later compiler.

//  Update at 2019-10-17:
//  1. update import user C++ function to CLIPS with 6.40 beta3 version.

#pragma once

#include <functional>
#include <cassert>
#include <vector>
#include <string>
#include <any>

namespace clips {
    /* The following codes are supported for return values and argument types:
       +------+---------------------------------------------------+
       | Code | Type                                              |
       +------+---------------------------------------------------+
       |  b   | Boolean                                           |
       |  d   | Double Precision Float                            |
       |  e   | External Address                                  |
       |  f   | Fact Address                                      |
       |  i   | Instance Address                                  |
       |  l   | Long Long Integer                                 |
       |  m   | Multifield                                        |
       |  n   | Instance Name                                     |
       |  s   | String                                            |
       |  y   | Symbol                                            |
       |  v   | Void—No Return Value                              |
       |  *   | Any Type                                          |
       +------+---------------------------------------------------+ */
    
    using integer           = long long;
    using real              = double;
    using boolean           = std::tuple<bool,        std::integral_constant<char,'b'>>;
    using string            = std::tuple<std::string, std::integral_constant<char,'s'>>;
    using symbol            = std::tuple<std::string, std::integral_constant<char,'y'>>;
    using instance_name     = std::tuple<std::string, std::integral_constant<char,'n'>>;
    //using external_address  = std::tuple<void*,       std::integral_constant<char,'e'>>;
    using multifield        = std::vector<std::any>;
//    inline std::ostream&operator<<(std::ostream&os, const string&x) { return os << std::get<0>(x); }
//    inline std::ostream&operator<<(std::ostream&os, const symbol&x) { return os << std::get<0>(x); }
    // ////////////////////////////////////////////////////////////////////////////////////////
    template<typename>struct type_code      {enum{value='*', expect_bits=0};};// * Any Type
    template<>struct type_code<bool>        {enum{value='b', expect_bits=BOOLEAN_BIT};};// b Boolean
    template<>struct type_code<boolean>     {enum{value='b', expect_bits=BOOLEAN_BIT};};// b Boolean

    template<>struct type_code<float>       {enum{value='d', expect_bits=FLOAT_BIT};};// d Double Precision Float
    template<>struct type_code<double>      {enum{value='d', expect_bits=FLOAT_BIT};};// d Double Precision Float
    template<>struct type_code<long double> {enum{value='d', expect_bits=FLOAT_BIT};};// d Double Precision Float

    template<>struct type_code</*     */char>      {enum{value='l', expect_bits=INTEGER_BIT};};// l Long Long Integer
    template<>struct type_code</*     */short>     {enum{value='l', expect_bits=INTEGER_BIT};};// l Long Long Integer
    template<>struct type_code</*     */int>       {enum{value='l', expect_bits=INTEGER_BIT};};// l Long Long Integer
    template<>struct type_code</*     */long>      {enum{value='l', expect_bits=INTEGER_BIT};};// l Long Long Integer
    template<>struct type_code</*     */long long> {enum{value='l', expect_bits=INTEGER_BIT};};// l Long Long Integer
    template<>struct type_code<unsigned char>      {enum{value='l', expect_bits=INTEGER_BIT};};// l Long Long Integer
    template<>struct type_code<unsigned short>     {enum{value='l', expect_bits=INTEGER_BIT};};// l Long Long Integer
    template<>struct type_code<unsigned int>       {enum{value='l', expect_bits=INTEGER_BIT};};// l Long Long Integer
    template<>struct type_code<unsigned long>      {enum{value='l', expect_bits=INTEGER_BIT};};// l Long Long Integer
    template<>struct type_code<unsigned long long> {enum{value='l', expect_bits=INTEGER_BIT};};// l Long Long Integer

    template<>struct type_code<      char*>   {enum{value='s', expect_bits=STRING_BIT|SYMBOL_BIT};};// s String, y Symbol
    template<>struct type_code<const char*>   {enum{value='s', expect_bits=STRING_BIT|SYMBOL_BIT};};// s String, y Symbol
    template<>struct type_code<std::string>   {enum{value='s', expect_bits=STRING_BIT|SYMBOL_BIT};};// s String, y Symbol
    template<>struct type_code<string>        {enum{value='s', expect_bits=STRING_BIT           };};// s String
    template<>struct type_code<symbol>        {enum{value='y', expect_bits=SYMBOL_BIT           };};// y Symbol
    template<>struct type_code<instance_name> {enum{value='n', expect_bits=INSTANCE_NAME_BIT    };};// n Instance Name
    template<>struct type_code<void>          {enum{value='v', expect_bits=VOID_BIT             };};// v Void—No Return Value
  //template<>struct type_code<double>        {enum{value='f', expect_bits=VOID_BIT             };};// f Fact Address
  //template<>struct type_code<short>         {enum{value='i', expect_bits=VOID_BIT             };};// i Instance Address
    template<>struct type_code<multifield>    {enum{value='m', expect_bits=MULTIFIELD_BIT       };};// m Multifield
    template<typename T>struct type_code<T*>  {enum{value='e', expect_bits=EXTERNAL_ADDRESS_BIT};};// e External Address

    template<typename T>struct type_code<const T>{ enum{
        value=type_code<T>::value,
        expect_bits=type_code<T>::expect_bits
    };};

    template<typename T>using   return_code = type_code<T>;
    template<typename T>using argument_code = type_code<T>;

    template<typename>struct argument;
#define CLIPS_ARGUMENT_VALUE(float, udfv_contents)                              \
/**/    static float value(Environment*CLIPS, UDFContext *udfc, unsigned i){    \
/**/        UDFValue udfv;                                                      \
/**/        UDFNthArgument(udfc, i, argument_code<float>::expect_bits, &udfv);  \
/**/        return udfv_contents;                                               \
/**/    }/* CLIPS_ARGUMENT_VALUE */
#define CLIPS_ARGUMENT_TEMPLATE(float, udfv_contents)                           \
/**/    template<>struct argument<float> {                                      \
/**/        CLIPS_ARGUMENT_VALUE(float, udfv_contents)                          \
/**/    };/* CLIPS_ARGUMENT_TEMPLATE */

    CLIPS_ARGUMENT_TEMPLATE(             float, /*                      */udfv.floatValue->contents)
    CLIPS_ARGUMENT_TEMPLATE(            double, /*                      */udfv.floatValue->contents)
    CLIPS_ARGUMENT_TEMPLATE(              char, /*                      */udfv.integerValue->contents)
    CLIPS_ARGUMENT_TEMPLATE(             short, /*                      */udfv.integerValue->contents)
    CLIPS_ARGUMENT_TEMPLATE(               int, static_cast<         int>(udfv.integerValue->contents))
    CLIPS_ARGUMENT_TEMPLATE(              long, /*                      */udfv.integerValue->contents)
    CLIPS_ARGUMENT_TEMPLATE(         long long, /*                      */udfv.integerValue->contents)
    CLIPS_ARGUMENT_TEMPLATE(unsigned      char, /*                      */udfv.integerValue->contents)
    CLIPS_ARGUMENT_TEMPLATE(unsigned     short, /*                      */udfv.integerValue->contents)
    CLIPS_ARGUMENT_TEMPLATE(unsigned       int, static_cast<unsigned int>(udfv.integerValue->contents))
    CLIPS_ARGUMENT_TEMPLATE(unsigned      long, /*                      */udfv.integerValue->contents)
    CLIPS_ARGUMENT_TEMPLATE(unsigned long long, /*                      */udfv.integerValue->contents)


    CLIPS_ARGUMENT_TEMPLATE(              bool,               udfv.lexemeValue != FalseSymbol(CLIPS))
    CLIPS_ARGUMENT_TEMPLATE(           boolean,       boolean{udfv.lexemeValue->contents})
    CLIPS_ARGUMENT_TEMPLATE(       const char*,               udfv.lexemeValue->contents )
    CLIPS_ARGUMENT_TEMPLATE(       std::string,   std::string{udfv.lexemeValue->contents})
    CLIPS_ARGUMENT_TEMPLATE(            string,        string{udfv.lexemeValue->contents})
    CLIPS_ARGUMENT_TEMPLATE(            symbol,        symbol{udfv.lexemeValue->contents})
    CLIPS_ARGUMENT_TEMPLATE(     instance_name, instance_name{udfv.lexemeValue->contents})
        
    template<class T>struct argument<T*> {
        CLIPS_ARGUMENT_VALUE(T*, udfv.externalAddressValue->contents);
    };
    template<class T>struct argument<const T> {
        static const T value(UDFContext *udfc, unsigned i){
            return argument<T>::value(udfc, i);
        }
    };
#undef CLIPS_ARGUMENT_VALUE
#undef CLIPS_ARGUMENT_TEMPLATE
    
    template<char code> struct create_primitive_value;
#define CLIPS_CREATE_PRIMITIVE_VALUE(code, CreateValue)         \
/**/    template<> struct create_primitive_value<code> {        \
/**/        template<typename R> static                         \
/**/        void apply(Environment*CLIPS, UDFValue*udfv, R&&x){ \
/**/            udfv->value = CreateValue/*(CLIPS, x)*/;        \
/**/        }                                                   \
/**/    };/* CLIPS_SELECT_ACTION */
    CLIPS_CREATE_PRIMITIVE_VALUE('b', CreateBoolean/*    */(CLIPS,std::get<0>(x)        ))
    CLIPS_CREATE_PRIMITIVE_VALUE('s', CreateString/*     */(CLIPS,std::get<0>(x).c_str()))
    CLIPS_CREATE_PRIMITIVE_VALUE('y', CreateSymbol/*     */(CLIPS,std::get<0>(x).c_str()))
    CLIPS_CREATE_PRIMITIVE_VALUE('n', CreateInstanceName   (CLIPS,std::get<0>(x).c_str()))
    CLIPS_CREATE_PRIMITIVE_VALUE('d', CreateFloat/*      */(CLIPS,x))
    CLIPS_CREATE_PRIMITIVE_VALUE('l', CreateInteger/*    */(CLIPS,x))
    CLIPS_CREATE_PRIMITIVE_VALUE('e', CreateExternalAddress(CLIPS,x))
#undef CLIPS_CREATE_PRIMITIVE_VALUE
    
    template<typename R, typename ... Args>struct build_arguments_code;
    template<typename R, typename A1, typename ... Args>
    struct build_arguments_code<R, A1, Args...> {
        static const std::string& apply(std::string&code, unsigned long i) {
            if (i <= 1) { code = "*"; }
            std::string currentCode = ";";
            if constexpr(std::is_same_v<const char*, A1> || std::is_same_v<char*, A1> ||
                         std::is_same_v<std::string, std::remove_reference_t<std::remove_cv_t<A1>>>) {
                currentCode += argument_code<clips::string>::value;
                currentCode += argument_code<clips::symbol>::value;
            } else {
                currentCode += argument_code<A1>::value;
            }
            return build_arguments_code<R,Args...>::apply(code+=currentCode, i+std::size(currentCode));
        }
    };
    template<typename R>struct build_arguments_code<R> {
        static const std::string& apply(std::string&code, unsigned long i) {
            return code;
        }
    };
 
    namespace __private{
        template<typename...i>struct m {};
        template<unsigned...i>struct n { enum { value=sizeof...(i), max_value=0xF}; };
        template<unsigned>struct build_n;
        template<>struct build_n<0x1> { typedef n<0x1> type; };
        template<>struct build_n<0x2> { typedef n<0x1,0x2> type; };
        template<>struct build_n<0x3> { typedef n<0x1,0x2,0x3> type; };
        template<>struct build_n<0x4> { typedef n<0x1,0x2,0x3,0x4> type; };
        template<>struct build_n<0x5> { typedef n<0x1,0x2,0x3,0x4,0x5> type; };
        template<>struct build_n<0x6> { typedef n<0x1,0x2,0x3,0x4,0x5,0x6> type; };
        template<>struct build_n<0x7> { typedef n<0x1,0x2,0x3,0x4,0x5,0x6,0x7> type; };
        template<>struct build_n<0x8> { typedef n<0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8> type; };
        template<>struct build_n<0x9> { typedef n<0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9> type; };
        template<>struct build_n<0xA> { typedef n<0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA> type; };
        template<>struct build_n<0xB> { typedef n<0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB> type; };
        template<>struct build_n<0xC> { typedef n<0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC> type; };
        template<>struct build_n<0xD> { typedef n<0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD> type; };
        template<>struct build_n<0xE> { typedef n<0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE> type; };
        template<>struct build_n<0xF> { typedef n<0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF> type; };
        
        template<typename R, typename, typename A,typename B>struct _invoke;
        template<typename R, typename...Args, int...i> struct _invoke<R, std::integral_constant<int, 0>, m<Args...>, n<i...>> {
            static R _function(Environment*CLIPS,std::function<R(Args...)>&&theFunc, UDFContext*udfc/*=nullptr*/) {
                return theFunc(argument<Args>::value(CLIPS, udfc, i)...);
            }
        };
        template<typename R, typename...Args, int...i> struct _invoke<R, Environment*, m<Args...>, n<i...>> {
            static R _function(Environment*CLIPS,std::function<R(Environment*, Args...)>&&theFunc, UDFContext*udfc/*=nullptr*/) {
                return theFunc(CLIPS, argument<Args>::value(CLIPS, udfc, i)...);
            }
        };
    }
    
    template<typename R, typename...Args>
    R invoke_function(Environment*CLIPS,std::function<R(Args...)>&&lambda, UDFContext*udfc/*=nullptr*/) {
        using namespace __private;
        static_assert(sizeof...(Args)<=n<>::max_value, "Error: Only support max 15 arguments in CLIPS's C function");
        return _invoke<R, std::integral_constant<int,0>, m<Args...>, typename build_n<sizeof...(Args)>::type>
        /*  */ ::_function(CLIPS, std::move(lambda), udfc);
    }
    template<typename R, typename...Args>
    R invoke_function(Environment*CLIPS,std::function<R(Environment*,Args...)>&&lambda, UDFContext*udfc/*=nullptr*/) {
        using namespace __private;
        static_assert(sizeof...(Args)<=n<>::max_value, "Error: Only support max 15 arguments in CLIPS's C function");
        return _invoke<R, Environment*, m<Args...>, typename build_n<sizeof...(Args)>::type>
        /*  */ ::_function(CLIPS, std::move(lambda), udfc);
    }

    template<typename R>
    R invoke_function(Environment*CLIPS,std::function<R(Environment*)>&&lambda, UDFContext*udfc/*=nullptr*/) {
        return lambda(CLIPS);
    }

    template<typename R>
    R invoke_function(Environment*CLIPS,std::function<R()>&&lambda, UDFContext*udfc/*=nullptr*/) {
        return lambda();
    }
    
    template<unsigned i, class R, class...Args> struct is_void_return {
        static std::function<R(Args...)> lambda;
        static void f(Environment*CLIPS, UDFContext *udfc, UDFValue*udfv) {
            enum { code = return_code<R>::value };
            create_primitive_value<code>::apply(CLIPS, udfv, invoke_function(CLIPS, std::move(lambda), udfc));
        }
    };
    template<unsigned i, class...Args> struct is_void_return<i, void, Args...> {
        static std::function<void(Args...)>lambda;
        static void f(Environment*CLIPS, UDFContext *udfc, UDFValue*udfv) {
            invoke_function(CLIPS,std::move(lambda), udfc);
        }
    };
    template<unsigned i,class R,class...Args> struct is_void_return<i, R, Environment*, Args...> {
        static std::function<R(Environment*, Args...)> lambda;
        static void f(Environment*CLIPS, UDFContext *udfc, UDFValue*udfv) {
            enum { code = return_code<R>::value };
            create_primitive_value<code>::apply(CLIPS, udfv, invoke_function(CLIPS, std::move(lambda), udfc));
        }
    };
    template<unsigned i,class...Args> struct is_void_return<i, void, Environment*, Args...> {
        static std::function<void(Environment*, Args...)>lambda;
        static void f(Environment*CLIPS, UDFContext *udfc, UDFValue*udfv) {
            invoke_function(CLIPS,std::move(lambda), udfc);
        }
    };
    
    template<unsigned i, typename R, typename...Args> std::function<   R(/*          */Args...)> is_void_return<i,   R,Args...>::lambda;
    template<unsigned i, /*       */ typename...Args> std::function<void(/*          */Args...)> is_void_return<i,void,Args...>::lambda;
    template<unsigned i, typename R, typename...Args> std::function<   R(Environment*, Args...)> is_void_return<i,    R, Environment*, Args...>::lambda;
    template<unsigned i, /*       */ typename...Args> std::function<void(Environment*, Args...)> is_void_return<i, void, Environment*, Args...>::lambda;
    template<unsigned i, typename R, typename...Args>
    void user_function(Environment*CLIPS, const char*name, std::function<R(Args...)>lambda, void*context=nullptr) {
        using namespace __private;
        std::string argumentsCode = "*";
        build_arguments_code<R, Args...>::apply(argumentsCode, 1);
        char returnCode[2] = {return_code<R>::value, '\0'};
        using UDF = is_void_return<i,R,Args...>;
        UDF::lambda = lambda;
        AddUDFError ok = \
        AddUDF(/* Environment*                   theEnv = */CLIPS,
               /* const char *        clipsFunctionName = */name,
               /* const char *              returnTypes = */returnCode,
               /* unsigned short                minArgs = */sizeof...(Args),
               /* unsigned short                maxArgs = */sizeof...(Args),
               /* const char *            argumentTypes = */argumentsCode.c_str(),
               /* UserDefinedFunction *cFunctionPointer = */UDF::f,
               /* const char *            cFunctionName = */name,
               /* void *                        context = */context);
        assert(AUE_NO_ERROR == ok);
    };
    template<unsigned i, typename R, typename...Args>
    void user_function(Environment*CLIPS, const char*name, std::function<R(Environment*, Args...)>lambda, void*context=nullptr) {
        using namespace __private;
        std::string argumentsCode = "*";
        build_arguments_code<R, Args...>::apply(argumentsCode, 1);// 参数数量和上面的不一样
        char returnCode[2] = {return_code<R>::value, '\0'};
        using UDF = is_void_return<i, R, Environment*, Args...>; // 这里也和上面不一样
        UDF::lambda = lambda;
        AddUDFError ok = \
        AddUDF(/* Environment*                   theEnv = */CLIPS,
               /* const char *        clipsFunctionName = */name,
               /* const char *              returnTypes = */returnCode,
               /* unsigned short                minArgs = */sizeof...(Args),
               /* unsigned short                maxArgs = */sizeof...(Args),
               /* const char *            argumentTypes = */argumentsCode.c_str(),
               /* UserDefinedFunction *cFunctionPointer = */UDF::f,
               /* const char *            cFunctionName = */name,
               /* void *                        context = */context);
        assert(AUE_NO_ERROR == ok);
    };
    
    template<unsigned i, typename R, typename...Args>
    void user_function(Environment*CLIPS,const char*name, R(*f)(Args...), void*context=nullptr) {
        user_function<i>(CLIPS, name, std::function<R(Args...)>(f), context);
    };

}//namespace clips

namespace clips {

    class CLIPS {
        std::shared_ptr<Environment> env;
    public:
        CLIPS():CLIPS(nullptr) {}
        explicit CLIPS(Environment*environment) {
            if (nullptr == environment) {
                this->env = std::shared_ptr<Environment>(CreateEnvironment(), [](Environment*x){
                    DestroyEnvironment(x);
                });
            } else {
                this->env = std::shared_ptr<Environment>(environment, [](Environment*x){
                    
                });
            }
        }
        operator Environment*() { return this->env.get(); }
        inline bool clear() { return Clear(env.get()); }
        inline void reset() { Reset(env.get()); }
        inline long long run(long long steps = -1) { return Run(env.get(), steps); }
        inline bool load(const char*file) {
            LoadError ok = Load(env.get(), file);
            assert(LE_NO_ERROR == ok);
            return LE_NO_ERROR == ok;
        }
        inline bool load_from_string(const char*script) {
            return LoadFromString(env.get(), script, std::strlen(script));
        }
        inline bool batch(const char*file) {
            return Batch(env.get(), file);
        }
        inline bool batch_star(const char*file) {
            return BatchStar(env.get(), file);
        }
        inline void build(const char*script) {
            BuildError ok = Build(env.get(), script);
            assert(BE_NO_ERROR == ok);
        }
        inline std::any eval(const char*script) {
            std::any ret;
            CLIPSValue value;
            EvalError ok = Eval(env.get(), script, &value);
            assert(EE_NO_ERROR == ok);
            if (INTEGER_TYPE == value.header->type) {
                ret = value.integerValue->contents;
            }
            if (FLOAT_TYPE == value.header->type) {
                ret = value.floatValue->contents;
            }
            if (SYMBOL_TYPE == value.header->type) {
                ret = clips::symbol{value.lexemeValue->contents};
            }
            if (STRING_TYPE == value.header->type) {
                ret = clips::string{value.lexemeValue->contents};
            }
            if (INSTANCE_NAME_TYPE == value.header->type) {
                ret = clips::instance_name{value.lexemeValue->contents};
            }
            if (EXTERNAL_ADDRESS_TYPE == value.header->type) {
                ret = value.externalAddressValue->contents;
            }
            return ret;
        }
    };
}//namespace clips

#ifndef CLIPS_EXTENSION_TEST_BENCH_ENABLED
#   define CLIPS_EXTENSION_TEST_BENCH_ENABLED 1
#endif//CLIPS_EXTENSION_TEST_BENCH_ENABLED

#ifndef CLIPS_EXTENSION_UTILITY_ENABLED
#   define CLIPS_EXTENSION_UTILITY_ENABLED 1
#endif//CLIPS_EXTENSION_UTILITY_ENABLED

#ifndef CLIPS_EXTENSION_SOCKET_ENABLED
#   define CLIPS_EXTENSION_SOCKET_ENABLED 1
#endif//CLIPS_EXTENSION_SOCKET_ENABLED

#ifndef CLIPS_EXTENSION_ZEROMQ_ENABLED
#   define CLIPS_EXTENSION_ZEROMQ_ENABLED 1
#endif//CLIPS_EXTENSION_ZEROMQ_ENABLED

#ifndef CLIPS_EXTENSION_MUSTACHE_ENABLED
#   define CLIPS_EXTENSION_MUSTACHE_ENABLED 1
#endif//CLIPS_EXTENSION_MUSTACHE_ENABLED

#ifndef CLIPS_EXTENSION_PROCESS_ENABLED
#   define CLIPS_EXTENSION_PROCESS_ENABLED 1
#endif//CLIPS_EXTENSION_PROCESS_ENABLED

namespace clips::extension {

#if CLIPS_EXTENSION_TEST_BENCH_ENABLED
    void test_benchmark_initialize(Environment*environment);
    void test_benchmark();
#endif//CLIPS_EXTENSION_TEST_BENCH_ENABLED

#if CLIPS_EXTENSION_UTILITY_ENABLED
    void utility_initialize(Environment*environment);
#endif//CLIPS_EXTENSION_UTILITY_ENABLED

#if CLIPS_EXTENSION_SOCKET_ENABLED
    void socket_initialize(Environment*environment);
#endif// CLIPS_EXTENSION_SOCKET_ENABLED

#if CLIPS_EXTENSION_ZEROMQ_ENABLED
    void zeromq_initialize(Environment*environment);
#endif// CLIPS_EXTENSION_ZEROMQ_ENABLED

#if CLIPS_EXTENSION_MUSTACHE_ENABLED
    void          mustache_initialize(Environment*environment);
    clips::string mustache_trim(const char*input);
    clips::string mustache_render(Environment*environment, const char*VIEW, const char*CONTEXT);
    clips::string mustache_render_with_partials(Environment*environment, const char* VIEW, const char* CONTEXT, const char*PARTIALS);
#endif// CLIPS_EXTENSION_MUSTACHE_ENABLED

#if CLIPS_EXTENSION_PROCESS_ENABLED
    void process_initialize(Environment*environment);
#endif// CLIPS_EXTENSION_PROCESS_ENABLED

}// namespace clips::extension {


