class Object {
  Object* _forwardee;

public:
  void forward_to(address new_addr);
  Object* forwardee();
  bool is_forwarded();
  size_t size();
  Iterator<Object**> object_fields();
};

class Heap {
  Semispace* _from_space;
  Semispace* _to_space;

  void swap_spaces();
  Object* evacuate(Object* obj);

public:
  Heap(address bottom, address end);

  address allocate(size_t size);
  void collect();
  void process_reference(Object** slot);
};

class Semispace {
  address _bottom;
  address _top;
  address _end;

public:
  Semispace(address bottom, address end);

  address bottom() { return _bottom; }
  address top()    { return _top; }
  address end()    { return _end; }

  bool contains(address obj);
  address allocate(size_t size);
  void reset();
};

void Object::forward_to(address new_addr) {
  _forwardee = new_addr;
}

Object* forwardee() {
  return _forwardee;
}

bool Object::is_forwarded() {
  return _forwardee != nullptr;
}

// Initialize the heap. Assuming contiguous address space
Heap::Heap(address bottom, address end) {
  size_t space_size = (end - bottom) / 2;
  address boundary = bottom + space_size;
  _from_space = new Semispace(bottom, boundary);
  _to_space   = new Semispace(boundary, end);
}

void Heap::swap_spaces() {
  Semispace* temp = _from_space;
  _from_space = _to_space;
  _to_space = temp;
  _to_space->reset();
}

address Heap::allocate(size_t size) {
  return _from_space->allocate();
}

Object* Heap::evacuate(Object* obj) {
  size_t size = obj->size();
  address new_addr = _to_space->allocate(size);
  copy( new_addr, obj, size);
  Object* new_obj = (Object*) new_addr;
  obj->forward_to(new_obj);

  return new_obj;
}

void Heap::collect() {

  address scanned = _to_space->bottom();
  foreach (Object** slot in ROOTS) {
    process_reference(slot);
  }

  while (scanned < _to_space->top()) {
    Object* parent_obj = (Object*) scanned;
    foreach (Object** slot in parent_obj->obejct_fields()) {
      process_reference(slot);
    }
    scanned += parent_obj->size();
  }

  swap_spaces();
}

void Heap::process_reference(Object** slot) {
  Object* obj = *slot;
  if (obj != nullptr && _from_space->contains(obj)) {
    Object* new_obj = obj->is_forwarded() ? obj->forwardee() 
                                          : evacuate(obj);   

    *slot = new_obj;
  }
}

Semispace::Semispace(address bottom, address end) {
  _bottom = bottom;
  _top    = bottom;
  _end    = end;
}

address Semispace::contains(address obj) {
  return _bottom <= obj && obj < _top;
}

address Semispace::allocate(size_t size) {
  if (_top + size <= end) {
    address obj = _top;
    _top += size;
  } else {
    return nullptr;
  }
}

void Semispace::reset() {
  _top = _bottom;
}
