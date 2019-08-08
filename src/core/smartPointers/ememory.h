#ifndef EMEMORY_H
#define EMEMORY_H

#include "stdpointer.h"
#include "stdselfref.h"
#include "selfref.h"

namespace enve {
    template <class T, typename... Args>
    inline auto make_shared(Args && ...args) {
        static_assert(std::is_base_of<StdSelfRef, T>::value ||
                      std::is_base_of<SelfRef, T>::value,
                      "enve::make_shared can only be used with StdSelfRef and SelfRef derived classes");

        return T::template sCreate<T>(std::forward<Args>(args)...);
    }

    template <class T>
    inline auto shared(T * const obj) {
        static_assert(std::is_base_of<StdSelfRef, T>::value ||
                      std::is_base_of<SelfRef, T>::value,
                      "enve::shared can only be used with StdSelfRef and SelfRef derived classes");

        return obj->template ref<T>();
    }

    template <class T, class B>
    inline auto shared(const B& obj) -> decltype(obj->template ref<T>()) {
        static_assert(std::is_base_of<StdSelfRef, T>::value ||
                      std::is_base_of<SelfRef, T>::value,
                      "enve::shared can only be used with StdSelfRef and SelfRef derived classes");
        if(obj) return obj->template ref<T>();
        return nullptr;
    }
}

#endif // EMEMORY_H
