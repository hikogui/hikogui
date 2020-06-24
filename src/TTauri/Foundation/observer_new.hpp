
#include "TTauri/Foundation/notifier.hpp"

namespace tt {

template<typename T>
class observable {
public:
    using value_type = T;
    using callback_type = std::function<void(value_type const &)>;

private:
    notifier<T> notifier;

public:
    virtual value_type load() const noexcept = 0;

    virtual void store(value_type value) noexcept = 0;

    size_t add_callback(callback_type) noexcept {
        return notifier.add_and_call(callback_type, load());
    }

    void remove_callback(size_t id) noexcept {
        notifier.remove(id);
    }
};

template<typename T>
class observable_unari : public observable<T> {
protected:
    std::unique_ptr<observable<value_type>> operant;
    value_type operant_cache;
};

template<typename T>
class observable_pass_through : public observable_unari<T> {
public:
    observable_pass_through() :
        observable_unari() {}
    
    virtual value_type load() const noexcept override {
        return operant_cache;
    }

    virtual void store(value_type value) noexcept override {
        operant.store(value);
    }
};

template<typename T>
class observable_not : public observable_unari<T> {

};



template<typename T>
class observing {

};

template<typename ValueType>
class observer {
public:
    using value_type = ValueType;
    using callback_type = std::function<void(value_type const &)>;

private:
    mutable fast_mutex mutex;
    ValueType value;

    observer *parent;

    notifier<value_type> notifier;

    void store_actual(value_type const &value) const noexcept {
        auto lock = std::scoped_lock(mutex);
        this->value = value;
        notifier(value);
    }

public:
    
    [[nodiscard]] value_type load() const noexcept {
        auto lock = std::scoped_lock(mutex);
        return value;
    }

    void store(value_type const &value) noexcept {
        observer tmp_parent;
        {
            auto lock = std::scoped_lock(mutex);
            tmp_parent = parent;
        }
        
        if (tmp_parent) {
            // Setting the value on the parent will eventually cause
            // our cashed value to update as well.
            tmp_parent->set(value);

        } else {
            store_actual(value);
        }
    }

    size_t add_callback(function_type const &func) noexcept {
        value_type tmp;
        {
            auto lock = std::scoped_lock(mutex);
            tmp = value;
        }
        return notfier.add_and_call(func, tmp);
    }

    void remove_callback(size_t id) noexcept {
        notifier.remove(id);
    }

};





}

