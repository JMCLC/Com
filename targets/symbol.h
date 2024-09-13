#ifndef __MML_TARGETS_SYMBOL_H__
#define __MML_TARGETS_SYMBOL_H__

#include <string>
#include <memory>
#include <cdk/types/basic_type.h>

namespace mml {

  class symbol {
    int _qualifier;
    std::shared_ptr<cdk::basic_type> _type;
    std::string _name;
    long _value; // hack!
    int _offset = 0;
    std::vector<std::shared_ptr<cdk::basic_type>> _argumentList;

  public:
    symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, long value) :
        _type(type), _name(name), _value(value) {
    }

    virtual ~symbol() {
      // EMPTY
    }

    int qualifier() {
      return _qualifier;
    }
    std::shared_ptr<cdk::basic_type> type() const {
      return _type;
    }
    bool is_typed(cdk::typename_type name) const {
      return _type->name() == name;
    }
    const std::string &name() const {
      return _name;
    }
    long value() const {
      return _value;
    }
    long value(long v) {
      return _value = v;
    }
    int offset() {
      return _offset;
    }
    void set_offset(int offset) {
      _offset = offset;
    }
    std::vector<std::shared_ptr<cdk::basic_type>> arguments() {
      return _argumentList;
    }
    static std::shared_ptr<symbol> createSymbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, long value = 0) {
      return std::make_shared<symbol>(type, name, value);
    }
    static std::shared_ptr<symbol> createSymbol(int qualifier, std::shared_ptr<cdk::basic_type> type, const std::string &name, long value = 0) {
      return std::make_shared<symbol>(type, name, value);
    }
  };

} // mml

#endif
