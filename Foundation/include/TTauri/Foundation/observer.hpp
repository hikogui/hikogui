// Copyright 2020 Pokitec
// All rights reserved.

#pragma once
#include "TTauri/Foundation/type_traits.hpp"
#include "TTauri/Foundation/notifier.hpp"
#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <type_traits>
#include <memory>
#include <variant>

namespace tt {

/** @file observer.hpp
 *
 * an observer is an object that observes an obexpr:
 *  - An observer is the (shared) owner of the obexpr tree.
 *  - Notification from the obexpr will:
 *    - Update the cached value.
 *    - Execute callbacks registered with the observer
 *  - The cached value can be read through the conversion operator.
 *  - Any write or read/modify/write operation will be forwarded to the expression.
 *  - Any operation on the observer will return a copy of the cached or computed value.
 *
 * an observable is an object which is observed by the obexpr_observer:
 *  - Any write or read/modify/write operation will cause a notifaction to
 *    be send to any registered obexpr_observer.
 *  - Any operation on the observable will return a copy of the cached or computed value.
 *  - To turn a observable to a obexpr, use the expr() function.
 *  
 * A obexpr is a expression object that forms a tree of other obexpr objects.
 *  - When a leaf value changes, its value is cashed and notification is send
 *    through the tree to the root object.
 *  - When a observer is a (shared) owner of a obexpr it will be notified with
 *    the calculated value of the expression.
 *  - certain obexpr objects can forward write or read/modify/write operation
 *    toward the leaf nodes.
 *  - leaf nodes can forward write or read/modify/write operations to the observable.
 */

class obexpr_owner {
public:
    /** Handle notifications received from operants.
    * The mutex of children are locked during notifications.
    */
    virtual void handle_notification() noexcept {
    }
};

template<typename T>
class obexpr_impl: public obexpr_owner {
public:

private:
    mutable std::mutex mutex;
    std::vector<obexpr_owner *> owners;

protected:

    void notify_owners() const noexcept {
        auto lock = std::scoped_lock(mutex);

        for (let owner: owners) {
            owner->handle_notification();
        }
    }


public:
    virtual ~obexpr_impl() {}
    obexpr_impl() noexcept {}

    /** Read the value of the expression.
     */
    [[nodiscard]] virtual T load() const noexcept = 0;
    
    /** Write the value back through the expression.
    */
    virtual void store(T const &) noexcept = 0;

    void add_owner(obexpr_owner *owner) noexcept {
        auto lock = std::scoped_lock(mutex);

        owners.push_back(owner);
        owner->handle_notification();
    }

    void remove_owner(obexpr_owner *owner) noexcept {
        auto lock = std::scoped_lock(mutex);

        let new_end = std::remove(owners.begin(), owners.end(), owner);
        owners.erase(new_end, owners.cend());
    }

    /** Handle notifications received from operants.
    * The mutex of children are locked during notifications.
    */
    void handle_notification() noexcept override {
        notify_owners();
    }
};

template<typename T, typename OP>
class obexpr_unari : public obexpr_impl<T> {
public:
    std::shared_ptr<obexpr_impl<OP>> op;

    ~obexpr_unari() {
        op->remove_owner(this);
    }

    obexpr_unari() = delete;
    obexpr_unari(obexpr_unari const &other) = delete;
    obexpr_unari(obexpr_unari &&other) = delete;
    obexpr_unari &operator=(obexpr_unari const &other) = delete;
    obexpr_unari &operator=(obexpr_unari &&other) = delete;


    obexpr_unari(std::shared_ptr<obexpr_impl<OP>> op) :
        obexpr_impl<T>(), op(std::move(op))
    {
        this->op->add_owner(this);
    }


};

template<typename OP>
class obexpr_not : public obexpr_unari<bool,OP> {
public:
    obexpr_not() = delete;
    obexpr_not(obexpr_not const &other) = delete;
    obexpr_not(obexpr_not &&other) = delete;
    obexpr_not &operator=(obexpr_not const &other) = delete;
    obexpr_not &operator=(obexpr_not &&other) = delete;

    obexpr_not(std::shared_ptr<obexpr_impl<OP>> op) :
        obexpr_unari<bool,OP>(op) {}

    [[nodiscard]] bool load() const noexcept override {
        return !(this->op->load());
    }

    void store(bool const &v) noexcept override {
        this->op->store(!v);
    }
};

template<typename T>
class observable {
private:
    std::atomic<T> value;
    notifier<T> _notifier;

public:
    using value_type = T;
    using callback_type = typename notifier<T>::callback_type;

    ~observable() = default;
    observable(observable const &) = delete;
    observable(observable &&) = delete;
    observable &operator=(observable const &) = delete;
    observable &operator=(observable &&) = delete;

    observable() :
        value() {}

    observable(T v) :
        value(std::move(v)) {}

    observable(T v, callback_type callback) :
        value(std::move(v))
    {
        _notifier.add_and_call(
            std::move(callback),
            value.load(std::memory_order::memory_order_relaxed)
        );
    }

    T operator=(T const &v) noexcept {
        value.store(v, std::memory_order::memory_order_relaxed);
        _notifier(v);
        return v;
    }

    operator T () const noexcept {
        return value.load(std::memory_order::memory_order_relaxed);
    }

    [[nodiscard]] size_t add_callback(callback_type callback) noexcept {
        return _notifier.add_and_call(
            std::move(callback),
            value.load(std::memory_order::memory_order_relaxed)
        );
    }

    void remove_callback(size_t id) noexcept {
        _notifier.remove(id);
    }
};


template<typename T>
class obexpr_observable : public obexpr_impl<T> {
protected:
    observable<T> *object;
    size_t callback_id;

public:
    ~obexpr_observable() {
        ttauri_assume(object);
        object->remove_callback(callback_id);
    }

    obexpr_observable() = delete;
    obexpr_observable(obexpr_observable const &other) = delete;
    obexpr_observable(obexpr_observable &&other) = delete;
    obexpr_observable &operator=(obexpr_observable const &other) = delete;
    obexpr_observable &operator=(obexpr_observable &&other) = delete;

    obexpr_observable(observable<T> &object) :
        obexpr_impl<T>(), object(&object)
    {
        ttauri_assume(this->object);

        callback_id = this->object->add_callback([this](auto...) {
            this->notify_owners();
        });
    }

    [[nodiscard]] T load() const noexcept override {
        ttauri_assume(object);
        return *object;
    }

    void store(T const &v) noexcept override {
        ttauri_assume(object);
        (*object) = v;
    }
};

template<typename T>
class obexpr {
public:
    std::shared_ptr<obexpr_impl<T>> expr;

    template<typename E, typename... Args>
    static obexpr make(Args &&... args) noexcept {
        return { std::make_shared<E>(std::forward<Args>(args)...) };
    }

    obexpr() = delete;
    ~obexpr() = default;
    obexpr(obexpr const &other) = default;
    obexpr(obexpr &&other) = default;
    obexpr &operator=(obexpr const &other) = default;
    obexpr &operator=(obexpr &&other) = default;

    obexpr(std::shared_ptr<obexpr_impl<T>> other) noexcept :
        expr(std::move(other)) {}

    obexpr(observable<T> &object) noexcept :
        expr(std::make_shared<obexpr_observable<T>>(object)) {}

    [[nodiscard]] friend obexpr<bool> operator!(obexpr const &rhs) noexcept {
        return obexpr::make<obexpr_not<T>>(rhs.expr);
    }
};

template<typename T>
class observer : public obexpr_owner {
private:
    notifier<T> _notifier;

    std::shared_ptr<obexpr_impl<T>> expr;
    std::atomic<T> value;

public:
    using value_type = T;
    using callback_type = typename notifier<T>::callback_type;

    ~observer() {
        if (expr) {
            expr->remove_owner(this);
        }
    }
    observer(observer const &) = delete;
    observer(observer &&) = delete;
    observer &operator=(observer &&) = delete;
    // The copy-assignment operator will only copy the value, not the expression, see below.

    observer() :
        value() {}

    observer(T value) :
        value(std::move(value)) {}

    observer(T value, callback_type f) :
        value(std::move(value))
    {
        _notifier.add_and_call(
            std::move(f),
            this->value.load(std::memory_order::memory_order_relaxed)
        );
    }

    observer(obexpr<T> const &e, callback_type f)
    {
        // Only add, do not call, since the add_owner below will cause
        // a call down the chain.
        _notifier.add(std::move(f));

        expr = e.expr;
        expr->add_owner(this);
    }

    observer(observable<T> &e, callback_type f) :
        observer(obexpr<T>{e}, std::move(f)) {}

    T operator=(obexpr<T> const &e) noexcept {
        expr = e.expr;
        expr->add_owner(this);
        return value.load(std::memory_order::memory_order_relaxed);
    }

    T operator=(observable<T> &e) noexcept {
        return (*this) = obexpr<T>{e};
    }

    operator T () const noexcept {
        return value.load(std::memory_order::memory_order_relaxed);
    }

    T operator=(T const &rhs) {
        if (expr) {
            expr->store(rhs);
        } else {
            value.store(rhs, std::memory_order::memory_order_relaxed);
            _notifier(rhs);
        }
        return rhs;
    }

    T operator=(observer const &rhs) noexcept {
        return (*this) = static_cast<T>(rhs);
    }

    void handle_notification() noexcept override {
        ttauri_assume(expr);
        auto new_value = expr->load();
        value.store(new_value);
        _notifier(new_value);
    }
};

}
