#include <iostream>


/*<-------------------- Nulltype ------------------------------------------------------------------------------------>*/


class Nulltype {
};


/*<-------------------- TypeList ------------------------------------------------------------------------------------>*/


template <typename ...Types>
struct TypeList {
    using Head = Nulltype;
    using Tail = Nulltype;
};

template <typename H, typename ...T>
struct TypeList<H, T...> {
    using Head = H;
    using Tail = TypeList<T...>;
};

typedef TypeList<> EmptyTypeList;

template <typename H>
struct TypeList<H> {
    using Head = H;
    using Tail = EmptyTypeList;
};


/*<-------------------- GetTail ------------------------------------------------------------------------------------->*/


template <typename TList>
struct GetTail;

template <typename ...TList>
struct GetTail<TypeList<TList...> > {
    using result = typename TypeList<TList...>::Tail;
};


/*<-------------------- GetHead ------------------------------------------------------------------------------------->*/


template <typename TList>
struct GetHead;

template <typename ...TList>
struct GetHead<TypeList<TList...> > {
    using result = typename TypeList<TList...>::Head;
};


/*<-------------------- PushBack ------------------------------------------------------------------------------------>*/


template <typename TList, typename T>
struct PushBack;

template <typename ...Types, typename T>
struct PushBack<TypeList<Types...>, T> {
    using result = TypeList<Types..., T>;
};

template<typename T>
struct PushBack<Nulltype, T> {
    using result = TypeList<T>;
};


/*<-------------------- PushFront ----------------------------------------------------------------------------------->*/


template <typename T, typename ...Types>
struct PushFront;

template <typename T, typename ...Types>
struct PushFront<TypeList<Types...>, T> {
    using result = TypeList<T, Types...>;
};

template<typename T>
struct PushFront<Nulltype, T> {
    using result = TypeList<T>;
};


/*<-------------------- IsTypeInList -------------------------------------------------------------------------------->*/


template <typename T, typename TList>
struct IsTypeInList;

template <typename T, typename ...Tail>
struct IsTypeInList<TypeList<T, Tail...>, T> {
    static const bool result = true;
};

template <typename ...TList, typename T>
struct IsTypeInList<TypeList<TList...>, T> {
    static const bool result = IsTypeInList<typename TypeList<TList...>::Tail, T>::result;
};

template <typename T>
struct IsTypeInList<TypeList<>, T> {
    static const bool result = false;
};


/*<-------------------- ListHasType --------------------------------------------------------------------------------->*/


template <typename TList, typename T>
struct ListHasType;

template <typename ...TList, typename T>
struct ListHasType<TypeList<TList...>, T> {
    using result = typename std::conditional<
            IsTypeInList<typename TypeList<TList...>::Head, T>::result,
            typename TypeList<TList...>::Head,
            typename ListHasType<typename TypeList<TList...>::Tail, T>::result
    >::type;
};

template <typename T>
struct ListHasType<TypeList<>, T> {
    using result = Nulltype;
};


/*<-------------------- ReverseList --------------------------------------------------------------------------------->*/


template <typename TList>
struct ReverseList;

template <typename ...TList>
struct ReverseList<TypeList<TList...>> {
    using result = typename PushBack<
            typename ReverseList<typename TypeList<TList...>::Tail>::result,
            typename TypeList<TList...>::Head
            >::result;
};

template <>
struct ReverseList<TypeList<>> {
    using result = TypeList<>;
};


/*<-------------------- GenScatterHierarchy ------------------------------------------------------------------------->*/


template <typename TList, template <class> class Unit>
struct GenScatterHierarchy;

template <typename ...TList, template <class> class Unit>
struct GenScatterHierarchy<TypeList<TList...>, Unit> :
        public Unit<typename TypeList<TList...>::Head>,
        public GenScatterHierarchy<typename TypeList<TList...>::Tail, Unit> {
    using LeftBase = Unit<typename TypeList<TList...>::Head>;
    using RightBase = GenScatterHierarchy<typename TypeList<TList...>::Tail, Unit>;
};

template <typename T, template <class> class Unit>
struct GenScatterHierarchy <TypeList<T>, Unit> :
        public Unit<T> {
    using LeftBase = Unit<T>;
    using RightBase = Nulltype;
};


/*<-------------------- GenLinearHierarchy -------------------------------------------------------------------------->*/


template <typename TList, template <class T, class Base> class Unit, typename Root = Nulltype>
struct GenLinearHierarchy;

template <typename ...TList, template <class, class> class Unit, typename Root>
struct GenLinearHierarchy<TypeList<TList...>, Unit, Root> :
        public Unit<typename TypeList<TList...>::Head,
                GenLinearHierarchy<typename TypeList<TList...>::Tail, Unit, Root>> {
};

template <typename TList, template <class, class> class Unit, typename Root>
struct GenLinearHierarchy<TypeList<TList>, Unit, Root> :
        public Unit<TList, Root> {
};


/*<-------------------- CheckConversion ----------------------------------------------------------------------------->*/


template <typename T, typename U>
class CheckConversion {
    class Small {
        char oneByte;
    };
    class Big {
        char twoBytes[2];
    };
    static Small Checker(U);
    static Big Checker(...);
public:
    const static bool canConvert = sizeof(Checker(T())) == sizeof(Small);
    const static bool haveSameType = false;
};

template <typename T>
class CheckConversion<T, T> {
public:
    const static bool canConvert = true;
    const static bool haveSameType = true;
};


/*<-------------------- CheckSuperSubClass -------------------------------------------------------------------------->*/


template<typename T, typename U>
struct CheckSuperSubClass{
    const static bool result =
            (CheckConversion<const U*, const T*>::canConvert && !std::is_same<const T*, const void*>::value);
};


/*<-------------------- GetListOfSuperClasses ----------------------------------------------------------------------->*/


template <typename ...TList>
struct GetListOfSuperClasses;

template <typename ...OpenTList, typename TList, typename ...NextTLists>
struct GetListOfSuperClasses<TypeList<OpenTList...>, TList, NextTLists...> {
    using NextListsResult = typename GetListOfSuperClasses<TList, NextTLists...>::result;
    using result = typename std::conditional<
            CheckSuperSubClass<typename TypeList<OpenTList...>::Head, typename GetHead<NextListsResult>::result>::result,
            TypeList<OpenTList...>,
            NextListsResult
            >::type;
};

template <typename TList>
struct GetListOfSuperClasses<TList> {
    using result = TList;
};


/*<-------------------- FactoryUnit --------------------------------------------------------------------------------->*/


template <typename T>
struct FactoryUnit {
    virtual T* Get(T) = 0;
    virtual ~FactoryUnit() = default;
};


/*<-------------------- AbstractFactory ----------------------------------------------------------------------------->*/


template <typename TList>
struct AbstractFactory :
        public GenScatterHierarchy<TList, FactoryUnit> {
    using products = TList;
    template <typename T>
    T* Get() {
        FactoryUnit<T>& unit = *this;
        return unit.Get(T());
    }
};


/*<-------------------- ConcreteFactoryUnit ------------------------------------------------------------------------->*/


template <class ConcreteProduct, class BaseT>
class ConcreteFactoryUnit : public BaseT {
    using mainProducts = typename BaseT::products;
public:
    using products = typename GetTail<mainProducts>::result;
    using AbstractProduct = typename GetHead<mainProducts>::result;
    ConcreteProduct* Get(AbstractProduct) override {
        return new ConcreteProduct;
    }
};


/*<-------------------- ConcreteFactory ----------------------------------------------------------------------------->*/


template <typename AbstractFactory_, typename TList,
        template <class, class> class CreatorClass = ConcreteFactoryUnit>
struct ConcreteFactory :
        public GenLinearHierarchy<TList, CreatorClass, AbstractFactory_> {
    using products = typename AbstractFactory_::products;
};


/*<-------------------- GetAbstractFactory -------------------------------------------------------------------------->*/


template <size_t N, size_t M, typename ...TLists>
struct GetAbstractFactory {
    using Factory = AbstractFactory<typename GetListOfSuperClasses<TLists...>::result>;

    template <typename T>
    struct GetConcreteFactory {
        using result = ConcreteFactory<Factory,
                typename ReverseList<typename ListHasType<TypeList<TLists...>, T>::result>::result>;
    };
};


/*<------------------------------------------------------------------------------------------------------------------>*/
/*<-------------------- EXAMPLE ------------------------------------------------------------------------------------->*/
/*<------------------------------------------------------------------------------------------------------------------>*/


struct Product {
    Product()  {
        name = "Abstract Product";
    }

    void ShowName() {
        std::cout << "I am " << name << std::endl;
    }

    std::string GetName() {
        return name;
    }

protected:
    std::string name;
};


struct Chair : public Product {
    Chair() {
        name = "Chair";
    }
};

struct Table : public Product {
    Table() {
        name = "Table";
    }
};

struct Sofa : public Product {
    Sofa() {
        name = "Sofa";
    }
};


struct WoodenChair : public Chair {
    WoodenChair() {
        name = "Wooden Chair";
    }
};

struct WoodenTable : public Table {
    WoodenTable() {
        name = "Wooden Table";
    }
};

struct WoodenSofa : public Sofa {
    WoodenSofa() {
        name = "Wooden Sofa";
    }
};


struct MetalChair : public Chair {
    MetalChair() {
        name = "Metal Chair";
    }
};

struct MetalTable : public Table {
    MetalTable() {
        name = "Metal Table";
    }
};

struct MetalSofa : public Sofa {
    MetalSofa() {
        name = "Metal Sofa";
    }
};


struct MetalRichChair : public MetalChair {
    MetalRichChair() {
        name = "Metal Rich Chair";
    }
};

struct MetalRichTable : public MetalTable {
    MetalRichTable() {
        name = "Metal Rich Table";
    }
};

struct MetalRichSofa : public MetalSofa {
    MetalRichSofa() {
        name = "Metal Rich Sofa";
    }
};


struct MetalPoorChair : public MetalChair {
    MetalPoorChair() {
        name = "Metal Poor Chair";
    }
};

struct MetalPoorTable : public MetalTable {
    MetalPoorTable() {
        name = "Metal Poor Table";
    }
};

struct MetalPoorSofa : public MetalSofa {
    MetalPoorSofa() {
        name = "Metal Poor Sofa";
    }
};


/*<------------------------------------------------------------------------------------------------------------------>*/


int main() {
    using MyFactoryHierarchy = GetAbstractFactory< 3, 5,
            TypeList<Chair, Table, Sofa>,
            TypeList<WoodenChair, WoodenTable, WoodenSofa>,
            TypeList<MetalChair, MetalTable, MetalSofa>,
            TypeList<MetalRichChair, MetalRichTable, MetalRichSofa>,
            TypeList<MetalPoorChair, MetalPoorTable, MetalPoorSofa>
    >;


    /*<---------------- EXAMPLE 1 ----------------------------------------------------------------------------------->*/


    MyFactoryHierarchy::Factory* MyFactory1 = new MyFactoryHierarchy::GetConcreteFactory<Chair>::result;
    auto sofa1 = MyFactory1->Get<Sofa>();
    auto table1 = MyFactory1->Get<Table>();
    auto chair1 = MyFactory1->Get<Chair>();

    std::cout << "Sofa | REAL: " << sofa1->GetName() << std::endl;
    std::cout << "Table | REAL: " << table1->GetName() << std::endl;
    std::cout << "Chair | REAL: " << chair1->GetName() << std::endl;

    delete sofa1;
    delete table1;
    delete chair1;

    delete MyFactory1;


    /*<---------------- EXAMPLE 2 ----------------------------------------------------------------------------------->*/


    MyFactoryHierarchy::Factory* MyFactory2 = new MyFactoryHierarchy::GetConcreteFactory<MetalRichTable>::result;
    auto chair2 = MyFactory2->Get<Chair>();
    auto sofa2 = MyFactory2->Get<Sofa>();
    auto table2 = MyFactory2->Get<Table>();

    std::cout << "Metal Rich Chair | REAL: " << chair2->GetName() << std::endl;
    std::cout << "Metal Rich Sofa | REAL: " << sofa2->GetName() << std::endl;
    std::cout << "Metal Rich Table | REAL: " << table2->GetName() << std::endl;

    delete chair2;
    delete sofa2;
    delete table2;

    delete MyFactory2;

    return 0;
}
