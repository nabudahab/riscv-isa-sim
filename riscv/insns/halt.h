extern std::set<size_t> halted_harts;
extern std::mutex halt_lock;
extern size_t global_nprocs;

{
  std::lock_guard<std::mutex> lock(halt_lock);
  size_t current_id = p->get_id();

  if (halted_harts.find(current_id) == halted_harts.end()) {
    fprintf(stderr, "Core %zu complete.\n", current_id);
    halted_harts.insert(current_id);
  }

  if (halted_harts.size() >= global_nprocs) {
    FILE *fp = fopen("memsim.mem", "w");
    if (fp) {
      // Use the reference to cfg
      const auto& cfg = p->get_cfg();

      // Iterate through every memory region in the layout
      for (const auto& layout : cfg.mem_layout) {
        // USE GETTERS: .base and .size are private
        reg_t region_base = layout.get_base();
        reg_t region_size = layout.get_size();

        // Standard 4-byte word dump
        for (reg_t addr = region_base; addr < region_base + region_size; addr += 4) {
          uint32_t word = 0;
          try {
            word = MMU.load<uint32_t>(addr);
          } catch (trap_t &t) {
            word = 0; 
          }
          fprintf(fp, "%08x\n", word);
        }
      }
      fclose(fp);
    }

    fprintf(stderr, "Hit halt, memory dump complete.\n");
    exit(0);
  }
}

// Yield the core to the scheduler
STATE.serialized = true;
set_pc(pc);