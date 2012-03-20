#ifndef CLONEABLE_H
#define CLONEABLE_H



namespace lc {
    class Cloneable {
        public:
            Cloneable();
            //   virtual T clone() = 0;
    };
}


/**
template<typename T>
struct  b { virtual T  f() =0; };

template<typename T>
struct d : b<T> { T f() {cout << "hi ries"; return T();  } } ;

int main(){ d<int> derived; cout   << ETYPE(derived.f()) << " "; derived.f(); }
**/


#endif // CLONEABLE_H
