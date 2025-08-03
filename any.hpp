#include <memory>

class Any {
  public:
    Any() = default;
    ~Any() = default;
    Any(Any &any) = default;
    template <typename T>
    Any(T &data) : data_(std::make_unique<Derive<T>>(data)) {}
    template <typename T>
    T cast() {
        Derive<T> *pd = dynamic_cast<Derive<T> *>(data_.get());
        if (pd == nullptr) {
            throw "type is unmatch!";
        }
        return pd->data_;
    }

  private:
    class Base {
      public:
        virtual ~Base() = default;
    };
    template <typename T>
    class Derive : public Base {
      public:
        Derive(T data) : data_(data) {}
        T data_;
    };

  private:
    std::unique_ptr<Base> data_;
};