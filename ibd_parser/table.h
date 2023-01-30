#pragma once
#include "dd_index.h"
#include <functional>
#include <string>

namespace innodb {

class Table;

class Schema {
public:
  Schema(const char* name) : name(name) {}
  void add_table(const char *name, Table *tb) { tables_[name] = tb; }

private:
  std::string name;
  std::unordered_map<std::string, Table*> tables_;
};

enum struct FieldType {
  LONG, VARCHAR
};

struct Field{
  std::string name;
  uint pos;
  FieldType type;
};

class Index {
  uint n_uniqs_;
  Table *tb_;
  PageID page_id_;

public:
  enum class Rec_Comp_Res {
    EQ_CONTINUE,
    EQ_BREAK,
    NEQ_CONTINUE,
    NEW_BREAK
  };
  using Rec_Comp_t = std::function<Rec_Comp_Res(const byte *)>;
  using Rec_Oper_t = std::function<void(byte *)>;
  friend class Table;
  // @brief search the key
  // @return 0 if found, -1 if not found
  int search(Rec_Oper_t &func, const Rec_Comp_t &comp);

  void set_tb(Table *tb) { tb_ = tb; }
  void set_n_uniqs(uint v) { n_uniqs_ = v; }
  void set_page_id(PageID pg_id) {page_id_ = pg_id;}

  static const Rec_Comp_t Always_True_Comp;
  static const Rec_Comp_t True_Once_Comp;

private:
};

class Table {
public:
  Table(const char *schema, const char *tb);
  virtual ~Table() = default;

  Index* get_cluster_idx();
  std::string tb_full_name() const { return schema_name_ + "." + tb_name_; }

  virtual bool is_dd_tb() const { return false; }

  bool is_inited() const { return inited_; }

protected:
  void set_fil_name(const char *fil) { fil_name_ = fil; }
  void set_space_id(SpaceID sp_id) { space_id_ = sp_id; }

PROTECTED:
  bool inited_ = false;
  std::string schema_name_;
  std::string tb_name_;
  std::string fil_name_;

  Schema *schema_;

  SpaceID space_id_;

  std::vector<Index*> indexes_;
  std::vector<Field*> fields_;

  dd::DD_Index idx_;
};

} // namespace innodb
