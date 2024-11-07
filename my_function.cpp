/*
my std::function implementation for training purposes

Инициализировать объект указателем на функцию или объектом-функцией и вызвать оператор () - Есть
Поддержка операторов присваивания и копирующего конструктора
Поддержка указателей на функции-члены - Есть
*/

#include <iostream>
#include <memory>

using std::cout;
using std::endl;
using std::unique_ptr;


//general template class is empty (it needed only for declaration)
template <typename R, typename... Args>
class MyFunction{}; 


//specialization of template class is doing all job
template <typename R, typename... Args>
class MyFunction<R(Args...)>  { 

  struct CommonInvoker;
  
  using Invoker_t = unique_ptr<CommonInvoker>;
  using Function_t = R(*)(Args...);
  
  struct CommonInvoker{
    CommonInvoker(){}
    virtual ~CommonInvoker() {}
    virtual R Invoke(Args...) = 0;
    virtual Invoker_t Clone() = 0;
  };
  
  struct FreeFunctionInvoker : CommonInvoker{
    Function_t function;
    FreeFunctionInvoker(Function_t input_f) : function(input_f) {}
    
    virtual ~FreeFunctionInvoker() {}
    R Invoke(Args... args) override { return function(args...); }
    Invoker_t Clone() override { return std::make_unique<FreeFunctionInvoker>(function); }
  };
  
  template <typename Functor_t>
  struct FunctorInvoker : CommonInvoker{
    Functor_t functor;
    FunctorInvoker(Functor_t ftor) : functor(ftor) {}
    virtual ~FunctorInvoker() {}
    R Invoke(Args... args) override { return functor(args...); }
    Invoker_t Clone() override { return std::make_unique<FunctorInvoker>(functor); }
  };
  
  template <typename Class_t, typename MemberFunc_t>
  struct MemberFunctionInvoker : CommonInvoker{
    using MemberFuncSign_t = MemberFunc_t Class_t::*;
    
    MemberFuncSign_t function; 
    
    MemberFunctionInvoker(MemberFuncSign_t func) : function(func) {}
    
    R Invoke(Args... args) override { return InvokeMemberFunction(args...); }
    Invoker_t Clone() override { return std::make_unique<MemberFunctionInvoker>(function); }
    
  private:
    template <typename ...RestArgs>
    R InvokeMemberFunction(Class_t obj, RestArgs... args) { return (obj.*function)(args...); }
  };
  
  Invoker_t invoker_;
  
public:
  
  //Ctor for free function case
  MyFunction(Function_t input_func) : invoker_(std::make_unique<FreeFunctionInvoker>(input_func)) { cout<<"MyFunction ctor for free function"<<endl; }
  
  //Ctor for Functor case
  template <typename Functor_t>
  MyFunction(Functor_t ftor) : invoker_(std::make_unique<FunctorInvoker<Functor_t>>(ftor)) { cout<<"MyFunction ctor for functor"<<endl; };
  
  //Ctor for Member function case
  template <typename Function_t, typename Class_t>
  MyFunction(Function_t Class_t::* f) : invoker_(std::make_unique<MemberFunctionInvoker<Class_t, Function_t>>(f)) { cout<<"MyFunction ctor for member function"<<endl; }

  //Copy-ctor
  MyFunction(MyFunction& oth) { cout<<"Copy ctor for MyFunction"<<endl; invoker_ = std::move(oth.invoker_->Clone()); }
  //Copy-assign
  MyFunction& operator= (const MyFunction& oth) { if(*oth == this) return *this; invoker_ = std::move(oth.invoker_->Clone()); }
  
  
  R operator()(Args... args) { return invoker_->Invoke(args...); }
};





//utility structs ant functions for testing
struct LIFE_QUESTION{};
struct ANOTHER_QUESTION{};
int IntFunc(LIFE_QUESTION) { cout<<"Hello from IntFunc(LIFE_QUESTION)"<<endl; return 42; }
int IntFunc(ANOTHER_QUESTION, char ch) { cout<<"Hello from IntFunc(ANOTHER_QUESTION) "<<ch<<endl; return 24; }

class Functor{ 
  int internal_state_;
  
public:
  Functor(int i) : internal_state_(i) {}
  int operator()(){ cout<<"Hello from Functor"<<endl; return internal_state_; } 
  int GetState(char ch) { cout<<"Hello from member function "<<ch<<endl; return internal_state_; }
};


int main(){
 
  cout<<"Test of MyFunction (implementation of std::function)"<<endl;
  
  cout<<"Using MyFunction for free functions:"<<endl;
  MyFunction<int(LIFE_QUESTION)> my_func(IntFunc);
  MyFunction<int(ANOTHER_QUESTION, char)> my_another_func = IntFunc;
  
  MyFunction<int(LIFE_QUESTION)> my_func_copy(my_func);

  cout<<my_func(LIFE_QUESTION()) << endl;  
  cout<<my_another_func(ANOTHER_QUESTION(), 'a')<<endl;
  cout<<my_func_copy(LIFE_QUESTION()) << endl;  

  cout<<endl<<"Using MyFunction for functors:"<<endl;
  Functor ftor(12);  
  MyFunction<int()> my_functor = ftor;
  cout<< my_functor() <<endl;
  
  cout<<endl<<"Using MyFunction for member functions:"<<endl;
  MyFunction<int(Functor, char)> my_member_function = &Functor::GetState; 
  my_member_function(ftor, 'a');
  
  
  
  return 0;
}