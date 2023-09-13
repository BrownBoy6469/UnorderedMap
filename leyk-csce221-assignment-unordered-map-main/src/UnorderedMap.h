#include <cstddef>    // size_t
#include <functional> // std::hash
#include <ios>
#include <utility>    // std::pair
#include <iostream>

#include "primes.h"



template <typename Key, typename T, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>>
class UnorderedMap {
    public:

    using key_type = Key;
    using mapped_type = T;
    using const_mapped_type = const T;
    using hasher = Hash;
    using key_equal = Pred;
    using value_type = std::pair<const key_type, mapped_type>;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    private:

    struct HashNode {
        HashNode *next;
        value_type val;

        HashNode(HashNode *next = nullptr) : next{next} {}
        HashNode(const value_type & val, HashNode * next = nullptr) : next { next }, val { val } { }
        HashNode(value_type && val, HashNode * next = nullptr) : next { next }, val { std::move(val) } { }
    };

    size_type _bucket_count;
    HashNode **_buckets;

    HashNode * _head;
    size_type _size;

    Hash _hash;
    key_equal _equal;

    static size_type _range_hash(size_type hash_code, size_type bucket_count) {
        return hash_code % bucket_count;
    }

    public:

    template <typename pointer_type, typename reference_type, typename _value_type>
    class basic_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = _value_type;
        using difference_type = ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;

    private:
        friend class UnorderedMap<Key, T, Hash, key_equal>;
        using HashNode = typename UnorderedMap<Key, T, Hash, key_equal>::HashNode;

        const UnorderedMap * _map;
        HashNode * _ptr;

        explicit basic_iterator(UnorderedMap const * map, HashNode *ptr) noexcept { 
            _map = map;
            _ptr = ptr;    
        }

    public:
        basic_iterator() { 
            _ptr = nullptr;
        };

        basic_iterator(const basic_iterator &) = default;
        basic_iterator(basic_iterator &&) = default;
        ~basic_iterator() = default;
        basic_iterator &operator=(const basic_iterator &) = default;
        basic_iterator &operator=(basic_iterator &&) = default;
        reference operator*() const { 
            return _ptr->val;
        }
        pointer operator->() const { 
            return &_ptr->val;   
        }
        basic_iterator &operator++() { 
            if(_ptr->next == nullptr) {
                size_type index = _map->_bucket(_ptr->val);
                for(int i = index + 1; i < _map->_bucket_count; i++) {
                    if(_map->_buckets[i] != nullptr) {
                        _ptr = _map->_buckets[i];
                        return *this;
                    }
                }
                _ptr = nullptr;
            }
            else {
                _ptr = _ptr->next;
            }            
            return *this;
        }
        basic_iterator operator++(int) { 
            basic_iterator _ptrCopy = *this;
            if(_ptr->next == nullptr) {
                size_type index = _map->_bucket(_ptr->val);
                for(int i = index + 1; i < _map->_bucket_count; i++) {
                    if(_map->_buckets[i] != nullptr) {
                        _ptr = _map->_buckets[i];
                        return _ptrCopy;
                    }
                }
                _ptr = nullptr;
            }
            else {
                _ptr = _ptr->next;
            }      
            return _ptrCopy;
        }
        bool operator==(const basic_iterator &other) const noexcept { 
            return _ptr == other._ptr;    
        }
        bool operator!=(const basic_iterator &other) const noexcept { 
            return _ptr != other._ptr;
        }
    };

    using iterator = basic_iterator<pointer, reference, value_type>;
    using const_iterator = basic_iterator<const_pointer, const_reference, const value_type>;

    class local_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::pair<const key_type, mapped_type>;
            using difference_type = ptrdiff_t;
            using pointer = value_type *;
            using reference = value_type &;

        private:
            friend class UnorderedMap<Key, T, Hash, key_equal>;
            using HashNode = typename UnorderedMap<Key, T, Hash, key_equal>::HashNode;

            HashNode * _node;

            explicit local_iterator( HashNode * node ) noexcept { 
                _node = node;
            }

        public:
            local_iterator() { 
                _node = nullptr;
            }

            local_iterator(const local_iterator &) = default;
            local_iterator(local_iterator &&) = default;
            ~local_iterator() = default;
            local_iterator &operator=(const local_iterator &) = default;
            local_iterator &operator=(local_iterator &&) = default;
            reference operator*() const { 
                return _node->val;
            }
            pointer operator->() const { 
                return &_node->val;
            }
            local_iterator & operator++() { 
                _node = _node->next;
                return *this;
            }
            local_iterator operator++(int) { 
                local_iterator _nodeCopy = *this;
                _node = _node->next;
                return _nodeCopy;
            }

            bool operator==(const local_iterator &other) const noexcept { 
                return _node == other._node;
            }
            bool operator!=(const local_iterator &other) const noexcept { 
                return _node != other._node;
            }
    };

private:

    size_type _bucket(size_t code) const { 
        return code % _bucket_count;
    }
    size_type _bucket(const Key & key) const { 
        return _bucket(_hash(key));
    }
    size_type _bucket(const value_type & val) const { 
        return _bucket(val.first);
    }

    HashNode*& _find(size_type code, size_type bucket, const Key & key) { 
        HashNode** x = &(_buckets[bucket]);
        while(*x != nullptr) {
            if(_equal((*x)->val.first, key)) {
                return *x;
            }
            x = &(*x)->next;
        }
        return *x;
    }

    HashNode*& _find(const Key & key) { 
        return (_find(_hash(key), _bucket(key), key));
    }

    HashNode * _insert_into_bucket(size_type bucket, value_type && value) { 
        HashNode* tmp = new HashNode(std::move(value));

        if(bucket >= _bucket_count) {
            return nullptr;
        }

        if(_head == nullptr) {
            _head = tmp;
        }
        size_type head_index = _bucket(_head->val);
        if(bucket <= head_index) {
            _head = tmp;
        }

        tmp->next = _buckets[bucket];
        _buckets[bucket] = tmp;

        _size++;
        return tmp;
    }

    void _move_content(UnorderedMap & src, UnorderedMap & dst) { 
        dst._size = src._size;
        dst._bucket_count = src._bucket_count;
        dst._buckets = src._buckets;
        dst._head = src._head;

        src._size = 0;
        //src._bucket_count = 0;
        src._buckets = new HashNode*[src.bucket_count()]{};
        src._head = nullptr;
    }

public:
    explicit UnorderedMap(size_type bucket_count, const Hash & hash = Hash { },
                const key_equal & equal = key_equal { }) 
    { 
        _size = 0;
        _bucket_count = next_greater_prime(bucket_count);
        _buckets = new HashNode*[_bucket_count]{};
        _head = nullptr;
    }

    ~UnorderedMap() { 
        clear();
        delete[] _buckets;
        _buckets = nullptr;
        _bucket_count = 0;
    }

    UnorderedMap(const UnorderedMap & other) { 
        _size = 0;
        _bucket_count = other.bucket_count();
        _buckets = new HashNode*[_bucket_count]{};
        _head = nullptr;

        const_iterator itr = other.cbegin();
        while(itr != other.cend()) {
            insert(*itr);
            itr++;
        }        
    }

    UnorderedMap(UnorderedMap && other) { 
        _move_content(other, *this);
    }

    UnorderedMap & operator=(const UnorderedMap & other) { 
        if(&_head == &other._head) {
            *this;
        }

        clear();        
        delete[] _buckets;

        _bucket_count = other.bucket_count();
        _buckets = new HashNode*[_bucket_count]{};

        const_iterator itr = other.cbegin();
        while(itr != other.cend()) {
            insert(*itr);
            itr++;
        }
    }

    UnorderedMap & operator=(UnorderedMap && other) { 
        if(&_head == &other._head) {
            *this;
        }

        clear();        
        delete[] _buckets;

        _move_content(other, *this);
    }

    void clear() noexcept { 
        if(empty()) {
            return;
        }

        for(int i = 0; i < _bucket_count; i++) {
            HashNode* currHead = _buckets[i];
            HashNode* curr;
            while(currHead != nullptr) {
                curr = currHead->next;
                delete currHead;
                currHead = curr;
            }
            _size = 0;            
        }

        _head = nullptr;
    }

    size_type size() const noexcept { 
        return _size;
     }

    bool empty() const noexcept { 
        return _size == 0;
     }

    size_type bucket_count() const noexcept { 
        return _bucket_count;
     }

    iterator begin() { 
        iterator first(this, _head);
        return first;
    }
    iterator end() { 
        iterator last;
        return last;
    }

    const_iterator cbegin() const { 
        const_iterator first(this, _head);
        return first;
    };
    const_iterator cend() const { 
        const_iterator last;
        return last;
    };

    local_iterator begin(size_type n) { 
        local_iterator first(_buckets[n]);
        return first;
    }
    local_iterator end(size_type n) { 
        local_iterator last;
        return last;
    }

    size_type bucket_size(size_type n) { 
        size_type count = 0;
        for(auto i = begin(n); i != end(n); i++) {
            count++;
        }
        return count;
    }

    float load_factor() const { 
        return ((float)size())/((float)bucket_count());       
    }

    size_type bucket(const Key & key) const { 
        return _bucket(key);
    }

    std::pair<iterator, bool> insert(value_type && value) { 
        size_type index = _bucket(value);
        bool didInsert = true;

        HashNode*& x = _find(0, index, value.first);
        if(x != nullptr) {
            iterator itr(this, x);
            return std::pair<iterator, bool> {itr, false};
        }

        HashNode* ptr = _insert_into_bucket(index, std::move(value));
        
        iterator itr(this,ptr);
        if(ptr == nullptr) {
            didInsert = false;
        }
        return std::pair<iterator, bool> {itr, didInsert};
    }

    std::pair<iterator, bool> insert(const value_type & value) { 
        size_type index = _bucket(value);
        value_type tmpVal = value;
        bool didInsert = true;

        HashNode*& x = _find(0, index, value.first);
        if(x != nullptr) {
            iterator itr(this, x);
            return std::pair<iterator, bool> {itr, false};
        }

        HashNode* ptr = _insert_into_bucket(index, std::move(tmpVal));
        
        iterator itr(this,ptr);
        if(ptr == nullptr) {
            didInsert = false;
        }
        return std::pair<iterator, bool> {itr, didInsert};
    }

    iterator find(const Key & key) { 
        HashNode*& x = _find(key);
        return iterator(this, x);
    }

    T& operator[](const Key & key) { 
        HashNode*& x = _find(key);
        if(x != nullptr) {
            return (x->val).second;
        }
        else {
            std::pair<iterator, bool> y = insert(std::make_pair(key, T{}));
            return (*y.first).second;
        }
    }

    iterator erase(iterator pos) { 
        if(pos == end()) {
            return end();
        }

        HashNode*& x = _find((*pos).first);

        HashNode* xCopy = x;

        iterator newPos(this, x);
        newPos++;

        if(x == _head) {
            _head = newPos._ptr;
        }

        x = x->next;        
        delete xCopy;
        _size--;

        return newPos;
    }

    size_type erase(const Key & key) { 
        HashNode*& x = _find(key);
        if(x == nullptr) {
            return 0;
        }

        iterator itr = find(key);       
        itr = erase(itr);
        return 1;
        
    }

    template<typename KK, typename VV>
    friend void print_map(const UnorderedMap<KK, VV> & map, std::ostream & os);
};

template<typename K, typename V>
void print_map(const UnorderedMap<K, V> & map, std::ostream & os = std::cout) {
    using size_type = typename UnorderedMap<K, V>::size_type;
    using HashNode = typename UnorderedMap<K, V>::HashNode;

    for(size_type bucket = 0; bucket < map.bucket_count(); bucket++) {
        os << bucket << ": ";

        HashNode const * node = map._buckets[bucket];

        while(node) {
            os << "(" << node->val.first << ", " << node->val.second << ") ";
            node = node->next;
        }

        os << std::endl;
    }
}
