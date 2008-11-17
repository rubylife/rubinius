#include "builtin/tuple.hpp"
#include "vm.hpp"
#include "vm/object_utils.hpp"
#include "objectmemory.hpp"
#include "builtin/fixnum.hpp"
#include "builtin/compactlookuptable.hpp"

#include <cstdarg>
#include <iostream>

namespace rubinius {
  /* The Tuple#at primitive. */
  Object* Tuple::at_prim(STATE, Fixnum* index_obj) {
    return at(state, index_obj->to_native());
  }

  Object* Tuple::put(STATE, size_t idx, Object* val) {
    if(num_fields() <= idx) {
      Exception::object_bounds_exceeded_error(state, this, idx);
    }

    this->field[idx] = val;
    if(val->reference_p()) write_barrier(state, val);
    return val;
  }

  /* The Tuple#put primitive. */
  Object* Tuple::put_prim(STATE, Fixnum* index, Object* val) {
    return put(state, index->to_native(), val);
  }

  /* The Tuple#fields primitive. */
  Object* Tuple::fields_prim(STATE) {
    return Integer::from(state, num_fields());
  }

  Tuple* Tuple::create(STATE, size_t fields) {
    return (Tuple*)state->om->new_object(G(tuple), fields);
  }

  Tuple* Tuple::allocate(STATE, Fixnum* fields) {
    return Tuple::create(state, fields->to_native());
  }

  Tuple* Tuple::from(STATE, size_t fields, ...) {
    va_list ar;
    Tuple* tup = create(state, fields);

    va_start(ar, fields);
    for(size_t i = 0; i < fields; i++) {
      tup->put(state, i, va_arg(ar, Object*));
    }
    va_end(ar);

    return tup;
  }

  Tuple* Tuple::copy_from(STATE, Tuple* other, Fixnum* start, Fixnum *length, Fixnum* dest) {
    size_t osize = other->num_fields();
    size_t size = this->num_fields();

    int olend = start->to_native();
    int lend = dest->to_native();
    int olength = length->to_native();

    // left end should be within range
    if(olend < 0 || (size_t)olend > osize) Exception::object_bounds_exceeded_error(state, other, olend);
    if(lend < 0 || (size_t)lend > size) Exception::object_bounds_exceeded_error(state, this, lend);

    // length can not be negative and must fit in src/dest
    if(olength < 0) {
      Exception::object_bounds_exceeded_error(state, "length must be positive");
    }
    if((size_t)(olend + olength) > osize) {
      Exception::object_bounds_exceeded_error(state, "length should not exceed size of source");
    }
    if((size_t)olength > (size - lend)) {
      Exception::object_bounds_exceeded_error(state, "length should not exceed space in destination");
    }

    for(size_t src = olend, dst = lend; src < (size_t)(olend + olength); ++src, ++dst) {
      this->put(state, dst, other->at(state,src));
    }

    return this;
  }

  // @todo performance primitive; could be replaced with Ruby
  Tuple* Tuple::pattern(STATE, Fixnum* size, Object* val) {
    native_int cnt = size->to_native();
    Tuple* tuple = Tuple::create(state, cnt);

    for(native_int i = 0; i < cnt; i++) {
      tuple->put(state, i, val);
    }

    return tuple;
  }

  Tuple* Tuple::create_weakref(STATE, Object* obj) {
    Tuple* tup = Tuple::from(state, 1, obj);
    tup->RefsAreWeak = 1;
    return tup;
  }

  void Tuple::Info::mark(Object* obj, ObjectMark& mark) {
    Object* tmp;
    Tuple* tup = as<Tuple>(obj);

    for(size_t i = 0; i < tup->num_fields(); i++) {
      tmp = mark.call(tup->field[i]);
      if(tmp) mark.set(obj, &tup->field[i], tmp);
    }
  }

  void Tuple::Info::show(STATE, Object* self, int level) {
    Tuple* tup = as<Tuple>(self);
    size_t size = tup->num_fields();
    size_t stop = size < 6 ? size : 6;

    if(size == 0) {
      class_info(state, self, true);
      return;
    }

    class_info(state, self);
    std::cout << ": " << size << std::endl;
    ++level;
    for(size_t i = 0; i < stop; i++) {
      indent(level);
      Object* obj = tup->at(state, i);
      if(obj == tup) {
	class_info(state, self, true);
      } else {
	obj->show(state, level);
      }
    }
    if(tup->num_fields() > stop) ellipsis(level);
    close_body(level);
  }

  void Tuple::Info::show_simple(STATE, Object* self, int level) {
    Tuple* tup = as<Tuple>(self);
    size_t size = tup->num_fields();
    size_t stop = size < 6 ? size : 6;

    if(size == 0) {
      class_info(state, self, true);
      return;
    }

    class_info(state, self);
    std::cout << ": " << size << std::endl;
    ++level;
    for(size_t i = 0; i < stop; i++) {
      indent(level);
      Object* obj = tup->at(state, i);
      if(Tuple* t = try_as<Tuple>(obj)) {
	class_info(state, self);
	std::cout << ": " << t->num_fields() << ">" << std::endl;
      } else {
	obj->show_simple(state, level);
      }
    }
    if(tup->num_fields() > stop) ellipsis(level);
    close_body(level);
  }
}
