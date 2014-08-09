#ifndef NKIT_VX_H
#define NKIT_VX_H

#include "nkit/detail/str2id.h"
#include "nkit/dynamic_json.h"
#include "nkit/expat_parser.h"
#include "nkit/logger_brief.h"

namespace nkit
{
  //---------------------------------------------------------------------------
  class Path
  {
  private:
    friend std::ostream & operator <<(std::ostream & stream,
        const Path & elements);

  public:
    Path() {}

    Path(size_t element_id)
    {
      elements_.push_back(element_id);
    }

    Path(const std::string & path_spec, String2IdMap * str2id)
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
          elements_.push_back(id);
        }
      }

      if (!attr.empty())
        SetAttribute(attr);
    }

    void SetAttribute(const std::string & attribute_name)
    {
      attribute_name_ = attribute_name;
    }

    bool operator ==(const Path & another)
    {
      return elements_ == another.elements_;
    }

    Path operator /(size_t element_id)
    {
      assert(attribute_name_.empty());
      Path new_path(*this);
      new_path.elements_.push_back(element_id);
      return new_path;
    }

    Path operator /(const Path & tail)
    {
      assert(attribute_name_.empty());
      Path new_path(*this);
      std::vector<size_t>::const_iterator tail_element_id =
          tail.elements().begin(), end = tail.elements().end();
      for (; tail_element_id != end; ++tail_element_id)
        new_path.elements_.push_back(*tail_element_id);
      if (!tail.attribute_name_.empty())
        new_path.SetAttribute(tail.attribute_name());
      return new_path;
    }

    Path & operator /=(size_t element_id)
    {
      assert(attribute_name_.empty());
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
    std::vector<size_t> elements_;
    std::string attribute_name_;
  };

  //---------------------------------------------------------------------------
  std::ostream & operator <<(std::ostream & stream, const Path & path);

  //---------------------------------------------------------------------------
  const char * find_attribute_value(const char ** attrs,
      const char * attribute_name);

  //---------------------------------------------------------------------------
  template<typename VarBuilder>
  class Target: Uncopyable
  {
  public:
    typedef NKIT_SHARED_PTR(Target<VarBuilder>) Ptr;

  public:
    virtual ~Target() {}
    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void OnText(const char * text, size_t len) = 0;
    virtual void Clear() = 0;

    void SetOrInsertTo(const std::string & key_name,
        VarBuilder & var_builder)
    {
      var_builder.SetDictKeyValue(key_name, var());
    }

    void AppendTo(VarBuilder & var_builder)
    {
      var_builder.AppendToList(var());
    }

    virtual typename VarBuilder::type const & var()
    {
      return var_builder_.get();
    }

    virtual std::string ToString() const = 0;

  protected:
    Target() {}

  protected:
    VarBuilder var_builder_;
  };

  //---------------------------------------------------------------------------
  template<typename VarBuilder>
  class TargetItem: Uncopyable
  {
  public:
    typedef NKIT_SHARED_PTR(TargetItem<VarBuilder>) Ptr;

  public:
    static Ptr Create(const Path & path, const std::string & key_name,
        typename Target<VarBuilder>::Ptr target)
    {
      return Ptr(new TargetItem(path, key_name, target));
    }
    static Ptr Create(const Path & path,
        typename Target<VarBuilder>::Ptr target)
    {
      return Ptr(new TargetItem(path, target));
    }

    ~TargetItem() {}

    void SetOrInsertTo(VarBuilder & host_object)
    {
      target_->SetOrInsertTo(key_name_, host_object);
    }

    void AppendTo(VarBuilder & host_object)
    {
      target_->AppendTo(host_object);
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

    void OnExit()
    {
      target_->OnExit();
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
          + "\n----------------------------------";
    }

  private:
    TargetItem(const Path & path, const std::string & key_name,
        typename Target<VarBuilder>::Ptr target) :
        path_(path), key_name_(key_name), target_(target)
    {}

    TargetItem(const Path & path, typename Target<VarBuilder>::Ptr target) :
        path_(path), target_(target)
    {}

  private:
    Path path_;
    std::string key_name_;
    typename Target<VarBuilder>::Ptr target_;
  };

  //---------------------------------------------------------------------------
  template<typename VarBuilder>
  class ObjectTarget: public Target<VarBuilder>
  {
  public:
    typedef NKIT_SHARED_PTR(ObjectTarget<VarBuilder>) Ptr;
    typedef typename TargetItem<VarBuilder>::Ptr TargetItemPtr;

  public:
    static Ptr Create()
    {
      return Ptr(new ObjectTarget<VarBuilder>);
    }
    ~ObjectTarget()
    {
    }
    void AddTargetItem(TargetItemPtr target_item)
    {
      target_items_.push_back(target_item);
    }

  private:
    ObjectTarget()
    {
      Target<VarBuilder>::var_builder_.InitAsDict();
    }

    void OnEnter() {}

    void OnExit()
    {
      typename std::vector<TargetItemPtr>::iterator target_item =
          target_items_.begin(), end = target_items_.end();
      for (; target_item != end; ++target_item)
      {
        (*target_item)->SetOrInsertTo(Target<VarBuilder>::var_builder_);
        (*target_item)->Clear();
      }
    }

    void OnText(const char *, size_t) {}

    void Clear()
    {
      Target<VarBuilder>::var_builder_.InitAsDict();
    }

    std::string ToString() const
    {
      return "object target";
    }

  private:
    std::vector<TargetItemPtr> target_items_;
  };

  //---------------------------------------------------------------------------
  template<typename VarBuilder>
  class ListTarget: public Target<VarBuilder>
  {
    typedef Target<VarBuilder> MyParent;
  public:
    typedef typename TargetItem<VarBuilder>::Ptr TargetItemPtr;

    static typename MyParent::Ptr Create(TargetItemPtr target_item)
    {
      typename MyParent::Ptr result(new ListTarget<VarBuilder>(target_item));
      return result;
    }

    ~ListTarget() {}

  private:
    ListTarget(TargetItemPtr target_item)
      : target_item_(target_item)
    {
      Target<VarBuilder>::var_builder_.InitAsList();
    }

    void OnEnter() {}

    void OnExit()
    {
      target_item_->AppendTo(Target<VarBuilder>::var_builder_);
      target_item_->Clear();
    }

    void OnText(const char *, size_t) {}

    void Clear()
    {
      Target<VarBuilder>::var_builder_.InitAsList();
    }

    std::string ToString() const
    {
      return "list target";
    }

  private:
    TargetItemPtr target_item_;
  };

  //---------------------------------------------------------------------------
  template<typename VarBuilder,
    void (VarBuilder::*InitByString)(const std::string & value),
    void (VarBuilder::*InitByStringWithFormat)(const std::string & value,
        const std::string & format),
    void (VarBuilder::*InitByDefault)(void)
    >
  class ScalarTarget: public Target<VarBuilder>
  {
    typedef Target<VarBuilder> MyParent;

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

    void OnExit()
    {
      if (likely(!use_default_value_))
      {
        if (format_.empty())
          (Target<VarBuilder>::var_builder_.*InitByString)(value_);
        else
          (Target<VarBuilder>::var_builder_.*InitByStringWithFormat)(
              value_, format_);
      }
      value_.clear();
    }

    void OnText(const char * text, size_t len)
    {
      use_default_value_ = false;
      value_.append(text, len);
    }

    void Clear() {}

    virtual typename VarBuilder::type const & var()
    {
      if (unlikely(use_default_value_))
        return default_value_.get();
      else
      {
        use_default_value_ = true;
        return Target<VarBuilder>::var_builder_.get();
      }
    }

    std::string ToString() const
    {
      return "scalar target";
    }

  private:
    VarBuilder default_value_;
    bool use_default_value_;
    std::string value_;
    std::string format_;
  };

  //---------------------------------------------------------------------------
  template<typename VarBuilder>
  class PathNode: Uncopyable
  {
  public:
    typedef NKIT_SHARED_PTR(PathNode<VarBuilder>) Ptr;

  public:
    static Ptr CreateRoot()
    {
      return Ptr(new PathNode<VarBuilder>(0));
    }

    static void MoveToChild(PathNode<VarBuilder> ** current, size_t element_id)
    {
      std::vector<PathNode<VarBuilder>::Ptr> & children = (**current).children_;
      typename std::vector<PathNode<VarBuilder>::Ptr>::iterator child =
          children.begin(), end = children.end();
      for (; child != end; ++child)
      {
        if ((*child)->element_id_ == element_id)
        {
          *current = (*child).get();
          return;
        }
      }
      PathNode<VarBuilder>::Ptr new_child(
          new PathNode<VarBuilder>(*current, element_id));
      children.push_back(new_child);
      *current = new_child.get();
    }

    static bool MoveToParent(PathNode<VarBuilder> ** current)
    {
      if (!(*current)->parent_)
        return false;
      *current = (*current)->parent_;
      return true;
    }

    void OnEnter(const char ** attrs)
    {
      typename std::vector<typename TargetItem<VarBuilder>::Ptr>::iterator
        target_item = target_items_.begin(), end = target_items_.end();
      for (; target_item != end; ++target_item)
      {
        //CINFO(target_item->ToString());
        (*target_item)->OnEnter(attrs);
      }
    }

    void OnExit()
    {
      typename std::vector<typename TargetItem<VarBuilder>::Ptr>::iterator
        target_item = target_items_.begin(), end = target_items_.end();
      for (; target_item != end; ++target_item)
        (*target_item)->OnExit();
    }

    void OnText(const char * text, size_t len)
    {
      typename std::vector<typename TargetItem<VarBuilder>::Ptr>::iterator
        target_item = target_items_.begin(), end = target_items_.end();
      for (; target_item != end; ++target_item)
        (*target_item)->OnText(text, len);
    }

    void PutTargetItem(typename TargetItem<VarBuilder>::Ptr const & target_item)
    {
      PathNode<VarBuilder> * current = this;
      const Path & path = target_item->fool_path();
      std::vector<size_t>::const_iterator element_id = path.elements().begin(),
          end = path.elements().end();
      for (; element_id != end; ++element_id)
        MoveToChild(&current, *element_id);
      //CINFO(target_item->ToString());
      current->target_items_.push_back(target_item);
    }

    const Path & fool_path() const
    {
      return path_;
    }

  protected:
    PathNode(size_t element_id)
      : parent_(NULL)
      , element_id_(element_id)
      , absolute_counter_(0)
      , relative_counter_(0)
      , path_(element_id)
    {}

    PathNode(PathNode * parent, size_t element_id)
      : parent_(parent)
      , element_id_(element_id)
      , absolute_counter_(0)
      , relative_counter_(0)
      , path_(parent->path_ / element_id)
    {}

  private:
    PathNode<VarBuilder> * parent_;
    size_t element_id_;
    std::vector<Ptr> children_;
    size_t absolute_counter_;
    size_t relative_counter_;
    Path path_;
    std::vector<typename TargetItem<VarBuilder>::Ptr> target_items_;
  };

  //---------------------------------------------------------------------------
  template <typename VarBuilder>
  class Xml2VarBuilder: public ExpatParser<Xml2VarBuilder<VarBuilder> >
  {
  private:
    friend class ExpatParser<Xml2VarBuilder<VarBuilder> > ;

  public:
    typedef NKIT_SHARED_PTR(Xml2VarBuilder<VarBuilder>) Ptr;

  public:
    static Ptr Create(const std::string & taget_spec, std::string * error)
    {
      nkit::Dynamic d_taget_spec = nkit::DynamicFromJson(taget_spec, error);
      if (!d_taget_spec)
        return Ptr();

      typename PathNode<VarBuilder>::Ptr path_tree(
          PathNode<VarBuilder>::CreateRoot());

      String2IdMap str2id;

      typename Target<VarBuilder>::Ptr root_target(
          parse_target_spec(d_taget_spec, path_tree, &str2id, error));

      if (!root_target)
        return Ptr();

      return Ptr(
          new Xml2VarBuilder<VarBuilder>(str2id, path_tree, root_target));
    }

    ~Xml2VarBuilder() {}

    typename VarBuilder::type const & var() const
    {
      return root_target_->var();
    }

  private:
    Xml2VarBuilder(const String2IdMap & str2id,
          typename PathNode<VarBuilder>::Ptr path_tree,
          typename Target<VarBuilder>::Ptr root_target)
      : path_tree_(path_tree)
      , current_path_(path_tree_.get())
      , root_target_(root_target)
      , first_node_(true)
      , str2id_(str2id)
    {}

    bool OnStartElement(const char * el, const char ** attrs)
    {
      size_t element_id = str2id_.GetId(el);
      if (first_node_)
      {
        first_node_ = false;
        return true;
      }

      PathNode<VarBuilder>::MoveToChild(&current_path_, element_id);
      current_path_->OnEnter(attrs);
      return true;
    }

    bool OnEndElement(const char * el)
    {
      NKIT_FORCE_USED(el);
      current_path_->OnExit();
      PathNode<VarBuilder>::MoveToParent(&current_path_);
      return true;
    }

    bool OnText(const char * text, int len)
    {
      current_path_->OnText(text, static_cast<size_t>(len));
      return true;
    }

    void GetCustomError(std::string * error)
    {
      *error = error_;
    }

    //--------------------------------------------------------------------------
    static typename Target<VarBuilder>::Ptr parse_list_target_spec(
        Path parent_path, const nkit::Dynamic & target_spec,
        typename PathNode<VarBuilder>::Ptr path_tree, String2IdMap * str2id,
        std::string * error)
    {
      if (target_spec.size() != 2)
      {
        *error = "Target spec for list must have exactly two elements";
        typename Target<VarBuilder>::Ptr res;

        return res;
      }

      Path path(target_spec.GetByIndex(0).GetString(), str2id);
      Path fool_path(parent_path / path);
      typename Target<VarBuilder>::Ptr child_target =
          parse_target_spec(fool_path, target_spec.GetByIndex(1),
              path_tree, str2id, error);
      if (!child_target)
      {
        typename Target<VarBuilder>::Ptr res;
        return res;
      }
      typename ListTarget<VarBuilder>::Ptr result =
          ListTarget<VarBuilder>::Create(
              TargetItem<VarBuilder>::Create(fool_path, child_target));
      path_tree->PutTargetItem(TargetItem<VarBuilder>::Create(fool_path, result));

      return result;
    }

    //--------------------------------------------------------------------------
    static typename Target<VarBuilder>::Ptr parse_scalar_target_spec(
        Path parent_path, const std::string & target_spec,
        typename PathNode<VarBuilder>::Ptr path_tree, std::string * error)
    {
      static const std::string STRING_TYPE = "string";
      static const std::string INTEGER_TYPE = "integer";
      static const std::string NUMBER_TYPE = "number";
      static const std::string DATETIME_TYPE = "datetime";
      static const std::string BOOLEAN_TYPE = "boolean";

      nkit::StringVector spec_list;
      nkit::simple_split(target_spec, "|", &spec_list);
      if (spec_list.size() < 1)
      {
        *error = "Target spec for scalar must have at least one element";
        typename Target<VarBuilder>::Ptr res;
        return res;
      }

      typename Target<VarBuilder>::Ptr result;
      const std::string & type = spec_list[0];
      if (type == STRING_TYPE)
      {
        typedef ScalarTarget<VarBuilder,
            &VarBuilder::InitAsString,
            &VarBuilder::InitAsStringFormat,
            &VarBuilder::InitAsStringDefault> StringTarget;
        if (spec_list.size() >= 2)
          result = StringTarget::Create(spec_list[1]);
        else
          result = StringTarget::Create();
      }
      else if (type == INTEGER_TYPE)
      {
        typedef ScalarTarget<VarBuilder,
            &VarBuilder::InitAsInteger,
            &VarBuilder::InitAsIntegerFormat,
            &VarBuilder::InitAsIntegerDefault> IntegerTarget;
        if (spec_list.size() >= 2)
          result = IntegerTarget::Create(spec_list[1]);
        else
          result = IntegerTarget::Create();
      }
      else if (type == NUMBER_TYPE)
      {
        typedef ScalarTarget<VarBuilder,
            &VarBuilder::InitAsFloat,
            &VarBuilder::InitAsFloatFormat,
            &VarBuilder::InitAsFloatDefault> NumberTarget;
        if (spec_list.size() >= 3)
          result = NumberTarget::Create(spec_list[1], spec_list[2]);
        else if (spec_list.size() >= 2)
          result = NumberTarget::Create(spec_list[1]);
        else
          result = NumberTarget::Create();
      }
      else if (type == BOOLEAN_TYPE)
      {
        typedef ScalarTarget<VarBuilder,
            &VarBuilder::InitAsBoolean,
            &VarBuilder::InitAsBooleanFormat,
            &VarBuilder::InitAsBooleanDefault> BooleanTarget;
        if (spec_list.size() >= 2)
          result = BooleanTarget::Create(spec_list[1]);
        else
          result = BooleanTarget::Create();
      }
//#ifndef NKIT_WINNT
      else if (type == DATETIME_TYPE)
      {
        typedef ScalarTarget<VarBuilder,
            &VarBuilder::InitAsDatetime,
            &VarBuilder::InitAsDatetimeFormat,
            &VarBuilder::InitAsDatetimeDefault> DatetimeTarget;
        if (spec_list.size() >= 3)
          result = DatetimeTarget::Create(spec_list[1], spec_list[2]);
        else if (spec_list.size() >= 2)
          result = DatetimeTarget::Create(spec_list[1]);
        else
          result = DatetimeTarget::Create();
      }
//#endif
      else
      {
        *error = "Target spec for scalar does not support type '" + type + "'";
        typename Target<VarBuilder>::Ptr res;
        return res;
      }

      path_tree->PutTargetItem(
          TargetItem<VarBuilder>::Create(parent_path, result));
      return result;
    }

    //--------------------------------------------------------------------------
    static typename Target<VarBuilder>::Ptr parse_target_spec(Path parent_path,
        const nkit::Dynamic & target_spec,
        typename PathNode<VarBuilder>::Ptr path_tree, String2IdMap * str2id,
        std::string * error)
    {
      if (target_spec.IsList())
        return parse_list_target_spec(parent_path, target_spec,
            path_tree, str2id, error);
      else if (target_spec.IsDict())
        return parse_object_target_spec(parent_path, target_spec,
            path_tree, str2id, error);
      else if (target_spec.IsString())
        return parse_scalar_target_spec(parent_path,
            target_spec.GetConstString(), path_tree, error);
      else
      {
        *error = "Wrong target spec";
        typename Target<VarBuilder>::Ptr res;
        return res;
      }
    }

    //--------------------------------------------------------------------------
    static typename Target<VarBuilder>::Ptr parse_object_target_spec(
        Path parent_path, const nkit::Dynamic & target_spec,
        typename PathNode<VarBuilder>::Ptr path_tree, String2IdMap * str2id,
        std::string * error)
    {
      static const std::string DEFAULT_KEY = "_";

      typename ObjectTarget<VarBuilder>::Ptr result =
          ObjectTarget<VarBuilder>::Create();

      DDICT_FOREACH(pair, target_spec)
      {
        std::string path_spec, key;
        nkit::simple_split(pair->first, "->", &path_spec, &key);
        Path path(path_spec, str2id);
        if (key.empty() || key == DEFAULT_KEY)
          key = path.GetLastElementName(*str2id);
        Path fool_path(parent_path / path);
        typename Target<VarBuilder>::Ptr child_target = parse_target_spec(
            fool_path, pair->second, path_tree, str2id, error);
        if (!child_target)
        {
          typename Target<VarBuilder>::Ptr res;
          return res;
        }
        typename TargetItem<VarBuilder>::Ptr target_item =
            TargetItem<VarBuilder>::Create(fool_path, key, child_target);
        result->AddTargetItem(target_item);
      }

      path_tree->PutTargetItem(
          TargetItem<VarBuilder>::Create(parent_path, result));

      return result;
    }

    //--------------------------------------------------------------------------
    static typename Target<VarBuilder>::Ptr parse_target_spec(
        const nkit::Dynamic & target_spec,
        typename PathNode<VarBuilder>::Ptr path_tree, String2IdMap * str2id,
        std::string * error)
    {
      Path empty_path;

      if (target_spec.IsList())
        return parse_list_target_spec(empty_path, target_spec, path_tree,
            str2id, error);
      else if (target_spec.IsDict())
        return parse_object_target_spec(empty_path, target_spec, path_tree,
            str2id, error);
      else
      {
        *error = "Root target specification can not be scalar - "
            "only object or list";

        typename Target<VarBuilder>::Ptr res;
        return res;
      }
    }

  private:
    std::string error_;
    typename PathNode<VarBuilder>::Ptr path_tree_;
    PathNode<VarBuilder> * current_path_;
    typename Target<VarBuilder>::Ptr root_target_;
    bool first_node_;
    String2IdMap str2id_;
  };

} // namespace nkit

#endif // NKIT_VX_H
