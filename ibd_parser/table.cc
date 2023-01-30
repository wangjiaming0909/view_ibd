#include "table.h"
#include "buffer_pool.h"


namespace innodb {
Table::Table(const char *schema, const char *tb)
    : schema_name_(schema), tb_name_(tb) {}

Index *Table::get_cluster_idx() { return nullptr; }

const Index::Rec_Comp_t Index::Always_True_Comp = [](const byte *) {
  return Rec_Comp_Res::EQ_CONTINUE;
};

const Index::Rec_Comp_t Index::True_Once_Comp = [](const byte *) {
  return Rec_Comp_Res::EQ_BREAK;
};

int Index::search(Rec_Oper_t &func, const Rec_Comp_t &comp) {
}

byte* Index::get_first_rec() {
  auto& bp = buffer_pool_t::instance();
}

int Index::get_next_rec() {

}

} // namespace innodb
