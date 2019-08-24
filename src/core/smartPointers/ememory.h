// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
