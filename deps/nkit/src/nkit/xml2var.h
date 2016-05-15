#ifndef NKIT_XML2VAR_H
#define NKIT_XML2VAR_H

#include <stack>

#include "nkit/detail/str2id.h"
#include "nkit/dynamic_json.h"
#include "nkit/dynamic_getter.h"
#include "nkit/expat_parser.h"
#include "nkit/logger_brief.h"
#include "nkit/tools.h"

namespace nkit
{
  namespace detail
  {
    struct Options
    {
      static const bool TRIM_DEFAULT;
      static const bool UNICODE_DEFAULT;
      static const bool ORDERED_DICT;

      typedef NKIT_SHARED_PTR(Options)Ptr;

      static Ptr Create()
      {
        return Ptr(new Options);
      }

      static Ptr Create(const Dynamic & options, std::string * error)
      {
        if (!options)
        return Create();
        DynamicGetter config(options);
        return Create(config, error);
      }

      static Ptr Create(const std::string & options, std::string * error)
      {
        if (options.empty())
        return Create();
        DynamicGetter config(options, "");
        return Create(config, error);
      }

      static Ptr Create(DynamicGetter & config, std::string * error)
      {
        Ptr ret(new Options);
        config
          .Get(".trim", &ret->trim_, TRIM_DEFAULT)
          .Get(".white_spaces", &ret->white_spaces_, WHITE_SPACES)
          .Get(".unicode", &ret->unicode_, UNICODE_DEFAULT)
          .Get(".attrkey", &ret->attrkey_, S_EMPTY_)
          .Get(".textkey", &ret->textkey_, S_EMPTY_)
          .Get(".ordered_dict", &ret->ordered_dict_, ORDERED_DICT)
        ;

        if (!config.ok())
        {
          *error = config.error();
          return Ptr();
        }

        return ret;
      }

      Options()
        : trim_(TRIM_DEFAULT)
        , white_spaces_(WHITE_SPACES)
        , unicode_(UNICODE_DEFAULT)
        , ordered_dict_(ORDERED_DICT)
      {}

      bool trim_;
      std::string white_spaces_;
      bool unicode_;
      bool ordered_dict_;
      std::string attrkey_;
      std::string textkey_;
    };
  } // namespace detail

  template<typename Policy>
  class VarBuilder: Uncopyable
  {
  public:
    typedef NKIT_SHARED_PTR(VarBuilder<Policy>)Ptr;
    typedef typename Policy::type type;

    VarBuilder(const detail::Options::Ptr & options)
      : p_(*options)
      , options_(options)
    {}

    void InitAsBoolean( std::string const & value )
    {
      p_.InitAsBoolean(value);
    }

    void InitAsBooleanFormat( std::string const & value, const std::string & )
    {
      p_.InitAsBoolean(value);
    }

    void InitAsInteger( std::string const & value )
    {
      p_.InitAsInteger(value);
    }

    void InitAsIntegerFormat( std::string const & value, const std::string & )
    {
      p_.InitAsInteger(value);
    }

    void InitAsString( std::string const & value )
    {
      p_.InitAsString(value);
    }

    void InitAsStringFormat( std::string const & value, const std::string & )
    {
      p_.InitAsString(value);
    }

    void InitAsUndefined()
    {
      p_.InitAsUndefined();
    }

    static const type & GetUndefined()
    {
      return Policy::GetUndefined();
    }

    void InitAsFloat( std::string const & value )
    {
      p_.InitAsFloatFormat( value, NKIT_FORMAT_DOUBLE );
    }

    void InitAsFloatFormat( std::string const & value,
        std::string const & format )
    {
      p_.InitAsFloatFormat( value, format.c_str() );
    }

    void InitAsDatetime( std::string const & value )
    {
      p_.InitAsDatetimeFormat( value, DATE_TIME_DEFAULT_FORMAT() );
    }

    void InitAsDatetimeFormat( std::string const & value,
        std::string const & format )
    {
      p_.InitAsDatetimeFormat( value, format.c_str() );
    }

    void InitAsList()
    {
      p_.InitAsList();
    }

    void InitAsDict()
    {
      p_.InitAsDict();
    }

    void SetAttrKey(const char ** attrs)
    {
      if (!options_->attrkey_.empty() && attrs[0])
      {
        VarBuilder<Policy> attr_builder(options_);
        attr_builder.InitAsDict();
        for (size_t i = 0; attrs[i] && attrs[i + 1]; ++(++i))
        attr_builder.SetDictKeyValue(std::string(attrs[i]),
            std::string(attrs[i + 1]));
        SetDictKeyValue(options_->attrkey_, attr_builder.get());
      }
    }

    void AppendToList( type const & obj )
    {
      p_.ListCheck();
      p_.AppendToList(obj);
    }

    void SetDictKeyValue( std::string const & key, type const & var )
    {
      p_.DictCheck();
      p_.SetDictKeyValue(key, var);
    }

    void SetDictKeyValue( std::string const & key, std::string const & var )
    {
      VarBuilder<Policy> string_value_builder(options_);
      string_value_builder.InitAsString(var);
      p_.SetDictKeyValue(key, string_value_builder.get());
    }

    void AppendToDictKeyList( std::string const & key, type const & var )
    {
      p_.AppendToDictKeyList(key, var);
    }

    void AppendToDictKeyList( std::string const & key, std::string const & var )
    {
      VarBuilder<Policy> string_value_builder(options_);
      string_value_builder.InitAsString(var);
      p_.AppendToDictKeyList(key, string_value_builder.get());
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
    detail::Options::Ptr options_;
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
      simple_split(path_spec, "/@", &path_spec_wo_attr, &attr);
      StringVector path_spec_list;
      simple_split(path_spec_wo_attr, "/", &path_spec_list);
      StringVector::const_iterator element = path_spec_list.begin(),
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
        return S_EMPTY_;
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
        result.append("/" + string_cast(*element_id));

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
  class Target: Uncopyable
  {
  public:
    typedef NKIT_SHARED_PTR(Target<T>) Ptr;

  public:
    virtual ~Target() {}
    virtual void OnEnter(const char ** attrs) = 0;
    virtual void OnExit(const char * el) = 0;
    virtual void OnText(const char * text, size_t len) = 0;
    virtual void Clear() = 0;

    virtual void PutTargetItem(
        NKIT_SHARED_PTR(TargetItem<T>) NKIT_UNUSED(target_item))
    {}

    //virtual Ptr Clone() const = 0;

    virtual bool must_use_default_value() const
    {
      return false;
    }

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
    Target(const detail::Options::Ptr & options)
      : options_(options)
      , var_builder_(options)
    {}

  protected:
    const detail::Options::Ptr options_;
    T var_builder_;
  };

  //----------------------------------------------------------------------------
  template<typename T>
  class TargetItem: Uncopyable
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

    bool must_use_default_value() const
    {
      return target_->must_use_default_value();
    }

    const std::string & key_name() const { return key_name_; }

    void SetKey(const std::string & key, bool is_attribute)
    {
      key_name_ = key;
      actual_key_name_ = key;
      key_name_is_attribute_ = is_attribute;
    }

    void SetOrInsertTo(const char * el, T & var_builder) const
    {
      if (likely(actual_key_name_ != S_STAR_))
        target_->SetOrInsertTo(actual_key_name_, var_builder);
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
      target_->OnEnter(attrs);
      const std::string & attribute_name = path_.attribute_name();
      if (!attribute_name.empty())
      {
        const char * attribute_value = find_attribute_value(attrs,
            attribute_name.c_str());
        if (attribute_value)
          target_->OnText(attribute_value, strlen(attribute_value));
      }

      if (key_name_is_attribute_)
      {
        const char * actual_key_name = find_attribute_value(attrs,
            key_name_.c_str());
        if (actual_key_name)
          actual_key_name_ = actual_key_name;
        else
          actual_key_name_ = key_name_;
      }
    }

    void OnExit(const char * el)
    {
      target_->OnExit(el);
      if (!actual_key_name_.empty())
      {
        target_->SetOrInsertTo(
          (actual_key_name_ == S_STAR_) ? el: actual_key_name_.c_str(),
          parent_target_
        );
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
    TargetItem(const Path & path, TargetPtr target)
      : path_(path)
      , target_(target)
      , parent_target_(NULL)
      , key_name_is_attribute_(false)
    {}

  private:
    Path path_;
    TargetPtr target_;
    Target<T> * parent_target_;
    std::string key_name_;
    bool key_name_is_attribute_;
    std::string actual_key_name_;
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
    static Ptr Create(const detail::Options::Ptr & options)
    {
      return Ptr(new ObjectTarget<T>(options));
    }

    ~ObjectTarget() {}

    void PutTargetItem(TargetItemPtr target_item)
    {
      if (AppendTargetItem(target_item))
        target_item->SetParentTarget(this);
    }

  private:
    ObjectTarget(const detail::Options::Ptr & options)
      : Target<T>(options)
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

    void OnEnter(const char ** attrs)
    {
      Target<T>::var_builder_.SetAttrKey(attrs);
    }

    void OnExit(const char * el)
    {
      Iterator it = target_items_.begin(), end = target_items_.end();
      for (; it != end; ++it)
      {
        TargetItemPtr target_item = (*it);
        if (target_item->must_use_default_value())
          target_item->SetOrInsertTo(el, Target<T>::var_builder_);
        target_item->Clear();
      }
    }

    void OnText(const char *, size_t) {}

    void Clear()
    {
      Target<T>::var_builder_.InitAsDict();
    }

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

    static Ptr Create(const detail::Options::Ptr & options)
    {
      return Ptr(new ListTarget<T>(options));
    }

    ~ListTarget() {}

    void PutTargetItem(TargetItemPtr target_item)
    {
      AppendTargetItem(target_item);
      target_item->SetParentTarget(this);
    }

  private:
    ListTarget(const detail::Options::Ptr & options)
      : Target<T>(options)
    {
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

    void OnEnter(const char ** NKIT_UNUSED(attrs)) {}

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

  private:
    TargetItemVector target_items_;
  };

  //----------------------------------------------------------------------------
  template<typename T,
    void (T::*InitByString)(const std::string & value),
    void (T::*InitByStringWithFormat)(const std::string & value,
        const std::string & format)
    >
  class ScalarTarget: public Target<T>
  {
    typedef Target<T> MyParent;
    typedef typename Target<T>::Ptr TargetPtr;

    static const size_t RESERVED_LENGTH = 10240;

  public:
    virtual ~ScalarTarget() {}

    static typename MyParent::Ptr Create(const detail::Options::Ptr & options,
        const std::string & default_value,
        const std::string & format)
    {
      return typename MyParent::Ptr(new ScalarTarget(options,
          default_value, format));
    }

    static typename MyParent::Ptr Create(const detail::Options::Ptr & options,
        const std::string & default_value)
    {
      return typename MyParent::Ptr(new ScalarTarget(options, default_value));
    }

    static typename MyParent::Ptr Create(const detail::Options::Ptr & options)
    {
      return typename MyParent::Ptr(new ScalarTarget(options));
    }

  private:
    ScalarTarget(const detail::Options::Ptr & options,
        const std::string & default_value,
        const std::string & format)
      : Target<T>(options)
      , default_value_(options)
      , use_default_value_(true)
      , has_default_value_(true)
      , value_("")
      , format_(format)
    {
      Init();
      std::string trimed_default_value;
      const std::string * p_default_value = & default_value;
      if (Target<T>::options_->trim_)
      {
        trimed_default_value = trim(default_value,
            Target<T>::options_->white_spaces_);
        p_default_value = & trimed_default_value;
      }

      if (format_.empty())
        (default_value_.*InitByString)(*p_default_value);
      else
        (default_value_.*InitByStringWithFormat)(*p_default_value, format_);
    }

    ScalarTarget(const detail::Options::Ptr & options,
        const std::string & default_value)
      : Target<T>(options)
      , default_value_(options)
      , use_default_value_(true)
      , has_default_value_(true)
      , value_("")
    {
      Init();
      (default_value_.*InitByString)(default_value);
    }

    ScalarTarget(const detail::Options::Ptr & options)
      : Target<T>(options)
      , default_value_(options)
      , use_default_value_(false)
      , has_default_value_(false)
      , value_("")
    {
      Init();
    }

    void Init()
    {
      value_.reserve(RESERVED_LENGTH);
    }

    void OnEnter(const char ** NKIT_UNUSED(attrs))
    {
      use_default_value_ = true;
    }

    void OnExit(const char * NKIT_UNUSED(el))
    {
      if (likely(!must_use_default_value()))
      {
        if (Target<T>::options_->trim_)
          value_ = trim(value_, Target<T>::options_->white_spaces_);

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
      use_default_value_ = true;
    }

    virtual bool must_use_default_value() const
    {
      return has_default_value_ && use_default_value_;
    }

    virtual typename T::type const & var() const
    {
      if (unlikely(must_use_default_value()))
        return default_value_.get();
      else
        return Target<T>::var_builder_.get();
    }

  private:
    T default_value_;
    mutable bool use_default_value_;
    mutable bool has_default_value_;
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
  class StructXml2VarBuilder: public ExpatParser<StructXml2VarBuilder<T> >
  {
  private:
    friend class ExpatParser<StructXml2VarBuilder<T> > ;
    typedef typename TargetItem<T>::Ptr TargetItemPtr;
    typedef typename Target<T>::Ptr TargetPtr;
    typedef typename PathNode<T>::Ptr PathNodePtr;
    typedef typename TargetItem<T>::Vector TargetItemVector;
    typedef typename TargetItemVector::iterator TargetItemVectorIterator;
    typedef std::map<std::string, TargetPtr> RootTargets;

  public:
    typedef NKIT_SHARED_PTR(StructXml2VarBuilder<T>) Ptr;

  public:
    static Ptr Create(const std::string & options, std::string * error)
    {
      detail::Options::Ptr o = detail::Options::Create(options, error);
      if (!o)
        return Ptr();
      return Ptr(new StructXml2VarBuilder<T>(o));
    }

    static Ptr Create(const std::string & options,
        const std::string & mappings, std::string * error)
    {
      detail::Options::Ptr o = detail::Options::Create(options, error);
      if (!o)
        return Ptr();
      Dynamic m = DynamicFromJson(mappings, error);
      if (!m)
        return Ptr();

      Ptr ret(new StructXml2VarBuilder<T>(o));

      DDICT_FOREACH(pair, m)
      {
        if (!ret->AddMapping(pair->first, pair->second, error))
          return Ptr();
      }

      return ret;
    }

    static Ptr Create(const Dynamic & options, std::string * error)
    {
      detail::Options::Ptr o = detail::Options::Create(options, error);
      if (!o)
        return Ptr();
      return Ptr(new StructXml2VarBuilder<T>(o));
    }

    bool AddMapping(const std::string & target_name,
        const std::string & mapping, std::string * error)
    {
      Dynamic m = DynamicFromJson(mapping, error);
      if (!m)
        return false;
      return AddMapping(target_name, m, error);
    }

    bool AddMapping(const std::string & target_name,
        const Dynamic & mapping, std::string * error)
    {
      String2IdMap str2id;
      TargetItemVector mask_target_items;
      PathNodePtr path_tree(PathNode<T>::CreateRoot());

      TargetPtr root_target = ParseMapping(mapping, options_, path_tree_,
          &mask_target_items_, &str2id_, error);
      if (!root_target)
        return false;
      root_targets_[target_name] = root_target;
      return true;
    }

    ~StructXml2VarBuilder() {}

    StringList mapping_names() const
    {
      StringList ret;
      typename RootTargets::const_iterator it = root_targets_.begin(),
          end = root_targets_.end();
      for (; it != end; ++it)
        ret.push_back(it->first);
      return ret;
    }

    const typename T::type & var(const std::string & target_name) const
    {
      typename RootTargets::const_iterator
        found = root_targets_.find(target_name),
        not_found = root_targets_.end();
      if (unlikely(found == not_found))
        return T::GetUndefined();
      else
        return found->second->var();
    }

  private:
    StructXml2VarBuilder(detail::Options::Ptr o)
      : path_tree_(PathNode<T>::CreateRoot())
      , current_node_(path_tree_.get())
      , options_(o)
      , root_targets_()
      , first_node_(true)
      , str2id_()
      , mask_target_items_()
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
        const detail::Options::Ptr & options,
        PathNodePtr path_tree,
        TargetItemVector * mask_target_items,
        std::string * error)
    {
      static const std::string STRING_TYPE = "string";
      static const std::string INTEGER_TYPE = "integer";
      static const std::string NUMBER_TYPE = "number";
      static const std::string DATETIME_TYPE = "datetime";
      static const std::string BOOLEAN_TYPE = "boolean";

      StringVector spec_list;
      simple_split(mapping, "|", &spec_list);
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
            &T::InitAsStringFormat> StringTarget;
        if (spec_list.size() >= 2)
          target = StringTarget::Create(options, spec_list[1]);
        else
          target = StringTarget::Create(options);
      }
      else if (type == INTEGER_TYPE)
      {
        typedef ScalarTarget<T,
            &T::InitAsInteger,
            &T::InitAsIntegerFormat> IntegerTarget;
        if (spec_list.size() >= 2)
          target = IntegerTarget::Create(options, spec_list[1]);
        else
          target = IntegerTarget::Create(options);
      }
      else if (type == NUMBER_TYPE)
      {
        typedef ScalarTarget<T,
            &T::InitAsFloat,
            &T::InitAsFloatFormat> NumberTarget;
        if (spec_list.size() >= 3)
          target = NumberTarget::Create(options, spec_list[1], spec_list[2]);
        else if (spec_list.size() >= 2)
          target = NumberTarget::Create(options, spec_list[1]);
        else
          target = NumberTarget::Create(options);
      }
      else if (type == BOOLEAN_TYPE)
      {
        typedef ScalarTarget<T,
            &T::InitAsBoolean,
            &T::InitAsBooleanFormat> BooleanTarget;
        if (spec_list.size() >= 2)
          target = BooleanTarget::Create(options, spec_list[1]);
        else
          target = BooleanTarget::Create(options);
      }
      else if (type == DATETIME_TYPE)
      {
        typedef ScalarTarget<T,
            &T::InitAsDatetime,
            &T::InitAsDatetimeFormat> DatetimeTarget;
        if (spec_list.size() >= 3)
          target = DatetimeTarget::Create(options, spec_list[1], spec_list[2]);
        else if (spec_list.size() >= 2)
          target = DatetimeTarget::Create(options, spec_list[1]);
        else
          target = DatetimeTarget::Create(options);
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
        const Dynamic & mapping,
        const detail::Options::Ptr & options,
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
          ListTarget<T>::Create(options);

      Path path(mapping.GetByIndex(0).GetString(), str2id);
      const Dynamic & sum_mapping = mapping.GetByIndex(1);

      Path fool_path(parent_path / path);

      TargetItemPtr child_target_item =
          ParseTargetSpec(target.get(), fool_path, sum_mapping,
              options, path_tree, mask_target_items, str2id, error);
      if (!child_target_item)
        return TargetItemPtr();

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
        const Dynamic & mapping,
        const detail::Options::Ptr & options,
        PathNodePtr path_tree,
        TargetItemVector * mask_target_items,
        String2IdMap * str2id,
        std::string * error)
    {
      typename ObjectTarget<T>::Ptr target =
          ObjectTarget<T>::Create(options);

      DDICT_FOREACH(pair, mapping)
      {
        std::string path_spec, key;
        simple_split(pair->first, "->", &path_spec, &key);
        Path path(path_spec, str2id);
        bool key_is_attribute = false;
        if (key.empty())
        {
          if (!path.attribute_name().empty())
          {
            key = path.attribute_name();
            if (key == S_STAR_)
            {
              *error = "Attribute name should not be '*'";
              return TargetItemPtr();
            }
          }
          else
            key = path.GetLastElementName(*str2id);
        }
        else if (starts_with(key, "@"))
        {
          key_is_attribute = true;
          key.erase(0, 1);
          if (key.empty())
          {
            *error = "Key alias should not be '@'";
            return TargetItemPtr();
          }
        }

        Path fool_path(parent_path / path);
        TargetItemPtr child_target_item = ParseTargetSpec(target.get(),
            fool_path, pair->second, options, path_tree, mask_target_items,
            str2id, error);
        if (!child_target_item)
          return TargetItemPtr();

        child_target_item->SetKey(key, key_is_attribute);

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
        const Dynamic & mapping,
        const detail::Options::Ptr & options,
        PathNodePtr path_tree,
        TargetItemVector * mask_target_items,
        String2IdMap * str2id,
        std::string * error)
    {
      TargetItemPtr target_item;
      if (mapping.IsList())
        target_item = ParseListTargetSpec(parent_target, parent_path,
          mapping, options, path_tree, mask_target_items, str2id, error);
      else if (mapping.IsDict())
        target_item = ParseObjectTargetSpec(parent_target, parent_path,
          mapping, options, path_tree, mask_target_items, str2id, error);
      else if (mapping.IsString())
        target_item = ParseScalarTargetSpec(parent_target, parent_path,
          mapping.GetConstString(), options, path_tree, mask_target_items, error);
      else
      {
        *error = "Child mapping can be dictionary (object), list or string";
        return TargetItemPtr();
      }

      return target_item;
    }

    //--------------------------------------------------------------------------
    static TargetPtr ParseMapping(
        const Dynamic & mapping,
        const detail::Options::Ptr & options,
        PathNodePtr path_tree,
        TargetItemVector * mask_target_items,
        String2IdMap * str2id,
        std::string * error)
    {
      Path empty_path;

      TargetItemPtr target_item;
      if (mapping.IsList())
        target_item = ParseListTargetSpec(NULL, empty_path, mapping,
            options, path_tree, mask_target_items, str2id, error);
      else if (mapping.IsDict())
        target_item = ParseObjectTargetSpec(NULL, empty_path, mapping,
            options, path_tree, mask_target_items, str2id, error);
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

  //----------------------------------------------------------------------------
  private:
    std::string error_;
    PathNodePtr path_tree_;
    PathNode<T> * current_node_;
    detail::Options::Ptr options_;
    Path current_path_;
    RootTargets root_targets_;
    bool first_node_;
    String2IdMap str2id_;
    TargetItemVector mask_target_items_;
  }; // StructXml2VarBuilder

  //----------------------------------------------------------------------------
  template <typename T>
  class AnyXml2VarBuilder: public ExpatParser<AnyXml2VarBuilder<T> >
  {
  private:
    friend class ExpatParser<AnyXml2VarBuilder<T> > ;
    typedef typename T::Ptr VarBuilderPtr;

  public:
    typedef NKIT_SHARED_PTR(AnyXml2VarBuilder<T>) Ptr;

  public:
    static Ptr Create(const std::string & options, std::string * error)
    {
      detail::Options::Ptr o = detail::Options::Create(options, error);
      if (!o)
        return Ptr();
      return Ptr(new AnyXml2VarBuilder<T>(o));
    }

    static Ptr Create(const Dynamic & options, std::string * error)
    {
      detail::Options::Ptr o = detail::Options::Create(options, error);
      if (!o)
        return Ptr();
      return Ptr(new AnyXml2VarBuilder<T>(o));
    }

    const typename T::type & var() const
    {
      return root_var_builder_->get();
    }

    const std::string & root_name() const
    {
      return root_name_;
    }

    bool Clear(const Dynamic & options, std::string * error)
    {
      detail::Options::Ptr o = detail::Options::Create(options, error);
      if (!o)
        return false;
      options_ = o;
      Clear();
      return true;
    }

    void Clear()
    {
      root_var_builder_ = VarBuilderPtr(new T(options_));
      first_ = true;
      root_name_.clear();
      clear_stack(current_text_stack_);
      clear_stack(is_simple_element_stack_);
      clear_stack(var_builder_stack_);

      current_text_stack_.push(S_EMPTY_);
      is_simple_element_stack_.push(false);
      root_var_builder_->InitAsDict();
      var_builder_stack_.push(root_var_builder_);
      if (options_->attrkey_.empty())
        options_->attrkey_ = "$";
      if (options_->textkey_.empty())
        options_->textkey_ = "_";
    }

  private:
    AnyXml2VarBuilder(detail::Options::Ptr o)
      : options_(o)
      , first_(true)
    {
      Clear();
    }

    bool OnStartElement(const char * el, const char ** attrs)
    {
      bool has_attrs = (attrs[0] != NULL);
      if (unlikely(first_))
      {
        root_name_.assign(el);
        first_ = false;
        if (has_attrs)
          root_var_builder_->SetAttrKey(attrs);
      }
      else
      {
        is_simple_element_stack_.top() = false;
        VarBuilderPtr var_builder_(new T(options_));
        var_builder_->InitAsDict();
        if (has_attrs)
          var_builder_->SetAttrKey(attrs);
        is_simple_element_stack_.push(!has_attrs);
        var_builder_stack_.push(var_builder_);
        current_text_stack_.push(S_EMPTY_);
      }
      return true;
    }

    bool OnEndElement(const char * el)
    {
      std::string & current_text = current_text_stack_.top();
      if (options_->trim_)
        current_text.assign(trim(current_text, options_->white_spaces_));

      if (is_simple_element_stack_.top())
      {
        var_builder_stack_.pop();
        var_builder_stack_.top()->AppendToDictKeyList(el, current_text);
      }
      else
      {
        VarBuilderPtr last = var_builder_stack_.top();
        if (!current_text.empty())
          last->SetDictKeyValue(options_->textkey_, current_text);
        var_builder_stack_.pop();
        if (!var_builder_stack_.empty())
          var_builder_stack_.top()->AppendToDictKeyList(el, (*last).get());
      }

      is_simple_element_stack_.pop();
      current_text_stack_.pop();
      return true;
    }

    bool OnText(const char * text, int len)
    {
      current_text_stack_.top().append(text, len);
      return true;
    }

    void GetCustomError(std::string * error)
    {
      *error = error_;
    }

  //----------------------------------------------------------------------------
  private:
    std::string error_;
    detail::Options::Ptr options_;
    VarBuilderPtr root_var_builder_;
    bool first_;
    std::string root_name_;
    std::stack<VarBuilderPtr> var_builder_stack_;
    std::stack<bool> is_simple_element_stack_;
    std::stack<std::string> current_text_stack_;
  }; // AnyXml2VarBuilder

} // namespace nkit

#endif // NKIT_XML2VAR_H
