#ifndef NKIT_VX_H
#define NKIT_VX_H

#include "nkit/detail/str2id.h"
#include "nkit/dynamic_json.h"
#include "nkit/expat_parser.h"
#include "nkit/logger_brief.h"

namespace nkit
{
  template< typename Policy >
  class VarBuilder
  {
  public:
    typedef typename Policy::type type;

    void InitAsBoolean( std::string const & value )
    {
      p_._InitAsBoolean(value);
    }

    void InitAsBooleanFormat( std::string const & value, const std::string & )
    {
      p_._InitAsBoolean(value);
    }

    void InitAsBooleanDefault()
    {
      p_._InitAsBoolean(nkit::S_FALSE_);
    }

    void InitAsInteger( std::string const & value )
    {
      p_._InitAsInteger(value);
    }

    void InitAsIntegerFormat( std::string const & value, const std::string & )
    {
      p_._InitAsInteger(value);
    }

    void InitAsIntegerDefault()
    {
      p_._InitAsInteger(nkit::S_ZERO_);
    }

    void InitAsString( std::string const & value )
    {
      p_._InitAsString(value);
    }

    void InitAsStringFormat( std::string const & value, const std::string & )
    {
      p_._InitAsString(value);
    }

    void InitAsStringDefault()
    {
      p_._InitAsString(nkit::S_EMPTY_);
    }

    void InitAsUndefined()
    {
      p_._InitAsUndefined();
    }

    void InitAsFloat( std::string const & value )
    {
      p_._InitAsFloatFormat( value, NKIT_FORMAT_DOUBLE );
    }

    void InitAsFloatFormat( std::string const & value,
        std::string const & format )
    {
      p_._InitAsFloatFormat( value, format.c_str() );
    }

    void InitAsFloatDefault()
    {
      p_._InitAsFloatFormat( nkit::S_ZERO_, NKIT_FORMAT_DOUBLE );
    }

    void InitAsDatetime( std::string const & value )
    {
      p_._InitAsDatetimeFormat( value, nkit::DATE_TIME_DEFAULT_FORMAT_ );
    }

    void InitAsDatetimeFormat( std::string const & value,
        std::string const & format )
    {
      p_._InitAsDatetimeFormat( value, format.c_str() );
    }

    void InitAsDatetimeDefault()
    {
      p_._InitAsDatetimeFormat( "", nkit::DATE_TIME_DEFAULT_FORMAT_);
    }

    void InitAsList()
    {
      p_._InitAsList();
    }

    void InitAsDict()
    {
      p_._InitAsDict();
    }

    void AppendToList( type const & obj )
    {
      p_._ListCheck();
      p_._AppendToList(obj);
    }

    void SetDictKeyValue( std::string const & key, type const & var )
    {
      p_._DictCheck();
      p_._SetDictKeyValue( key, var );
    }

    type const & get() const
    {
      return p_.get();
    }

    std::string ToString() const
    {
      return p_.ToString();
    }

  private:
    Policy p_;
  };

  //----------------------------------------------------------------------------
  class Path
  {
  private:
    friend std::ostream & operator <<(std::ostream & stream,
        const Path & elements);

  public:
    Path() : is_mask_(false) {}

    Path(size_t element_id)
      : is_mask_(String2IdMap::STAR_ID == element_id)
    {
      elements_.push_back(element_id);
    }

    Path(const std::string & path_spec, String2IdMap * str2id)
      : is_mask_(false)
    {
      std::string path_spec_wo_attr, attr;
      nkit::simple_split(path_spec, "/@", &path_spec_wo_attr, &attr);
      nkit::StringVector path_spec_list;
      nkit::simple_split(path_spec_wo_attr, "/", &path_spec_list);
      nkit::StringVector::const_iterator element = path_spec_list.begin(),
          end = path_spec_list.end();
      for (; element != end; ++element)
      {
        if (!element->empty())
        {
          size_t id = str2id->GetId(element->c_str());
          operator /=(id);
        }
      }

      if (!attr.empty())
        SetAttribute(attr);
    }

    void SetAttribute(const std::string & attribute_name)
    {
      attribute_name_ = attribute_name;
    }

    // If one of the path is a mask (i.e. contains '*'), then make a
    // special compare: '*' is equal to any element name
    bool operator ==(const Path & another) const
    {
      if (!is_mask() && !another.is_mask())
        return elements_ == another.elements_;

      if (elements_.size() != another.elements_.size())
        return false;

      struct Comparator
      {
        static bool compare(size_t i, size_t j)
        {
          if ( (i == String2IdMap::STAR_ID) || j == (String2IdMap::STAR_ID) )
            return true;
          return (i==j);
        }
      };

      return std::equal(elements_.begin(), elements_.end(),
          another.elements_.begin(), Comparator::compare);
    }

    Path & BubbleUp()
    {
      assert(attribute_name_.empty());
      if (!elements_.empty())
        elements_.pop_back();
      return *this;
    }

    Path operator /(size_t element_id)
    {
      assert(attribute_name_.empty());
      Path new_path(*this);
      new_path /= element_id;
      return new_path;
    }

    Path operator /(const Path & tail)
    {
      assert(attribute_name_.empty());
      Path new_path(*this);
      std::vector<size_t>::const_iterator tail_element_id =
          tail.elements().begin(), end = tail.elements().end();
      for (; tail_element_id != end; ++tail_element_id)
        new_path /= *tail_element_id;
      if (!tail.attribute_name_.empty())
        new_path.SetAttribute(tail.attribute_name());
      return new_path;
    }

    Path & operator /=(size_t element_id)
    {
      assert(attribute_name_.empty());
      is_mask_ = (is_mask_ || (String2IdMap::STAR_ID == element_id));
      elements_.push_back(element_id);
      return *this;
    }

    std::string GetLastElementName(const String2IdMap & str2id) const
    {
      if (elements_.empty())
        return nkit::S_EMPTY_;
      return str2id.GetString(elements_.back());
    }

    const std::vector<size_t> & elements() const
    {
      return elements_;
    }

    const std::string & attribute_name() const
    {
      return attribute_name_;
    }

    bool is_mask() const { return is_mask_; }

    size_t size() const { return elements_.size(); }

    std::string ToString() const
    {
      std::string result;
      std::vector<size_t>::const_iterator element_id = elements().begin(),
          end = elements().end();
      for (; element_id != end; ++element_id)
        result.append("/" + nkit::string_cast(*element_id));

      if (!attribute_name_.empty())
        result.append("/@" + attribute_name_);

      return result;
    }

  private:
    bool is_mask_;
    std::vector<size_t> elements_;
    std::string attribute_name_;
  };

  //----------------------------------------------------------------------------
  std::ostream & operator <<(std::ostream & stream, const Path & path);

  //----------------------------------------------------------------------------
  const char * find_attribute_value(const char ** attrs,
      const char * attribute_name);

  //----------------------------------------------------------------------------
  template<typename T>
  class TargetItem;

  //----------------------------------------------------------------------------
  template<typename T>
  class Target
  {
  public:
    typedef NKIT_SHARED_PTR(Target<T>) Ptr;

  public:
    virtual ~Target() {}
    virtual void OnEnter() = 0;
    virtual void OnExit(const char * el) = 0;
    virtual void OnText(const char * text, size_t len) = 0;
    virtual void Clear() = 0;

    virtual void PutTargetItem(
        NKIT_SHARED_PTR(TargetItem<T>) NKIT_UNUSED(target_item))
    {}

    virtual Ptr Clone() const = 0;

    void SetOrInsertTo(const std::string & key_name,
        T & var_builder) const
    {
      var_builder.SetDictKeyValue(key_name, var());
    }

    void SetOrInsertTo(const char * key_name,
        T & var_builder) const
    {
      var_builder.SetDictKeyValue(key_name, var());
    }

    void SetOrInsertTo(const char * key_name,
        Target * parent_target) const
    {
      parent_target->var_builder_.SetDictKeyValue(key_name, var());
    }

    void AppendTo(T & var_builder) const
    {
      var_builder.AppendToList(var());
    }

    virtual typename T::type const & var() const
    {
      return var_builder_.get();
    }

    virtual std::string ToString() const
    {
      return var_builder_.ToString();
    }

  protected:
    Target() {}

  protected:
    T var_builder_;
  };

  //----------------------------------------------------------------------------
  template<typename T>
  class TargetItem
  {
  public:
    typedef NKIT_SHARED_PTR(TargetItem<T>) Ptr;
    typedef std::vector<Ptr> Vector;
    typedef typename Target<T>::Ptr TargetPtr;

  public:
    static Ptr Create(const Path & path, const std::string & key_name,
        TargetPtr target)
    {
      return Ptr(new TargetItem(path, key_name, target));
    }

    static Ptr Create(const Path & path, TargetPtr target)
    {
      return Ptr(new TargetItem(path, target));
    }

//    Ptr CloneWithNewPathKey(const Path & path, const std::string & key) const
//    {
//      assert(!path.is_mask());
//      return Ptr(new TargetItem(*this, path,
//          key_name_.empty() ? key: key_name_));
//    }
//
    Ptr Clone() const
    {
      return Ptr(new TargetItem(*this));
    }

    ~TargetItem() {}

    TargetPtr target() const { return target_; }

    void SetParentTarget(Target<T> * parent_target)
    {
      parent_target_ = parent_target;
    }

    Target<T> * parent_target() const
    {
      return parent_target_;
    }

    const std::string & key_name() const { return key_name_; }

    void SetKey(const std::string & key) { key_name_ = key; }

    void SetOrInsertTo(const char * el, T & var_builder) const
    {
      if (likely(key_name_ != S_STAR_))
        target_->SetOrInsertTo(key_name_, var_builder);
      else
        target_->SetOrInsertTo(el, var_builder);
    }

    void AppendTo(T & var_builder) const
    {
      target_->AppendTo(var_builder);
    }

    void Clear()
    {
      target_->Clear();
    }

    void OnEnter(const char ** attrs)
    {
      target_->OnEnter();
      const std::string & attribute_name = path_.attribute_name();
      if (!attribute_name.empty())
      {
        const char * attribute_value = find_attribute_value(attrs,
            attribute_name.c_str());
        if (attribute_value)
          target_->OnText(attribute_value, strlen(attribute_value));
      }
    }

    void OnExit(const char * el)
    {
      target_->OnExit(el);
      if (!key_name_.empty())
      {
        target_->SetOrInsertTo(key_name_ == S_STAR_? el: key_name_.c_str(),
                parent_target_);
        //Clear();
      }
    }

    void OnText(const char * text, size_t len)
    {
      if (path_.attribute_name().empty())
        target_->OnText(text, len);
    }

    const Path & fool_path() const
    {
      return path_;
    }

    std::string ToString() const
    {
      return std::string("path: ") + path_.ToString() + "\nkey_name: "
          + key_name_ + "\ntarget: " + target_->ToString()
          + "\nparent target: " + string_cast((uint64_t)parent_target_)
          + "\n----------------------------------";
    }

  private:
//    TargetItem(const TargetItem & another, const Path & path,
//        const std::string & key)
//      : path_(path)
//      , key_name_(key)
//      , target_(another.target_->Clone())
//      , parent_target_(another.parent_target_)
//    {}
//
//    TargetItem(const Path & path, const std::string & key_name,
//        TargetPtr target)
//      : path_(path)
//      , key_name_(key_name)
//      , target_(target)
//      , parent_target_(NULL)
//    {}

    TargetItem(const Path & path, TargetPtr target)
      : path_(path)
      , target_(target)
      , parent_target_(NULL)
    {}

  private:
    Path path_;
    std::string key_name_;
    TargetPtr target_;
    Target<T> * parent_target_;
  };

  //----------------------------------------------------------------------------
  template<typename T>
  class ObjectTarget: public Target<T>
  {
  public:
    typedef NKIT_SHARED_PTR(ObjectTarget<T>) Ptr;
    typedef typename Target<T>::Ptr TargetPtr;
    typedef typename TargetItem<T>::Ptr TargetItemPtr;
    typedef typename TargetItem<T>::Vector TargetItemVector;
    typedef typename TargetItemVector::iterator Iterator;
    typedef typename TargetItemVector::const_iterator ConstIterator;

  public:
    static Ptr Create()
    {
      return Ptr(new ObjectTarget<T>);
    }

    ~ObjectTarget() {}

    void PutTargetItem(TargetItemPtr target_item)
    {
      if (AppendTargetItem(target_item))
        target_item->SetParentTarget(this);
    }

    TargetPtr Clone() const
    {
      Ptr result(new ObjectTarget);
      ConstIterator it = target_items_.begin(), end = target_items_.end();
      for (; it != end; ++it)
        result->PutTargetItem((*it)->Clone());
      return result;
    }

  private:
    ObjectTarget()
    {
      Clear();
    }

    bool AppendTargetItem(TargetItemPtr const & target_item)
    {
      // key must not exists
      const std::string & key_name = target_item->key_name();
      Iterator it = target_items_.begin(),
          end = target_items_.end();
      for (; it != end; ++it)
        if ((*it)->key_name() == key_name)
          return false;

      size_t size = target_item->fool_path().size();

      it = target_items_.begin(),
          end = target_items_.end();
      for (; it != end; ++it)
      {
        if (size < (*it)->fool_path().size())
          break;
      }

      if (it == end)
        target_items_.push_back(target_item);
      else
        target_items_.insert(it, target_item);

      return true;
    }

    void OnEnter() {}

    void OnExit(const char * NKIT_UNUSED(el))
    {
      Iterator it = target_items_.begin(), end = target_items_.end();
      for (; it != end; ++it)
      {
        TargetItemPtr target_item = (*it);
//        target_item->SetOrInsertTo(el, Target<T>::var_builder_);
        target_item->Clear();
      }
    }

    void OnText(const char *, size_t) {}

    void Clear()
    {
      Target<T>::var_builder_.InitAsDict();
    }
//
//    std::string ToString() const
//    {
//      return "object target";
//    }

  private:
    TargetItemVector target_items_;
  };

  //----------------------------------------------------------------------------
  template<typename T>
  class ListTarget: public Target<T>
  {
    typedef Target<T> MyParent;
    typedef typename Target<T>::Ptr TargetPtr;

  public:
    typedef typename TargetItem<T>::Ptr TargetItemPtr;
    typedef typename TargetItem<T>::Vector TargetItemVector;
    typedef typename TargetItemVector::iterator Iterator;
    typedef typename TargetItemVector::const_iterator ConstIterator;
    typedef NKIT_SHARED_PTR(ListTarget<T>) Ptr;

    static Ptr Create()
    {
      return Ptr(new ListTarget<T>);
    }

    ~ListTarget() {}

    void PutTargetItem(TargetItemPtr target_item)
    {
      AppendTargetItem(target_item);
      target_item->SetParentTarget(this);
    }

    TargetPtr Clone() const
    {
      typename ListTarget::Ptr result(new ListTarget);
      ConstIterator it = target_items_.begin(), end = target_items_.end();
      for (; it != end; ++it)
      {
        TargetItemPtr target_item = (*it)->Clone();
        result->PutTargetItem(target_item);
      }
      return result;
    }

  private:
    ListTarget() {
      Clear();
    }

    void AppendTargetItem(TargetItemPtr const & target_item)
    {
      size_t size = target_item->fool_path().size();

      Iterator it = target_items_.begin(),
          end = target_items_.end();
      for (; it != end; ++it)
      {
        if (size < (*it)->fool_path().size())
          break;
      }

      if (it == end)
        target_items_.push_back(target_item);
      else
        target_items_.insert(it, target_item);
    }

    void OnEnter() {}

    void OnExit(const char * NKIT_UNUSED(el))
    {
      Iterator it = target_items_.begin(), end = target_items_.end();
      for (; it != end; ++it)
      {
        TargetItemPtr target_item = (*it);
        target_item->AppendTo(Target<T>::var_builder_);
        target_item->Clear();
      }
    }

    void OnText(const char *, size_t) {}

    void Clear()
    {
      Target<T>::var_builder_.InitAsList();
    }
//
//    std::string ToString() const
//    {
//      return "list target";
//    }

  private:
    TargetItemVector target_items_;
  };

  //----------------------------------------------------------------------------
  template<typename T,
    void (T::*InitByString)(const std::string & value),
    void (T::*InitByStringWithFormat)(const std::string & value,
        const std::string & format),
    void (T::*InitByDefault)(void)
    >
  class ScalarTarget: public Target<T>
  {
    typedef Target<T> MyParent;
    typedef typename Target<T>::Ptr TargetPtr;

    static const size_t RESERVED_LENGTH = 10240;

  public:
    virtual ~ScalarTarget() {}

    static typename MyParent::Ptr Create(const std::string & default_value,
        const std::string & format)
    {
      return typename MyParent::Ptr(new ScalarTarget(default_value, format));
    }

    static typename MyParent::Ptr Create(const std::string & default_value)
    {
      return typename MyParent::Ptr(new ScalarTarget(default_value));
    }

    static typename MyParent::Ptr Create()
    {
      return typename MyParent::Ptr(new ScalarTarget);
    }

  private:
    ScalarTarget(const std::string & default_value,
        const std::string & format)
      : use_default_value_(true)
      , format_(format)
    {
      Init();
      if (format_.empty())
        (default_value_.*InitByString)(default_value);
      else
        (default_value_.*InitByStringWithFormat)(default_value, format_);
    }

    ScalarTarget(const std::string & default_value)
      : use_default_value_(true)
    {
      Init();
      (default_value_.*InitByString)(default_value);
    }

    ScalarTarget()
      : use_default_value_(true)
    {
      Init();
      (default_value_.*InitByDefault)();
    }

    void Init()
    {
      value_.reserve(RESERVED_LENGTH);
    }

    void OnEnter()
    {
      use_default_value_ = true;
    }

    void OnExit(const char * NKIT_UNUSED(el))
    {
      if (likely(!use_default_value_))
      {
        if (format_.empty())
          (Target<T>::var_builder_.*InitByString)(value_);
        else
          (Target<T>::var_builder_.*InitByStringWithFormat)(
              value_, format_);
      }
      value_.clear();
    }

    void OnText(const char * text, size_t len)
    {
      use_default_value_ = false;
      value_.append(text, len);
    }

    void Clear()
    {
      value_.clear();
    }

    TargetPtr Clone() const
    {
      return TargetPtr(new ScalarTarget(*this));
    }

    virtual typename T::type const & var() const
    {
      if (unlikely(use_default_value_))
        return default_value_.get();
      else
      {
        use_default_value_ = true;
        return Target<T>::var_builder_.get();
      }
    }
//
//    std::string ToString() const
//    {
//      return "scalar target: " + value_;
//    }

  private:
    T default_value_;
    mutable bool use_default_value_;
    std::string value_;
    std::string format_;
  };

  //---------------------------------------------------------------------------
  template<typename T>
  class PathNode: Uncopyable
  {
  public:
    typedef NKIT_SHARED_PTR(PathNode<T>) Ptr;
    typedef typename TargetItem<T>::Ptr TargetItemPtr;
    typedef typename TargetItem<T>::Vector TargetItemVector;
    typedef typename TargetItemVector::iterator Iterator;
    typedef typename TargetItemVector::const_iterator ConstIterator;

  public:
    static Ptr CreateRoot()
    {
      return Ptr(new PathNode<T>(0));
    }

    static void MoveToChild(PathNode<T> ** current, size_t element_id)
    {
      std::vector<Ptr> & children = (**current).children_;
      typename std::vector<Ptr>::iterator child =
          children.begin(), end = children.end();
      for (; child != end; ++child)
      {
        if ((*child)->element_id_ == element_id)
        {
          *current = (*child).get();
          return;
        }
      }
      Ptr new_child(new PathNode<T>(*current, element_id));
      children.push_back(new_child);
      *current = new_child.get();
    }

    static bool MoveToParent(PathNode<T> ** current)
    {
      if (!(*current)->parent_)
        return false;
      *current = (*current)->parent_;
      return true;
    }

    void OnEnter(const char ** attrs)
    {
      Iterator target_item = target_items_.begin(), end = target_items_.end();
      for (; target_item != end; ++target_item)
        (*target_item)->OnEnter(attrs);
    }

    void OnExit(const char * el)
    {
      Iterator target_item = target_items_.begin(), end = target_items_.end();
      for (; target_item != end; ++target_item)
        (*target_item)->OnExit(el);
    }

    void OnText(const char * text, size_t len)
    {
      Iterator target_item = target_items_.begin(), end = target_items_.end();
      for (; target_item != end; ++target_item)
        (*target_item)->OnText(text, len);
    }

    void PutTargetItem(TargetItemPtr const & target_item)
    {
      PathNode<T> * current = this;
      const Path & path = target_item->fool_path();
      std::vector<size_t>::const_iterator element_id = path.elements().begin(),
          end = path.elements().end();
      for (; element_id != end; ++element_id)
        MoveToChild(&current, *element_id);
      current->AppendTargetItem(target_item);
    }

    void AppendTargetItem(TargetItemPtr const & target_item)
    {
      size_t size = target_item->fool_path().size();

      Iterator it = target_items_.begin(), end = target_items_.end();
      for (; it != end; ++it)
      {
        if (size < (*it)->fool_path().size())
          break;
      }

      if (it == end)
        target_items_.push_back(target_item);
      else
        target_items_.insert(it, target_item);
    }

    const Path & fool_path() const
    {
      return path_;
    }

    bool is_filled_from_mask_target_items() const
    {
      return filled_from_mask_target_items_;
    }

    void MarkFilledFromMaskTargetItems()
    {
      filled_from_mask_target_items_ = true;
    }

  protected:
    PathNode(size_t element_id)
      : parent_(NULL)
      , element_id_(element_id)
      , absolute_counter_(0)
      , relative_counter_(0)
      , path_(element_id)
      , filled_from_mask_target_items_(false)
    {}

    PathNode(PathNode * parent, size_t element_id)
      : parent_(parent)
      , element_id_(element_id)
      , absolute_counter_(0)
      , relative_counter_(0)
      , path_(parent->path_ / element_id)
      , filled_from_mask_target_items_(false)
    {}

  private:
    PathNode<T> * parent_;
    size_t element_id_;
    std::vector<Ptr> children_;
    size_t absolute_counter_;
    size_t relative_counter_;
    Path path_;
    bool filled_from_mask_target_items_;
    TargetItemVector target_items_;
  };

  //----------------------------------------------------------------------------
  template <typename T>
  class Xml2VarBuilder: public ExpatParser<Xml2VarBuilder<T> >
  {
  private:
    friend class ExpatParser<Xml2VarBuilder<T> > ;
    typedef typename TargetItem<T>::Ptr TargetItemPtr;
    typedef typename Target<T>::Ptr TargetPtr;
    typedef typename PathNode<T>::Ptr PathNodePtr;
    typedef typename TargetItem<T>::Vector TargetItemVector;
    typedef typename TargetItemVector::iterator TargetItemVectorIterator;

  public:
    typedef NKIT_SHARED_PTR(Xml2VarBuilder<T>) Ptr;

  public:
    static Ptr Create(const std::string & taget_spec, std::string * error)
    {
      nkit::Dynamic d_taget_spec = nkit::DynamicFromJson(taget_spec, error);
      if (!d_taget_spec)
        return Ptr();

      return Create(d_taget_spec, error);
    }

    static Ptr Create(const Dynamic & taget_spec, std::string * error)
    {
      String2IdMap str2id;
      TargetItemVector mask_target_items;
      PathNodePtr path_tree(PathNode<T>::CreateRoot());

      TargetPtr root_target = ParseRootTargetSpec(taget_spec, path_tree,
          &mask_target_items, &str2id, error);
      if (!root_target)
        return Ptr();

      return Ptr(
          new Xml2VarBuilder<T>(str2id, path_tree,
              mask_target_items, root_target));
    }

    ~Xml2VarBuilder() {}

    typename T::type const & var() const
    {
      return root_target_->var();
    }

  private:
    Xml2VarBuilder(const String2IdMap & str2id, PathNodePtr path_tree,
          TargetItemVector mask_target_items, TargetPtr root_target)
      : path_tree_(path_tree)
      , current_node_(path_tree_.get())
      , root_target_(root_target)
      , first_node_(true)
      , str2id_(str2id)
      , mask_target_items_(mask_target_items)
    {}

    bool OnStartElement(const char * el, const char ** attrs)
    {
      size_t element_id = str2id_.GetId(el);

      if (first_node_)
      {
        first_node_ = false;
        return true;
      }

      current_path_ /= element_id;
      PathNode<T>::MoveToChild(&current_node_, element_id);

      if (!mask_target_items_.empty())
      {
        TargetItemVectorIterator it = mask_target_items_.begin(),
            end = mask_target_items_.end();
        for (; it != end; ++it)
        {
          TargetItemPtr mask_target_item = (*it);
          if (mask_target_item->fool_path() == current_path_)
            mask_target_item->OnEnter(attrs);
        }
      }

//      // fill from mask-target-items
//      if (!current_node_->is_filled_from_mask_target_items())
//      {
//        TargetItemVectorIterator it = mask_target_items_.begin(),
//            end = mask_target_items_.end();
//        for (; it != end; ++it)
//        {
//          TargetItemPtr mask_target_item = (*it);
//          if (mask_target_item->fool_path() == current_path_)
//          {
//            TargetItemPtr concret_target_item =
//                mask_target_item->CloneWithNewPathKey(current_path_, el);
//            if (!concret_target_item)
//              continue;
//            if (concret_target_item->parent_target())
//              concret_target_item->parent_target()->PutTargetItem(
//                concret_target_item);
//            current_node_->AppendTargetItem(concret_target_item);
//          }
//        }
//
//        current_node_->MarkFilledFromMaskTargetItems();
//      }

      current_node_->OnEnter(attrs);
      return true;
    }

    bool OnEndElement(const char * el)
    {
      if (!mask_target_items_.empty())
      {
        TargetItemVectorIterator it = mask_target_items_.begin(),
            end = mask_target_items_.end();
        for (; it != end; ++it)
        {
          TargetItemPtr mask_target_item = (*it);
          if (mask_target_item->fool_path() == current_path_)
            mask_target_item->OnExit(el);
        }
      }

      current_node_->OnExit(el);
      current_path_.BubbleUp();
      PathNode<T>::MoveToParent(&current_node_);
      return true;
    }

    bool OnText(const char * text, int len)
    {
      current_node_->OnText(text, static_cast<size_t>(len));

      if (!mask_target_items_.empty())
      {
        TargetItemVectorIterator it = mask_target_items_.begin(),
            end = mask_target_items_.end();
        for (; it != end; ++it)
        {
          TargetItemPtr mask_target_item = (*it);
          if (mask_target_item->fool_path() == current_path_)
            mask_target_item->OnText(text, static_cast<size_t>(len));
        }
      }

      return true;
    }

    void GetCustomError(std::string * error)
    {
      *error = error_;
    }

    //--------------------------------------------------------------------------
    static TargetItemPtr ParseScalarTargetSpec(
        Target<T> * parent_target,
        Path parent_path,
        const std::string & mapping,
        PathNodePtr path_tree,
        TargetItemVector * mask_target_items,
        std::string * error)
    {
      static const std::string STRING_TYPE = "string";
      static const std::string INTEGER_TYPE = "integer";
      static const std::string NUMBER_TYPE = "number";
      static const std::string DATETIME_TYPE = "datetime";
      static const std::string BOOLEAN_TYPE = "boolean";

      nkit::StringVector spec_list;
      nkit::simple_split(mapping, "|", &spec_list);
      if (spec_list.size() < 1)
      {
        *error = "Type definition for scalar must be of following format:\n"
            "- integer|optional_integer_default\n"
            "- number|optional_number_default\n"
            "- string|optional_string_default\n"
            "- boolean|optional_boolean_default\n"
            "- datetime|mandatory_default_dateTime_value"
            		"|mandatory_formatting_string";
        return TargetItemPtr();
      }

      TargetPtr target;
      const std::string & type = spec_list[0];
      if (type == STRING_TYPE)
      {
        typedef ScalarTarget<T,
            &T::InitAsString,
            &T::InitAsStringFormat,
            &T::InitAsStringDefault> StringTarget;
        if (spec_list.size() >= 2)
          target = StringTarget::Create(spec_list[1]);
        else
          target = StringTarget::Create();
      }
      else if (type == INTEGER_TYPE)
      {
        typedef ScalarTarget<T,
            &T::InitAsInteger,
            &T::InitAsIntegerFormat,
            &T::InitAsIntegerDefault> IntegerTarget;
        if (spec_list.size() >= 2)
          target = IntegerTarget::Create(spec_list[1]);
        else
          target = IntegerTarget::Create();
      }
      else if (type == NUMBER_TYPE)
      {
        typedef ScalarTarget<T,
            &T::InitAsFloat,
            &T::InitAsFloatFormat,
            &T::InitAsFloatDefault> NumberTarget;
        if (spec_list.size() >= 3)
          target = NumberTarget::Create(spec_list[1], spec_list[2]);
        else if (spec_list.size() >= 2)
          target = NumberTarget::Create(spec_list[1]);
        else
          target = NumberTarget::Create();
      }
      else if (type == BOOLEAN_TYPE)
      {
        typedef ScalarTarget<T,
            &T::InitAsBoolean,
            &T::InitAsBooleanFormat,
            &T::InitAsBooleanDefault> BooleanTarget;
        if (spec_list.size() >= 2)
          target = BooleanTarget::Create(spec_list[1]);
        else
          target = BooleanTarget::Create();
      }
      else if (type == DATETIME_TYPE)
      {
        typedef ScalarTarget<T,
            &T::InitAsDatetime,
            &T::InitAsDatetimeFormat,
            &T::InitAsDatetimeDefault> DatetimeTarget;
        if (spec_list.size() >= 3)
          target = DatetimeTarget::Create(spec_list[1], spec_list[2]);
        else if (spec_list.size() >= 2)
          target = DatetimeTarget::Create(spec_list[1]);
        else
          target = DatetimeTarget::Create();
      }
      else
      {
        *error = "Scalar does not support type '" + type + "'";
        return TargetItemPtr();
      }

      TargetItemPtr target_item = TargetItem<T>::Create(parent_path, target);
      target_item->SetParentTarget(parent_target);

      if (parent_path.is_mask())
        mask_target_items->push_back(target_item);
      else
        path_tree->PutTargetItem(target_item);

      return target_item;
    }

    //--------------------------------------------------------------------------
    static TargetItemPtr ParseListTargetSpec(
        Target<T> * parent_target,
        Path parent_path,
        const nkit::Dynamic & mapping,
        PathNodePtr path_tree,
        TargetItemVector * mask_target_items,
        String2IdMap * str2id,
        std::string * error)
    {
      size_t count = mapping.size();
      if (count != 2)
      {
        *error = "List mapping must have two elements: "
          "path/to/xml/element/with/data and sub-mapping";
        return TargetItemPtr();
      }

      typename ListTarget<T>::Ptr target =
          ListTarget<T>::Create();

      Path path(mapping.GetByIndex(0).GetString(), str2id);
      const Dynamic & sum_mapping = mapping.GetByIndex(1);

      Path fool_path(parent_path / path);

      TargetItemPtr child_target_item =
          ParseTargetSpec(target.get(), fool_path, sum_mapping,
              path_tree, mask_target_items, str2id, error);
      if (!child_target_item)
        return TargetItemPtr();

//      if (fool_path.is_mask())
//        child_target_item->SetParentTarget(target.get());
//      else if (!child_target_item->fool_path().is_mask())
        target->PutTargetItem(child_target_item);

      TargetItemPtr target_item = TargetItem<T>::Create(fool_path,
      		target);
      target_item->SetParentTarget(parent_target);

      if (fool_path.is_mask())
        mask_target_items->push_back(target_item);
      else
        path_tree->PutTargetItem(target_item);

      return target_item;
    }

    //--------------------------------------------------------------------------
    static TargetItemPtr ParseObjectTargetSpec(
        Target<T> * parent_target,
        Path parent_path,
        const nkit::Dynamic & mapping,
        PathNodePtr path_tree,
        TargetItemVector * mask_target_items,
        String2IdMap * str2id,
        std::string * error)
    {
      typename ObjectTarget<T>::Ptr target =
          ObjectTarget<T>::Create();

      DDICT_FOREACH(pair, mapping)
      {
        std::string path_spec, key;
        nkit::simple_split(pair->first, "->", &path_spec, &key);
        Path path(path_spec, str2id);
        if (key.empty())
        {
          if (!path.attribute_name().empty())
          {
            key = path.attribute_name();
            if (key == S_STAR_)
            {
              *error = "Attribute name can not be '*'";
              return TargetItemPtr();
            }
          }
          else
            key = path.GetLastElementName(*str2id);
        }

        Path fool_path(parent_path / path);
        TargetItemPtr child_target_item = ParseTargetSpec(target.get(),
            fool_path, pair->second, path_tree, mask_target_items,
            str2id, error);
        if (!child_target_item)
          return TargetItemPtr();

        child_target_item->SetKey(key);

//        if (fool_path.is_mask())
//          child_target_item->SetParentTarget(target.get());
//        else
          target->PutTargetItem(child_target_item);
      }

      TargetItemPtr target_item =
          TargetItem<T>::Create(parent_path, target);
      target_item->SetParentTarget(parent_target);

      if (parent_path.is_mask())
        mask_target_items->push_back(target_item);
      else
        path_tree->PutTargetItem(target_item);

      return target_item;
    }

    //--------------------------------------------------------------------------
    static TargetItemPtr ParseTargetSpec(
        Target<T> * parent_target,
        Path parent_path,
        const nkit::Dynamic & mapping,
        PathNodePtr path_tree,
        TargetItemVector * mask_target_items,
        String2IdMap * str2id,
        std::string * error)
    {
      TargetItemPtr target_item;
      if (mapping.IsList())
        target_item = ParseListTargetSpec(parent_target, parent_path,
          mapping, path_tree, mask_target_items, str2id, error);
      else if (mapping.IsDict())
        target_item = ParseObjectTargetSpec(parent_target, parent_path,
          mapping, path_tree, mask_target_items, str2id, error);
      else if (mapping.IsString())
        target_item = ParseScalarTargetSpec(parent_target, parent_path,
          mapping.GetConstString(), path_tree, mask_target_items, error);
      else
      {
        *error = "Child mapping can be dictionary (object), list or string";
        return TargetItemPtr();
      }

      return target_item;
    }

    //--------------------------------------------------------------------------
    static TargetPtr ParseRootTargetSpec(
        const nkit::Dynamic & mapping,
        PathNodePtr path_tree,
        TargetItemVector * mask_target_items,
        String2IdMap * str2id,
        std::string * error)
    {
      Path empty_path;

      TargetItemPtr target_item;
      if (mapping.IsList())
        target_item = ParseListTargetSpec(NULL, empty_path, mapping,
          path_tree, mask_target_items, str2id, error);
      else if (mapping.IsDict())
        target_item = ParseObjectTargetSpec(NULL, empty_path, mapping,
          path_tree, mask_target_items, str2id, error);
      else
      {
        *error =
            "Root mapping can not be scalar - only dictionary (object) or list";
        return TargetPtr();
      }

      if (!target_item)
        return TargetPtr();

      return target_item->target();
    }

    //--------------------------------------------------------------------------
  private:
    std::string error_;
    PathNodePtr path_tree_;
    PathNode<T> * current_node_;
    Path current_path_;
    TargetPtr root_target_;
    bool first_node_;
    String2IdMap str2id_;
    TargetItemVector mask_target_items_;
  };

} // namespace nkit

#endif // NKIT_VX_H
