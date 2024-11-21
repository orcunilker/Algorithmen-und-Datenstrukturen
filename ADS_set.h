#ifndef ADS_SET_H
#define ADS_SET_H

// ADS SS2023 - Separate Chaining
// Orcun Doeger
// a11716448

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>


template <typename Key, size_t N = 7>
class ADS_set {
public:
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using const_iterator = Iterator;
  using iterator = const_iterator;
  using key_equal = std::equal_to<key_type>;                       // Hashing
  using hasher = std::hash<key_type>;                              // Hashing

private:
  struct Element {
    Element(key_type k, Element* e = nullptr): key{k}, nextElement{e} {}
    key_type key;
    Element* nextElement;
  };
  struct Node {
    Element* e;
  };

  Node *table {nullptr};

  size_type table_size {0};
  size_type current_size {0};
  float max_lf {0.7};
  
  Element *add(const key_type &key);
  Element *addElement(Element* e);
  Element *locate(const key_type &key) const;
  size_type h(const key_type &key) const { return hasher{}(key) % table_size; }
  void reserve(size_type n);
  void rehash(size_type n);

public:
  ADS_set();
  ADS_set(std::initializer_list<key_type> ilist): ADS_set{} { insert(ilist); }
  template<typename InputIt> ADS_set(InputIt first, InputIt last): ADS_set{} { insert(first,last); }
  ADS_set(const ADS_set &other);

  ~ADS_set();

  ADS_set &operator=(const ADS_set &other);
  ADS_set &operator=(std::initializer_list<key_type> ilist);

  size_type size() const { return current_size; }
  bool empty() const { return current_size == 0; }

  void insert(std::initializer_list<key_type> ilist) { insert(ilist.begin(), ilist.end()); }
  std::pair<iterator,bool> insert(const key_type &key);
  template<typename InputIt> void insert(InputIt first, InputIt last);

  void clear();
  size_type erase(const key_type &key);

  size_type count(const key_type &key) const { return locate(key) != nullptr; }
  iterator find(const key_type &key) const;

  void dump(std::ostream &o = std::cerr) const;

  void swap(ADS_set &other);

  const_iterator begin() const { return const_iterator(&table[0], table[0].e, &table[table_size]); }
  const_iterator end() const { return const_iterator(&table[table_size], nullptr, &table[table_size]); }

  friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
    if (lhs.current_size != rhs.current_size) return false;
    for (const auto &k: lhs) if (!rhs.count(k)) return false;
    return true;
  }
  friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) { return !(lhs == rhs); }
};




template <typename Key, size_t N>
ADS_set<Key,N>::ADS_set(): table{new Node[N+1]}, table_size{N}, current_size{0} {
  for (size_type idx {0}; idx < table_size; ++idx) {
      table[idx].e = nullptr;
  }
}

template <typename Key, size_t N>
ADS_set<Key,N>::ADS_set(const ADS_set &other): table{new Node[N+1]}, table_size{N}, current_size{0} {
  for (size_type idx {0}; idx < table_size; ++idx) {
      table[idx].e = nullptr;
  }
  rehash(other.table_size);
  for (const auto &k: other) add(k);
}

template <typename Key, size_t N>
ADS_set<Key,N>::~ADS_set() {
  for (size_type idx {0}; idx < table_size; ++idx) {
    Element* cursorElement = table[idx].e;
    while(cursorElement){
      Element* thisElement = cursorElement;
      cursorElement = cursorElement->nextElement;
      delete thisElement;
      thisElement = nullptr;
    }
  }
  delete[] table; 
}


template <typename Key, size_t N>
typename ADS_set<Key,N>::iterator ADS_set<Key,N>::find(const key_type &key) const {
  if (Element* e {locate(key)}){
    size_type table_idx {h(key)};
    
    return iterator(&table[table_idx], e, &table[table_size]);
  }
  return end();
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::size_type ADS_set<Key,N>::erase(const key_type &key) {
  Element* delEl {locate(key)};

  if (delEl) {
    size_type table_idx = h(key);
    
    if (table[table_idx].e == delEl) {
        table[table_idx].e = delEl->nextElement;
    } else {
        Element* cursorElement = table[table_idx].e;
        while (cursorElement && cursorElement->nextElement != delEl) {
            cursorElement = cursorElement->nextElement;
        }
        if (cursorElement) {
            cursorElement->nextElement = delEl->nextElement;
        }
    }

    delete delEl;
    delEl = nullptr;
    --current_size;
    return 1;
  }
  return 0;
}


template <typename Key, size_t N>
void ADS_set<Key,N>::swap(ADS_set &other) {
  using std::swap;
  swap(table, other.table);
  swap(table_size, other.table_size);
  swap(current_size, other.current_size);
  swap(max_lf, other.max_lf);
}

template <typename Key, size_t N>
void ADS_set<Key,N>::clear() {
  ADS_set tmp;
  swap(tmp);
}

template <typename Key, size_t N>
ADS_set<Key,N> &ADS_set<Key,N>::operator=(const ADS_set &other) {
  ADS_set tmp{other};
  swap(tmp);
  return *this;
}

template <typename Key, size_t N>
ADS_set<Key,N> &ADS_set<Key,N>::operator=(std::initializer_list<key_type> ilist) {
  ADS_set tmp{ilist};
  swap(tmp);
  return *this;
}


template <typename Key, size_t N>
template<typename InputIt> void ADS_set<Key,N>::insert(InputIt first, InputIt last) {
  for (auto it {first}; it != last; ++it) {
    if (!count(*it)) {
      reserve(current_size+1);
      add(*it);
    }
  }
}

template <typename Key, size_t N>
std::pair<typename ADS_set<Key,N>::iterator,bool> ADS_set<Key,N>::insert(const key_type &key) {
  if (Element* e {locate(key)}){
    size_type table_idx {h(key)};
    return {iterator(&table[table_idx], e, &table[table_size]),false};
  } 
  reserve(current_size+1);
  Element* newElement = add(key);
  size_type table_idx {h(key)};
  return {iterator(&table[table_idx], newElement, &table[table_size]),true};
}


template <typename Key, size_t N>
typename ADS_set<Key,N>::Element *ADS_set<Key,N>::add(const key_type &key) {
  size_type idx {h(key)};
  if(count(key)) return locate(key);
  if(table[idx].e == nullptr){
    table[idx].e = new Element(key, nullptr);
    ++current_size;
    return table[idx].e;
  }
  else {
    Element* oldFirstElement = table[idx].e;
    Element* newFirstElement = new Element(key, oldFirstElement);
    table[idx].e = newFirstElement;

    ++current_size;
    return newFirstElement;
  }
}
template <typename Key, size_t N>
typename ADS_set<Key,N>::Element *ADS_set<Key,N>::addElement(ADS_set<Key,N>::Element* e) { // When rehashing
  if(e){
    size_type idx {h(e->key)};

    if(table[idx].e == nullptr){
      e->nextElement = nullptr;
      table[idx].e = e;
    }
    else {
      Element* oldFirstElement = table[idx].e;
      table[idx].e = e;
      e->nextElement = oldFirstElement;
    }
    ++current_size;
    return e;
  }
  return nullptr;
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::Element *ADS_set<Key,N>::locate(const key_type &key) const {
  size_type idx {h(key)};
  Element* cursorElement = table[idx].e;
  while(cursorElement != nullptr){
    if(key_equal{}(cursorElement->key, key)) return cursorElement;
    cursorElement = cursorElement->nextElement;
  }
  return nullptr;
}




template <typename Key, size_t N>
void ADS_set<Key,N>::reserve(size_type n) {
  if (table_size * max_lf >= n) return;
  size_type new_table_size {table_size};
  while (new_table_size * max_lf < n){
    ++(new_table_size *= 2);
  }
  if (new_table_size % 2 == 0) ++new_table_size; // Ungerade
  rehash(new_table_size);
}

template <typename Key, size_t N>
void ADS_set<Key,N>::rehash(size_type n) {
  size_type new_table_size {std::max(N,std::max(n,size_type(current_size/max_lf)))};

  Node *new_table {new Node [new_table_size+1]};
  for (size_type idx {0}; idx < new_table_size; ++idx) { // Elemente in der neuen Tabelle müssen initalisiert werden
    new_table[idx].e = nullptr;
  }

  Node *old_table {table};
  size_type old_table_size {table_size};
  
  current_size = 0;
  table = new_table;
  table_size = new_table_size;
  
  if(old_table == nullptr) return;
  for (size_type idx {0}; idx < old_table_size; ++idx) {
    Element* cursorElement = old_table[idx].e;
    while(cursorElement != nullptr){
        Element* elementToAdd = cursorElement;
        cursorElement = cursorElement->nextElement;
        addElement(elementToAdd);
    }
  }

  delete[] old_table;
}


template <typename Key, size_t N>
void ADS_set<Key,N>::dump(std::ostream &o) const {
  o << "table_size = " << table_size << ", current_size = " << current_size << "\n";
  for (size_type idx {0}; idx < table_size; ++idx) {
    o << idx << ": ";
    Element* cursorElement = table[idx].e;
    while(cursorElement){
      o << " --{" << cursorElement->key << "}";
      cursorElement = cursorElement->nextElement;
    }
    o << "\n";
  }
}


template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
  Node* current_node;
  Element* current_element;
  const Node* table_end;

public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using pointer = const value_type *;
  using iterator_category = std::forward_iterator_tag;

  explicit Iterator(Node* n = nullptr, Element* e = nullptr, const Node* te = nullptr): current_node{n}, current_element{e}, table_end{te} { 
      while (!current_element && current_node != table_end) {
          ++current_node;
          if(current_node != table_end) current_element = current_node->e;
      }
  }
  reference operator*() const { return current_element->key;}
  pointer operator->() const { return &current_element->key;}
  Iterator& operator++() { 
      if(current_element && current_element->nextElement) {
          current_element = current_element->nextElement;
      }
      else current_element = nullptr;
      while(!current_element && current_node != table_end){
        ++current_node;
        if(current_node != table_end) current_element = current_node->e;
      }
      return *this;
  }
  Iterator operator++(int) {
      Iterator tmp = *this;
      ++*this;
      return tmp;
  }
  friend bool operator==(const Iterator &lhs, const Iterator &rhs) {
      return lhs.current_node == rhs.current_node && lhs.current_element == rhs.current_element;
  }
  friend bool operator!=(const Iterator &lhs, const Iterator &rhs) { return !(lhs == rhs); }
};

template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }


#endif // ADS_SET_H
