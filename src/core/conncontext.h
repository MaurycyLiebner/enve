#ifndef CONNCONTEXT_H
#define CONNCONTEXT_H
#include <QObject>
#include <memory>

class ConnContext {
public:
    ConnContext() {}
    ConnContext(ConnContext&) = delete;

    ~ConnContext() {
        for(const auto& conn : mConns)
            QObject::disconnect(conn);
    }

    ConnContext& operator<<(const QMetaObject::Connection& connection) {
        mConns << connection;
        return *this;
    }
private:
    QList<QMetaObject::Connection> mConns;
};

template <typename T>
class ConnContextObjList : private QList<T> {
public:
    ConnContextObjList() {}
    ConnContextObjList(ConnContextObjList&) = delete;

    //! @brief Appends the object and returns corresponding connection context
    ConnContext& addObj(const T& obj) {
        return insertObj(this->count(), obj);
    }

    //! @brief Adds the object at index and returns corresponding connection context
    ConnContext& insertObj(const int index, const T& obj) {
        const auto cctx = std::make_shared<ConnContext>();
        this->insert(index, obj);
        mCCtxs.insert(index, cctx);
        return *cctx.get();
    }

    //! @brief Removes(but does not delete) the object and corresponding connections
    void removeObj(const T& obj) {
        for(int i = 0; i < this->count(); i++) {
            const auto iObj = this->at(i);
            if(iObj == obj) {
                this->removeAt(i);
                mCCtxs.removeAt(i);
                break;
            }
        }
    }

    void moveObj(const int from, const int to) {
        this->move(from, to);
        mCCtxs.move(from, to);
    }

    T takeObjAt(const int id) {
        const auto obj = this->takeAt(id);
        mCCtxs.removeAt(id);
        return obj;
    }

    T takeObjLast() {
        const auto obj = this->takeLast();
        mCCtxs.removeLast();
        return obj;
    }

    void clear() {
        QList<T>::clear();
        mCCtxs.clear();
    }

    const QList<T>& getList() const { return *this; }

    using QList<T>::begin;
    using QList<T>::end;
    using QList<T>::cbegin;
    using QList<T>::cend;
    using QList<T>::rbegin;
    using QList<T>::rend;
    using QList<T>::crbegin;
    using QList<T>::crend;
    using QList<T>::count;
    using QList<T>::isEmpty;
    using QList<T>::at;
    using QList<T>::first;
    using QList<T>::last;
    using QList<T>::operator[];
private:
    QList<std::shared_ptr<ConnContext>> mCCtxs;
};

#endif // CONNCONTEXT_H
